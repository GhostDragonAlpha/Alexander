// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Templates/Function.h"
#include "AsyncLoadingComponent.generated.h"

// Async loading task status
UENUM(BlueprintType)
enum class EAsyncTaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Cancelled   UMETA(DisplayName = "Cancelled")
};

// Async loading task
struct FAsyncLoadingTask
{
    FString SystemID;
    EAsyncTaskStatus Status;
    double StartTime;
    double EndTime;

    FAsyncLoadingTask()
        : Status(EAsyncTaskStatus::Pending)
        , StartTime(0.0)
        , EndTime(0.0)
    {}
};

DECLARE_DELEGATE_OneParam(FOnAsyncTaskComplete, const FString& /*SystemID*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAsyncLoadingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAsyncLoadingComponent();

    // Initialize async loading with thread pool size
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    void Initialize(int32 ThreadPoolSize);

    // Queue a system for async loading
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    void QueueLoadingTask(const FString& SystemID);

    // Queue a system for async unloading
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    void QueueUnloadingTask(const FString& SystemID);

    // Get task status
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    EAsyncTaskStatus GetTaskStatus(const FString& SystemID) const;

    // Cancel a task
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    void CancelTask(const FString& SystemID);

    // Cancel all tasks
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    void CancelAllTasks();

    // Get number of pending tasks
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    int32 GetPendingTaskCount() const;

    // Get number of active tasks
    UFUNCTION(BlueprintCallable, Category = "AsyncLoading")
    int32 GetActiveTaskCount() const;

    // Events
    FOnAsyncTaskComplete OnTaskComplete;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Task management
    TMap<FString, TSharedPtr<FAsyncLoadingTask>> LoadingTasks;
    TQueue<TSharedPtr<FAsyncLoadingTask>> PendingTasks;
    TArray<TSharedPtr<FAsyncLoadingTask>> ActiveTasks;
    TArray<TSharedPtr<FAsyncLoadingTask>> CompletedTasks;

    // Thread pool
    int32 MaxThreadPoolSize;
    FCriticalSection TaskLock;

    // Processing functions
    void ProcessPendingTasks();
    void ProcessCompletedTasks();
    void ExecuteTask(TSharedPtr<FAsyncLoadingTask> Task);
};