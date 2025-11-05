// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickAnalysisComponent.h"
#include "TickOptimizationComponent.h"
#include "TickReportingComponent.h"
#include "TickBudgetComponent.h"
#include "TickOptimizationComponent.h"
#include "TickOptimizationManager.generated.h"

/**
 * Tick Optimization Manager Component
 * Coordinates multiple specialized components for comprehensive tick optimization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTickOptimizationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UTickOptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration (delegated to sub-components)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    ETickOptimizationStrategy OptimizationStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    float OptimizationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    bool bEnableTickBudgeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    float MaxTickBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    bool bDisableTickOnDistantActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Settings")
    float DistanceThreshold;

    // Events (delegated to optimization component)
    UPROPERTY(BlueprintAssignable, Category = "Tick|Events")
    FOnTickOptimizationComplete OnOptimizationComplete;

    // Sub-components (public access for advanced users)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tick|Components")
    UTickAnalysisComponent* AnalysisComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tick|Components")
    UTickOptimizationComponent* OptimizationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tick|Components")
    UTickReportingComponent* ReportingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tick|Components")
    UTickBudgetComponent* BudgetComponent;

    // Analysis Functions (delegated to AnalysisComponent)
    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    void AnalyzeAllComponents() { if (AnalysisComponent) AnalysisComponent->AnalyzeAllComponents(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    TArray<FTickComponentInfo> GetTickingComponents() const { return AnalysisComponent ? AnalysisComponent->GetTickingComponents() : TArray<FTickComponentInfo>(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    FTickComponentInfo GetComponentTickInfo(UActorComponent* Component) const { return AnalysisComponent ? AnalysisComponent->GetComponentTickInfo(Component) : FTickComponentInfo(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    TArray<UActorComponent*> GetInefficientComponents() const { return AnalysisComponent ? AnalysisComponent->GetInefficientComponents() : TArray<UActorComponent*>(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    float GetTotalTickTime() const { return AnalysisComponent ? AnalysisComponent->GetTotalTickTime() : 0.0f; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Analysis")
    int32 GetTickingComponentCount() const { return AnalysisComponent ? AnalysisComponent->GetTickingComponentCount() : 0; }

    // Optimization Functions (delegated to OptimizationComponent)
    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    FTickOptimizationReport OptimizeAllTicks() { return OptimizationComponent && AnalysisComponent ? OptimizationComponent->OptimizeAllTicks(AnalysisComponent) : FTickOptimizationReport(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void OptimizeComponent(UActorComponent* Component, ETickPriority Priority) { if (OptimizationComponent) OptimizationComponent->OptimizeComponent(Component, Priority); }

    void SetComponentTickInterval(UActorComponent* Component, float Interval) { if (OptimizationComponent) OptimizationComponent->SetComponentTickInterval(Component, Interval); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void DisableComponentTick(UActorComponent* Component) { if (OptimizationComponent) OptimizationComponent->DisableComponentTick(Component); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void EnableComponentTick(UActorComponent* Component) { if (OptimizationComponent) OptimizationComponent->EnableComponentTick(Component); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void ApplyTickBudgeting() { if (BudgetComponent && AnalysisComponent) BudgetComponent->ApplyTickBudgeting(AnalysisComponent, OptimizationComponent); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void OptimizeByDistance() { if (OptimizationComponent && AnalysisComponent) OptimizationComponent->OptimizeByDistance(AnalysisComponent); }

    // Strategy Management (delegated to OptimizationComponent)
    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void SetOptimizationStrategy(ETickOptimizationStrategy NewStrategy);

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyPerformanceStrategy() { if (OptimizationComponent && AnalysisComponent) OptimizationComponent->ApplyPerformanceStrategy(AnalysisComponent); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyBalancedStrategy() { if (OptimizationComponent && AnalysisComponent) OptimizationComponent->ApplyBalancedStrategy(AnalysisComponent); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyQualityStrategy() { if (OptimizationComponent && AnalysisComponent) OptimizationComponent->ApplyQualityStrategy(AnalysisComponent); }

    // Recommendations (delegated to ReportingComponent)
    UFUNCTION(BlueprintCallable, Category = "Tick|Recommendations")
    TArray<FString> GetOptimizationRecommendations() const { return ReportingComponent && AnalysisComponent ? ReportingComponent->GetOptimizationRecommendations(AnalysisComponent) : TArray<FString>(); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Recommendations")
    FString GenerateTickReport() const { return ReportingComponent && AnalysisComponent ? ReportingComponent->GenerateTickReport(AnalysisComponent) : TEXT("Error: Components not available"); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Recommendations")
    bool ExportTickReport(const FString& Filename) const { return ReportingComponent && AnalysisComponent ? ReportingComponent->ExportTickReport(Filename, AnalysisComponent) : false; }

    // Utility Functions (delegated to AnalysisComponent)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Utility")
    float GetOptimalTickInterval(ETickPriority Priority) const { return AnalysisComponent ? AnalysisComponent->GetOptimalTickInterval(Priority) : 0.0333f; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Utility")
    ETickPriority DetermineTickPriority(UActorComponent* Component) const { return AnalysisComponent ? AnalysisComponent->DetermineTickPriority(Component) : ETickPriority::Medium; }

    UFUNCTION(BlueprintCallable, Category = "Tick|Utility")
    void ResetAllTickIntervals() { if (OptimizationComponent && AnalysisComponent) OptimizationComponent->ResetAllTickIntervals(AnalysisComponent); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Utility")
    void SaveTickConfiguration(const FString& ConfigName) { if (ReportingComponent && AnalysisComponent) ReportingComponent->SaveTickConfiguration(ConfigName, AnalysisComponent); }

    UFUNCTION(BlueprintCallable, Category = "Tick|Utility")
    void LoadTickConfiguration(const FString& ConfigName) { if (ReportingComponent && AnalysisComponent) ReportingComponent->LoadTickConfiguration(ConfigName, AnalysisComponent); }

private:
    // Internal state
    float TimeSinceLastOptimization;
    
    // Internal functions
    void GatherTickingComponents();
    void AnalyzeComponent(UActorComponent* Component);
    float CalculateOptimalInterval(UActorComponent* Component) const;
    void ApplyOptimization(UActorComponent* Component, float NewInterval);
    bool ShouldComponentTickEveryFrame(UActorComponent* Component) const;
    float EstimatePerformanceGain(const TArray<UActorComponent*>& Components) const;
    void UpdateTickBudget(float DeltaTime);
    FString GetComponentCategory(UActorComponent* Component) const;
    bool IsPlayerNearby(AActor* Actor) const;
};
