// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AIDialogueSystem.generated.h"

// Forward declarations
class UNPCBrain;
class UMemorySystem;
class UPersonalityProfile;

/**
 * Dialogue intent types
 */
UENUM(BlueprintType)
enum class EDialogueIntent : uint8
{
	Greeting UMETA(DisplayName = "Greeting"),
	Question UMETA(DisplayName = "Question"),
	Statement UMETA(DisplayName = "Statement"),
	Request UMETA(DisplayName = "Request"),
	Trade UMETA(DisplayName = "Trade"),
	Combat UMETA(DisplayName = "Combat"),
	Farewell UMETA(DisplayName = "Farewell"),
	Information UMETA(DisplayName = "Information"),
	Help UMETA(DisplayName = "Help"),
	Flirt UMETA(DisplayName = "Flirt"),
	Threat UMETA(DisplayName = "Threat"),
	Bargain UMETA(DisplayName = "Bargain"),
	Gossip UMETA(DisplayName = "Gossip")
};

/**
 * Emotion states for NPCs
 */
UENUM(BlueprintType)
enum class EEmotionState : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Happy UMETA(DisplayName = "Happy"),
	Sad UMETA(DisplayName = "Sad"),
	Angry UMETA(DisplayName = "Angry"),
	Fearful UMETA(DisplayName = "Fearful"),
	Excited UMETA(DisplayName = "Excited"),
	Bored UMETA(DisplayName = "Bored"),
	Surprised UMETA(DisplayName = "Surprised"),
	Disgusted UMETA(DisplayName = "Disgusted"),
	Confused UMETA(DisplayName = "Confused"),
	Anxious UMETA(DisplayName = "Anxious")
};

/**
 * Memory type classification
 */
UENUM(BlueprintType)
enum class EMemoryType : uint8
{
	Conversation UMETA(DisplayName = "Conversation"),
	Interaction UMETA(DisplayName = "Interaction"),
	Observation UMETA(DisplayName = "Observation"),
	Relationship UMETA(DisplayName = "Relationship"),
	Location UMETA(DisplayName = "Location"),
	Item UMETA(DisplayName = "Item"),
	Quest UMETA(DisplayName = "Quest"),
	Skill UMETA(DisplayName = "Skill"),
	Emotion UMETA(DisplayName = "Emotion")
};

/**
 * Dialogue context information
 */
USTRUCT(BlueprintType)
struct FAlexanderDialogueContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	EDialogueIntent PlayerIntent = EDialogueIntent::Greeting;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString PlayerStatement = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FVector PlayerDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	float TimeOfDay = 12.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	FString CurrentLocation = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	TArray<FString> NearbyActors;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	bool bIsInCombat = false;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	bool bPlayerIsArmed = false;

	UPROPERTY(BlueprintReadWrite, Category = "Dialogue")
	float RelationshipLevel = 0.0f;
};

/**
 * Memory fragment for NPC memory system
 */
USTRUCT(BlueprintType)
struct FMemoryFragment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	FString MemoryID;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	EMemoryType MemoryType = EMemoryType::Conversation;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	FString Content = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	float Importance = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	float EmotionalWeight = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	TArray<FString> AssociatedActors;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	FString Location = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	bool bIsLongTerm = false;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	int32 AccessCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Memory")
	FDateTime LastAccessed;
};

/**
 * Personality trait definition
 */
USTRUCT(BlueprintType)
struct FPersonalityTrait
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Personality")
	FString TraitName = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Personality")
	float Value = 0.5f; // 0.0 to 1.0

	UPROPERTY(BlueprintReadWrite, Category = "Personality")
	float Importance = 1.0f; // How much this trait affects behavior
};

/**
 * AI Dialogue System - LLM-powered natural conversations
 * Creates living, breathing NPCs with memory and personality
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UAIDialogueSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIDialogueSystem();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// DIALOGUE PROCESSING
	// ============================================================================

	// Process player input and generate NPC response
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString ProcessDialogue(const FString& PlayerInput, const FAlexanderDialogueContext& Context);

	// Get dialogue response from LLM
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FString GenerateLLMResponse(const FString& Prompt, const FAlexanderDialogueContext& Context);

	// Parse player intent from input
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EDialogueIntent ParsePlayerIntent(const FString& PlayerInput);

	// Get appropriate emotion based on context
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	EEmotionState GetEmotionalResponse(const FAlexanderDialogueContext& Context);

	// ============================================================================
	// MEMORY SYSTEM
	// ============================================================================

	// Store memory fragment
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void StoreMemory(const FMemoryFragment& Memory);

	// Retrieve relevant memories
	UFUNCTION(BlueprintCallable, Category = "Memory")
	TArray<FMemoryFragment> GetRelevantMemories(const FString& Query, int32 MaxResults = 5);

	// Update memory importance
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void UpdateMemoryImportance(const FString& MemoryID, float NewImportance);

	// Forget old/unimportant memories
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ForgetMemories(float Threshold = 0.1f);

	// Get memory summary for context
	UFUNCTION(BlueprintCallable, Category = "Memory")
	FString GetMemoryContext(const FString& Query);

	// ============================================================================
	// PERSONALITY SYSTEM
	// ============================================================================

	// Initialize personality profile
	UFUNCTION(BlueprintCallable, Category = "Personality")
	void InitializePersonality();

	// Get personality trait value
	UFUNCTION(BlueprintCallable, Category = "Personality")
	float GetPersonalityTrait(const FString& TraitName);

	// Update personality based on experiences
	UFUNCTION(BlueprintCallable, Category = "Personality")
	void UpdatePersonality(const FString& TraitName, float Delta);

	// Get personality description
	UFUNCTION(BlueprintCallable, Category = "Personality")
	FString GetPersonalityDescription();

	// ============================================================================
	// DIALOGUE DATABASE INTEGRATION
	// ============================================================================

	// Get contextual dialogue from database
	UFUNCTION(BlueprintCallable, Category = "Dialogue Database")
	FText GetContextualDialogue(const FName& ConversationContext, const TArray<FName>& ActiveTags);

	// Get random greeting
	UFUNCTION(BlueprintCallable, Category = "Dialogue Database")
	FText GetGreetingDialogue();

	// Get random farewell
	UFUNCTION(BlueprintCallable, Category = "Dialogue Database")
	FText GetFarewellDialogue();

	// Load dialogue database
	UFUNCTION(BlueprintCallable, Category = "Dialogue Database")
	void LoadDialogueDatabase(UDialogueContentDatabase* Database);

	// ============================================================================
	// RELATIONSHIP SYSTEM
	// ============================================================================

	// Get relationship level with player
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	float GetRelationshipLevel(const FString& ActorID);

	// Update relationship
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	void UpdateRelationship(const FString& ActorID, float Delta);

	// Get relationship status
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	FString GetRelationshipStatus(const FString& ActorID);

	// ============================================================================
	// EMOTIONAL SYSTEM
	// ============================================================================

	// Get current emotion
	UFUNCTION(BlueprintCallable, Category = "Emotion")
	EEmotionState GetCurrentEmotion() const { return CurrentEmotion; }

	// Set emotion based on events
	UFUNCTION(BlueprintCallable, Category = "Emotion")
	void SetEmotion(EEmotionState NewEmotion, float Intensity = 1.0f);

	// Update emotion over time
	UFUNCTION(BlueprintCallable, Category = "Emotion")
	void UpdateEmotion(float DeltaTime);

	// Get emotion description
	UFUNCTION(BlueprintCallable, Category = "Emotion")
	FString GetEmotionDescription();

	// ============================================================================
	// CONTEXT AWARENESS
	// ============================================================================

	// Update current context
	UFUNCTION(BlueprintCallable, Category = "Context")
	void UpdateContext(const FAlexanderDialogueContext& NewContext);

	// Get current context
	UFUNCTION(BlueprintCallable, Category = "Context")
	FAlexanderDialogueContext GetCurrentContext() const { return CurrentContext; }

	// Analyze environment for context
	UFUNCTION(BlueprintCallable, Category = "Context")
	FAlexanderDialogueContext AnalyzeEnvironment();

	// ============================================================================
	// DIALOGUE HISTORY
	// ============================================================================

	// Add to dialogue history
	UFUNCTION(BlueprintCallable, Category = "History")
	void AddToHistory(const FString& Speaker, const FString& Message);

	// Get recent dialogue
	UFUNCTION(BlueprintCallable, Category = "History")
	TArray<FString> GetRecentDialogue(int32 Count = 10);

	// Clear dialogue history
	UFUNCTION(BlueprintCallable, Category = "History")
	void ClearHistory();

	// ============================================================================
	// ADVANCED FEATURES
	// ============================================================================

	// Generate gossip about other NPCs
	UFUNCTION(BlueprintCallable, Category = "Social")
	FString GenerateGossip(const FString& TargetNPC);

	// Generate quest based on conversation
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString GenerateQuestDialogue(const FAlexanderDialogueContext& Context);

	// Handle trade negotiations
	UFUNCTION(BlueprintCallable, Category = "Trade")
	FString HandleTradeNegotiation(const FString& Item, float OfferedPrice);

	// Generate combat taunts/banter
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FString GenerateCombatDialogue(bool bIsTaunting);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// LLM API endpoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	FString LLMEndpoint = TEXT("http://localhost:8000/api/dialogue");

	// API key for LLM service
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	FString APIKey = TEXT("");

	// Maximum memory fragments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 MaxMemoryFragments = 1000;

	// Memory decay rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float MemoryDecayRate = 0.001f;

	// Personality learning rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
	float PersonalityLearningRate = 0.1f;

	// Enable debug mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugMode = false;

	// ===== Dialogue Content Database =====
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UDialogueContentDatabase* DialogueDatabase = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FName NPCPersonalityType = FName("Neutral");

protected:
	// Internal systems
	UPROPERTY()
	UNPCBrain* NPCBrain;

	UPROPERTY()
	UMemorySystem* MemorySystem;

	UPROPERTY()
	UPersonalityProfile* PersonalityProfile;

	// Current state
	UPROPERTY()
	EEmotionState CurrentEmotion = EEmotionState::Neutral;

	UPROPERTY()
	FAlexanderDialogueContext CurrentContext;

	UPROPERTY()
	TArray<FString> DialogueHistory;

	// Performance tracking
	float LastDialogueProcessTime = 0.0f;
	int32 DialogueProcessCount = 0;

private:
	// Helper functions
	FString BuildLLMPrompt(const FString& PlayerInput, const FAlexanderDialogueContext& Context);
	FString ParseLLMResponse(const FString& RawResponse);
	void UpdateInternalState(float DeltaTime);
	bool IsMemoryRelevant(const FMemoryFragment& Memory, const FString& Query);
	float CalculateEmotionalWeight(const FMemoryFragment& Memory);
	
	// Response generation helpers
	FString GenerateGreeting(const FAlexanderDialogueContext& Context);
	FString GenerateQuestionResponse(const FString& Question, const FAlexanderDialogueContext& Context);
	FString GenerateFarewell(const FAlexanderDialogueContext& Context);
	FString GenerateGenericResponse(const FString& Input, const FAlexanderDialogueContext& Context);
};
