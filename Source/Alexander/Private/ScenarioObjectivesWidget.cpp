// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScenarioObjectivesWidget.h"

UScenarioObjectivesWidget::UScenarioObjectivesWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActiveScenario = nullptr;
}

void UScenarioObjectivesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ActiveScenario)
	{
		// Bind to scenario events
		ActiveScenario->OnScenarioPhaseChanged.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioPhaseChanged);
		ActiveScenario->OnObjectiveCompleted.AddDynamic(this, &UScenarioObjectivesWidget::OnObjectiveCompleted);
		ActiveScenario->OnScenarioCompleted.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioCompleted);
		ActiveScenario->OnScenarioFailed.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioFailed);

		// Initial display update
		RefreshDisplay();
	}
	else
	{
		ClearDisplay();
	}
}

void UScenarioObjectivesWidget::NativeDestruct()
{
	// Unbind from events
	if (ActiveScenario)
	{
		ActiveScenario->OnScenarioPhaseChanged.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioPhaseChanged);
		ActiveScenario->OnObjectiveCompleted.RemoveDynamic(this, &UScenarioObjectivesWidget::OnObjectiveCompleted);
		ActiveScenario->OnScenarioCompleted.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioCompleted);
		ActiveScenario->OnScenarioFailed.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioFailed);
	}

	Super::NativeDestruct();
}

void UScenarioObjectivesWidget::InitializeWidget(AGameplayScenario* InScenario)
{
	if (!InScenario)
	{
		UE_LOG(LogTemp, Warning, TEXT("ScenarioObjectivesWidget: Null scenario passed to InitializeWidget"));
		ClearDisplay();
		return;
	}

	// Unbind from old scenario if exists
	if (ActiveScenario)
	{
		ActiveScenario->OnScenarioPhaseChanged.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioPhaseChanged);
		ActiveScenario->OnObjectiveCompleted.RemoveDynamic(this, &UScenarioObjectivesWidget::OnObjectiveCompleted);
		ActiveScenario->OnScenarioCompleted.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioCompleted);
		ActiveScenario->OnScenarioFailed.RemoveDynamic(this, &UScenarioObjectivesWidget::OnScenarioFailed);
	}

	ActiveScenario = InScenario;

	// Bind to new scenario
	ActiveScenario->OnScenarioPhaseChanged.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioPhaseChanged);
	ActiveScenario->OnObjectiveCompleted.AddDynamic(this, &UScenarioObjectivesWidget::OnObjectiveCompleted);
	ActiveScenario->OnScenarioCompleted.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioCompleted);
	ActiveScenario->OnScenarioFailed.AddDynamic(this, &UScenarioObjectivesWidget::OnScenarioFailed);

	RefreshDisplay();
}

void UScenarioObjectivesWidget::RefreshDisplay()
{
	if (!ActiveScenario)
	{
		ClearDisplay();
		return;
	}

	// Update scenario info
	UpdateScenarioInfo(ActiveScenario->ScenarioName, ActiveScenario->ScenarioDescription);

	// Update phase
	EScenarioPhase CurrentPhase = ActiveScenario->GetCurrentPhase();
	UpdatePhaseDisplay(CurrentPhase);

	// Update objectives
	TArray<FScenarioObjective> Objectives = ActiveScenario->GetActiveObjectives();
	UpdateObjectivesList(Objectives);

	// Update overall progress
	float Progress = ActiveScenario->GetScenarioProgress();
	UpdateScenarioProgress(Progress);
}

void UScenarioObjectivesWidget::ClearDisplay()
{
	ActiveScenario = nullptr;
	HideAllDisplays();
}

void UScenarioObjectivesWidget::OnScenarioPhaseChanged(EScenarioPhase NewPhase)
{
	UE_LOG(LogTemp, Display, TEXT("ScenarioObjectivesWidget: Phase changed to %s"), *GetPhaseName(NewPhase));
	UpdatePhaseDisplay(NewPhase);
	RefreshDisplay();
}

void UScenarioObjectivesWidget::OnObjectiveCompleted(const FScenarioObjective& Objective)
{
	UE_LOG(LogTemp, Display, TEXT("ScenarioObjectivesWidget: Objective completed: %s"),
		*Objective.ObjectiveTitle.ToString());

	ShowObjectiveCompletedNotification(Objective);
	RefreshDisplay();
}

void UScenarioObjectivesWidget::OnScenarioCompleted(const FScenarioRewards& Rewards)
{
	UE_LOG(LogTemp, Display, TEXT("ScenarioObjectivesWidget: Scenario completed!"));
	ShowScenarioCompletedScreen(Rewards);
}

void UScenarioObjectivesWidget::OnScenarioFailed(const FText& FailureReason)
{
	UE_LOG(LogTemp, Display, TEXT("ScenarioObjectivesWidget: Scenario failed: %s"),
		*FailureReason.ToString());
	ShowScenarioFailedScreen(FailureReason);
}

FString UScenarioObjectivesWidget::GetPhaseName(EScenarioPhase Phase) const
{
	switch (Phase)
	{
		case EScenarioPhase::NotStarted:   return TEXT("Not Started");
		case EScenarioPhase::Introduction: return TEXT("Introduction");
		case EScenarioPhase::Tutorial:     return TEXT("Tutorial");
		case EScenarioPhase::Exploration:  return TEXT("Exploration");
		case EScenarioPhase::Mission:      return TEXT("Mission");
		case EScenarioPhase::Completed:    return TEXT("Completed");
		case EScenarioPhase::Failed:       return TEXT("Failed");
		default:                           return TEXT("Unknown");
	}
}

FLinearColor UScenarioObjectivesWidget::GetPhaseColor(EScenarioPhase Phase) const
{
	switch (Phase)
	{
		case EScenarioPhase::NotStarted:   return FLinearColor(0.5f, 0.5f, 0.5f);  // Gray
		case EScenarioPhase::Introduction: return FLinearColor(0.0f, 0.8f, 1.0f);  // Cyan
		case EScenarioPhase::Tutorial:     return FLinearColor(0.0f, 1.0f, 0.5f);  // Green
		case EScenarioPhase::Exploration:  return FLinearColor(1.0f, 0.8f, 0.0f);  // Yellow
		case EScenarioPhase::Mission:      return FLinearColor(1.0f, 0.5f, 0.0f);  // Orange
		case EScenarioPhase::Completed:    return FLinearColor(0.0f, 1.0f, 0.0f);  // Bright Green
		case EScenarioPhase::Failed:       return FLinearColor(1.0f, 0.0f, 0.0f);  // Red
		default:                           return FLinearColor(1.0f, 1.0f, 1.0f);  // White
	}
}

bool UScenarioObjectivesWidget::IsObjectiveRequired(const FScenarioObjective& Objective) const
{
	return !Objective.bOptional;
}
