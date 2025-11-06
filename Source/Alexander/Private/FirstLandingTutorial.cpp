// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstLandingTutorial.h"
#include "Kismet/GameplayStatics.h"

AFirstLandingTutorial::AFirstLandingTutorial()
{
    ScenarioName = FText::FromString(TEXT("First Landing"));
    ScenarioDescription = FText::FromString(
        TEXT("Learn the basics of spaceship flight, planetary landing, and colony establishment. ")
        TEXT("This tutorial will guide you through your first landing on a habitable planet.")
    );

    bAutoStart = true;
    TimeLimit = 0.0f; // No time limit for tutorial

    bInShip = false;
    bFlightStarted = false;
    bPlanetApproached = false;
    bInAtmosphere = false;
    bLanded = false;
    bExitedShip = false;
    bCropPlanted = false;
    TargetPlanet = nullptr;
    DistanceToPlanet = 0.0f;
}

void AFirstLandingTutorial::BeginPlay()
{
    Super::BeginPlay();

    SetupObjectives();
    SetupRewards();
}

void AFirstLandingTutorial::SetupObjectives()
{
    // Objective 1: Enter your spaceship
    FScenarioObjective Obj1;
    Obj1.ObjectiveTitle = FText::FromString(TEXT("Enter Your Spaceship"));
    Obj1.ObjectiveDescription = FText::FromString(TEXT("Approach and enter the spaceship using VR hand controls"));
    Obj1.bCompleted = false;
    Obj1.bOptional = false;
    AddObjective(Obj1);

    // Objective 2: Activate flight controls
    FScenarioObjective Obj2;
    Obj2.ObjectiveTitle = FText::FromString(TEXT("Activate Flight Controls"));
    Obj2.ObjectiveDescription = FText::FromString(TEXT("Take the pilot's seat and grab the flight controls"));
    Obj2.bCompleted = false;
    Obj2.bOptional = false;
    AddObjective(Obj2);

    // Objective 3: Approach the planet
    FScenarioObjective Obj3;
    Obj3.ObjectiveTitle = FText::FromString(TEXT("Approach the Planet"));
    Obj3.ObjectiveDescription = FText::FromString(TEXT("Fly toward the green planet marker until you're within 100km"));
    Obj3.bCompleted = false;
    Obj3.bOptional = false;
    AddObjective(Obj3);

    // Objective 4: Enter atmosphere
    FScenarioObjective Obj4;
    Obj4.ObjectiveTitle = FText::FromString(TEXT("Enter Atmosphere"));
    Obj4.ObjectiveDescription = FText::FromString(TEXT("Descend carefully into the planet's atmosphere"));
    Obj4.bCompleted = false;
    Obj4.bOptional = false;
    AddObjective(Obj4);

    // Objective 5: Land on the landing pad
    FScenarioObjective Obj5;
    Obj5.ObjectiveTitle = FText::FromString(TEXT("Land Safely"));
    Obj5.ObjectiveDescription = FText::FromString(TEXT("Follow the landing guidance to touch down on the designated pad"));
    Obj5.bCompleted = false;
    Obj5.bOptional = false;
    AddObjective(Obj5);

    // Objective 6: Exit ship and explore
    FScenarioObjective Obj6;
    Obj6.ObjectiveTitle = FText::FromString(TEXT("Exit Ship"));
    Obj6.ObjectiveDescription = FText::FromString(TEXT("Leave your ship and explore the planet surface"));
    Obj6.bCompleted = false;
    Obj6.bOptional = false;
    AddObjective(Obj6);

    // Objective 7: Plant a crop
    FScenarioObjective Obj7;
    Obj7.ObjectiveTitle = FText::FromString(TEXT("Plant Your First Crop"));
    Obj7.ObjectiveDescription = FText::FromString(TEXT("Use the farming tool to plant a seed in the designated plot"));
    Obj7.bCompleted = false;
    Obj7.bOptional = false;
    AddObjective(Obj7);

    // Objective 8: Return to ship (optional)
    FScenarioObjective Obj8;
    Obj8.ObjectiveTitle = FText::FromString(TEXT("Return to Ship"));
    Obj8.ObjectiveDescription = FText::FromString(TEXT("Walk back to your spaceship"));
    Obj8.bCompleted = false;
    Obj8.bOptional = true; // Optional objective
    AddObjective(Obj8);
}

void AFirstLandingTutorial::SetupRewards()
{
    Rewards.ExperiencePoints = 500;
    Rewards.Credits = 1000;
    Rewards.UnlockedFeatures.Add(FName("BasicFarming"));
    Rewards.UnlockedFeatures.Add(FName("PlanetaryTravel"));
    Rewards.UnlockedLocations.Add(FName("StarterColony"));
}

void AFirstLandingTutorial::OnPhaseIntroduction_Implementation()
{
    Super::OnPhaseIntroduction_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Tutorial: Introduction Phase"));

    // Show welcome message
    // In a real implementation, this would trigger UI elements
    // For now, just log the intro message

    UE_LOG(LogTemp, Display, TEXT("=============================================="));
    UE_LOG(LogTemp, Display, TEXT("WELCOME TO ALEXANDER"));
    UE_LOG(LogTemp, Display, TEXT("=============================================="));
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("You are humanity's last hope."));
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("Your mission: Establish colonies across the galaxy,"));
    UE_LOG(LogTemp, Display, TEXT("build sustainable ecosystems, and preserve human civilization."));
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("This tutorial will teach you the basics of:"));
    UE_LOG(LogTemp, Display, TEXT("- Spaceship flight and navigation"));
    UE_LOG(LogTemp, Display, TEXT("- Planetary landing procedures"));
    UE_LOG(LogTemp, Display, TEXT("- Basic farming and colony management"));
    UE_LOG(LogTemp, Display, TEXT(""));
    UE_LOG(LogTemp, Display, TEXT("=============================================="));

    // Auto-advance to tutorial phase after 5 seconds
    FTimerHandle IntroTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(IntroTimerHandle, [this]()
    {
        AdvanceToPhase(EScenarioPhase::Tutorial);
    }, 5.0f, false);
}

void AFirstLandingTutorial::OnPhaseTutorial_Implementation()
{
    Super::OnPhaseTutorial_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Tutorial: Training Phase - Learning Basic Controls"));

    // Find the tutorial planet
    // In a real implementation, this would find the actual planet actor
    TArray<AActor*> FoundPlanets;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundPlanets);

    // For now, just log that we're ready for the player
    UE_LOG(LogTemp, Display, TEXT("Tutorial objectives are now active."));
    UE_LOG(LogTemp, Display, TEXT("First objective: Enter Your Spaceship"));
}

void AFirstLandingTutorial::OnPhaseExploration_Implementation()
{
    Super::OnPhaseExploration_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Tutorial: Exploration Phase"));
    UE_LOG(LogTemp, Display, TEXT("Great job landing! Now explore the planet surface."));
}

void AFirstLandingTutorial::OnPhaseMission_Implementation()
{
    Super::OnPhaseMission_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Tutorial: Mission Phase - Establishing First Colony"));
    UE_LOG(LogTemp, Display, TEXT("Time to establish your first colony outpost!"));
}

void AFirstLandingTutorial::OnPlayerEnteredShip()
{
    if (bInShip) return;

    bInShip = true;
    CompleteObjective(FText::FromString(TEXT("Enter Your Spaceship")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: You've entered the ship! Now activate the flight controls."));
}

void AFirstLandingTutorial::OnPlayerStartedFlight()
{
    if (bFlightStarted) return;

    bFlightStarted = true;
    CompleteObjective(FText::FromString(TEXT("Activate Flight Controls")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: Flight controls activated! Head toward the green planet."));
}

void AFirstLandingTutorial::OnPlayerApproachedPlanet()
{
    if (bPlanetApproached) return;

    bPlanetApproached = true;
    CompleteObjective(FText::FromString(TEXT("Approach the Planet")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: Good navigation! Now prepare for atmospheric entry."));
}

void AFirstLandingTutorial::OnPlayerEnteredAtmosphere()
{
    if (bInAtmosphere) return;

    bInAtmosphere = true;
    CompleteObjective(FText::FromString(TEXT("Enter Atmosphere")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: You're in the atmosphere! Follow the landing guidance."));

    // Advance to exploration phase
    AdvanceToPhase(EScenarioPhase::Exploration);
}

void AFirstLandingTutorial::OnPlayerLanded()
{
    if (bLanded) return;

    bLanded = true;
    CompleteObjective(FText::FromString(TEXT("Land Safely")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: Perfect landing! Exit the ship to explore."));
}

void AFirstLandingTutorial::OnPlayerExitedShip()
{
    if (bExitedShip) return;

    bExitedShip = true;
    CompleteObjective(FText::FromString(TEXT("Exit Ship")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: Welcome to your new home! Let's plant some crops."));

    // Advance to mission phase
    AdvanceToPhase(EScenarioPhase::Mission);
}

void AFirstLandingTutorial::OnPlayerPlantedCrop()
{
    if (bCropPlanted) return;

    bCropPlanted = true;
    CompleteObjective(FText::FromString(TEXT("Plant Your First Crop")));

    UE_LOG(LogTemp, Display, TEXT("Tutorial: Excellent! Your first crop is planted. Colony established!"));

    // Tutorial will auto-complete when all required objectives are done
}

void AFirstLandingTutorial::OnPlayerReturnedToShip()
{
    CompleteObjective(FText::FromString(TEXT("Return to Ship")));
    UE_LOG(LogTemp, Display, TEXT("Tutorial: Back at the ship. You're ready for the next adventure!"));
}
