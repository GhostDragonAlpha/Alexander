// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickBudgetComponent.generated.h"

/**
 * Component for managing tick performance budgets
 */
UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTickBudgetComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTickBudgetComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Budget Management
    UFUNCTION(BlueprintCallable, Category = "Tick|Budget")
    void ApplyTickBudgeting(class UTickAnalysisComponent* AnalysisComponent, class UTickOptimizationComponent* OptimizationComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Budget")
    void UpdateTickBudget(float DeltaTime, class UTickAnalysisComponent* AnalysisComponent, class UTickOptimizationComponent* OptimizationComponent);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Budget")
    float MaxTickBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick|Budget")
    bool bEnableTickBudgeting;

private:
    // Budget tracking
    float TotalTickBudget;
};