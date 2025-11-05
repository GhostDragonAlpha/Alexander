// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickOptimizationComponent.generated.h"

/**
 * Tick optimization strategy
 */
UENUM(BlueprintType)
enum class ETickOptimizationStrategy : uint8
{
    Performance     UMETA(DisplayName = "Performance - Maximize FPS"),
    Balanced        UMETA(DisplayName = "Balanced - Balance Quality/Performance"),
    Quality         UMETA(DisplayName = "Quality - Maximize Quality"),
    Custom          UMETA(DisplayName = "Custom - Manual Configuration")
};

/**
 * Tick optimization report
 */
USTRUCT(BlueprintType)
struct FTickOptimizationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalComponentsAnalyzed;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 ComponentsOptimized;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float EstimatedPerformanceGain;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> Recommendations;

    FTickOptimizationReport()
        : TotalComponentsAnalyzed(0)
        , ComponentsOptimized(0)
        , EstimatedPerformanceGain(0.0f)
    {}
};

/**
 * Delegate for tick optimization events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickOptimizationComplete, const FTickOptimizationReport&, Report);

/**
 * Component for applying tick optimization strategies
 */
UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTickOptimizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTickOptimizationComponent();

    // Optimization Functions
    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    FTickOptimizationReport OptimizeAllTicks(class UTickAnalysisComponent* AnalysisComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void OptimizeComponent(UActorComponent* Component, ETickPriority Priority);

    void SetComponentTickInterval(UActorComponent* Component, float Interval);

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void DisableComponentTick(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void EnableComponentTick(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    void OptimizeByDistance(class UTickAnalysisComponent* AnalysisComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Optimization")
    TArray<FString> GetOptimizationRecommendations(class UTickAnalysisComponent* AnalysisComponent) const;

    // Strategy Management
    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void SetOptimizationStrategy(ETickOptimizationStrategy NewStrategy);

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyPerformanceStrategy(class UTickAnalysisComponent* AnalysisComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyBalancedStrategy(class UTickAnalysisComponent* AnalysisComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Strategy")
    void ApplyQualityStrategy(class UTickAnalysisComponent* AnalysisComponent);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Tick|Utility")
    void ResetAllTickIntervals(class UTickAnalysisComponent* AnalysisComponent);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Tick|Events")
    FOnTickOptimizationComplete OnOptimizationComplete;

private:
    // Configuration
    ETickOptimizationStrategy OptimizationStrategy;

    // Internal functions
    float CalculateOptimalInterval(UActorComponent* Component, class UTickAnalysisComponent* AnalysisComponent) const;
    void ApplyOptimization(UActorComponent* Component, float NewInterval);
    float EstimatePerformanceGain(const TArray<UActorComponent*>& Components, class UTickAnalysisComponent* AnalysisComponent) const;
};