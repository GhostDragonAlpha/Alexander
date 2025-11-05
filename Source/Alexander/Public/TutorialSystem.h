// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TutorialSystem.generated.h"

/**
 * Tutorial step definition
 */
USTRUCT(BlueprintType)
struct FTutorialStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bRequiresCompletion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName CompletionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FVector> HighlightLocations;
};

/**
 * Tutorial sequence definition
 */
USTRUCT(BlueprintType)
struct FTutorialSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FName SequenceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FText SequenceTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	TArray<FTutorialStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bCanSkip = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bShowOnce = true;
};

/**
 * Tutorial system for in-game guidance
 */
UCLASS()
class ALEXANDER_API UTutorialSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tutorial management
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorial(FName TutorialName);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StopCurrentTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SkipCurrentTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void CompleteCurrentStep();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void MarkTutorialCompleted(FName TutorialName);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool HasCompletedTutorial(FName TutorialName) const;

	// Tutorial queries
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialActive() const { return bTutorialActive; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	FTutorialStep GetCurrentStep() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetTotalSteps() const;

	// Predefined tutorials
	void InitializeFarmingTutorial();
	void InitializeLandingTutorial();
	void InitializeVRInteractionTutorial();
	void InitializeBiomeExplorationTutorial();

protected:
	void UpdateTutorial(float DeltaTime);
	void AdvanceToNextStep();
	void ShowTutorialUI();
	void HideTutorialUI();

private:
	UPROPERTY()
	TMap<FName, FTutorialSequence> TutorialSequences;

	UPROPERTY()
	TSet<FName> CompletedTutorials;

	UPROPERTY()
	FTutorialSequence CurrentTutorial;

	UPROPERTY()
	int32 CurrentStepIndex = 0;

	UPROPERTY()
	float StepTimer = 0.0f;

	UPROPERTY()
	bool bTutorialActive = false;

	FTimerHandle UpdateTimerHandle;
};
