// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIDialogueSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "MemorySystem.h"
#include "PersonalityProfile.h"
#include "NPCBrain.h"

/**
 * Test suite for AI Dialogue System
 * Validates memory, personality, and dialogue generation
 */

class AIDialogueSystemTest
{
public:
    static bool RunAllTests()
    {
        bool AllTestsPassed = true;
        
        AllTestsPassed &= TestMemorySystem();
        AllTestsPassed &= TestPersonalityProfile();
        AllTestsPassed &= TestDialogueProcessing();
        AllTestsPassed &= TestEmotionalSystem();
        
        return AllTestsPassed;
    }

private:
    static bool TestMemorySystem()
    {
        // Test memory creation and storage
        UMemorySystem* MemorySystem = NewObject<UMemorySystem>();
        
        FMemoryFragment TestMemory;
        TestMemory.MemoryID = TEXT("test_001");
        TestMemory.MemoryType = EMemoryType::Conversation;
        TestMemory.Content = TEXT("Test conversation content");
        TestMemory.Importance = 0.8f;
        TestMemory.Timestamp = FDateTime::Now();
        
        MemorySystem->Memories.Add(TestMemory);
        
        // Test memory retrieval
        TArray<FMemoryFragment> RetrievedMemories = MemorySystem->SearchMemoriesByType(EMemoryType::Conversation);
        
        bool TestPassed = RetrievedMemories.Num() > 0 && 
                          RetrievedMemories[0].MemoryID == TEXT("test_001");
        
        if (TestPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Memory System Test Passed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Memory System Test Failed"));
        }
        
        return TestPassed;
    }

    static bool TestPersonalityProfile()
    {
        // Test personality initialization
        UPersonalityProfile* Profile = NewObject<UPersonalityProfile>();
        Profile->InitializeRandomPersonality();
        
        // Test trait setting and getting
        Profile->SetTrait(TEXT("test_trait"), 0.75f);
        float TraitValue = Profile->GetTraitValue(TEXT("test_trait"));
        
        // Test personality type calculation
        FString PersonalityType = Profile->GetPersonalityType();
        
        bool TestPassed = FMath::Abs(TraitValue - 0.75f) < 0.01f && 
                          !PersonalityType.IsEmpty();
        
        if (TestPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Personality Profile Test Passed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Personality Profile Test Failed"));
        }
        
        return TestPassed;
    }

    static bool TestDialogueProcessing()
    {
        // Test dialogue intent parsing
        UAIDialogueSystem* DialogueSystem = NewObject<UAIDialogueSystem>();
        
        // Test greeting intent
        EDialogueIntent GreetingIntent = DialogueSystem->ParsePlayerIntent(TEXT("Hello there!"));
        
        // Test question intent
        EDialogueIntent QuestionIntent = DialogueSystem->ParsePlayerIntent(TEXT("What time is it?"));
        
        // Test trade intent
        EDialogueIntent TradeIntent = DialogueSystem->ParsePlayerIntent(TEXT("I want to buy something"));
        
        bool TestPassed = GreetingIntent == EDialogueIntent::Greeting &&
                          QuestionIntent == EDialogueIntent::Question &&
                          TradeIntent == EDialogueIntent::Trade;
        
        if (TestPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Dialogue Processing Test Passed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Dialogue Processing Test Failed"));
        }
        
        return TestPassed;
    }

    static bool TestEmotionalSystem()
    {
        // Test emotion setting and description
        UAIDialogueSystem* DialogueSystem = NewObject<UAIDialogueSystem>();
        
        // Test setting emotion
        DialogueSystem->SetEmotion(EEmotionState::Happy, 0.8f);
        EEmotionState CurrentEmotion = DialogueSystem->GetCurrentEmotion();
        
        // Test emotion description
        FString EmotionDesc = DialogueSystem->GetEmotionDescription();
        
        bool TestPassed = CurrentEmotion == EEmotionState::Happy &&
                          EmotionDesc == TEXT("happy");
        
        if (TestPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Emotional System Test Passed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Emotional System Test Failed"));
        }
        
        return TestPassed;
    }
};