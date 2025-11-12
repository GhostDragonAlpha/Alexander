// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickOptimizationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

UTickOptimizationManager::UTickOptimizationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // FIX 2: Changed from 0.1f to 1.0f to reduce overhead

    // Create sub-components
    AnalysisComponent = CreateDefaultSubobject<UTickAnalysisComponent>(TEXT("AnalysisComponent"));
    OptimizationComponent = CreateDefaultSubobject<UTickOptimizationComponent>(TEXT("OptimizationComponent"));
    ReportingComponent = CreateDefaultSubobject<UTickReportingComponent>(TEXT("ReportingComponent"));
    BudgetComponent = CreateDefaultSubobject<UTickBudgetComponent>(TEXT("BudgetComponent"));

    // Default settings (now delegated to components)
    OptimizationStrategy = ETickOptimizationStrategy::Balanced;
    bAutoOptimize = true;
    OptimizationInterval = 30.0f;
    bEnableTickBudgeting = true;
    MaxTickBudgetMs = 5.0f;
    bDisableTickOnDistantActors = true;
    DistanceThreshold = 5000.0f; // 50 meters

    // Distance-based culling defaults
    HighPriorityDistance = 1000.0f;
    MediumPriorityDistance = 2500.0f;
    LowPriorityDistance = 5000.0f;
    CullDistance = 10000.0f;

    // Performance targets
    TargetFPS = 60.0f;
    MinFPS = 30.0f;

    // Internal state
    TimeSinceLastOptimization = 0.0f;
    CurrentFPS = 60.0f;
    LastFPSTime = 0.0f;
    FrameCount = 0;
    bIsPerformanceDegraded = false;
}

void UTickOptimizationManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize sub-components
    if (AnalysisComponent)
    {
        AnalysisComponent->AnalyzeAllComponents();
    }

    // Apply default optimization strategy
    if (OptimizationComponent)
    {
        OptimizationComponent->SetOptimizationStrategy(OptimizationStrategy);
        OptimizationComponent->OnOptimizationComplete.AddDynamic(this, &UTickOptimizationManager::HandleOptimizationComplete);
    }

    // Configure budget component
    if (BudgetComponent)
    {
        BudgetComponent->MaxTickBudgetMs = MaxTickBudgetMs;
        BudgetComponent->bEnableTickBudgeting = bEnableTickBudgeting;
    }

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization Manager: Initialized with %s strategy"),
        *UEnum::GetValueAsString(OptimizationStrategy));
}

void UTickOptimizationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Generate final report
    if (ReportingComponent && AnalysisComponent)
    {
        FString FinalReport = ReportingComponent->GenerateTickReport(AnalysisComponent);
        UE_LOG(LogTemp, Log, TEXT("Tick Optimization Manager: Final Report\n%s"), *FinalReport);
    }

    Super::EndPlay(EndPlayReason);
}

void UTickOptimizationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Monitor performance
    MonitorPerformance(DeltaTime);

    // Auto-optimize if enabled
    if (bAutoOptimize && OptimizationComponent && AnalysisComponent)
    {
        TimeSinceLastOptimization += DeltaTime;
        if (TimeSinceLastOptimization >= OptimizationInterval)
        {
            OptimizationComponent->OptimizeAllTicks(AnalysisComponent);
            TimeSinceLastOptimization = 0.0f;
        }
    }

    // Update tick budget
    if (bEnableTickBudgeting && BudgetComponent && AnalysisComponent && OptimizationComponent)
    {
        BudgetComponent->UpdateTickBudget(DeltaTime, AnalysisComponent, OptimizationComponent);
    }

    // Update actor tick states (distance and visibility based)
    UpdateActorTickStates();

    // Update component tick states
    UpdateComponentTickStates();

    // Optimize by distance
    if (bDisableTickOnDistantActors && OptimizationComponent && AnalysisComponent)
    {
        OptimizationComponent->OptimizeByDistance(AnalysisComponent);
    }
}

void UTickOptimizationManager::SetOptimizationStrategy(ETickOptimizationStrategy NewStrategy)
{
    OptimizationStrategy = NewStrategy;
    if (OptimizationComponent)
    {
        OptimizationComponent->SetOptimizationStrategy(NewStrategy);
    }
}

void UTickOptimizationManager::GatherTickingComponents()
{
    if (AnalysisComponent)
    {
        AnalysisComponent->GatherTickingComponents();
    }
}

void UTickOptimizationManager::HandleOptimizationComplete(const FTickOptimizationReport& Report)
{
    // Handle optimization completion
    UE_LOG(LogTemp, Log, TEXT("Tick Optimization completed: %d components optimized, %.2fms saved"),
        Report.ComponentsOptimized, Report.EstimatedPerformanceGain);
    
    // Broadcast to our own delegate
    OnOptimizationComplete.Broadcast(Report);
}

void UTickOptimizationManager::MonitorPerformance(float DeltaTime)
{
    FrameCount++;
    LastFPSTime += DeltaTime;
    
    if (LastFPSTime >= 1.0f) // Update FPS every second
    {
        CurrentFPS = FrameCount / LastFPSTime;
        FrameCount = 0;
        LastFPSTime = 0.0f;
        
        // Adjust tick rates based on performance
        if (CurrentFPS < MinFPS && !bIsPerformanceDegraded)
        {
            ReduceTickRates();
            bIsPerformanceDegraded = true;
        }
        else if (CurrentFPS > TargetFPS && bIsPerformanceDegraded)
        {
            RestoreTickRates();
            bIsPerformanceDegraded = false;
        }
    }
}

void UTickOptimizationManager::UpdateActorTickStates()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsPendingKillPending()) continue;
        
        // Skip player and critical actors
        if (Actor == PC->GetPawn() || Actor->ActorHasTag(TEXT("Critical")))
        {
            ActorTickPriorities.Add(Actor, ETickPriority::Critical);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        ETickPriority Priority = CalculateTickPriority(Actor, Distance);
        
        ApplyTickOptimization(Actor, Priority);
        ActorTickPriorities.Add(Actor, Priority);
    }
}

ETickPriority UTickOptimizationManager::CalculateTickPriority(AActor* Actor, float Distance) const
{
    if (!Actor) return ETickPriority::Disabled;
    
    // Check visibility first
    if (!IsActorVisible(Actor))
    {
        return ETickPriority::Dormant;
    }
    
    // Check for high priority tags
    if (Actor->ActorHasTag(TEXT("HighPriority")))
    {
        return ETickPriority::High;
    }
    else if (Actor->ActorHasTag(TEXT("MediumPriority")))
    {
        return ETickPriority::Medium;
    }
    else if (Actor->ActorHasTag(TEXT("LowPriority")))
    {
        return ETickPriority::Low;
    }
    
    // Distance-based priority
    if (Distance < HighPriorityDistance)
    {
        // Check if actor is in view frustum
        if (IsActorInFrustum(Actor))
        {
            return ETickPriority::High;
        }
        return ETickPriority::Medium;
    }
    else if (Distance < MediumPriorityDistance)
    {
        return ETickPriority::Medium;
    }
    else if (Distance < LowPriorityDistance)
    {
        return ETickPriority::Low;
    }
    else if (Distance < CullDistance)
    {
        return ETickPriority::VeryLow;
    }
    else
    {
        return ETickPriority::Dormant;
    }
}

void UTickOptimizationManager::ApplyTickOptimization(AActor* Actor, ETickPriority Priority)
{
    if (!Actor) return;
    
    switch (Priority)
    {
        case ETickPriority::Critical:
            // Always tick at full rate
            Actor->SetActorTickInterval(0.0f);
            Actor->SetActorTickEnabled(true);
            break;
            
        case ETickPriority::High:
            // Tick at 60 FPS
            Actor->SetActorTickInterval(0.016f);
            Actor->SetActorTickEnabled(true);
            break;
            
        case ETickPriority::Medium:
            // Tick at 30 FPS
            Actor->SetActorTickInterval(0.033f);
            Actor->SetActorTickEnabled(true);
            break;
            
        case ETickPriority::Low:
            // Tick at 10 FPS
            Actor->SetActorTickInterval(0.1f);
            Actor->SetActorTickEnabled(true);
            break;
            
        case ETickPriority::VeryLow:
            // Tick at 1 FPS
            Actor->SetActorTickInterval(1.0f);
            Actor->SetActorTickEnabled(true);
            break;
            
        case ETickPriority::Dormant:
            // Disable ticking
            Actor->SetActorTickEnabled(false);
            break;
    }
}

bool UTickOptimizationManager::IsActorVisible(AActor* Actor) const
{
    if (!Actor) return false;
    
    // Check if actor is in view frustum
    if (!IsActorInFrustum(Actor))
    {
        return false;
    }
    
    // Additional visibility checks can be added here
    // For example, occlusion culling, line of sight, etc.
    
    return true;
}

bool UTickOptimizationManager::IsActorInFrustum(AActor* Actor) const
{
    if (!Actor) return false;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return false;
    
    // Get player viewpoint
    FVector ViewLocation;
    FRotator ViewRotation;
    PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
    
    // Calculate direction to actor
    FVector ActorLocation = Actor->GetActorLocation();
    FVector DirectionToActor = (ActorLocation - ViewLocation).GetSafeNormal();
    
    // Check if actor is in front of player
    FVector ForwardVector = ViewRotation.Vector();
    float DotProduct = FVector::DotProduct(ForwardVector, DirectionToActor);
    
    // If dot product is positive, actor is in front of player
    // This is a simplified frustum check - for production, use proper frustum culling
    return DotProduct > 0.0f;
}

void UTickOptimizationManager::UpdateComponentTickStates()
{
    if (!AnalysisComponent) return;
    
    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();
    
    for (UActorComponent* Component : Components)
    {
        if (!Component || Component->IsBeingDestroyed()) continue;
        UpdateComponentTickState(Component);
    }
}

void UTickOptimizationManager::UpdateComponentTickState(UActorComponent* Component)
{
    if (!Component) return;
    
    AActor* Owner = Component->GetOwner();
    if (!Owner) return;
    
    // Get actor priority
    ETickPriority ActorPriority = ETickPriority::Medium;
    if (ActorTickPriorities.Contains(Owner))
    {
        ActorPriority = ActorTickPriorities[Owner];
    }
    
    // Adjust component priority based on actor priority
    switch (ActorPriority)
    {
        case ETickPriority::Dormant:
            // Disable component ticking when actor is dormant
            Component->SetComponentTickEnabled(false);
            ComponentTickPriorities.Add(Component, ETickPriority::Dormant);
            break;
            
        case ETickPriority::Critical:
        case ETickPriority::High:
            // Keep critical components ticking
            Component->SetComponentTickEnabled(true);
            ComponentTickPriorities.Add(Component, ETickPriority::High);
            break;
            
        case ETickPriority::Medium:
        case ETickPriority::Low:
        case ETickPriority::VeryLow:
            // Reduce tick rate for non-essential components
            if (Component->PrimaryComponentTick.bCanEverTick)
            {
                float Interval = (ActorPriority == ETickPriority::Medium) ? 0.033f : 0.1f;
                Component->PrimaryComponentTick.TickInterval = Interval;
                Component->SetComponentTickEnabled(true);
                ComponentTickPriorities.Add(Component, ActorPriority);
            }
            break;
    }
}

void UTickOptimizationManager::ReduceTickRates()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance degraded - reducing tick rates"));
    
    // Reduce all non-critical tick rates by 50%
    for (auto& Pair : ActorTickPriorities)
    {
        AActor* Actor = Pair.Key;
        if (!Actor || Pair.Value == ETickPriority::Critical) continue;
        
        float CurrentInterval = Actor->GetActorTickInterval();
        if (CurrentInterval > 0.0f)
        {
            Actor->SetActorTickInterval(CurrentInterval * 1.5f);
        }
        else
        {
            Actor->SetActorTickInterval(0.033f); // Drop from 60 FPS to 30 FPS
        }
    }
}

void UTickOptimizationManager::RestoreTickRates()
{
    UE_LOG(LogTemp, Log, TEXT("Performance restored - restoring tick rates"));
    
    // Restore original tick rates by re-applying optimization
    UpdateActorTickStates();
    UpdateComponentTickStates();
}

FString UTickOptimizationManager::GetTickStats() const
{
    FString Stats;
    Stats += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentFPS);
    Stats += FString::Printf(TEXT("Target FPS: %.1f\n"), TargetFPS);
    Stats += FString::Printf(TEXT("Min FPS: %.1f\n"), MinFPS);
    Stats += FString::Printf(TEXT("Performance Degraded: %s\n"), bIsPerformanceDegraded ? TEXT("Yes") : TEXT("No"));
    Stats += FString::Printf(TEXT("Total Actors: %d\n"), ActorTickPriorities.Num());
    Stats += FString::Printf(TEXT("Total Components: %d\n"), ComponentTickPriorities.Num());
    
    // Count actors by priority
    TMap<ETickPriority, int32> PriorityCounts;
    for (const auto& Pair : ActorTickPriorities)
    {
        PriorityCounts.FindOrAdd(Pair.Value)++;
    }
    
    Stats += TEXT("\nActor Priority Distribution:\n");
    for (const auto& Pair : PriorityCounts)
    {
        Stats += FString::Printf(TEXT("  %s: %d\n"), *UEnum::GetValueAsString(Pair.Key), Pair.Value);
    }
    
    return Stats;
}

void UTickOptimizationManager::SetActorPriority(AActor* Actor, ETickPriority Priority)
{
    if (!Actor) return;
    
    ApplyTickOptimization(Actor, Priority);
    ActorTickPriorities.Add(Actor, Priority);
}

TArray<AActor*> UTickOptimizationManager::GetDormantActors() const
{
    TArray<AActor*> DormantActors;
    
    for (const auto& Pair : ActorTickPriorities)
    {
        if (Pair.Value == ETickPriority::Dormant)
        {
            DormantActors.Add(Pair.Key);
        }
    }
    
    return DormantActors;
}

void UTickOptimizationManager::ResetTickOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Resetting tick optimization to defaults"));
    
    // Reset all actors to tick every frame
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor)
        {
            Actor->SetActorTickInterval(0.0f);
            Actor->SetActorTickEnabled(true);
        }
    }
    
    // Reset all components
    if (AnalysisComponent)
    {
        TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();
        for (UActorComponent* Component : Components)
        {
            if (Component)
            {
                Component->PrimaryComponentTick.TickInterval = 0.0f;
                Component->SetComponentTickEnabled(true);
            }
        }
    }
    
    // Clear priority maps
    ActorTickPriorities.Empty();
    ComponentTickPriorities.Empty();
    bIsPerformanceDegraded = false;
}
