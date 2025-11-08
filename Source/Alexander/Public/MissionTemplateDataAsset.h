// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MissionTemplateDataAsset.generated.h"

/**
 * Mission type enumeration
 */
UENUM(BlueprintType)
enum class EMissionTypeDataAsset : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Exploration UMETA(DisplayName = "Exploration"),
	Trading UMETA(DisplayName = "Trading"),
	Diplomatic UMETA(DisplayName = "Diplomatic"),
	Rescue UMETA(DisplayName = "Rescue"),
	Stealth UMETA(DisplayName = "Stealth"),
	Mining UMETA(DisplayName = "Mining"),
	Construction UMETA(DisplayName = "Construction"),
	Escort UMETA(DisplayName = "Escort"),
	Investigation UMETA(DisplayName = "Investigation"),
	Assassination UMETA(DisplayName = "Assassination"),
	Smuggling UMETA(DisplayName = "Smuggling")
};

/**
 * Mission objective structure
 */
USTRUCT(BlueprintType)
struct FMissionObjectiveDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName ObjectiveType; // Kill, Collect, Goto, Scan, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 TargetCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bIsComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> RequiredObjectives; // Objectives that must complete first

	FMissionObjectiveDataAsset()
		: ObjectiveID(NAME_None)
		, ObjectiveType(NAME_None)
		, TargetCount(1)
		, CurrentCount(0)
		, bOptional(false)
		, bIsComplete(false)
	{
	}
};

/**
 * Mission rewards structure
 */
USTRUCT(BlueprintType)
struct FMissionRewardsDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 Credits = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 Experience = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 ReputationGain = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName FactionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<FName> ItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<FName> UnlockRewards; // Unlocked features/areas

	FMissionRewardsDataAsset()
		: Credits(1000)
		, Experience(100)
		, ReputationGain(10)
		, FactionID(NAME_None)
	{
	}
};

/**
 * Mission template structure - used for procedural generation
 */
USTRUCT(BlueprintType)
struct FMissionTemplateData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName TemplateID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMissionTypeDataAsset Type = EMissionTypeDataAsset::Combat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText BriefingText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FMissionObjectiveDataAsset> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FMissionRewardsDataAsset Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> RequiredTags; // Context requirements (e.g., "Space", "Planet", "Station")

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> ExcludedTags; // Contexts where this mission shouldn't appear

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MinPlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MaxPlayerLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 DifficultyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float EstimatedDuration = 600.0f; // Seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName FactionID; // Faction offering mission

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MinReputation = 0; // Minimum reputation with faction

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float RepeatCooldown = 0.0f; // Hours before can repeat

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> PrerequisiteMissions; // Missions that must be completed first

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 GenerationWeight = 100; // Higher = more likely to generate

	FMissionTemplateData()
		: TemplateID(NAME_None)
		, Type(EMissionTypeDataAsset::Combat)
		, MinPlayerLevel(1)
		, MaxPlayerLevel(100)
		, DifficultyLevel(1)
		, EstimatedDuration(600.0f)
		, FactionID(NAME_None)
		, MinReputation(0)
		, bIsRepeatable(false)
		, RepeatCooldown(0.0f)
		, GenerationWeight(100)
	{
	}
};

/**
 * Mission context for procedural generation
 */
USTRUCT(BlueprintType)
struct FMissionContextDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	TArray<FName> AvailableTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	FName NearestFaction = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	int32 FactionReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	float DifficultyModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	bool bAllowDangerousMissions = true;

	FMissionContextDataAsset()
		: PlayerLevel(1)
		, Location(FVector::ZeroVector)
		, NearestFaction(NAME_None)
		, FactionReputation(0)
		, DifficultyModifier(1.0f)
		, bAllowDangerousMissions(true)
	{
	}
};

/**
 * Mission data asset for template storage
 */
UCLASS(BlueprintType)
class ALEXANDER_API UMissionTemplateDataAssetClass : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FMissionTemplateData> Templates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName CategoryName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText CategoryDescription;

	/**
	 * Get templates by mission type
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	TArray<FMissionTemplateData> GetTemplatesByType(EMissionTypeDataAsset Type) const;

	/**
	 * Get templates for faction
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	TArray<FMissionTemplateData> GetTemplatesForFaction(FName FactionID) const;

	/**
	 * Get templates matching context
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	TArray<FMissionTemplateData> GetTemplatesForContext(const FMissionContextDataAsset& Context) const;

	/**
	 * Get random template matching criteria
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	FMissionTemplateData GetRandomTemplate(EMissionTypeDataAsset Type, int32 PlayerLevel) const;

	/**
	 * Check if template is valid for context
	 */
	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsTemplateValidForContext(const FMissionTemplateData& Template, const FMissionContextDataAsset& Context) const;

	/**
	 * Get all template IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Mission")
	TArray<FName> GetAllTemplateIDs() const;

	/**
	 * Find template by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Mission")
	FMissionTemplateData FindTemplateByID(FName TemplateID) const;

	/**
	 * Get templates by difficulty range
	 */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	TArray<FMissionTemplateData> GetTemplatesByDifficulty(int32 MinDifficulty, int32 MaxDifficulty) const;
};
