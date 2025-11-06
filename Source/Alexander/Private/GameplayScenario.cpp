// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayScenario.h"
#include "PlayerProgression.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AGameplayScenario::AGameplayScenario()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    CurrentPhase = EScenarioPhase::NotStarted;
    ElapsedTime = 0.0f;
    bScenarioStarted = false;
    PlayerProgression = nullptr;
    bAutoAwardProgressionRewards = true;
}

void AGameplayScenario::BeginPlay()
{
    Super::BeginPlay();

    // Load player progression if auto-award is enabled
    if (bAutoAwardProgressionRewards)
    {
        PlayerProgression = UPlayerProgression::LoadProgression();
        if (PlayerProgression)
        {
            UE_LOG(LogTemp, Log, TEXT("Scenario: Loaded player progression (Level %d)"),
                PlayerProgression->GetPlayerLevel());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Scenario: Failed to load player progression - rewards will not be awarded"));
        }
    }

    if (bAutoStart)
    {
        StartScenario();
    }
}

void AGameplayScenario::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bScenarioStarted || CurrentPhase == EScenarioPhase::Completed || CurrentPhase == EScenarioPhase::Failed)
    {
        return;
    }

    ElapsedTime += DeltaTime;

    // Check time limit
    if (TimeLimit > 0.0f)
    {
        CheckTimeLimit();
    }

    // Check completion conditions
    CheckScenarioCompletion();
}

void AGameplayScenario::StartScenario()
{
    if (bScenarioStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scenario %s already started"), *ScenarioName.ToString());
        return;
    }

    bScenarioStarted = true;
    ElapsedTime = 0.0f;
    CurrentPhase = EScenarioPhase::Introduction;

    UE_LOG(LogTemp, Log, TEXT("Starting scenario: %s"), *ScenarioName.ToString());

    // Broadcast phase change
    OnScenarioPhaseChanged.Broadcast(CurrentPhase);

    // Start introduction phase
    OnPhaseIntroduction();
}

void AGameplayScenario::CompleteScenario()
{
    if (!bScenarioStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete scenario that hasn't started"));
        return;
    }

    if (CurrentPhase == EScenarioPhase::Completed || CurrentPhase == EScenarioPhase::Failed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scenario already finished"));
        return;
    }

    CurrentPhase = EScenarioPhase::Completed;

    UE_LOG(LogTemp, Log, TEXT("Scenario completed: %s (Time: %.1fs)"), *ScenarioName.ToString(), ElapsedTime);

    // Automatically award progression rewards
    if (bAutoAwardProgressionRewards && PlayerProgression)
    {
        // Award experience points
        if (Rewards.ExperiencePoints > 0)
        {
            PlayerProgression->AddExperience(Rewards.ExperiencePoints);
            UE_LOG(LogTemp, Display, TEXT("Awarded %d XP - Player now at level %d"),
                Rewards.ExperiencePoints, PlayerProgression->GetPlayerLevel());
        }

        // Award credits
        if (Rewards.Credits > 0)
        {
            PlayerProgression->AddCredits(Rewards.Credits);
            UE_LOG(LogTemp, Display, TEXT("Awarded %d Credits - Player now has %d credits"),
                Rewards.Credits, PlayerProgression->GetCredits());
        }

        // Unlock features
        for (const FName& Feature : Rewards.UnlockedFeatures)
        {
            PlayerProgression->UnlockFeature(Feature);
            UE_LOG(LogTemp, Display, TEXT("Unlocked feature: %s"), *Feature.ToString());

            // Award skill XP based on feature type
            if (Feature.ToString().Contains(TEXT("Farming")))
            {
                PlayerProgression->AddSkillXP(EPlayerSkill::Farming, 100.0f);
            }
            else if (Feature.ToString().Contains(TEXT("Combat")))
            {
                PlayerProgression->AddSkillXP(EPlayerSkill::Combat, 100.0f);
            }
            else if (Feature.ToString().Contains(TEXT("Engineering")))
            {
                PlayerProgression->AddSkillXP(EPlayerSkill::Engineering, 100.0f);
            }
            else if (Feature.ToString().Contains(TEXT("Trading")))
            {
                PlayerProgression->AddSkillXP(EPlayerSkill::Trading, 100.0f);
            }
            else if (Feature.ToString().Contains(TEXT("Exploration")) ||
                     Feature.ToString().Contains(TEXT("Travel")))
            {
                PlayerProgression->AddSkillXP(EPlayerSkill::Exploration, 100.0f);
            }
        }

        // Unlock locations
        for (const FName& Location : Rewards.UnlockedLocations)
        {
            PlayerProgression->UnlockLocation(Location);
            UE_LOG(LogTemp, Display, TEXT("Unlocked location: %s"), *Location.ToString());
        }

        // Save progression
        if (PlayerProgression->SaveProgression())
        {
            UE_LOG(LogTemp, Log, TEXT("Player progression saved successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to save player progression"));
        }
    }

    // Broadcast completion
    OnScenarioCompleted.Broadcast(Rewards);
    OnScenarioPhaseChanged.Broadcast(CurrentPhase);
}

void AGameplayScenario::FailScenario(const FText& FailureReason)
{
    if (!bScenarioStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot fail scenario that hasn't started"));
        return;
    }

    if (CurrentPhase == EScenarioPhase::Completed || CurrentPhase == EScenarioPhase::Failed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scenario already finished"));
        return;
    }

    CurrentPhase = EScenarioPhase::Failed;

    UE_LOG(LogTemp, Warning, TEXT("Scenario failed: %s - %s"), *ScenarioName.ToString(), *FailureReason.ToString());

    // Broadcast failure
    OnScenarioFailed.Broadcast(FailureReason);
    OnScenarioPhaseChanged.Broadcast(CurrentPhase);
}

void AGameplayScenario::AdvanceToPhase(EScenarioPhase NewPhase)
{
    if (!bScenarioStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance phase - scenario not started"));
        return;
    }

    if (CurrentPhase == EScenarioPhase::Completed || CurrentPhase == EScenarioPhase::Failed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance phase - scenario finished"));
        return;
    }

    EScenarioPhase OldPhase = CurrentPhase;
    CurrentPhase = NewPhase;

    UE_LOG(LogTemp, Log, TEXT("Scenario %s: Phase %d -> %d"),
           *ScenarioName.ToString(), (int32)OldPhase, (int32)NewPhase);

    // Broadcast phase change
    OnScenarioPhaseChanged.Broadcast(CurrentPhase);

    // Call phase-specific handlers
    switch (CurrentPhase)
    {
        case EScenarioPhase::Introduction:
            OnPhaseIntroduction();
            break;
        case EScenarioPhase::Tutorial:
            OnPhaseTutorial();
            break;
        case EScenarioPhase::Exploration:
            OnPhaseExploration();
            break;
        case EScenarioPhase::Mission:
            OnPhaseMission();
            break;
        default:
            break;
    }
}

void AGameplayScenario::AddObjective(const FScenarioObjective& Objective)
{
    Objectives.Add(Objective);
    UE_LOG(LogTemp, Log, TEXT("Added objective: %s"), *Objective.ObjectiveTitle.ToString());
}

void AGameplayScenario::CompleteObjective(const FText& ObjectiveTitle)
{
    for (FScenarioObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveTitle.EqualTo(ObjectiveTitle))
        {
            if (Objective.bCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Objective already completed: %s"), *ObjectiveTitle.ToString());
                return;
            }

            Objective.bCompleted = true;
            Objective.ProgressPercent = 100.0f;

            UE_LOG(LogTemp, Log, TEXT("Objective completed: %s"), *ObjectiveTitle.ToString());

            // Broadcast objective completion
            OnObjectiveCompleted.Broadcast(Objective);

            // Check if all objectives are complete
            CheckScenarioCompletion();
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Objective not found: %s"), *ObjectiveTitle.ToString());
}

void AGameplayScenario::UpdateObjectiveProgress(const FText& ObjectiveTitle, float Progress)
{
    for (FScenarioObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveTitle.EqualTo(ObjectiveTitle))
        {
            Objective.ProgressPercent = FMath::Clamp(Progress, 0.0f, 100.0f);

            // Auto-complete if progress reaches 100%
            if (Objective.ProgressPercent >= 100.0f && !Objective.bCompleted)
            {
                CompleteObjective(ObjectiveTitle);
            }

            return;
        }
    }
}

bool AGameplayScenario::AreAllObjectivesComplete() const
{
    if (Objectives.Num() == 0)
    {
        return false;
    }

    for (const FScenarioObjective& Objective : Objectives)
    {
        // Skip optional objectives
        if (Objective.bOptional)
        {
            continue;
        }

        if (!Objective.bCompleted)
        {
            return false;
        }
    }

    return true;
}

TArray<FScenarioObjective> AGameplayScenario::GetActiveObjectives() const
{
    TArray<FScenarioObjective> ActiveObjectives;

    for (const FScenarioObjective& Objective : Objectives)
    {
        if (!Objective.bCompleted)
        {
            ActiveObjectives.Add(Objective);
        }
    }

    return ActiveObjectives;
}

float AGameplayScenario::GetScenarioProgress() const
{
    if (Objectives.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = 0;
    int32 RequiredCount = 0;

    for (const FScenarioObjective& Objective : Objectives)
    {
        if (!Objective.bOptional)
        {
            RequiredCount++;
            if (Objective.bCompleted)
            {
                CompletedCount++;
            }
        }
    }

    if (RequiredCount == 0)
    {
        return 0.0f;
    }

    return (float)CompletedCount / (float)RequiredCount * 100.0f;
}

bool AGameplayScenario::IsScenarioActive() const
{
    return bScenarioStarted &&
           CurrentPhase != EScenarioPhase::NotStarted &&
           CurrentPhase != EScenarioPhase::Completed &&
           CurrentPhase != EScenarioPhase::Failed;
}

void AGameplayScenario::OnPhaseIntroduction_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Phase: Introduction - %s"), *ScenarioDescription.ToString());
}

void AGameplayScenario::OnPhaseTutorial_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Phase: Tutorial"));
}

void AGameplayScenario::OnPhaseExploration_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Phase: Exploration"));
}

void AGameplayScenario::OnPhaseMission_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Phase: Mission"));
}

void AGameplayScenario::CheckScenarioCompletion()
{
    // Auto-complete if all objectives are done
    if (AreAllObjectivesComplete())
    {
        CompleteScenario();
    }
}

void AGameplayScenario::CheckTimeLimit()
{
    if (TimeLimit > 0.0f && ElapsedTime >= TimeLimit)
    {
        FailScenario(FText::FromString(TEXT("Time limit exceeded")));
    }
}
