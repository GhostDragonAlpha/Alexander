// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerProgression.h"
#include "ProgressionDisplayWidget.generated.h"

/**
 * Progression Display Widget
 *
 * UMG widget for displaying player progression information:
 * - Level and XP progress
 * - All 8 skills with progress bars
 * - Currency (credits)
 * - Recent unlocks
 *
 * Automatically updates when progression events fire
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UProgressionDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UProgressionDisplayWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize with player progression reference
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void InitializeWidget(UPlayerProgression* InProgression);

	// Manual refresh (call if progression updates externally)
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void RefreshDisplay();

protected:
	// Reference to player progression
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	UPlayerProgression* PlayerProgression;

	// Event handlers for progression updates
	UFUNCTION()
	void OnPlayerLevelUp(int32 NewLevel, int32 OldLevel);

	UFUNCTION()
	void OnSkillLevelUp(EPlayerSkill Skill, int32 NewLevel);

	UFUNCTION()
	void OnCreditsChanged(int32 NewAmount, int32 Delta);

	UFUNCTION()
	void OnFeatureUnlocked(FName FeatureName);

	// Blueprint implementable events for visual updates
	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void UpdateLevelDisplay(int32 Level, float XPPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void UpdateSkillDisplay(EPlayerSkill Skill, int32 Level, float XPPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void UpdateCreditsDisplay(int32 Credits);

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void ShowLevelUpNotification(int32 NewLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void ShowSkillLevelUpNotification(EPlayerSkill Skill, int32 NewLevel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void ShowFeatureUnlockedNotification(const FString& FeatureName);

	// Helper functions for Blueprint
	UFUNCTION(BlueprintPure, Category = "Progression")
	FString GetSkillName(EPlayerSkill Skill) const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	FString GetRankName(EPlayerRank Rank) const;
};
