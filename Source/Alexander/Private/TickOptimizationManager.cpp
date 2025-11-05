// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickOptimizationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTickOptimizationManager::UTickOptimizationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds

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

    // Internal state
    TimeSinceLastOptimization = 0.0f;
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
        Report.OptimizedComponents.Num(), Report.EstimatedPerformanceGainMs);
    
    // Broadcast to our own delegate
    OnOptimizationComplete.Broadcast(Report);
}
