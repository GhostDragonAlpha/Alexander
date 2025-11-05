#include "AdvancedAIBehaviors.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Animation/AnimInstance.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UAdvancedAIBehaviors::UAdvancedAIBehaviors()
{
    LastGroupUpdateTime = 0.0f;
}

void UAdvancedAIBehaviors::InitializeAI(AActor* AIActor, UBehaviorTree* BehaviorTree)
{
    if (!AIActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAdvancedAIBehaviors::InitializeAI - AIActor is null"));
        return;
    }

    // Get or create AI Controller
    AAIController* AIController = nullptr;
    ACharacter* AICharacter = Cast<ACharacter>(AIActor);
    
    if (AICharacter)
    {
        AIController = Cast<AAIController>(AICharacter->GetController());
        if (!AIController)
        {
            // Spawn AI controller if none exists
            AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
            if (AIController)
            {
                AIController->Possess(AICharacter);
            }
        }
    }

    if (AIController)
    {
        AIControllers.Add(AIActor, AIController);

        // Set up behavior tree if provided
        if (BehaviorTree)
        {
            AIController->RunBehaviorTree(BehaviorTree);
        }

        // Initialize default state
        AIStates.Add(AIActor, EAIState::Idle);

        // Initialize default personality if not set
        if (!AIPersonalities.Contains(AIActor))
        {
            FAIPersonality DefaultPersonality;
            AIPersonalities.Add(AIActor, DefaultPersonality);
        }

        // Initialize default schedule if not set
        if (!AISchedules.Contains(AIActor))
        {
            FAIDailySchedule DefaultSchedule;
            AISchedules.Add(AIActor, DefaultSchedule);
        }

        // Initialize empty memory and relationship maps
        if (!AIMemories.Contains(AIActor))
        {
            AIMemories.Add(AIActor, TArray<FAIMemoryEntry>());
        }

        if (!AIRelationships.Contains(AIActor))
        {
            AIRelationships.Add(AIActor, TMap<AActor*, FAIRelationship>());
        }

        UE_LOG(LogTemp, Log, TEXT("AI Initialized: %s"), *AIActor->GetName());
    }
}

void UAdvancedAIBehaviors::ShutdownAI(AActor* AIActor)
{
    if (!AIActor) return;

    // Remove from all systems
    AIStates.Remove(AIActor);
    AIPersonalities.Remove(AIActor);
    AISchedules.Remove(AIActor);
    AIMemories.Remove(AIActor);
    AIRelationships.Remove(AIActor);
    AIEmergencies.Remove(AIActor);
    AITasks.Remove(AIActor);
    StateChangeTimers.Remove(AIActor);

    // Remove from groups
    for (auto& GroupPair : AIGroups)
    {
        FAIGroupBehavior& Group = GroupPair.Value;
        Group.GroupMembers.Remove(AIActor);
    }

    // Clean up AI controller
    if (AIControllers.Contains(AIActor))
    {
        AAIController* AIController = AIControllers[AIActor];
        if (AIController)
        {
            AIController->StopMovement();
            AIController->ClearFocus(EAIFocusPriority::Default);
        }
        AIControllers.Remove(AIActor);
    }

    UE_LOG(LogTemp, Log, TEXT("AI Shutdown: %s"), *AIActor->GetName());
}

void UAdvancedAIBehaviors::UpdateAI(float DeltaTime)
{
    // Update all registered AI actors
    for (auto& StatePair : AIStates)
    {
        AActor* AIActor = StatePair.Key;
        if (AIActor && AIActor->IsValidLowLevel())
        {
            UpdateIndividualAI(AIActor, DeltaTime);
        }
    }

    // Update group behaviors periodically
    LastGroupUpdateTime += DeltaTime;
    if (LastGroupUpdateTime >= GroupUpdateInterval)
    {
        UpdateGroupBehaviors(LastGroupUpdateTime);
        LastGroupUpdateTime = 0.0f;
    }
}

void UAdvancedAIBehaviors::UpdateIndividualAI(AActor* AIActor, float DeltaTime)
{
    if (!AIActor) return;

    // Decay memories and relationships
    DecayMemories(AIActor, DeltaTime);
    DecayRelationships(AIActor, DeltaTime);

    // Process different behavior systems
    ProcessEmergencyBehavior(AIActor, DeltaTime);
    ProcessTaskBehavior(AIActor, DeltaTime);
    ProcessScheduledBehavior(AIActor, DeltaTime);
    ProcessPersonalityDrivenBehavior(AIActor, DeltaTime);
    ProcessSocialBehavior(AIActor, DeltaTime);

    // Update emotional state
    UpdateEmotionalState(AIActor);

    // Determine if state change is needed
    EAIState CurrentState = AIStates[AIActor];
    EAIState BestState = DetermineBestState(AIActor);
    
    if (ShouldChangeState(AIActor, BestState) && CurrentState != BestState)
    {
        SetAIState(AIActor, BestState);
    }

    // Execute current state behavior
    ExecuteStateBehavior(AIActor, AIStates[AIActor]);
}

void UAdvancedAIBehaviors::ProcessEmergencyBehavior(AActor* AIActor, float DeltaTime)
{
    if (!AIEmergencies.Contains(AIActor)) return;

    const FAIEmergencyResponse& Emergency = AIEmergencies[AIActor];
    
    // Handle panic behavior
    if (Emergency.PanicLevel > EmergencyPanicThreshold)
    {
        // Flee to safe location
        if (AIControllers.Contains(AIActor))
        {
            AAIController* AIController = AIControllers[AIActor];
            if (AIController)
            {
                AIController->MoveToLocation(Emergency.SafeLocation);
            }
        }
    }

    // Execute required actions
    for (const FString& Action : Emergency.RequiredActions)
    {
        if (Action == "CallForHelp")
        {
            CallForHelp(AIActor, nullptr);
        }
        else if (Action == "PlayPanicAnimation")
        {
            PlayEmote(AIActor, "Panic");
        }
    }
}

void UAdvancedAIBehaviors::ProcessTaskBehavior(AActor* AIActor, float DeltaTime)
{
    if (!AITasks.Contains(AIActor)) return;

    FAITaskDefinition& Task = AITasks[AIActor];
    
    // Update task duration
    Task.Duration -= DeltaTime;
    
    if (Task.Duration <= 0.0f)
    {
        CompleteCurrentTask(AIActor);
    }
    else
    {
        // Execute task behavior based on type
        if (Task.TaskType == "MoveToLocation" && AIControllers.Contains(AIActor))
        {
            AAIController* AIController = AIControllers[AIActor];
            if (AIController)
            {
                AIController->MoveToLocation(Task.TargetLocation);
            }
        }
        else if (Task.TaskType == "InteractWithActor" && Task.TargetActor)
        {
            float Distance = GetDistanceTo(AIActor, Task.TargetActor);
            if (Distance > SocialInteractionDistance)
            {
                if (AIControllers.Contains(AIActor))
                {
                    AAIController* AIController = AIControllers[AIActor];
                    if (AIController)
                    {
                        AIController->MoveToActor(Task.TargetActor, SocialInteractionDistance);
                    }
                }
            }
        }
    }
}

void UAdvancedAIBehaviors::ProcessScheduledBehavior(AActor* AIActor, float DeltaTime)
{
    if (!AISchedules.Contains(AIActor)) return;

    const FAIDailySchedule& Schedule = AISchedules[AIActor];
    
    // Get current game time (simplified - in real implementation would get from game world)
    float CurrentHour = GetWorld()->GetTimeSeconds() / 3600.0f;
    CurrentHour = FMath::Fmod(CurrentHour, 24.0f);
    
    UpdateScheduleBasedOnTime(AIActor, CurrentHour);
}

void UAdvancedAIBehaviors::ProcessPersonalityDrivenBehavior(AActor* AIActor, float DeltaTime)
{
    if (!AIPersonalities.Contains(AIActor)) return;

    const FAIPersonality& Personality = AIPersonalities[AIActor];
    
    // Personality-driven random behaviors
    float RandomChance = FMath::FRandRange(0.0f, 1.0f);
    
    if (RandomChance < Personality.Curiosity * 0.01f) // 1% chance per second * curiosity
    {
        // Look around randomly
        if (AIControllers.Contains(AIActor))
        {
            AAIController* AIController = AIControllers[AIActor];
            if (AIController)
            {
                FVector RandomLocation = AIActor->GetActorLocation() + 
                    FVector(FMath::FRandRange(-500, 500), FMath::FRandRange(-500, 500), 0);
                AIController->SetFocalPoint(RandomLocation);
            }
        }
    }
    
    if (RandomChance < Personality.Sociability * 0.005f) // 0.5% chance per second * sociability
    {
        // Initiate social interaction
        AActor* SocialTarget = FindBestSocialTarget(AIActor);
        if (SocialTarget)
        {
            InitiateSocialInteraction(AIActor, SocialTarget);
        }
    }
}

void UAdvancedAIBehaviors::ProcessSocialBehavior(AActor* AIActor, float DeltaTime)
{
    // Check for nearby actors to interact with
    TArray<AActor*> NearbyActors = FindNearbyActors(AIActor, SocialInteractionDistance);
    
    for (AActor* NearbyActor : NearbyActors)
    {
        if (NearbyActor != AIActor && AIStates.Contains(NearbyActor))
        {
            // Update relationship based on proximity
            UpdateRelationship(AIActor, NearbyActor, 0.01f, 0.01f); // Small positive change
        }
    }
}

void UAdvancedAIBehaviors::SetAIState(AActor* AIActor, EAIState NewState)
{
    if (!AIActor || !AIStates.Contains(AIActor)) return;

    EAIState OldState = AIStates[AIActor];
    AIStates[AIActor] = NewState;

    // Handle state transition logic
    AAIController* AIController = AIControllers.Contains(AIActor) ? AIControllers[AIActor] : nullptr;
    
    switch (NewState)
    {
        case EAIState::Idle:
            if (AIController)
            {
                AIController->StopMovement();
                AIController->ClearFocus(EAIFocusPriority::Default);
            }
            break;
            
        case EAIState::Combat:
            // Combat state handled by EnterCombat function
            break;
            
        case EAIState::Flee:
            // Flee state handled by emergency system
            break;
            
        case EAIState::Social:
            // Social state handled by social interaction system
            break;
            
        default:
            break;
    }

    // Broadcast state change
    OnAIStateChanged.Broadcast(AIActor, OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("AI State Changed: %s from %d to %d"), 
        *AIActor->GetName(), (int32)OldState, (int32)NewState);
}

EAIState UAdvancedAIBehaviors::GetAIState(AActor* AIActor) const
{
    if (!AIActor || !AIStates.Contains(AIActor)) return EAIState::Idle;
    return AIStates[AIActor];
}

bool UAdvancedAIBehaviors::CanTransitionToState(AActor* AIActor, EAIState TargetState)
{
    if (!AIActor || !AIStates.Contains(AIActor)) return false;

    EAIState CurrentState = AIStates[AIActor];
    
    // Emergency states can always be entered
    if (TargetState == EAIState::Emergency || TargetState == EAIState::Flee)
        return true;
    
    // Cannot transition from emergency to non-emergency unless resolved
    if (CurrentState == EAIState::Emergency && TargetState != EAIState::Emergency)
        return !IsInEmergency(AIActor);
    
    // Combat state restrictions
    if (CurrentState == EAIState::Combat && TargetState == EAIState::Sleep)
        return false;
    
    return true;
}

void UAdvancedAIBehaviors::SetPersonality(AActor* AIActor, const FAIPersonality& Personality)
{
    if (!AIActor) return;
    AIPersonalities.Add(AIActor, Personality);
}

FAIPersonality UAdvancedAIBehaviors::GetPersonality(AActor* AIActor) const
{
    if (!AIActor || !AIPersonalities.Contains(AIActor)) 
    {
        FAIPersonality DefaultPersonality;
        return DefaultPersonality;
    }
    return AIPersonalities[AIActor];
}

float UAdvancedAIBehaviors::GetPersonalityTrait(AActor* AIActor, const FString& TraitName) const
{
    if (!AIActor || !AIPersonalities.Contains(AIActor)) return 0.5f;

    const FAIPersonality& Personality = AIPersonalities[AIActor];
    
    if (TraitName == "Aggressiveness") return Personality.Aggressiveness;
    if (TraitName == "Sociability") return Personality.Sociability;
    if (TraitName == "Curiosity") return Personality.Curiosity;
    if (TraitName == "Bravery") return Personality.Bravery;
    if (TraitName == "WorkEthic") return Personality.WorkEthic;
    if (TraitName == "Loyalty") return Personality.Loyalty;
    
    return 0.5f;
}

void UAdvancedAIBehaviors::SetDailySchedule(AActor* AIActor, const FAIDailySchedule& Schedule)
{
    if (!AIActor) return;
    AISchedules.Add(AIActor, Schedule);
}

FAIDailySchedule UAdvancedAIBehaviors::GetDailySchedule(AActor* AIActor) const
{
    if (!AIActor || !AISchedules.Contains(AIActor)) 
    {
        FAIDailySchedule DefaultSchedule;
        return DefaultSchedule;
    }
    return AISchedules[AIActor];
}

void UAdvancedAIBehaviors::UpdateScheduleBasedOnTime(AActor* AIActor, float CurrentHour)
{
    if (!AIActor || !AISchedules.Contains(AIActor)) return;

    const FAIDailySchedule& Schedule = AISchedules[AIActor];
    FString CurrentActivity = GetCurrentScheduledActivity(AIActor, CurrentHour);
    
    // Set AI state based on scheduled activity
    if (CurrentActivity == "Work")
    {
        if (AIStates[AIActor] != EAIState::Work)
            SetAIState(AIActor, EAIState::Work);
    }
    else if (CurrentActivity == "Sleep")
    {
        if (AIStates[AIActor] != EAIState::Sleep)
            SetAIState(AIActor, EAIState::Sleep);
    }
    else if (CurrentActivity == "Social")
    {
        if (AIStates[AIActor] != EAIState::Social)
            SetAIState(AIActor, EAIState::Social);
    }
    else if (CurrentActivity == "Eat")
    {
        if (AIStates[AIActor] != EAIState::Eat)
            SetAIState(AIActor, EAIState::Eat);
    }
}

FString UAdvancedAIBehaviors::GetCurrentScheduledActivity(AActor* AIActor, float CurrentHour) const
{
    if (!AIActor || !AISchedules.Contains(AIActor)) return "Idle";

    const FAIDailySchedule& Schedule = AISchedules[AIActor];
    
    // Determine activity based on current hour
    if (CurrentHour >= Schedule.SleepHour || CurrentHour < Schedule.WakeUpHour)
        return "Sleep";
    else if (CurrentHour >= Schedule.WorkStartHour && CurrentHour < Schedule.LunchHour)
        return "Work";
    else if (CurrentHour >= Schedule.LunchHour && CurrentHour < Schedule.LunchHour + 1)
        return "Eat";
    else if (CurrentHour >= Schedule.LunchHour + 1 && CurrentHour < Schedule.WorkEndHour)
        return "Work";
    else if (CurrentHour >= Schedule.DinnerHour && CurrentHour < Schedule.DinnerHour + 1)
        return "Eat";
    else if (CurrentHour >= Schedule.WorkEndHour && CurrentHour < Schedule.DinnerHour)
        return "Social";
    else
        return "Social";
}

void UAdvancedAIBehaviors::CreateAIGroup(const FString& GroupName, const TArray<AActor*>& Members)
{
    FAIGroupBehavior NewGroup;
    NewGroup.GroupName = GroupName;
    NewGroup.GroupMembers = Members;
    
    // Set first member as leader if follow leader is enabled
    if (Members.Num() > 0)
    {
        NewGroup.LeaderActor = Members[0];
    }
    
    AIGroups.Add(GroupName, NewGroup);
    
    UE_LOG(LogTemp, Log, TEXT("Created AI Group: %s with %d members"), 
        *GroupName, Members.Num());
}

void UAdvancedAIBehaviors::AddToAIGroup(const FString& GroupName, AActor* AIActor)
{
    if (!AIActor || !AIGroups.Contains(GroupName)) return;

    FAIGroupBehavior& Group = AIGroups[GroupName];
    if (!Group.GroupMembers.Contains(AIActor))
    {
        Group.GroupMembers.Add(AIActor);
    }
}

void UAdvancedAIBehaviors::RemoveFromAIGroup(const FString& GroupName, AActor* AIActor)
{
    if (!AIActor || !AIGroups.Contains(GroupName)) return;

    FAIGroupBehavior& Group = AIGroups[GroupName];
    Group.GroupMembers.Remove(AIActor);
    
    // Update leader if necessary
    if (Group.LeaderActor == AIActor && Group.GroupMembers.Num() > 0)
    {
        Group.LeaderActor = Group.GroupMembers[0];
    }
}

void UAdvancedAIBehaviors::UpdateGroupBehaviors(float DeltaTime)
{
    for (auto& GroupPair : AIGroups)
    {
        FAIGroupBehavior& Group = GroupPair.Value;
        
        for (AActor* AIActor : Group.GroupMembers)
        {
            if (!AIActor || !AIControllers.Contains(AIActor)) continue;

            AAIController* AIController = AIControllers[AIActor];
            if (!AIController) continue;

            FVector CohesionForce = CalculateGroupCohesion(Group, AIActor);
            FVector SeparationForce = CalculateGroupSeparation(Group, AIActor);
            FVector AlignmentForce = CalculateGroupAlignment(Group, AIActor);
            
            FVector CombinedForce = CohesionForce + SeparationForce + AlignmentForce;
            
            if (CombinedForce.Size() > 0.0f)
            {
                FVector TargetLocation = AIActor->GetActorLocation() + CombinedForce;
                AIController->MoveToLocation(TargetLocation);
            }
        }
    }
}

TArray<AActor*> UAdvancedAIBehaviors::GetGroupMembers(const FString& GroupName) const
{
    if (!AIGroups.Contains(GroupName)) return TArray<AActor*>();
    return AIGroups[GroupName].GroupMembers;
}

void UAdvancedAIBehaviors::AddMemory(AActor* AIActor, const FAIMemoryEntry& Memory)
{
    if (!AIActor) return;

    TArray<FAIMemoryEntry>& Memories = AIMemories.FindOrAdd(AIActor);
    
    // Create memory with timestamp
    FAIMemoryEntry NewMemory = Memory;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    Memories.Add(NewMemory);
    
    // Limit memory count to prevent memory bloat
    if (Memories.Num() > 100)
    {
        Memories.RemoveAt(0);
    }
}

TArray<FAIMemoryEntry> UAdvancedAIBehaviors::GetMemories(AActor* AIActor, const FString& MemoryType) const
{
    if (!AIActor || !AIMemories.Contains(AIActor)) return TArray<FAIMemoryEntry>();

    const TArray<FAIMemoryEntry>& Memories = AIMemories[AIActor];
    
    if (MemoryType.IsEmpty())
        return Memories;
    
    TArray<FAIMemoryEntry> FilteredMemories;
    for (const FAIMemoryEntry& Memory : Memories)
    {
        if (Memory.MemoryType == MemoryType)
        {
            FilteredMemories.Add(Memory);
        }
    }
    
    return FilteredMemories;
}

void UAdvancedAIBehaviors::ForgetOldMemories(AActor* AIActor, float MaxAge)
{
    if (!AIActor || !AIMemories.Contains(AIActor)) return;

    TArray<FAIMemoryEntry>& Memories = AIMemories[AIActor];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - Memories[i].Timestamp > MaxAge)
        {
            Memories.RemoveAt(i);
        }
    }
}

FAIMemoryEntry UAdvancedAIBehaviors::GetMostImportantMemory(AActor* AIActor) const
{
    if (!AIActor || !AIMemories.Contains(AIActor)) 
    {
        FAIMemoryEntry EmptyMemory;
        return EmptyMemory;
    }

    const TArray<FAIMemoryEntry>& Memories = AIMemories[AIActor];
    FAIMemoryEntry MostImportant;
    float HighestImportance = 0.0f;
    
    for (const FAIMemoryEntry& Memory : Memories)
    {
        if (Memory.Importance > HighestImportance)
        {
            HighestImportance = Memory.Importance;
            MostImportant = Memory;
        }
    }
    
    return MostImportant;
}

void UAdvancedAIBehaviors::UpdateRelationship(AActor* AIActor, AActor* TargetActor, float AffectionDelta, float RespectDelta)
{
    if (!AIActor || !TargetActor) return;

    TMap<AActor*, FAIRelationship>& Relationships = AIRelationships.FindOrAdd(AIActor);
    FAIRelationship& Relationship = Relationships.FindOrAdd(TargetActor);
    
    Relationship.TargetActor = TargetActor;
    Relationship.Affection = FMath::Clamp(Relationship.Affection + AffectionDelta, -1.0f, 1.0f);
    Relationship.Respect = FMath::Clamp(Relationship.Respect + RespectDelta, -1.0f, 1.0f);
    Relationship.InteractionCount++;
    Relationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    OnAIRelationshipChanged.Broadcast(AIActor, TargetActor);
}

FAIRelationship UAdvancedAIBehaviors::GetRelationship(AActor* AIActor, AActor* TargetActor) const
{
    if (!AIActor || !TargetActor || !AIRelationships.Contains(AIActor)) 
    {
        FAIRelationship EmptyRelationship;
        return EmptyRelationship;
    }

    const TMap<AActor*, FAIRelationship>& Relationships = AIRelationships[AIActor];
    if (Relationships.Contains(TargetActor))
    {
        return Relationships[TargetActor];
    }
    
    FAIRelationship DefaultRelationship;
    return DefaultRelationship;
}

TArray<AActor*> UAdvancedAIBehaviors::GetFriends(AActor* AIActor, float Threshold) const
{
    TArray<AActor*> Friends;
    
    if (!AIActor || !AIRelationships.Contains(AIActor)) return Friends;

    const TMap<AActor*, FAIRelationship>& Relationships = AIRelationships[AIActor];
    for (const auto& RelationshipPair : Relationships)
    {
        if (RelationshipPair.Value.Affection >= Threshold)
        {
            Friends.Add(RelationshipPair.Key);
        }
    }
    
    return Friends;
}

TArray<AActor*> UAdvancedAIBehaviors::GetEnemies(AActor* AIActor, float Threshold) const
{
    TArray<AActor*> Enemies;
    
    if (!AIActor || !AIRelationships.Contains(AIActor)) return Enemies;

    const TMap<AActor*, FAIRelationship>& Relationships = AIRelationships[AIActor];
    for (const auto& RelationshipPair : Relationships)
    {
        if (RelationshipPair.Value.Affection <= Threshold)
        {
            Enemies.Add(RelationshipPair.Key);
        }
    }
    
    return Enemies;
}

void UAdvancedAIBehaviors::TriggerEmergency(AActor* AIActor, const FAIEmergencyResponse& Emergency)
{
    if (!AIActor) return;

    AIEmergencies.Add(AIActor, Emergency);
    SetAIState(AIActor, EAIState::Emergency);
    
    OnAIEmergencyTriggered.Broadcast(AIActor, Emergency.EmergencyType);
    
    UE_LOG(LogTemp, Log, TEXT("Emergency triggered for AI: %s - %s"), 
        *AIActor->GetName(), *Emergency.EmergencyType);
}

void UAdvancedAIBehaviors::HandleGlobalEmergency(const FString& EmergencyType, FVector EmergencyLocation)
{
    // Trigger emergency response for all nearby AI actors
    for (auto& StatePair : AIStates)
    {
        AActor* AIActor = StatePair.Key;
        if (!AIActor) continue;

        float Distance = FVector::Dist(AIActor->GetActorLocation(), EmergencyLocation);
        if (Distance < 2000.0f) // Within 2km
        {
            FAIEmergencyResponse Emergency;
            Emergency.EmergencyType = EmergencyType;
            Emergency.ResponsePriority = 1.0f;
            Emergency.PanicLevel = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
            Emergency.SafeLocation = AIActor->GetActorLocation() + 
                (AIActor->GetActorLocation() - EmergencyLocation).GetSafeNormal() * 1000.0f;
            Emergency.RequiredActions.Add("Evacuate");
            
            TriggerEmergency(AIActor, Emergency);
        }
    }
}

bool UAdvancedAIBehaviors::IsInEmergency(AActor* AIActor) const
{
    return AIEmergencies.Contains(AIActor);
}

void UAdvancedAIBehaviors::ResolveEmergency(AActor* AIActor)
{
    if (!AIActor) return;

    AIEmergencies.Remove(AIActor);
    
    // Return to previous state or idle
    EAIState NewState = EAIState::Idle;
    if (StateChangeTimers.Contains(AIActor))
    {
        // Could restore previous state here if tracked
    }
    
    SetAIState(AIActor, NewState);
}

void UAdvancedAIBehaviors::AssignTask(AActor* AIActor, const FAITaskDefinition& Task)
{
    if (!AIActor) return;
    AITasks.Add(AIActor, Task);
}

FAITaskDefinition UAdvancedAIBehaviors::GetCurrentTask(AActor* AIActor) const
{
    if (!AIActor || !AITasks.Contains(AIActor)) 
    {
        FAITaskDefinition EmptyTask;
        return EmptyTask;
    }
    return AITasks[AIActor];
}

void UAdvancedAIBehaviors::CompleteCurrentTask(AActor* AIActor)
{
    if (!AIActor) return;
    AITasks.Remove(AIActor);
}

bool UAdvancedAIBehaviors::HasActiveTask(AActor* AIActor) const
{
    return AITasks.Contains(AIActor);
}

void UAdvancedAIBehaviors::InitiateSocialInteraction(AActor* AIActor, AActor* TargetActor)
{
    if (!AIActor || !TargetActor) return;

    // Move towards target
    if (AIControllers.Contains(AIActor))
    {
        AAIController* AIController = AIControllers[AIActor];
        if (AIController)
        {
            AIController->MoveToActor(TargetActor, SocialInteractionDistance);
        }
    }

    // Set social state
    SetAIState(AIActor, EAIState::Social);
    
    // Update relationship
    UpdateRelationship(AIActor, TargetActor, 0.05f, 0.02f);
    
    // Play social emote
    PlayEmote(AIActor, "Greet");
}

void UAdvancedAIBehaviors::RespondToSocialInteraction(AActor* AIActor, AActor* InitiatorActor)
{
    if (!AIActor || !InitiatorActor) return;

    // Face the initiator
    if (AIControllers.Contains(AIActor))
    {
        AAIController* AIController = AIControllers[AIActor];
        if (AIController)
        {
            AIController->SetFocalPoint(InitiatorActor->GetActorLocation());
        }
    }

    // Update relationship
    UpdateRelationship(AIActor, InitiatorActor, 0.03f, 0.01f);
    
    // Play response emote
    PlayEmote(AIActor, "Acknowledge");
}

TArray<AActor*> UAdvancedAIBehaviors::FindNearbyActors(AActor* AIActor, float Radius) const
{
    TArray<AActor*> NearbyActors;
    
    if (!AIActor) return NearbyActors;

    FVector Location = AIActor->GetActorLocation();
    
    for (const auto& StatePair : AIStates)
    {
        AActor* OtherActor = StatePair.Key;
        if (OtherActor && OtherActor != AIActor)
        {
            float Distance = FVector::Dist(Location, OtherActor->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyActors.Add(OtherActor);
            }
        }
    }
    
    return NearbyActors;
}

AActor* UAdvancedAIBehaviors::FindBestSocialTarget(AActor* AIActor) const
{
    if (!AIActor) return nullptr;

    TArray<AActor*> NearbyActors = FindNearbyActors(AIActor, SocialInteractionDistance);
    AActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    for (AActor* Target : NearbyActors)
    {
        float Score = 0.0f;
        
        // Relationship score
        FAIRelationship Relationship = GetRelationship(AIActor, Target);
        Score += Relationship.Affection * 0.5f;
        Score += Relationship.Respect * 0.3f;
        
        // Personality compatibility
        if (AIPersonalities.Contains(AIActor) && AIPersonalities.Contains(Target))
        {
            const FAIPersonality& MyPersonality = AIPersonalities[AIActor];
            const FAIPersonality& TheirPersonality = AIPersonalities[Target];
            
            float SociabilityMatch = FMath::Abs(MyPersonality.Sociability - TheirPersonality.Sociability);
            Score += (1.0f - SociabilityMatch) * 0.2f;
        }
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Target;
        }
    }
    
    return BestTarget;
}

void UAdvancedAIBehaviors::EnterCombat(AActor* AIActor, AActor* TargetActor)
{
    if (!AIActor || !TargetActor) return;

    SetAIState(AIActor, EAIState::Combat);
    
    // Add combat memory
    FAIMemoryEntry CombatMemory;
    CombatMemory.MemoryType = "Combat";
    CombatMemory.RelatedActor = TargetActor;
    CombatMemory.Location = TargetActor->GetActorLocation();
    CombatMemory.Importance = 1.0f;
    AddMemory(AIActor, CombatMemory);
    
    // Update relationship
    UpdateRelationship(AIActor, TargetActor, -0.5f, -0.2f);
    
    UE_LOG(LogTemp, Log, TEXT("AI %s entered combat with %s"), 
        *AIActor->GetName(), *TargetActor->GetName());
}

void UAdvancedAIBehaviors::ExitCombat(AActor* AIActor)
{
    if (!AIActor) return;

    SetAIState(AIActor, EAIState::Idle);
    
    if (AIControllers.Contains(AIActor))
    {
        AAIController* AIController = AIControllers[AIActor];
        if (AIController)
        {
            AIController->ClearFocus(EAIFocusPriority::Default);
        }
    }
}

bool UAdvancedAIBehaviors::ShouldFlee(AActor* AIActor, AActor* ThreatActor) const
{
    if (!AIActor || !ThreatActor) return false;

    // Check personality
    if (AIPersonalities.Contains(AIActor))
    {
        const FAIPersonality& Personality = AIPersonalities[AIActor];
        if (Personality.Bravery > CombatFleeThreshold)
            return false;
    }
    
    // Check health using character health system
    ACharacter* AICharacter = Cast<ACharacter>(AIActor);
    if (AICharacter)
    {
        // Get health from damage system or health component
        float MaxHealth = 100.0f;
        float CurrentHealth = MaxHealth; // Default to full health

        // Try to get health from Actor's properties if available
        if (AICharacter->GetClass()->FindPropertyByName(FName("Health")))
        {
            float* HealthPtr = AICharacter->GetClass()->FindPropertyByName(FName("Health"))->ContainerPtrToValuePtr<float>(AICharacter);
            if (HealthPtr)
            {
                CurrentHealth = *HealthPtr;
            }
        }

        float HealthPercentage = (CurrentHealth / MaxHealth);
        if (HealthPercentage < CombatFleeThreshold)
            return true;
    }
    
    // Check odds
    int32 AllieCount = GetFriends(AIActor).Num();
    int32 EnemyCount = GetEnemies(AIActor).Num();
    
    if (EnemyCount > AllieCount * 2)
        return true;
    
    return false;
}

void UAdvancedAIBehaviors::CallForHelp(AActor* AIActor, AActor* ThreatActor)
{
    if (!AIActor) return;

    // Find nearby allies
    TArray<AActor*> Allies = GetFriends(AIActor, 0.3f); // Low threshold for help
    float HelpRadius = 1000.0f;
    
    for (AActor* Ally : Allies)
    {
        float Distance = GetDistanceTo(AIActor, Ally);
        if (Distance <= HelpRadius)
        {
            // Add help request memory to ally
            FAIMemoryEntry HelpMemory;
            HelpMemory.MemoryType = "HelpRequest";
            HelpMemory.RelatedActor = AIActor;
            HelpMemory.Location = AIActor->GetActorLocation();
            HelpMemory.Importance = 0.8f;
            AddMemory(Ally, HelpMemory);
            
            // If threat is specified, add combat memory
            if (ThreatActor)
            {
                FAIMemoryEntry ThreatMemory;
                ThreatMemory.MemoryType = "Threat";
                ThreatMemory.RelatedActor = ThreatActor;
                ThreatMemory.Location = ThreatActor->GetActorLocation();
                ThreatMemory.Importance = 0.9f;
                AddMemory(Ally, ThreatMemory);
            }
        }
    }
}

float UAdvancedAIBehaviors::GetDistanceTo(AActor* AIActor, AActor* TargetActor) const
{
    if (!AIActor || !TargetActor) return 0.0f;
    return FVector::Dist(AIActor->GetActorLocation(), TargetActor->GetActorLocation());
}

bool UAdvancedAIBehaviors::CanSee(AActor* AIActor, AActor* TargetActor) const
{
    if (!AIActor || !TargetActor) return false;

    FVector StartLocation = AIActor->GetActorLocation();
    FVector EndLocation = TargetActor->GetActorLocation();
    float Distance = FVector::Dist(StartLocation, EndLocation);
    
    // Check distance limit (simplified vision range)
    if (Distance > 1500.0f) return false;
    
    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(AIActor);
    QueryParams.AddIgnoredActor(TargetActor);
    
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return bHasLineOfSight;
}

bool UAdvancedAIBehaviors::CanHear(AActor* AIActor, FVector SoundLocation, float SoundVolume) const
{
    if (!AIActor) return false;

    float Distance = FVector::Dist(AIActor->GetActorLocation(), SoundLocation);
    float HearingRange = SoundVolume * 500.0f; // Scale volume to distance
    
    return Distance <= HearingRange;
}

void UAdvancedAIBehaviors::PlayEmote(AActor* AIActor, const FString& EmoteName)
{
    if (!AIActor) return;

    ACharacter* AICharacter = Cast<ACharacter>(AIActor);
    if (!AICharacter) return;

    // Get animation instance
    UAnimInstance* AnimInstance = AICharacter->GetMesh() ? AICharacter->GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("AI %s has no AnimInstance for emote"), *AIActor->GetName());
        return;
    }

    // Map emote names to montage slots or animation sequences
    UAnimMontage* EmoteMontage = nullptr;

    // Try to find montage by name convention
    FString MontagePath = FString::Printf(TEXT("/Game/Animations/Emotes/%s_Montage"), *EmoteName);
    EmoteMontage = LoadObject<UAnimMontage>(nullptr, *MontagePath, nullptr, LOAD_None, nullptr);

    if (EmoteMontage)
    {
        // Play the montage
        float Duration = AnimInstance->Montage_Play(EmoteMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("AI %s playing emote: %s (Duration: %.2fs)"), *AIActor->GetName(), *EmoteName, Duration);

        // Broadcast emote event if delegates are set up
        OnAIEmotionChanged.Broadcast(AIActor, EmoteName);
    }
    else
    {
        // Fallback to pose or simple animation
        UE_LOG(LogTemp, Warning, TEXT("AI %s could not find emote montage: %s"), *AIActor->GetName(), *EmoteName);

        // Still broadcast the event for gameplay purposes
        OnAIEmotionChanged.Broadcast(AIActor, EmoteName);
    }
}

void UAdvancedAIBehaviors::PlayVoiceLine(AActor* AIActor, USoundCue* VoiceCue)
{
    if (!AIActor || !VoiceCue) return;

    // Play voice sound at AI location
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), VoiceCue, AIActor->GetActorLocation());
}

EAIState UAdvancedAIBehaviors::DetermineBestState(AActor* AIActor)
{
    if (!AIActor) return EAIState::Idle;

    // Priority order: Emergency > Combat > Task > Scheduled > Personality-driven
    
    // Check emergency first
    if (IsInEmergency(AIActor))
        return EAIState::Emergency;
    
    // Check combat
    if (AIStates[AIActor] == EAIState::Combat)
    {
        // Check if should flee
        TArray<AActor*> Enemies = GetEnemies(AIActor);
        for (AActor* Enemy : Enemies)
        {
            if (ShouldFlee(AIActor, Enemy))
                return EAIState::Flee;
        }
        return EAIState::Combat;
    }
    
    // Check active tasks
    if (HasActiveTask(AIActor))
    {
        FAITaskDefinition Task = GetCurrentTask(AIActor);
        if (Task.TaskType == "MoveToLocation")
            return EAIState::Patrol;
        else if (Task.TaskType == "InteractWithActor")
            return EAIState::Social;
        else
            return EAIState::Work;
    }
    
    // Check scheduled activities
    float CurrentHour = GetWorld()->GetTimeSeconds() / 3600.0f;
    CurrentHour = FMath::Fmod(CurrentHour, 24.0f);
    FString Activity = GetCurrentScheduledActivity(AIActor, CurrentHour);
    
    if (Activity == "Work") return EAIState::Work;
    if (Activity == "Sleep") return EAIState::Sleep;
    if (Activity == "Social") return EAIState::Social;
    if (Activity == "Eat") return EAIState::Eat;
    
    // Default to idle
    return EAIState::Idle;
}

bool UAdvancedAIBehaviors::ShouldChangeState(AActor* AIActor, EAIState PotentialState)
{
    if (!AIActor) return false;
    
    // Check if transition is allowed
    if (!CanTransitionToState(AIActor, PotentialState))
        return false;
    
    // Add cooldown for state changes to prevent rapid switching
    if (StateChangeTimers.Contains(AIActor))
    {
        float TimeSinceChange = GetWorld()->GetTimeSeconds() - StateChangeTimers[AIActor];
        if (TimeSinceChange < 2.0f) // 2 second cooldown
            return false;
    }
    
    return true;
}

void UAdvancedAIBehaviors::ExecuteStateBehavior(AActor* AIActor, EAIState State)
{
    if (!AIActor) return;

    // State-specific execution logic
    switch (State)
    {
        case EAIState::Idle:
            // Idle animations and behaviors
            break;
            
        case EAIState::Patrol:
            // Patrol movement handled by task system
            break;
            
        case EAIState::Work:
            // Work animations and behaviors
            break;
            
        case EAIState::Social:
            // Social behaviors handled by social system
            break;
            
        case EAIState::Sleep:
            // Sleep animations
            break;
            
        case EAIState::Eat:
            // Eating animations
            break;
            
        default:
            break;
    }
}

FVector UAdvancedAIBehaviors::CalculateGroupCohesion(const FAIGroupBehavior& Group, AActor* AIActor)
{
    if (Group.GroupMembers.Num() <= 1) return FVector::ZeroVector;

    FVector GroupCenter = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (Member && Member != AIActor)
        {
            GroupCenter += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0) return FVector::ZeroVector;
    
    GroupCenter /= ValidMembers;
    FVector ToCenter = GroupCenter - AIActor->GetActorLocation();
    
    if (ToCenter.Size() > Group.CohesionDistance)
    {
        return ToCenter.GetSafeNormal() * 100.0f;
    }
    
    return FVector::ZeroVector;
}

FVector UAdvancedAIBehaviors::CalculateGroupSeparation(const FAIGroupBehavior& Group, AActor* AIActor)
{
    FVector SeparationForce = FVector::ZeroVector;
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (Member && Member != AIActor)
        {
            FVector ToMember = Member->GetActorLocation() - AIActor->GetActorLocation();
            float Distance = ToMember.Size();
            
            if (Distance < Group.SeparationDistance && Distance > 0.0f)
            {
                SeparationForce -= ToMember.GetSafeNormal() * (Group.SeparationDistance - Distance);
            }
        }
    }
    
    return SeparationForce;
}

FVector UAdvancedAIBehaviors::CalculateGroupAlignment(const FAIGroupBehavior& Group, AActor* AIActor)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (Member && Member != AIActor)
        {
            // Get velocity from character movement component
            ACharacter* MemberCharacter = Cast<ACharacter>(Member);
            if (MemberCharacter && MemberCharacter->GetCharacterMovement())
            {
                AverageVelocity += MemberCharacter->GetCharacterMovement()->Velocity;
                ValidMembers++;
            }
        }
    }
    
    if (ValidMembers == 0) return FVector::ZeroVector;
    
    AverageVelocity /= ValidMembers;
    return AverageVelocity * Group.AlignmentStrength;
}

void UAdvancedAIBehaviors::DecayMemories(AActor* AIActor, float DeltaTime)
{
    if (!AIActor || !AIMemories.Contains(AIActor)) return;

    TArray<FAIMemoryEntry>& Memories = AIMemories[AIActor];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        float Age = CurrentTime - Memories[i].Timestamp;
        if (Age > MemoryDecayTime)
        {
            Memories.RemoveAt(i);
        }
    }
}

void UAdvancedAIBehaviors::DecayRelationships(AActor* AIActor, float DeltaTime)
{
    if (!AIActor || !AIRelationships.Contains(AIActor)) return;

    TMap<AActor*, FAIRelationship>& Relationships = AIRelationships[AIActor];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& RelationshipPair : Relationships)
    {
        FAIRelationship& Relationship = RelationshipPair.Value;
        
        // Decay relationships over time without interaction
        float TimeSinceInteraction = CurrentTime - Relationship.LastInteractionTime;
        if (TimeSinceInteraction > 3600.0f) // 1 hour
        {
            Relationship.Affection *= (1.0f - RelationshipDecayRate * DeltaTime);
            Relationship.Respect *= (1.0f - RelationshipDecayRate * DeltaTime);
        }
    }
}

void UAdvancedAIBehaviors::UpdateEmotionalState(AActor* AIActor)
{
    if (!AIActor) return;

    // Calculate emotional state based on various factors
    FString CurrentEmotion = "Neutral";
    
    // Check emergency state
    if (IsInEmergency(AIActor))
    {
        const FAIEmergencyResponse& Emergency = AIEmergencies[AIActor];
        if (Emergency.PanicLevel > 0.7f)
            CurrentEmotion = "Panic";
        else if (Emergency.PanicLevel > 0.4f)
            CurrentEmotion = "Scared";
        else
            CurrentEmotion = "Alert";
    }
    // Check combat state
    else if (AIStates[AIActor] == EAIState::Combat)
    {
        CurrentEmotion = "Aggressive";
    }
    // Check social state
    else if (AIStates[AIActor] == EAIState::Social)
    {
        CurrentEmotion = "Friendly";
    }
    
    // Broadcast emotion change
    OnAIEmotionChanged.Broadcast(AIActor, CurrentEmotion);
}