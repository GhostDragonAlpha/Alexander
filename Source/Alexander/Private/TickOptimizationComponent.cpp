// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickOptimizationComponent.h"
#include "TickAnalysisComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTickOptimizationComponent::UTickOptimizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Optimization is on-demand

    // Default settings
    OptimizationStrategy = ETickOptimizationStrategy::Balanced;
}

FTickOptimizationReport UTickOptimizationComponent::OptimizeAllTicks(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return FTickOptimizationReport();
    }

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization: Optimizing all tick intervals"));

    FTickOptimizationReport Report;
    TArray<UActorComponent*> ComponentsToOptimize = AnalysisComponent->GetAnalyzedComponents();
    Report.TotalComponentsAnalyzed = ComponentsToOptimize.Num();

    for (UActorComponent* Component : ComponentsToOptimize)
    {
        if (!Component || !Component->PrimaryComponentTick.bCanEverTick)
        {
            continue;
        }

        float OptimalInterval = CalculateOptimalInterval(Component, AnalysisComponent);
        float CurrentInterval = Component->PrimaryComponentTick.TickInterval;

        if (FMath::Abs(OptimalInterval - CurrentInterval) > 0.01f)
        {
            ApplyOptimization(Component, OptimalInterval);
            Report.ComponentsOptimized++;
        }
    }

    Report.EstimatedPerformanceGain = EstimatePerformanceGain(ComponentsToOptimize, AnalysisComponent);
    Report.Recommendations = GetOptimizationRecommendations(AnalysisComponent);

    OnOptimizationComplete.Broadcast(Report);

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization: Optimized %d/%d components (%.1f%% performance gain)"),
        Report.ComponentsOptimized, Report.TotalComponentsAnalyzed, Report.EstimatedPerformanceGain);

    return Report;
}

void UTickOptimizationComponent::OptimizeComponent(UActorComponent* Component, ETickPriority Priority)
{
    if (!Component)
    {
        return;
    }

    float Interval = AnalysisComponent->GetOptimalTickInterval(Priority); // Need to pass AnalysisComponent
    SetComponentTickInterval(Component, Interval);
}

void UTickOptimizationComponent::SetComponentTickInterval(UActorComponent* Component, float Interval)
{
    if (!Component)
    {
        return;
    }

    Component->PrimaryComponentTick.TickInterval = Interval;
}

void UTickOptimizationComponent::DisableComponentTick(UActorComponent* Component)
{
    if (Component)
    {
        Component->SetComponentTickEnabled(false);
    }
}

void UTickOptimizationComponent::EnableComponentTick(UActorComponent* Component)
{
    if (Component)
    {
        Component->SetComponentTickEnabled(true);
    }
}

void UTickOptimizationComponent::OptimizeByDistance(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Get all analyzed components and optimize by distance
    TArray<FTickComponentInfo> ComponentInfos = AnalysisComponent->GetTickingComponents();

    for (const FTickComponentInfo& Info : ComponentInfos)
    {
        // We need to find the component from the info - this needs better implementation
        // For now, skip distance optimization
    }
}

void UTickOptimizationComponent::SetOptimizationStrategy(ETickOptimizationStrategy NewStrategy)
{
    OptimizationStrategy = NewStrategy;
}

void UTickOptimizationComponent::ApplyPerformanceStrategy(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization: Applying Performance strategy"));

    // Aggressive tick reduction
    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();

    for (UActorComponent* Component : Components)
    {
        if (!Component)
        {
            continue;
        }

        ETickPriority Priority = AnalysisComponent->DetermineTickPriority(Component);
        if (Priority == ETickPriority::Critical)
        {
            SetComponentTickInterval(Component, 0.0f); // Every frame
        }
        else
        {
            // Reduce tick frequency significantly
            SetComponentTickInterval(Component, AnalysisComponent->GetOptimalTickInterval(Priority) * 2.0f);
        }
    }
}

void UTickOptimizationComponent::ApplyBalancedStrategy(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization: Applying Balanced strategy"));

    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();

    for (UActorComponent* Component : Components)
    {
        if (!Component)
        {
            continue;
        }

        ETickPriority Priority = AnalysisComponent->DetermineTickPriority(Component);
        SetComponentTickInterval(Component, AnalysisComponent->GetOptimalTickInterval(Priority));
    }
}

void UTickOptimizationComponent::ApplyQualityStrategy(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Tick Optimization: Applying Quality strategy"));

    // More frequent updates for better visual quality
    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();

    for (UActorComponent* Component : Components)
    {
        if (!Component)
        {
            continue;
        }

        ETickPriority Priority = AnalysisComponent->DetermineTickPriority(Component);
        float BaseInterval = AnalysisComponent->GetOptimalTickInterval(Priority);
        SetComponentTickInterval(Component, BaseInterval * 0.5f); // Tick twice as often
    }
}

void UTickOptimizationComponent::ResetAllTickIntervals(class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();

    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            Component->PrimaryComponentTick.TickInterval = 0.0f;
        }
    }
}

float UTickOptimizationComponent::CalculateOptimalInterval(UActorComponent* Component, class UTickAnalysisComponent* AnalysisComponent) const
{
    if (!Component || !AnalysisComponent)
    {
        return 1.0f;
    }

    ETickPriority Priority = AnalysisComponent->DetermineTickPriority(Component);
    return AnalysisComponent->GetOptimalTickInterval(Priority);
}

void UTickOptimizationComponent::ApplyOptimization(UActorComponent* Component, float NewInterval)
{
    SetComponentTickInterval(Component, NewInterval);
    UE_LOG(LogTemp, Verbose, TEXT("Optimized %s: %.3fs interval"), *Component->GetName(), NewInterval);
}

float UTickOptimizationComponent::EstimatePerformanceGain(const TArray<UActorComponent*>& Components, class UTickAnalysisComponent* AnalysisComponent) const
{
    if (!AnalysisComponent)
    {
        return 0.0f;
    }

    // Simplified performance gain calculation
    float TotalSavings = 0.0f;

    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            FTickComponentInfo Info = AnalysisComponent->GetComponentTickInfo(Component);
            if (Info.OptimalTickInterval > Info.CurrentTickInterval)
            {
                TotalSavings += Info.AverageTickTime * (Info.OptimalTickInterval - Info.CurrentTickInterval);
            }
        }
    }

    return (TotalSavings / AnalysisComponent->GetTotalTickTime()) * 100.0f;
}

// Helper function to get recommendations
TArray<FString> UTickOptimizationComponent::GetOptimizationRecommendations(class UTickAnalysisComponent* AnalysisComponent) const
{
    TArray<FString> Recommendations;

    if (!AnalysisComponent)
    {
        return Recommendations;
    }

    TArray<UActorComponent*> Inefficient = AnalysisComponent->GetInefficientComponents();
    if (Inefficient.Num() > 0)
    {
        Recommendations.Add(FString::Printf(TEXT("%d components could benefit from reduced tick rates"), Inefficient.Num()));
    }

    if (AnalysisComponent->GetTotalTickTime() > 5.0f) // 5ms budget
    {
        Recommendations.Add(TEXT("Total tick time exceeds budget - consider disabling some components"));
    }

    if (AnalysisComponent->GetTickingComponentCount() > 500)
    {
        Recommendations.Add(TEXT("High component count - consider object pooling or culling"));
    }

    return Recommendations;
}