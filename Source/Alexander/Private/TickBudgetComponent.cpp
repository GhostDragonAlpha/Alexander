// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickBudgetComponent.h"
#include "TickAnalysisComponent.h"
#include "TickOptimizationComponent.h"

UTickBudgetComponent::UTickBudgetComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check budget every second

    // Default settings
    MaxTickBudgetMs = 5.0f;
    bEnableTickBudgeting = true;
    TotalTickBudget = 0.0f;
}

void UTickBudgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Budget checking is handled by UpdateTickBudget, called from main manager
}

void UTickBudgetComponent::ApplyTickBudgeting(class UTickAnalysisComponent* AnalysisComponent, class UTickOptimizationComponent* OptimizationComponent)
{
    // Implement tick budgeting logic
    // This would distribute available tick time among components based on priority
    UE_LOG(LogTemp, Verbose, TEXT("Tick Budget: Applying tick budgeting (Budget: %.2fms)"), MaxTickBudgetMs);
}

void UTickBudgetComponent::UpdateTickBudget(float DeltaTime, class UTickAnalysisComponent* AnalysisComponent, class UTickOptimizationComponent* OptimizationComponent)
{
    if (!bEnableTickBudgeting || !AnalysisComponent || !OptimizationComponent)
    {
        return;
    }

    TotalTickBudget += AnalysisComponent->GetTotalTickTime();

    if (TotalTickBudget > MaxTickBudgetMs)
    {
        // Budget exceeded, apply more aggressive optimization
        OptimizationComponent->ApplyPerformanceStrategy(AnalysisComponent);
        TotalTickBudget = 0.0f;

        UE_LOG(LogTemp, Warning, TEXT("Tick Budget: Budget exceeded (%.2fms > %.2fms), applying performance optimization"),
            TotalTickBudget, MaxTickBudgetMs);
    }
}