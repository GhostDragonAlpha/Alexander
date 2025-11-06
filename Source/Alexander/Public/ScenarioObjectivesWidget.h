// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayScenario.h"
#include "ScenarioObjectivesWidget.generated.h"

/**
 * Scenario Objectives Widget
 *
 * UMG widget for displaying active scenario objectives:
 * - Scenario name and description
 * - Current phase
 * - Active objectives with progress bars
 * - Optional objectives marked separately
 * - Overall scenario progress
 *
 * Automatically updates when scenario events fire
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UScenarioObjectivesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UScenarioObjectivesWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize with scenario reference
	UFUNCTION(BlueprintCallable, Category = "Scenario")
	void InitializeWidget(AGameplayScenario* InScenario);

	// Manual refresh
	UFUNCTION(BlueprintCallable, Category = "Scenario")
	void RefreshDisplay();

	// Clear all displays (when no active scenario)
	UFUNCTION(BlueprintCallable, Category = "Scenario")
	void ClearDisplay();

protected:
	// Reference to active scenario
	UPROPERTY(BlueprintReadOnly, Category = "Scenario")
	AGameplayScenario* ActiveScenario;

	// Event handlers for scenario updates
	UFUNCTION()
	void OnScenarioPhaseChanged(EScenarioPhase NewPhase);

	UFUNCTION()
	void OnObjectiveCompleted(const FScenarioObjective& Objective);

	UFUNCTION()
	void OnScenarioCompleted(const FScenarioRewards& Rewards);

	UFUNCTION()
	void OnScenarioFailed(const FText& FailureReason);

	// Blueprint implementable events for visual updates
	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void UpdateScenarioInfo(const FText& ScenarioName, const FText& Description);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void UpdatePhaseDisplay(EScenarioPhase Phase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void UpdateObjectivesList(const TArray<FScenarioObjective>& Objectives);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void UpdateScenarioProgress(float ProgressPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void ShowObjectiveCompletedNotification(const FScenarioObjective& Objective);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void ShowScenarioCompletedScreen(const FScenarioRewards& Rewards);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void ShowScenarioFailedScreen(const FText& FailureReason);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scenario")
	void HideAllDisplays();

	// Helper functions for Blueprint
	UFUNCTION(BlueprintPure, Category = "Scenario")
	FString GetPhaseName(EScenarioPhase Phase) const;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	FLinearColor GetPhaseColor(EScenarioPhase Phase) const;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	bool IsObjectiveRequired(const FScenarioObjective& Objective) const;
};
