// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerProgression.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

const FString UPlayerProgression::SaveSlotName = TEXT("PlayerProgressionSave");
const int32 UPlayerProgression::UserIndex = 0;

UPlayerProgression::UPlayerProgression()
{
    PlayerLevel = 1;
    TotalExperience = 0.0f;
    ExperienceToNextLevel = CalculateXPForLevel(2);
    Credits = 1000; // Starting credits

    InitializeSkills();
}

void UPlayerProgression::InitializeSkills()
{
    // Initialize all skills at level 1
    Skills.Empty();

    for (int32 i = 0; i < (int32)EPlayerSkill::Leadership + 1; i++)
    {
        FSkillLevel Skill;
        Skill.SkillType = (EPlayerSkill)i;
        Skill.CurrentLevel = 1;
        Skill.CurrentXP = 0.0f;
        Skill.XPToNextLevel = CalculateSkillXPForLevel(2);
        Skills.Add(Skill);
    }
}

void UPlayerProgression::AddExperience(int32 Amount)
{
    if (Amount <= 0) return;

    int32 OldLevel = PlayerLevel;
    TotalExperience += Amount;

    UE_LOG(LogTemp, Log, TEXT("Player gained %d XP (Total: %.0f)"), Amount, TotalExperience);

    CheckLevelUp();

    if (PlayerLevel > OldLevel)
    {
        CheckRankUp();
    }
}

void UPlayerProgression::AddSkillXP(EPlayerSkill Skill, float Amount)
{
    if (Amount <= 0.0f) return;

    FSkillLevel* SkillData = FindSkill(Skill);
    if (!SkillData) return;

    SkillData->CurrentXP += Amount;

    UE_LOG(LogTemp, Log, TEXT("Skill %d gained %.1f XP"), (int32)Skill, Amount);

    CheckSkillLevelUp(Skill);
}

void UPlayerProgression::AddCredits(int32 Amount)
{
    if (Amount == 0) return;

    int32 OldCredits = Credits;
    Credits += Amount;
    Credits = FMath::Max(0, Credits); // Don't go negative

    UE_LOG(LogTemp, Log, TEXT("Credits changed by %d (Total: %d)"), Amount, Credits);

    OnCreditsChanged.Broadcast(Credits, Amount);
}

bool UPlayerProgression::SpendCredits(int32 Amount)
{
    if (Amount <= 0) return false;
    if (Credits < Amount) return false;

    AddCredits(-Amount);
    return true;
}

void UPlayerProgression::UnlockFeature(FName FeatureName)
{
    if (Unlocks.UnlockedFeatures.Contains(FeatureName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Feature already unlocked: %s"), *FeatureName.ToString());
        return;
    }

    Unlocks.UnlockedFeatures.Add(FeatureName);
    UE_LOG(LogTemp, Display, TEXT("Feature unlocked: %s"), *FeatureName.ToString());

    OnFeatureUnlocked.Broadcast(FeatureName);
}

void UPlayerProgression::UnlockLocation(FName LocationName)
{
    if (Unlocks.UnlockedLocations.Contains(LocationName))
    {
        return;
    }

    Unlocks.UnlockedLocations.Add(LocationName);
    UE_LOG(LogTemp, Display, TEXT("Location unlocked: %s"), *LocationName.ToString());

    OnFeatureUnlocked.Broadcast(LocationName);
}

void UPlayerProgression::UnlockShip(FName ShipName)
{
    if (Unlocks.UnlockedShips.Contains(ShipName))
    {
        return;
    }

    Unlocks.UnlockedShips.Add(ShipName);
    UE_LOG(LogTemp, Display, TEXT("Ship unlocked: %s"), *ShipName.ToString());

    OnFeatureUnlocked.Broadcast(ShipName);
}

void UPlayerProgression::UnlockEquipment(FName EquipmentName)
{
    if (Unlocks.UnlockedEquipment.Contains(EquipmentName))
    {
        return;
    }

    Unlocks.UnlockedEquipment.Add(EquipmentName);
    UE_LOG(LogTemp, Display, TEXT("Equipment unlocked: %s"), *EquipmentName.ToString());

    OnFeatureUnlocked.Broadcast(EquipmentName);
}

bool UPlayerProgression::IsFeatureUnlocked(FName FeatureName) const
{
    return Unlocks.HasUnlocked(FeatureName);
}

EPlayerRank UPlayerProgression::GetPlayerRank() const
{
    if (PlayerLevel >= 50) return EPlayerRank::Legend;
    if (PlayerLevel >= 40) return EPlayerRank::Admiral;
    if (PlayerLevel >= 30) return EPlayerRank::Captain;
    if (PlayerLevel >= 20) return EPlayerRank::Commander;
    if (PlayerLevel >= 10) return EPlayerRank::Lieutenant;
    if (PlayerLevel >= 5) return EPlayerRank::Ensign;
    return EPlayerRank::Cadet;
}

FSkillLevel UPlayerProgression::GetSkillLevel(EPlayerSkill Skill) const
{
    const FSkillLevel* SkillData = FindSkill(Skill);
    if (SkillData)
    {
        return *SkillData;
    }

    // Return default if not found
    FSkillLevel Default;
    Default.SkillType = Skill;
    Default.CurrentLevel = 1;
    Default.CurrentXP = 0.0f;
    Default.XPToNextLevel = 100.0f;
    return Default;
}

float UPlayerProgression::GetLevelProgress() const
{
    if (ExperienceToNextLevel <= 0.0f)
    {
        return 100.0f;
    }

    // Calculate XP earned in current level
    float CurrentLevelXP = CalculateXPForLevel(PlayerLevel);
    float XPEarnedInLevel = TotalExperience - CurrentLevelXP;
    float XPNeededForLevel = ExperienceToNextLevel - CurrentLevelXP;

    // Return percentage progress
    return FMath::Clamp((XPEarnedInLevel / XPNeededForLevel) * 100.0f, 0.0f, 100.0f);
}

void UPlayerProgression::IncrementPlanetsVisited()
{
    Statistics.PlanetsVisited++;
    UE_LOG(LogTemp, Log, TEXT("Planets visited: %d"), Statistics.PlanetsVisited);
}

void UPlayerProgression::IncrementPlanetsColonized()
{
    Statistics.PlanetsColonized++;
    UE_LOG(LogTemp, Log, TEXT("Planets colonized: %d"), Statistics.PlanetsColonized);
}

void UPlayerProgression::AddDistanceTraveled(float Distance)
{
    Statistics.DistanceTraveled += Distance;
}

void UPlayerProgression::IncrementScenariosCompleted()
{
    Statistics.ScenariosCompleted++;
    UE_LOG(LogTemp, Log, TEXT("Scenarios completed: %d"), Statistics.ScenariosCompleted);
}

bool UPlayerProgression::SaveProgression()
{
    bool bSuccess = UGameplayStatics::SaveGameToSlot(this, SaveSlotName, UserIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Player progression saved successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save player progression"));
    }

    return bSuccess;
}

UPlayerProgression* UPlayerProgression::LoadProgression()
{
    UPlayerProgression* LoadedData = Cast<UPlayerProgression>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex)
    );

    if (LoadedData)
    {
        UE_LOG(LogTemp, Log, TEXT("Player progression loaded successfully (Level %d)"),
               LoadedData->PlayerLevel);
        return LoadedData;
    }

    // Create new progression if no save exists
    UE_LOG(LogTemp, Log, TEXT("No save found, creating new player progression"));
    UPlayerProgression* NewProgression = NewObject<UPlayerProgression>();
    return NewProgression;
}

void UPlayerProgression::CheckLevelUp()
{
    while (TotalExperience >= ExperienceToNextLevel)
    {
        int32 OldLevel = PlayerLevel;
        PlayerLevel++;
        ExperienceToNextLevel = CalculateXPForLevel(PlayerLevel + 1);

        UE_LOG(LogTemp, Display, TEXT("========================================"));
        UE_LOG(LogTemp, Display, TEXT("LEVEL UP! %d -> %d"), OldLevel, PlayerLevel);
        UE_LOG(LogTemp, Display, TEXT("========================================"));

        OnLevelUp.Broadcast(PlayerLevel, OldLevel);
    }
}

void UPlayerProgression::CheckSkillLevelUp(EPlayerSkill Skill)
{
    FSkillLevel* SkillData = FindSkill(Skill);
    if (!SkillData) return;

    while (SkillData->CurrentXP >= SkillData->XPToNextLevel)
    {
        int32 OldLevel = SkillData->CurrentLevel;
        SkillData->CurrentLevel++;
        SkillData->CurrentXP -= SkillData->XPToNextLevel;
        SkillData->XPToNextLevel = CalculateSkillXPForLevel(SkillData->CurrentLevel + 1);

        UE_LOG(LogTemp, Display, TEXT("Skill Level Up! Skill %d: %d -> %d"),
               (int32)Skill, OldLevel, SkillData->CurrentLevel);

        OnSkillLevelUp.Broadcast(Skill, SkillData->CurrentLevel);
    }
}

void UPlayerProgression::CheckRankUp()
{
    static EPlayerRank LastRank = EPlayerRank::Cadet;
    EPlayerRank CurrentRank = GetPlayerRank();

    if (CurrentRank != LastRank)
    {
        UE_LOG(LogTemp, Display, TEXT("========================================"));
        UE_LOG(LogTemp, Display, TEXT("RANK UP! New Rank: %d"), (int32)CurrentRank);
        UE_LOG(LogTemp, Display, TEXT("========================================"));

        OnRankUp.Broadcast(CurrentRank);
        LastRank = CurrentRank;
    }
}

float UPlayerProgression::CalculateXPForLevel(int32 Level) const
{
    // Exponential XP curve: XP = 100 * (Level ^ 1.5)
    return 100.0f * FMath::Pow((float)Level, 1.5f);
}

float UPlayerProgression::CalculateSkillXPForLevel(int32 Level) const
{
    // Linear XP curve for skills: XP = 50 * Level
    return 50.0f * (float)Level;
}

FSkillLevel* UPlayerProgression::FindSkill(EPlayerSkill SkillType)
{
    for (FSkillLevel& Skill : Skills)
    {
        if (Skill.SkillType == SkillType)
        {
            return &Skill;
        }
    }
    return nullptr;
}

const FSkillLevel* UPlayerProgression::FindSkill(EPlayerSkill SkillType) const
{
    for (const FSkillLevel& Skill : Skills)
    {
        if (Skill.SkillType == SkillType)
        {
            return &Skill;
        }
    }
    return nullptr;
}
