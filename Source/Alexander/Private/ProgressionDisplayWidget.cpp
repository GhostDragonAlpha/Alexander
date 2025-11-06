// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProgressionDisplayWidget.h"

UProgressionDisplayWidget::UProgressionDisplayWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerProgression = nullptr;
}

void UProgressionDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Auto-load player progression if not already set
	if (!PlayerProgression)
	{
		PlayerProgression = UPlayerProgression::LoadProgression();
		if (PlayerProgression)
		{
			UE_LOG(LogTemp, Log, TEXT("ProgressionDisplayWidget: Loaded player progression"));
		}
	}

	// Bind to progression events
	if (PlayerProgression)
	{
		PlayerProgression->OnLevelUp.AddDynamic(this, &UProgressionDisplayWidget::OnPlayerLevelUp);
		PlayerProgression->OnSkillLevelUp.AddDynamic(this, &UProgressionDisplayWidget::OnSkillLevelUp);
		PlayerProgression->OnCreditsChanged.AddDynamic(this, &UProgressionDisplayWidget::OnCreditsChanged);
		PlayerProgression->OnFeatureUnlocked.AddDynamic(this, &UProgressionDisplayWidget::OnFeatureUnlocked);

		// Initial display update
		RefreshDisplay();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ProgressionDisplayWidget: No player progression available"));
	}
}

void UProgressionDisplayWidget::NativeDestruct()
{
	// Unbind from events
	if (PlayerProgression)
	{
		PlayerProgression->OnLevelUp.RemoveDynamic(this, &UProgressionDisplayWidget::OnPlayerLevelUp);
		PlayerProgression->OnSkillLevelUp.RemoveDynamic(this, &UProgressionDisplayWidget::OnSkillLevelUp);
		PlayerProgression->OnCreditsChanged.RemoveDynamic(this, &UProgressionDisplayWidget::OnCreditsChanged);
		PlayerProgression->OnFeatureUnlocked.RemoveDynamic(this, &UProgressionDisplayWidget::OnFeatureUnlocked);
	}

	Super::NativeDestruct();
}

void UProgressionDisplayWidget::InitializeWidget(UPlayerProgression* InProgression)
{
	if (!InProgression)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProgressionDisplayWidget: Null progression passed to InitializeWidget"));
		return;
	}

	// Unbind from old progression if exists
	if (PlayerProgression)
	{
		PlayerProgression->OnLevelUp.RemoveDynamic(this, &UProgressionDisplayWidget::OnPlayerLevelUp);
		PlayerProgression->OnSkillLevelUp.RemoveDynamic(this, &UProgressionDisplayWidget::OnSkillLevelUp);
		PlayerProgression->OnCreditsChanged.RemoveDynamic(this, &UProgressionDisplayWidget::OnCreditsChanged);
		PlayerProgression->OnFeatureUnlocked.RemoveDynamic(this, &UProgressionDisplayWidget::OnFeatureUnlocked);
	}

	PlayerProgression = InProgression;

	// Bind to new progression
	PlayerProgression->OnLevelUp.AddDynamic(this, &UProgressionDisplayWidget::OnPlayerLevelUp);
	PlayerProgression->OnSkillLevelUp.AddDynamic(this, &UProgressionDisplayWidget::OnSkillLevelUp);
	PlayerProgression->OnCreditsChanged.AddDynamic(this, &UProgressionDisplayWidget::OnCreditsChanged);
	PlayerProgression->OnFeatureUnlocked.AddDynamic(this, &UProgressionDisplayWidget::OnFeatureUnlocked);

	RefreshDisplay();
}

void UProgressionDisplayWidget::RefreshDisplay()
{
	if (!PlayerProgression)
	{
		return;
	}

	// Update level display
	int32 Level = PlayerProgression->GetPlayerLevel();
	float XPPercent = PlayerProgression->GetLevelProgress();
	UpdateLevelDisplay(Level, XPPercent);

	// Update all skill displays
	TArray<FSkillLevel> Skills = PlayerProgression->GetAllSkills();
	for (const FSkillLevel& Skill : Skills)
	{
		UpdateSkillDisplay(Skill.SkillType, Skill.CurrentLevel, Skill.GetProgressPercent());
	}

	// Update credits
	UpdateCreditsDisplay(PlayerProgression->GetCredits());
}

void UProgressionDisplayWidget::OnPlayerLevelUp(int32 NewLevel, int32 OldLevel)
{
	UE_LOG(LogTemp, Display, TEXT("ProgressionDisplayWidget: Player leveled up %d -> %d"), OldLevel, NewLevel);

	// Update display
	float XPPercent = PlayerProgression->GetLevelProgress();
	UpdateLevelDisplay(NewLevel, XPPercent);

	// Show notification
	ShowLevelUpNotification(NewLevel);
}

void UProgressionDisplayWidget::OnSkillLevelUp(EPlayerSkill Skill, int32 NewLevel)
{
	UE_LOG(LogTemp, Display, TEXT("ProgressionDisplayWidget: Skill %s leveled up to %d"),
		*GetSkillName(Skill), NewLevel);

	// Update skill display
	FSkillLevel SkillData = PlayerProgression->GetSkillLevel(Skill);
	UpdateSkillDisplay(Skill, SkillData.CurrentLevel, SkillData.GetProgressPercent());

	// Show notification
	ShowSkillLevelUpNotification(Skill, NewLevel);
}

void UProgressionDisplayWidget::OnCreditsChanged(int32 NewAmount, int32 Delta)
{
	UpdateCreditsDisplay(NewAmount);
}

void UProgressionDisplayWidget::OnFeatureUnlocked(FName FeatureName)
{
	UE_LOG(LogTemp, Display, TEXT("ProgressionDisplayWidget: Feature unlocked: %s"), *FeatureName.ToString());
	ShowFeatureUnlockedNotification(FeatureName.ToString());
}

FString UProgressionDisplayWidget::GetSkillName(EPlayerSkill Skill) const
{
	switch (Skill)
	{
		case EPlayerSkill::Piloting:    return TEXT("Piloting");
		case EPlayerSkill::Combat:      return TEXT("Combat");
		case EPlayerSkill::Engineering: return TEXT("Engineering");
		case EPlayerSkill::Science:     return TEXT("Science");
		case EPlayerSkill::Farming:     return TEXT("Farming");
		case EPlayerSkill::Trading:     return TEXT("Trading");
		case EPlayerSkill::Exploration: return TEXT("Exploration");
		case EPlayerSkill::Leadership:  return TEXT("Leadership");
		default:                        return TEXT("Unknown");
	}
}

FString UProgressionDisplayWidget::GetRankName(EPlayerRank Rank) const
{
	switch (Rank)
	{
		case EPlayerRank::Cadet:      return TEXT("Cadet");
		case EPlayerRank::Ensign:     return TEXT("Ensign");
		case EPlayerRank::Lieutenant: return TEXT("Lieutenant");
		case EPlayerRank::Commander:  return TEXT("Commander");
		case EPlayerRank::Captain:    return TEXT("Captain");
		case EPlayerRank::Admiral:    return TEXT("Admiral");
		case EPlayerRank::Legend:     return TEXT("Legend");
		default:                      return TEXT("Unknown");
	}
}
