// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TutorialSequenceData.generated.h"

/**
 * Tutorial validation type
 */
UENUM(BlueprintType)
enum class ETutorialValidationType : uint8
{
	None UMETA(DisplayName = "None"),
	LocationReached UMETA(DisplayName = "Location Reached"),
	ActionPerformed UMETA(DisplayName = "Action Performed"),
	ItemCollected UMETA(DisplayName = "Item Collected"),
	EnemyDefeated UMETA(DisplayName = "Enemy Defeated"),
	ButtonPressed UMETA(DisplayName = "Button Pressed"),
	WidgetInteracted UMETA(DisplayName = "Widget Interacted"),
	TimeElapsed UMETA(DisplayName = "Time Elapsed"),
	ValueReached UMETA(DisplayName = "Value Reached"),
	ComponentActivated UMETA(DisplayName = "Component Activated"),
	SystemUsed UMETA(DisplayName = "System Used"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Tutorial hint display type
 */
UENUM(BlueprintType)
enum class ETutorialHintType : uint8
{
	Text UMETA(DisplayName = "Text Only"),
	Icon UMETA(DisplayName = "Icon Only"),
	TextAndIcon UMETA(DisplayName = "Text and Icon"),
	Animation UMETA(DisplayName = "Animation"),
	Highlight UMETA(DisplayName = "Highlight"),
	Arrow UMETA(DisplayName = "Arrow"),
	FullScreen UMETA(DisplayName = "Full Screen")
};

/**
 * Tutorial validation condition
 */
USTRUCT(BlueprintType)
struct FTutorialValidation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ETutorialValidationType ValidationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName ValidationID; // Unique identifier for this validation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FVector TargetLocation; // For location-based validations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float TargetRadius = 100.0f; // Radius for location checks

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName ActionName; // For action-based validations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName ItemID; // For item-based validations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 RequiredCount = 1; // Number of times condition must be met

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float TimeoutDuration = 0.0f; // Time before step fails (0 = no timeout)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName CustomValidationFunction; // Blueprint function to call for custom validation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bOptional = false; // If true, validation is optional

	FTutorialValidation()
		: ValidationType(ETutorialValidationType::None)
		, ValidationID(NAME_None)
		, TargetLocation(FVector::ZeroVector)
		, TargetRadius(100.0f)
		, ActionName(NAME_None)
		, ItemID(NAME_None)
		, RequiredCount(1)
		, TimeoutDuration(0.0f)
		, CustomValidationFunction(NAME_None)
		, bOptional(false)
	{
	}
};

/**
 * Tutorial objective structure
 */
USTRUCT(BlueprintType)
struct FTutorialObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText ObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FTutorialValidation> Validations; // All validations required for objective

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bShowInUI = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 Priority = 0; // Display priority in UI

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float RewardXP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 RewardCredits = 0;

	FTutorialObjective()
		: ObjectiveID(NAME_None)
		, bShowInUI(true)
		, Priority(0)
		, RewardXP(0.0f)
		, RewardCredits(0)
	{
	}
};

/**
 * Tutorial step structure
 */
USTRUCT(BlueprintType)
struct FTutorialStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName StepID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText StepTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText StepDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ETutorialHintType HintType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	UTexture2D* HintIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FTutorialObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FName> RequiredPreviousSteps; // Steps that must be completed first

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float DisplayDuration = 0.0f; // How long to show hint (0 = until completed)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FVector HintLocation; // World location for hint display

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	AActor* HintTargetActor; // Actor to attach hint to

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName OnStepStartEvent; // Event to trigger when step starts

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName OnStepCompleteEvent; // Event to trigger when step completes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bPauseGameDuringStep = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bSkippable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bMandatory = false; // Must complete to progress

	FTutorialStep()
		: StepID(NAME_None)
		, HintType(ETutorialHintType::Text)
		, HintIcon(nullptr)
		, DisplayDuration(0.0f)
		, HintLocation(FVector::ZeroVector)
		, HintTargetActor(nullptr)
		, OnStepStartEvent(NAME_None)
		, OnStepCompleteEvent(NAME_None)
		, bPauseGameDuringStep(false)
		, bSkippable(true)
		, bMandatory(false)
	{
	}
};

/**
 * Tutorial sequence context
 */
UENUM(BlueprintType)
enum class ETutorialContext : uint8
{
	General UMETA(DisplayName = "General"),
	Flight UMETA(DisplayName = "Flight"),
	Combat UMETA(DisplayName = "Combat"),
	Navigation UMETA(DisplayName = "Navigation"),
	Trading UMETA(DisplayName = "Trading"),
	Mining UMETA(DisplayName = "Mining"),
	Docking UMETA(DisplayName = "Docking"),
	UI UMETA(DisplayName = "UI"),
	VR UMETA(DisplayName = "VR"),
	Multiplayer UMETA(DisplayName = "Multiplayer"),
	Advanced UMETA(DisplayName = "Advanced")
};

/**
 * Tutorial difficulty level
 */
UENUM(BlueprintType)
enum class ETutorialDifficulty : uint8
{
	Beginner UMETA(DisplayName = "Beginner"),
	Intermediate UMETA(DisplayName = "Intermediate"),
	Advanced UMETA(DisplayName = "Advanced"),
	Expert UMETA(DisplayName = "Expert")
};

/**
 * Tutorial sequence data asset
 * Stores a complete tutorial sequence with steps, objectives, and validations
 */
UCLASS(BlueprintType)
class ALEXANDER_API UTutorialSequenceData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName SequenceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText SequenceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText SequenceDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ETutorialContext Context;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ETutorialDifficulty Difficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FTutorialStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FName> PrerequisiteSequences; // Other sequences that must be completed first

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 EstimatedDurationSeconds = 300; // Estimated time to complete

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float TotalRewardXP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 TotalRewardCredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bAutoStart = false; // Start automatically when conditions are met

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	int32 Priority = 0; // Display priority in tutorial list

	// Blueprint callable functions

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	TArray<FTutorialStep> GetStepsByContext(ETutorialContext ContextFilter) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	FTutorialStep GetStepByID(FName StepID) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool IsSequenceValidForPlayer(int32 PlayerLevel, const TArray<FName>& CompletedSequences) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	int32 GetTotalStepCount() const { return Steps.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	int32 GetTotalObjectiveCount() const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	float GetProgressPercentage(const TArray<FName>& CompletedSteps) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	TArray<FName> GetNextAvailableSteps(const TArray<FName>& CompletedSteps) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool ArePrerequisitesMet(const TArray<FName>& CompletedSequences) const;
};
