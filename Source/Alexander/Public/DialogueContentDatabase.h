// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueContentDatabase.generated.h"

/**
 * Dialogue response structure
 */
USTRUCT(BlueprintType)
struct FDialogueResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ResponseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName NextDialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FName> RequiredFlags; // Flags that must be set to show this response

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FName> SetFlags; // Flags to set when this response is selected

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 ReputationRequirement = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bEndsConversation = false;

	FDialogueResponse()
		: NextDialogueID(NAME_None)
		, ReputationRequirement(0)
		, bEndsConversation(false)
	{
	}
};

/**
 * Dialogue entry structure
 */
USTRUCT(BlueprintType)
struct FDialogueEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName DialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueResponse> Responses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName EmotionalTone; // Friendly, Hostile, Neutral, Angry, Happy, Sad, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName SpeakerPersonality; // Merchant, Soldier, Scientist, Pirate, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FName> RequiredTags; // Context requirements (e.g., "FirstMeeting", "HasMission", "InCombat")

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FName> ExcludedTags; // Contexts where this dialogue shouldn't appear

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 Priority = 100; // Higher priority = more likely to be selected

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bCanRepeat = true;

	FDialogueEntry()
		: DialogueID(NAME_None)
		, EmotionalTone(NAME_None)
		, SpeakerPersonality(NAME_None)
		, Priority(100)
		, bCanRepeat(true)
	{
	}
};

/**
 * Dialogue entry array wrapper (needed for TMap compatibility)
 */
USTRUCT(BlueprintType)
struct FDialogueEntryArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueEntry> Entries;

	FDialogueEntryArray() {}
};

/**
 * AI Dialogue context for selection
 */
USTRUCT(BlueprintType)
struct FAIDialogueContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	FName NPCPersonality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	FName ConversationContext; // Greeting, Trading, Mission, Farewell, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	TArray<FName> ActiveTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	TArray<FName> SetFlags; // Player conversation flags

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	int32 PlayerReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	int32 TimesSpoken = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
	bool bFirstMeeting = true;

	FAIDialogueContext()
		: NPCPersonality(NAME_None)
		, ConversationContext(NAME_None)
		, PlayerReputation(0)
		, TimesSpoken(0)
		, bFirstMeeting(true)
	{
	}
};

/**
 * Dialogue category for organization
 */
UENUM(BlueprintType)
enum class EDialogueCategory : uint8
{
	Greeting UMETA(DisplayName = "Greetings"),
	Farewell UMETA(DisplayName = "Farewells"),
	Mission UMETA(DisplayName = "Mission Related"),
	Trading UMETA(DisplayName = "Trading/Commerce"),
	Combat UMETA(DisplayName = "Combat/Threat"),
	Information UMETA(DisplayName = "Information/Lore"),
	Ambient UMETA(DisplayName = "Ambient/Idle"),
	Reaction UMETA(DisplayName = "Reactions"),
	Quest UMETA(DisplayName = "Quest Dialogue"),
	Social UMETA(DisplayName = "Social Chatter")
};

/**
 * Dialogue content database data asset
 */
UCLASS(BlueprintType)
class ALEXANDER_API UDialogueContentDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName DatabaseID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText DatabaseDescription;

	// Greetings by NPC personality
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Greetings")
	TMap<FName, FDialogueEntryArray> Greetings;

	// Farewells by personality
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Farewells")
	TMap<FName, FDialogueEntryArray> Farewells;

	// Mission-related dialogue by mission type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Missions")
	TArray<FDialogueEntry> MissionDialogue;

	// Trading dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Trading")
	TArray<FDialogueEntry> TradingDialogue;

	// Combat/threat dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Combat")
	TArray<FDialogueEntry> CombatDialogue;

	// Information and lore dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Information")
	TArray<FDialogueEntry> InformationDialogue;

	// Random ambient dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Ambient")
	TArray<FDialogueEntry> AmbientDialogue;

	// Reaction dialogue (surprise, anger, joy, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Reactions")
	TMap<FName, FDialogueEntryArray> ReactionDialogue;

	// All dialogue entries for searching
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueEntry> AllDialogue;

	/**
	 * Get greeting dialogue for personality
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueEntry> GetGreetingsForPersonality(FName Personality) const;

	/**
	 * Get farewell dialogue for personality
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueEntry> GetFarewellsForPersonality(FName Personality) const;

	/**
	 * Get dialogue matching context
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueEntry> GetDialogueForContext(const FAIDialogueContext& Context, EDialogueCategory Category) const;

	/**
	 * Get random dialogue entry matching context
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FDialogueEntry GetRandomDialogue(const FAIDialogueContext& Context, EDialogueCategory Category) const;

	/**
	 * Check if dialogue entry is valid for context
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueValidForContext(const FDialogueEntry& Dialogue, const FAIDialogueContext& Context) const;

	/**
	 * Find dialogue by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FDialogueEntry FindDialogueByID(FName DialogueID) const;

	/**
	 * Get all dialogue IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	TArray<FName> GetAllDialogueIDs() const;

	/**
	 * Get dialogue by emotional tone
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueEntry> GetDialogueByTone(FName EmotionalTone) const;

	/**
	 * Get total dialogue count
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetTotalDialogueCount() const { return AllDialogue.Num(); }

	/**
	 * Initialize/rebuild the AllDialogue array from categorized arrays
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void RebuildDialogueIndex();
};
