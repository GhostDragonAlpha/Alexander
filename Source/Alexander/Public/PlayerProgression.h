// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerProgression.generated.h"

/**
 * Player Skill Type
 * Different skills that players can level up
 */
UENUM(BlueprintType)
enum class EPlayerSkill : uint8
{
    Piloting,           // Spaceship flight and navigation
    Combat,             // Weapons and tactical combat
    Engineering,        // Ship systems and repairs
    Science,            // Research and discovery
    Farming,            // Agriculture and colony management
    Trading,            // Economy and negotiation
    Exploration,        // Planet discovery and mapping
    Leadership          // Crew management and diplomacy
};

/**
 * Player Rank
 * Overall player progression tiers
 */
UENUM(BlueprintType)
enum class EPlayerRank : uint8
{
    Cadet,              // Starting rank
    Ensign,             // Basic training complete
    Lieutenant,         // Competent spacefarer
    Commander,          // Experienced leader
    Captain,            // Ship commander
    Admiral,            // Fleet commander
    Legend              // Master of the galaxy
};

/**
 * Skill Level Data
 * Information about a specific skill's progression
 */
USTRUCT(BlueprintType)
struct FSkillLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlayerSkill SkillType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentXP = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float XPToNextLevel = 100.0f;

    // Get progress percentage to next level (0-100)
    float GetProgressPercent() const
    {
        if (XPToNextLevel <= 0.0f) return 100.0f;
        return FMath::Clamp((CurrentXP / XPToNextLevel) * 100.0f, 0.0f, 100.0f);
    }
};

/**
 * Unlock Data
 * Tracks what features/content the player has unlocked
 */
USTRUCT(BlueprintType)
struct FPlayerUnlocks
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedShips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockedEquipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> CompletedScenarios;

    bool HasUnlocked(FName FeatureName) const
    {
        return UnlockedFeatures.Contains(FeatureName) ||
               UnlockedLocations.Contains(FeatureName) ||
               UnlockedShips.Contains(FeatureName) ||
               UnlockedEquipment.Contains(FeatureName);
    }
};

/**
 * Player Statistics
 * Tracks various player achievements and statistics
 */
USTRUCT(BlueprintType)
struct FPlayerStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlayTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlanetsVisited = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlanetsColonized = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceTraveled = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ScenariosCompleted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CropsHarvested = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TradesMade = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CombatVictories = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DiscoveriesMade = 0;
};

/**
 * Player Progression System
 * Manages player leveling, skills, unlocks, and statistics
 * Integrates with GameplayScenario reward system
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UPlayerProgression : public USaveGame
{
    GENERATED_BODY()

public:
    UPlayerProgression();

    // === Experience & Leveling ===

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddExperience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddSkillXP(EPlayerSkill Skill, float Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    int32 GetPlayerLevel() const { return PlayerLevel; }

    UFUNCTION(BlueprintPure, Category = "Progression")
    float GetTotalExperience() const { return TotalExperience; }

    UFUNCTION(BlueprintPure, Category = "Progression")
    EPlayerRank GetPlayerRank() const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    FSkillLevel GetSkillLevel(EPlayerSkill Skill) const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    float GetLevelProgress() const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    TArray<FSkillLevel> GetAllSkills() const { return Skills; }

    // === Currency ===

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddCredits(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool SpendCredits(int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Progression")
    int32 GetCredits() const { return Credits; }

    // === Unlocks ===

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UnlockFeature(FName FeatureName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UnlockLocation(FName LocationName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UnlockShip(FName ShipName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void UnlockEquipment(FName EquipmentName);

    UFUNCTION(BlueprintPure, Category = "Progression")
    bool IsFeatureUnlocked(FName FeatureName) const;

    UFUNCTION(BlueprintPure, Category = "Progression")
    FPlayerUnlocks GetAllUnlocks() const { return Unlocks; }

    // === Statistics ===

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void IncrementPlanetsVisited();

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void IncrementPlanetsColonized();

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddDistanceTraveled(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void IncrementScenariosCompleted();

    UFUNCTION(BlueprintPure, Category = "Progression")
    FPlayerStatistics GetStatistics() const { return Statistics; }

    // === Save/Load ===

    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool SaveProgression();

    UFUNCTION(BlueprintCallable, Category = "Progression")
    static UPlayerProgression* LoadProgression();

    // === Events ===

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, OldLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillLevelUp, EPlayerSkill, Skill, int32, NewLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRankUp, EPlayerRank, NewRank);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFeatureUnlocked, FName, FeatureName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreditsChanged, int32, NewAmount, int32, Delta);

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FOnSkillLevelUp OnSkillLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FOnRankUp OnRankUp;

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FOnFeatureUnlocked OnFeatureUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FOnCreditsChanged OnCreditsChanged;

protected:
    // === Core Progression Data ===

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    int32 PlayerLevel;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    float TotalExperience;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    float ExperienceToNextLevel;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    int32 Credits;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    TArray<FSkillLevel> Skills;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    FPlayerUnlocks Unlocks;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Progression")
    FPlayerStatistics Statistics;

private:
    void CheckLevelUp();
    void CheckSkillLevelUp(EPlayerSkill Skill);
    void CheckRankUp();

    float CalculateXPForLevel(int32 Level) const;
    float CalculateSkillXPForLevel(int32 Level) const;

    void InitializeSkills();
    FSkillLevel* FindSkill(EPlayerSkill SkillType);
    const FSkillLevel* FindSkill(EPlayerSkill SkillType) const;

    static const FString SaveSlotName;
    static const int32 UserIndex;
};
