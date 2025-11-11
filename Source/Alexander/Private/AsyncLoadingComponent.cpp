// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsyncLoadingComponent.h"
#include "Async/Async.h"

UAsyncLoadingComponent::UAsyncLoadingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    MaxThreadPoolSize = 4;
}

void UAsyncLoadingComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AsyncLoadingComponent initialized with thread pool size: %d"), MaxThreadPoolSize);
}

void UAsyncLoadingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ProcessPendingTasks();
    ProcessCompletedTasks();
}

void UAsyncLoadingComponent::Initialize(int32 ThreadPoolSize)
{
    MaxThreadPoolSize = FMath::Max(1, ThreadPoolSize);
    UE_LOG(LogTemp, Log, TEXT("AsyncLoadingComponent thread pool size set to: %d"), MaxThreadPoolSize);
}

void UAsyncLoadingComponent::QueueLoadingTask(const FString& SystemID, TFunction<void()> TaskFunction, TFunction<void()> CompletionCallback)
{
    FScopeLock Lock(&TaskLock);

    // Cancel existing task for this system if any
    CancelTask(SystemID);

    auto Task = MakeShareable(new FAsyncLoadingTask());
    Task->SystemID = SystemID;
    Task->TaskFunction = TaskFunction;
    Task->CompletionCallback = CompletionCallback;
    Task->Status = EAsyncTaskStatus::Pending;

    LoadingTasks.Add(SystemID, Task);
    PendingTasks.Enqueue(Task);

    UE_LOG(LogTemp, Verbose, TEXT("Queued loading task for system: %s"), *SystemID);
}

void UAsyncLoadingComponent::QueueUnloadingTask(const FString& SystemID, TFunction<void()> TaskFunction, TFunction<void()> CompletionCallback)
{
    // Unloading uses the same mechanism as loading
    QueueLoadingTask(SystemID, TaskFunction, CompletionCallback);
}

EAsyncTaskStatus UAsyncLoadingComponent::GetTaskStatus(const FString& SystemID) const
{
    FScopeLock Lock(&TaskLock);

    const TSharedPtr<FAsyncLoadingTask>* TaskPtr = LoadingTasks.Find(SystemID);
    if (TaskPtr)
    {
        return (*TaskPtr)->Status;
    }

    return EAsyncTaskStatus::Pending; // Default to pending if not found
}

void UAsyncLoadingComponent::CancelTask(const FString& SystemID)
{
    FScopeLock Lock(&TaskLock);

    TSharedPtr<FAsyncLoadingTask>* TaskPtr = LoadingTasks.Find(SystemID);
    if (TaskPtr)
    {
        if ((*TaskPtr)->Status == EAsyncTaskStatus::InProgress)
        {
            (*TaskPtr)->Status = EAsyncTaskStatus::Cancelled;
        }
        else if ((*TaskPtr)->Status == EAsyncTaskStatus::Pending)
        {
            // Remove from pending queue
            TQueue<TSharedPtr<FAsyncLoadingTask>> NewQueue;
            TSharedPtr<FAsyncLoadingTask> QueuedTask;
            while (PendingTasks.Dequeue(QueuedTask))
            {
                if (QueuedTask->SystemID != SystemID)
                {
                    NewQueue.Enqueue(QueuedTask);
                }
            }
            PendingTasks = MoveTemp(NewQueue);
            LoadingTasks.Remove(SystemID);
        }
    }
}

void UAsyncLoadingComponent::CancelAllTasks()
{
    FScopeLock Lock(&TaskLock);

    // Cancel all pending tasks
    TSharedPtr<FAsyncLoadingTask> Task;
    while (PendingTasks.Dequeue(Task))
    {
        if (Task.IsValid())
        {
            Task->Status = EAsyncTaskStatus::Cancelled;
        }
    }

    // Cancel all active tasks
    for (auto& ActiveTask : ActiveTasks)
    {
        if (ActiveTask.IsValid())
        {
            ActiveTask->Status = EAsyncTaskStatus::Cancelled;
        }
    }

    LoadingTasks.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cancelled all async loading tasks"));
}

int32 UAsyncLoadingComponent::GetPendingTaskCount() const
{
    FScopeLock Lock(&TaskLock);
    return PendingTasks.Count();
}

int32 UAsyncLoadingComponent::GetActiveTaskCount() const
{
    FScopeLock Lock(&TaskLock);
    return ActiveTasks.Num();
}

void UAsyncLoadingComponent::ProcessPendingTasks()
{
    FScopeLock Lock(&TaskLock);

    // Start new tasks if we have available threads
    while (ActiveTasks.Num() < MaxThreadPoolSize && !PendingTasks.IsEmpty())
    {
        TSharedPtr<FAsyncLoadingTask> Task;
        if (PendingTasks.Dequeue(Task) && Task.IsValid())
        {
            Task->Status = EAsyncTaskStatus::InProgress;
            Task->StartTime = FPlatformTime::Seconds();
            ActiveTasks.Add(Task);

            // Execute task on thread pool
            Async(EAsyncExecution::ThreadPool, [this, Task]() {
                ExecuteTask(Task);
            });
        }
    }
}

void UAsyncLoadingComponent::ProcessCompletedTasks()
{
    FScopeLock Lock(&TaskLock);

    // Process completed tasks on game thread
    for (int32 i = CompletedTasks.Num() - 1; i >= 0; --i)
    {
        TSharedPtr<FAsyncLoadingTask> Task = CompletedTasks[i];
        if (Task.IsValid() && Task->CompletionCallback)
        {
            // Execute completion callback on game thread
            AsyncTask(ENamedThreads::GameThread, [Task]() {
                if (Task->CompletionCallback)
                {
                    Task->CompletionCallback();
                }
            });

            // Remove from tracking
            LoadingTasks.Remove(Task->SystemID);
            CompletedTasks.RemoveAt(i);
        }
    }
}

void UAsyncLoadingComponent::ExecuteTask(TSharedPtr<FAsyncLoadingTask> Task)
{
    if (!Task.IsValid() || !Task->TaskFunction)
    {
        return;
    }

    try
    {
        // Execute the task
        Task->TaskFunction();
        
        // Update status
        {
            FScopeLock Lock(&TaskLock);
            Task->Status = EAsyncTaskStatus::Completed;
            Task->EndTime = FPlatformTime::Seconds();
            ActiveTasks.Remove(Task);
            CompletedTasks.Add(Task);
        }

        // Broadcast completion
        OnTaskComplete.ExecuteIfBound(Task->SystemID);

        UE_LOG(LogTemp, Verbose, TEXT("Completed async task for system: %s (%.2f seconds)"), 
            *Task->SystemID, Task->EndTime - Task->StartTime);
    }
    catch (const std::exception& e)
    {
        FScopeLock Lock(&TaskLock);
        Task->Status = EAsyncTaskStatus::Failed;
        Task->EndTime = FPlatformTime::Seconds();
        ActiveTasks.Remove(Task);
        CompletedTasks.Add(Task);

        UE_LOG(LogTemp, Error, TEXT("Async task failed for system: %s - %s"), 
            *Task->SystemID, UTF8_TO_TCHAR(e.what()));
    }
    catch (...)
    {
        FScopeLock Lock(&TaskLock);
        Task->Status = EAsyncTaskStatus::Failed;
        Task->EndTime = FPlatformTime::Seconds();
        ActiveTasks.Remove(Task);
        CompletedTasks.Add(Task);

        UE_LOG(LogTemp, Error, TEXT("Async task failed for system: %s - Unknown error"), *Task->SystemID);
    }
}