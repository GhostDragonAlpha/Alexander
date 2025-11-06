#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TimelineComponent.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundCue.h"
#include "AdvancedAIBehaviors.generated.h"

class ACharacter;
class UBehaviorTree;
class UBlackboardData;
class UAnimationMontage;
class USoundCue;
class AAIController;

// AI Behavior States
UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle,
    Patrol,
    Work,
    Social,
    Combat,
    Flee,
    Interact,
    Sleep,
    Eat,
    Emergency,
    Custom
};

// AI Personality Traits
USTRUCT(BlueprintType)
struct FAIPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggressiveness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Bravery = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorkEthic = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Loyalty = 0.5f;
};

// AI Daily Schedule
USTRUCT(BlueprintType)
struct FAIDailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WakeUpHour = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WorkStartHour = 9;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LunchHour = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WorkEndHour = 17;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinnerHour = 19;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SleepHour = 23;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> WorkLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SocialLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RestLocations;
};

// AI Group Behavior
USTRUCT(BlueprintType)
struct FAIGroupBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GroupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFollowLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* LeaderActor;
};

// AI Memory Entry
USTRUCT(BlueprintType)
struct FAIMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RelatedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> AdditionalData;
};

// AI Relationship Data
USTRUCT(BlueprintType)
struct FAIRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Affection = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Respect = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Trust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime = 0.0f;
};

// AI Emergency Response
USTRUCT(BlueprintType)
struct FAIEmergencyResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EmergencyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResponsePriority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SafeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresHelp = false;
};

// AI Task Definition
USTRUCT(BlueprintType)
struct FAITaskDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TaskType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> TaskParameters;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIStateChanged, AActor*, AIActor, EAIState, OldState, EAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIEmotionChanged, AActor*, AIActor, FString, Emotion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIRelationshipChanged, AActor*, AIActor, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIEmergencyTriggered, AActor*, AIActor, const FString&, EmergencyType);

/**
 * Advanced AI Behavior System
 * Provides sophisticated AI behaviors including daily routines, group behaviors, 
 * personality-driven actions, and dynamic responses to world events.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI))
class ALEXANDER_API UAdvancedAIBehaviors : public UObject
{
    GENERATED_BODY()

public:
    UAdvancedAIBehaviors();

    // Core AI Management
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void InitializeAI(AActor* AIActor, UBehaviorTree* BehaviorTree = nullptr);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void ShutdownAI(AActor* AIActor);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void UpdateAI(float DeltaTime);

    // State Management
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void SetAIState(AActor* AIActor, EAIState NewState);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    EAIState GetAIState(AActor* AIActor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    bool CanTransitionToState(AActor* AIActor, EAIState TargetState);

    // Personality System
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void SetPersonality(AActor* AIActor, const FAIPersonality& Personality);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FAIPersonality GetPersonality(AActor* AIActor) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    float GetPersonalityTrait(AActor* AIActor, const FString& TraitName) const;

    // Daily Schedule System
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void SetDailySchedule(AActor* AIActor, const FAIDailySchedule& Schedule);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FAIDailySchedule GetDailySchedule(AActor* AIActor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void UpdateScheduleBasedOnTime(AActor* AIActor, float CurrentHour);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FString GetCurrentScheduledActivity(AActor* AIActor, float CurrentHour) const;

    // Group Behaviors
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void CreateAIGroup(const FString& GroupName, const TArray<AActor*>& Members);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void AddToAIGroup(const FString& GroupName, AActor* AIActor);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void RemoveFromAIGroup(const FString& GroupName, AActor* AIActor);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void UpdateGroupBehaviors(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    TArray<AActor*> GetGroupMembers(const FString& GroupName) const;

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void AddMemory(AActor* AIActor, const FAIMemoryEntry& Memory);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    TArray<FAIMemoryEntry> GetMemories(AActor* AIActor, const FString& MemoryType = "") const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void ForgetOldMemories(AActor* AIActor, float MaxAge);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FAIMemoryEntry GetMostImportantMemory(AActor* AIActor) const;

    // Relationship System
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void UpdateRelationship(AActor* AIActor, AActor* TargetActor, float AffectionDelta, float RespectDelta);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FAIRelationship GetRelationship(AActor* AIActor, AActor* TargetActor) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    TArray<AActor*> GetFriends(AActor* AIActor, float Threshold = 0.5f) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    TArray<AActor*> GetEnemies(AActor* AIActor, float Threshold = -0.5f) const;

    // Emergency Response
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void TriggerEmergency(AActor* AIActor, const FAIEmergencyResponse& Emergency);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void HandleGlobalEmergency(const FString& EmergencyType, FVector EmergencyLocation);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    bool IsInEmergency(AActor* AIActor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void ResolveEmergency(AActor* AIActor);

    // Task System
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void AssignTask(AActor* AIActor, const FAITaskDefinition& Task);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    FAITaskDefinition GetCurrentTask(AActor* AIActor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void CompleteCurrentTask(AActor* AIActor);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    bool HasActiveTask(AActor* AIActor) const;

    // Social Behaviors
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void InitiateSocialInteraction(AActor* AIActor, AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void RespondToSocialInteraction(AActor* AIActor, AActor* InitiatorActor);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    TArray<AActor*> FindNearbyActors(AActor* AIActor, float Radius) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    AActor* FindBestSocialTarget(AActor* AIActor) const;

    // Combat Behaviors
    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void EnterCombat(AActor* AIActor, AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void ExitCombat(AActor* AIActor);

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    bool ShouldFlee(AActor* AIActor, AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void CallForHelp(AActor* AIActor, AActor* ThreatActor);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    float GetDistanceTo(AActor* AIActor, AActor* TargetActor) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    bool CanSee(AActor* AIActor, AActor* TargetActor) const;

    UFUNCTION(BlueprintPure, Category = "AI Behaviors")
    bool CanHear(AActor* AIActor, FVector SoundLocation, float SoundVolume) const;

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void PlayEmote(AActor* AIActor, const FString& EmoteName);

    UFUNCTION(BlueprintCallable, Category = "AI Behaviors")
    void PlayVoiceLine(AActor* AIActor, USoundCue* VoiceCue);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "AI Behaviors")
    FOnAIStateChanged OnAIStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI Behaviors")
    FOnAIEmotionChanged OnAIEmotionChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI Behaviors")
    FOnAIRelationshipChanged OnAIRelationshipChanged;

    UPROPERTY(BlueprintAssignable, Category = "AI Behaviors")
    FOnAIEmergencyTriggered OnAIEmergencyTriggered;

protected:
    // AI Data Storage
    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, EAIState> AIStates;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, FAIPersonality> AIPersonalities;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, FAIDailySchedule> AISchedules;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<FString, FAIGroupBehavior> AIGroups;

    // UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors") // Removed - UHT doesn't support nested containers in TMap
    TMap<AActor*, TArray<FAIMemoryEntry>> AIMemories;

    // UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors") // Removed - UHT doesn't support nested containers in TMap
    TMap<AActor*, TMap<AActor*, FAIRelationship>> AIRelationships;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, FAIEmergencyResponse> AIEmergencies;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, FAITaskDefinition> AITasks;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behaviors")
    TMap<AActor*, AAIController*> AIControllers;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float MemoryDecayTime = 3600.0f; // 1 hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float RelationshipDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float EmergencyPanicThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float SocialInteractionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float CombatFleeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behaviors")
    float GroupUpdateInterval = 0.5f;

private:
    float LastGroupUpdateTime;
    TMap<AActor*, float> StateChangeTimers;

    void UpdateIndividualAI(AActor* AIActor, float DeltaTime);
    void ProcessPersonalityDrivenBehavior(AActor* AIActor, float DeltaTime);
    void ProcessScheduledBehavior(AActor* AIActor, float DeltaTime);
    void ProcessEmergencyBehavior(AActor* AIActor, float DeltaTime);
    void ProcessTaskBehavior(AActor* AIActor, float DeltaTime);
    void ProcessSocialBehavior(AActor* AIActor, float DeltaTime);

    EAIState DetermineBestState(AActor* AIActor);
    bool ShouldChangeState(AActor* AIActor, EAIState PotentialState);
    void ExecuteStateBehavior(AActor* AIActor, EAIState State);

    FVector CalculateGroupCohesion(const FAIGroupBehavior& Group, AActor* AIActor);
    FVector CalculateGroupSeparation(const FAIGroupBehavior& Group, AActor* AIActor);
    FVector CalculateGroupAlignment(const FAIGroupBehavior& Group, AActor* AIActor);

    void DecayMemories(AActor* AIActor, float DeltaTime);
    void DecayRelationships(AActor* AIActor, float DeltaTime);
    void UpdateEmotionalState(AActor* AIActor);
};