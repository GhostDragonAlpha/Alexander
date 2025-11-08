// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIDialogueSystem.h"
#include "DialogueContentDatabase.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Http.h"
#include "Components/InputComponent.h"          // For UInputComponent
#include "GameFramework/PlayerInput.h"          // For player input handling

UAIDialogueSystem::UAIDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // 2 Hz for emotional state updates

    // Default configuration
    MaxMemoryFragments = 1000;
    MemoryDecayRate = 0.001f;
    PersonalityLearningRate = 0.1f;
    bDebugMode = false;

    CurrentEmotion = EEmotionState::Neutral;
}

void UAIDialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize personality
    InitializePersonality();

    // Initialize context
    CurrentContext = AnalyzeEnvironment();
}

void UAIDialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update emotional state
    UpdateEmotion(DeltaTime);

    // Update internal state
    UpdateInternalState(DeltaTime);
}

// ============================================================================
// DIALOGUE PROCESSING
// ============================================================================

FString UAIDialogueSystem::ProcessDialogue(const FString& PlayerInput, const FAlexanderDialogueContext& Context)
{
    float StartTime = FPlatformTime::Seconds();

    // Update context
    UpdateContext(Context);

    // Parse player intent
    EDialogueIntent Intent = ParsePlayerIntent(PlayerInput);
    CurrentContext.PlayerIntent = Intent;
    CurrentContext.PlayerStatement = PlayerInput;

    // Get relevant memories for context
    FString MemoryContext = GetMemoryContext(PlayerInput);

    // Get emotional response
    EEmotionState EmotionalResponse = GetEmotionalResponse(Context);
    SetEmotion(EmotionalResponse, 0.8f);

    // Generate LLM response
    FString Response = GenerateLLMResponse(PlayerInput, Context);

    // Store conversation in memory
    FMemoryFragment ConversationMemory;
    ConversationMemory.MemoryID = FGuid::NewGuid().ToString();
    ConversationMemory.MemoryType = EMemoryType::Conversation;
    ConversationMemory.Content = FString::Printf(TEXT("Player said: '%s'. I responded: '%s'"), *PlayerInput, *Response);
    ConversationMemory.Timestamp = FDateTime::Now();
    ConversationMemory.Importance = 0.5f;
    ConversationMemory.EmotionalWeight = CalculateEmotionalWeight(ConversationMemory);
    StoreMemory(ConversationMemory);

    // Add to dialogue history
    AddToHistory(TEXT("Player"), PlayerInput);
    AddToHistory(TEXT("NPC"), Response);

    // Track performance
    LastDialogueProcessTime = FPlatformTime::Seconds() - StartTime;
    DialogueProcessCount++;

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue processed in %.2f ms"), LastDialogueProcessTime * 1000.0f);
    }

    return Response;
}

FString UAIDialogueSystem::GenerateLLMResponse(const FString& Prompt, const FAlexanderDialogueContext& Context)
{
    // Build comprehensive prompt for LLM
    FString FullPrompt = BuildLLMPrompt(Prompt, Context);

    // In a real implementation, this would make an HTTP request to LLM API
    // For now, we'll generate rule-based responses

    FString Response;

    // Get personality description for context
    FString PersonalityDesc = GetPersonalityDescription();
    FString EmotionDesc = GetEmotionDescription();

    // Generate contextual response based on intent
    switch (Context.PlayerIntent)
    {
    case EDialogueIntent::Greeting:
        Response = GenerateGreeting(Context);
        break;

    case EDialogueIntent::Question:
        Response = GenerateQuestionResponse(Prompt, Context);
        break;

    case EDialogueIntent::Trade:
        Response = HandleTradeNegotiation(TEXT("GenericItem"), 100.0f);
        break;

    case EDialogueIntent::Combat:
        Response = GenerateCombatDialogue(true);
        break;

    case EDialogueIntent::Gossip:
        Response = GenerateGossip(TEXT("RandomNPC"));
        break;

    case EDialogueIntent::Farewell:
        Response = GenerateFarewell(Context);
        break;

    default:
        Response = GenerateGenericResponse(Prompt, Context);
        break;
    }

    return Response;
}

EDialogueIntent UAIDialogueSystem::ParsePlayerIntent(const FString& PlayerInput)
{
    FString LowerInput = PlayerInput.ToLower();

    // Greeting patterns
    if (LowerInput.Contains(TEXT("hello")) || LowerInput.Contains(TEXT("hi ")) || 
        LowerInput.Contains(TEXT("greetings")) || LowerInput.Contains(TEXT("hey")))
    {
        return EDialogueIntent::Greeting;
    }

    // Question patterns
    if (LowerInput.Contains(TEXT("?")) || LowerInput.Contains(TEXT("what")) || 
        LowerInput.Contains(TEXT("where")) || LowerInput.Contains(TEXT("when")) ||
        LowerInput.Contains(TEXT("why")) || LowerInput.Contains(TEXT("how")) ||
        LowerInput.Contains(TEXT("who")))
    {
        return EDialogueIntent::Question;
    }

    // Trade patterns
    if (LowerInput.Contains(TEXT("buy")) || LowerInput.Contains(TEXT("sell")) || 
        LowerInput.Contains(TEXT("trade")) || LowerInput.Contains(TEXT("price")))
    {
        return EDialogueIntent::Trade;
    }

    // Combat patterns
    if (LowerInput.Contains(TEXT("fight")) || LowerInput.Contains(TEXT("attack")) || 
        LowerInput.Contains(TEXT("kill")) || LowerInput.Contains(TEXT("die")))
    {
        return EDialogueIntent::Combat;
    }

    // Gossip patterns
    if (LowerInput.Contains(TEXT("heard")) || LowerInput.Contains(TEXT("rumor")) || 
        LowerInput.Contains(TEXT("gossip")) || LowerInput.Contains(TEXT("tell me about")))
    {
        return EDialogueIntent::Gossip;
    }

    // Farewell patterns
    if (LowerInput.Contains(TEXT("goodbye")) || LowerInput.Contains(TEXT("bye")) || 
        LowerInput.Contains(TEXT("farewell")) || LowerInput.Contains(TEXT("see you")))
    {
        return EDialogueIntent::Farewell;
    }

    // Request patterns
    if (LowerInput.Contains(TEXT("please")) || LowerInput.Contains(TEXT("help")) || 
        LowerInput.Contains(TEXT("need")) || LowerInput.Contains(TEXT("quest")))
    {
        return EDialogueIntent::Request;
    }

    // Default to statement
    return EDialogueIntent::Statement;
}

EEmotionState UAIDialogueSystem::GetEmotionalResponse(const FAlexanderDialogueContext& Context)
{
    // Base emotion on context
    if (Context.bIsInCombat)
    {
        return Context.bPlayerIsArmed ? EEmotionState::Fearful : EEmotionState::Angry;
    }

    // Relationship-based emotions
    if (Context.RelationshipLevel > 0.7f)
    {
        return EEmotionState::Happy;
    }
    else if (Context.RelationshipLevel < 0.3f)
    {
        return EEmotionState::Anxious;
    }

    // Intent-based emotions
    switch (Context.PlayerIntent)
    {
    case EDialogueIntent::Greeting:
        return EEmotionState::Happy;
    case EDialogueIntent::Combat:
        return EEmotionState::Angry;
    case EDialogueIntent::Threat:
        return EEmotionState::Fearful;
    case EDialogueIntent::Flirt:
        return Context.RelationshipLevel > 0.5f ? EEmotionState::Excited : EEmotionState::Surprised;
    default:
        return EEmotionState::Neutral;
    }
}

// ============================================================================
// MEMORY SYSTEM
// ============================================================================

void UAIDialogueSystem::StoreMemory(const FMemoryFragment& Memory)
{
    // Check if memory limit reached
    if (DialogueHistory.Num() >= MaxMemoryFragments)
    {
        ForgetMemories(0.2f); // Forget low-importance memories
    }

    // Store as JSON string in history for simplicity
    FString MemoryJSON = FString::Printf(
        TEXT("{\"id\":\"%s\",\"type\":\"%d\",\"content\":\"%s\",\"importance\":%.2f}"),
        *Memory.MemoryID,
        (int32)Memory.MemoryType,
        *Memory.Content,
        Memory.Importance
    );

    DialogueHistory.Add(MemoryJSON);
}

TArray<FMemoryFragment> UAIDialogueSystem::GetRelevantMemories(const FString& Query, int32 MaxResults)
{
    TArray<FMemoryFragment> RelevantMemories;

    // Simple relevance scoring based on keyword matching
    for (const FString& MemoryJSON : DialogueHistory)
    {
        FMemoryFragment Memory;
        // Parse basic memory data (simplified)
        if (MemoryJSON.Contains(Query))
        {
            Memory.Content = MemoryJSON;
            Memory.Importance = 0.5f;
            RelevantMemories.Add(Memory);
        }

        if (RelevantMemories.Num() >= MaxResults)
        {
            break;
        }
    }

    return RelevantMemories;
}

void UAIDialogueSystem::UpdateMemoryImportance(const FString& MemoryID, float NewImportance)
{
    // Update importance in memory storage
    // Implementation would search and update the memory fragment
}

void UAIDialogueSystem::ForgetMemories(float Threshold)
{
    // Remove low-importance memories
    int32 InitialCount = DialogueHistory.Num();
    
    // Keep only recent and important memories
    if (DialogueHistory.Num() > MaxMemoryFragments * 0.8f)
    {
        int32 TargetSize = MaxMemoryFragments / 2;
        int32 RemoveCount = DialogueHistory.Num() - TargetSize;
        
        // Remove oldest memories first
        for (int32 i = 0; i < RemoveCount; ++i)
        {
            if (DialogueHistory.Num() > 0)
            {
                DialogueHistory.RemoveAt(0);
            }
        }
    }

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Forgot %d memories (Threshold: %.2f)"), 
               InitialCount - DialogueHistory.Num(), Threshold);
    }
}

FString UAIDialogueSystem::GetMemoryContext(const FString& Query)
{
    TArray<FMemoryFragment> Memories = GetRelevantMemories(Query, 5);

    FString Context = TEXT("Relevant memories:\n");
    for (const FMemoryFragment& Memory : Memories)
    {
        Context += FString::Printf(TEXT("- %s\n"), *Memory.Content);
    }

    return Context;
}

// ============================================================================
// PERSONALITY SYSTEM
// ============================================================================

void UAIDialogueSystem::InitializePersonality()
{
    // Initialize with random personality traits
    // In a full implementation, these would be loaded from configuration
}

float UAIDialogueSystem::GetPersonalityTrait(const FString& TraitName)
{
    // Return personality trait value (0.0-1.0)
    // This would lookup from PersonalityProfile
    return 0.5f;
}

void UAIDialogueSystem::UpdatePersonality(const FString& TraitName, float Delta)
{
    // Update personality trait based on experiences
    float CurrentValue = GetPersonalityTrait(TraitName);
    float NewValue = FMath::Clamp(CurrentValue + (Delta * PersonalityLearningRate), 0.0f, 1.0f);

    // Store updated value
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Personality trait '%s' updated: %.2f -> %.2f"), 
               *TraitName, CurrentValue, NewValue);
    }
}

FString UAIDialogueSystem::GetPersonalityDescription()
{
    // Generate personality description based on traits
    float Friendliness = GetPersonalityTrait(TEXT("Friendliness"));
    float Honesty = GetPersonalityTrait(TEXT("Honesty"));
    float Bravery = GetPersonalityTrait(TEXT("Bravery"));

    FString Description;

    if (Friendliness > 0.7f)
        Description += TEXT("friendly and outgoing, ");
    else if (Friendliness < 0.3f)
        Description += TEXT("reserved and cautious, ");

    if (Honesty > 0.7f)
        Description += TEXT("honest and straightforward, ");
    else if (Honesty < 0.3f)
        Description += TEXT("cunning and deceptive, ");

    if (Bravery > 0.7f)
        Description += TEXT("brave and confident");
    else if (Bravery < 0.3f)
        Description += TEXT("fearful and hesitant");

    return Description;
}

// ============================================================================
// RELATIONSHIP SYSTEM
// ============================================================================

float UAIDialogueSystem::GetRelationshipLevel(const FString& ActorID)
{
    // Return relationship level with specified actor
    return CurrentContext.RelationshipLevel;
}

void UAIDialogueSystem::UpdateRelationship(const FString& ActorID, float Delta)
{
    CurrentContext.RelationshipLevel = FMath::Clamp(
        CurrentContext.RelationshipLevel + Delta, 
        -1.0f, 
        1.0f
    );

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Relationship with %s: %.2f (Delta: %.2f)"), 
               *ActorID, CurrentContext.RelationshipLevel, Delta);
    }
}

FString UAIDialogueSystem::GetRelationshipStatus(const FString& ActorID)
{
    float Level = GetRelationshipLevel(ActorID);

    if (Level > 0.8f) return TEXT("Best Friend");
    if (Level > 0.6f) return TEXT("Good Friend");
    if (Level > 0.4f) return TEXT("Friend");
    if (Level > 0.2f) return TEXT("Acquaintance");
    if (Level > -0.2f) return TEXT("Neutral");
    if (Level > -0.4f) return TEXT("Disliked");
    if (Level > -0.6f) return TEXT("Enemy");
    return TEXT("Hated");
}

// ============================================================================
// EMOTIONAL SYSTEM
// ============================================================================

void UAIDialogueSystem::SetEmotion(EEmotionState NewEmotion, float Intensity)
{
    CurrentEmotion = NewEmotion;

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("Emotion changed to: %s (Intensity: %.2f)"), 
               *GetEmotionDescription(), Intensity);
    }
}

void UAIDialogueSystem::UpdateEmotion(float DeltaTime)
{
    // Emotions decay back to neutral over time
    // This would implement emotional state transitions
}

FString UAIDialogueSystem::GetEmotionDescription()
{
    switch (CurrentEmotion)
    {
    case EEmotionState::Happy: return TEXT("Happy");
    case EEmotionState::Sad: return TEXT("Sad");
    case EEmotionState::Angry: return TEXT("Angry");
    case EEmotionState::Fearful: return TEXT("Fearful");
    case EEmotionState::Excited: return TEXT("Excited");
    case EEmotionState::Bored: return TEXT("Bored");
    case EEmotionState::Surprised: return TEXT("Surprised");
    case EEmotionState::Disgusted: return TEXT("Disgusted");
    case EEmotionState::Confused: return TEXT("Confused");
    case EEmotionState::Anxious: return TEXT("Anxious");
    default: return TEXT("Neutral");
    }
}

// ============================================================================
// CONTEXT AWARENESS
// ============================================================================

void UAIDialogueSystem::UpdateContext(const FAlexanderDialogueContext& NewContext)
{
    CurrentContext = NewContext;
}

FAlexanderDialogueContext UAIDialogueSystem::AnalyzeEnvironment()
{
    FAlexanderDialogueContext Context;

    if (AActor* Owner = GetOwner())
    {
        Context.PlayerLocation = Owner->GetActorLocation();
        Context.PlayerDirection = Owner->GetActorForwardVector();
    }

    // Analyze time of day, nearby actors, etc.
    // This would integrate with game systems

    return Context;
}

// ============================================================================
// DIALOGUE HISTORY
// ============================================================================

void UAIDialogueSystem::AddToHistory(const FString& Speaker, const FString& Message)
{
    FString Entry = FString::Printf(TEXT("[%s]: %s"), *Speaker, *Message);
    DialogueHistory.Add(Entry);

    // Keep history manageable
    if (DialogueHistory.Num() > 100)
    {
        DialogueHistory.RemoveAt(0);
    }
}

TArray<FString> UAIDialogueSystem::GetRecentDialogue(int32 Count)
{
    TArray<FString> Recent;
    int32 StartIndex = FMath::Max(0, DialogueHistory.Num() - Count);

    for (int32 i = StartIndex; i < DialogueHistory.Num(); ++i)
    {
        Recent.Add(DialogueHistory[i]);
    }

    return Recent;
}

void UAIDialogueSystem::ClearHistory()
{
    DialogueHistory.Empty();
}

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

FString UAIDialogueSystem::GenerateGossip(const FString& TargetNPC)
{
    TArray<FString> GossipTemplates = {
        TEXT("I heard that %s has been acting strange lately..."),
        TEXT("Did you know %s was involved in that incident?"),
        TEXT("Between you and me, %s isn't what they seem."),
        TEXT("Word around here is that %s is up to something."),
        TEXT("I don't usually gossip, but %s... well, you didn't hear it from me.")
    };

    int32 Index = FMath::RandRange(0, GossipTemplates.Num() - 1);
    return FString::Printf(TEXT("%s"), *GossipTemplates[Index]);
}

FString UAIDialogueSystem::GenerateQuestDialogue(const FAlexanderDialogueContext& Context)
{
    return TEXT("I have a task that needs doing, if you're interested. It won't be easy, but the reward is worth it.");
}

FString UAIDialogueSystem::HandleTradeNegotiation(const FString& Item, float OfferedPrice)
{
    float Friendliness = GetPersonalityTrait(TEXT("Friendliness"));
    float Greed = GetPersonalityTrait(TEXT("Greed"));

    FString Response;

    if (Greed > 0.7f)
    {
        Response = FString::Printf(TEXT("That's not nearly enough for %s. I need at least %.0f more credits."), 
                                   *Item, OfferedPrice * 0.5f);
    }
    else if (Friendliness > 0.7f)
    {
        Response = FString::Printf(TEXT("For you? I can part with %s for %.0f credits."), 
                                   *Item, OfferedPrice * 0.8f);
    }
    else
    {
        Response = FString::Printf(TEXT("%.0f credits for %s. Take it or leave it."), 
                                   OfferedPrice, *Item);
    }

    return Response;
}

FString UAIDialogueSystem::GenerateCombatDialogue(bool bIsTaunting)
{
    if (bIsTaunting)
    {
        TArray<FString> Taunts = {
            TEXT("Is that the best you can do?"),
            TEXT("You'll have to do better than that!"),
            TEXT("I've fought worse in my sleep!"),
            TEXT("Come on, show me what you've got!"),
            TEXT("This is getting boring.")
        };

        return Taunts[FMath::RandRange(0, Taunts.Num() - 1)];
    }
    else
    {
        TArray<FString> Threats = {
            TEXT("You made a big mistake!"),
            TEXT("This ends now!"),
            TEXT("Prepare yourself!"),
            TEXT("You're going to regret this!"),
            TEXT("Time to finish this!")
        };

        return Threats[FMath::RandRange(0, Threats.Num() - 1)];
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FString UAIDialogueSystem::BuildLLMPrompt(const FString& PlayerInput, const FAlexanderDialogueContext& Context)
{
    FString Prompt = TEXT("System: You are an NPC in a space simulation game. ");
    Prompt += FString::Printf(TEXT("Personality: %s. "), *GetPersonalityDescription());
    Prompt += FString::Printf(TEXT("Current emotion: %s. "), *GetEmotionDescription());
    Prompt += FString::Printf(TEXT("Relationship with player: %s. "), 
                             *GetRelationshipStatus(TEXT("Player")));

    // Add context
    if (Context.bIsInCombat)
    {
        Prompt += TEXT("You are currently in combat. ");
    }

    // Add memory context
    Prompt += GetMemoryContext(PlayerInput);

    // Add player input
    Prompt += FString::Printf(TEXT("\nPlayer: %s\nNPC:"), *PlayerInput);

    return Prompt;
}

FString UAIDialogueSystem::ParseLLMResponse(const FString& RawResponse)
{
    // Clean up LLM response
    return RawResponse.TrimStartAndEnd();
}

void UAIDialogueSystem::UpdateInternalState(float DeltaTime)
{
    // Apply memory decay
    // Update personality based on experiences
    // Process emotional state changes
}

bool UAIDialogueSystem::IsMemoryRelevant(const FMemoryFragment& Memory, const FString& Query)
{
    // Check if memory is relevant to query
    return Memory.Content.Contains(Query);
}

float UAIDialogueSystem::CalculateEmotionalWeight(const FMemoryFragment& Memory)
{
    // Calculate emotional significance of memory
    float Weight = 0.0f;

    switch (Memory.MemoryType)
    {
    case EMemoryType::Emotion:
        Weight = 1.0f;
        break;
    case EMemoryType::Relationship:
        Weight = 0.8f;
        break;
    case EMemoryType::Conversation:
        Weight = 0.5f;
        break;
    default:
        Weight = 0.3f;
        break;
    }

    return Weight;
}

// Response generation helpers
FString UAIDialogueSystem::GenerateGreeting(const FAlexanderDialogueContext& Context)
{
    float Relationship = Context.RelationshipLevel;

    if (Relationship > 0.7f)
    {
        return TEXT("Hey there, friend! Great to see you again!");
    }
    else if (Relationship > 0.3f)
    {
        return TEXT("Hello. How can I help you?");
    }
    else if (Relationship < 0.0f)
    {
        return TEXT("What do you want?");
    }

    return TEXT("Greetings, traveler.");
}

FString UAIDialogueSystem::GenerateQuestionResponse(const FString& Question, const FAlexanderDialogueContext& Context)
{
    // Analyze question and provide contextual response
    return TEXT("That's an interesting question. Let me think about that...");
}

FString UAIDialogueSystem::GenerateFarewell(const FAlexanderDialogueContext& Context)
{
    float Relationship = Context.RelationshipLevel;

    if (Relationship > 0.7f)
    {
        return TEXT("Take care out there! Come back soon!");
    }
    else if (Relationship > 0.3f)
    {
        return TEXT("Safe travels.");
    }

    return TEXT("Goodbye.");
}

FString UAIDialogueSystem::GenerateGenericResponse(const FString& Input, const FAlexanderDialogueContext& Context)
{
    TArray<FString> Responses = {
        TEXT("I see what you mean."),
        TEXT("That's interesting."),
        TEXT("Tell me more."),
        TEXT("I understand."),
        TEXT("Go on...")
    };

    return Responses[FMath::RandRange(0, Responses.Num() - 1)];
}

// ===== Dialogue Database Integration =====

void UAIDialogueSystem::LoadDialogueDatabase(UDialogueContentDatabase* Database)
{
	if (!Database)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadDialogueDatabase: Invalid database"));
		return;
	}

	DialogueDatabase = Database;
	UE_LOG(LogTemp, Log, TEXT("Loaded dialogue database with %d entries"), 
		DialogueDatabase->GetTotalDialogueCount());
}

FText UAIDialogueSystem::GetContextualDialogue(
	const FName& ConversationContext, 
	const TArray<FName>& ActiveTags)
{
	if (!DialogueDatabase)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetContextualDialogue: No dialogue database loaded"));
		return FText::FromString(TEXT("..."));
	}

	// Build dialogue context
	FAIDialogueContext Context;
	Context.NPCPersonality = NPCPersonalityType;
	Context.ConversationContext = ConversationContext;
	Context.ActiveTags = ActiveTags;
	Context.PlayerReputation = 0; // Should be retrieved from game state
	Context.TimesSpoken = DialogueHistory.Num();
	Context.bFirstMeeting = (DialogueHistory.Num() == 0);

	// Determine category
	EDialogueCategory Category = EDialogueCategory::Social;
	if (ConversationContext == FName("Mission"))
	{
		Category = EDialogueCategory::Mission;
	}
	else if (ConversationContext == FName("Trading"))
	{
		Category = EDialogueCategory::Trading;
	}
	else if (ConversationContext == FName("Combat"))
	{
		Category = EDialogueCategory::Combat;
	}
	else if (ConversationContext == FName("Information"))
	{
		Category = EDialogueCategory::Information;
	}

	// Get random dialogue matching context
	FDialogueEntry Entry = DialogueDatabase->GetRandomDialogue(Context, Category);
	
	if (Entry.DialogueID != NAME_None)
	{
		UE_LOG(LogTemp, Log, TEXT("Selected dialogue: %s"), *Entry.DialogueID.ToString());
		return Entry.DialogueText;
	}

	// Fallback to generic response
	return FText::FromString(GenerateGenericResponse(TEXT(""), CurrentContext));
}

FText UAIDialogueSystem::GetGreetingDialogue()
{
	if (!DialogueDatabase)
	{
		return FText::FromString(TEXT("Hello."));
	}

	TArray<FDialogueEntry> Greetings = DialogueDatabase->GetGreetingsForPersonality(NPCPersonalityType);
	
	if (Greetings.Num() > 0)
	{
		// Build context for filtering
		FAIDialogueContext Context;
		Context.NPCPersonality = NPCPersonalityType;
		Context.ConversationContext = FName("Greeting");
		Context.bFirstMeeting = (DialogueHistory.Num() == 0);
		
		// Filter valid greetings
		TArray<FDialogueEntry> ValidGreetings;
		for (const FDialogueEntry& Entry : Greetings)
		{
			if (DialogueDatabase->IsDialogueValidForContext(Entry, Context))
			{
				ValidGreetings.Add(Entry);
			}
		}
		
		if (ValidGreetings.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, ValidGreetings.Num() - 1);
			return ValidGreetings[RandomIndex].DialogueText;
		}
	}

	// Fallback
	return FText::FromString(GenerateGreeting(CurrentContext));
}

FText UAIDialogueSystem::GetFarewellDialogue()
{
	if (!DialogueDatabase)
	{
		return FText::FromString(TEXT("Goodbye."));
	}

	TArray<FDialogueEntry> Farewells = DialogueDatabase->GetFarewellsForPersonality(NPCPersonalityType);
	
	if (Farewells.Num() > 0)
	{
		FAIDialogueContext Context;
		Context.NPCPersonality = NPCPersonalityType;
		Context.ConversationContext = FName("Farewell");
		
		// Filter valid farewells
		TArray<FDialogueEntry> ValidFarewells;
		for (const FDialogueEntry& Entry : Farewells)
		{
			if (DialogueDatabase->IsDialogueValidForContext(Entry, Context))
			{
				ValidFarewells.Add(Entry);
			}
		}
		
		if (ValidFarewells.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, ValidFarewells.Num() - 1);
			return ValidFarewells[RandomIndex].DialogueText;
		}
	}

	// Fallback
	return FText::FromString(GenerateFarewell(CurrentContext));
}
