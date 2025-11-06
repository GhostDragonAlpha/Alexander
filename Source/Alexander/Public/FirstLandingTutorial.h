// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayScenario.h"
#include "FirstLandingTutorial.generated.h"

/**
 * First Landing Tutorial
 *
 * Teaches the player how to:
 * - Control their spaceship in VR
 * - Approach a planet
 * - Enter atmosphere
 * - Land safely
 * - Exit ship and explore
 * - Plant first crops
 * - Return to ship
 *
 * This is the introductory tutorial that every player experiences
 */
UCLASS(Blueprintable)
class ALEXANDER_API AFirstLandingTutorial : public AGameplayScenario
{
    GENERATED_BODY()

public:
    AFirstLandingTutorial();

    virtual void BeginPlay() override;

protected:
    // Phase implementations
    virtual void OnPhaseIntroduction_Implementation() override;
    virtual void OnPhaseTutorial_Implementation() override;
    virtual void OnPhaseExploration_Implementation() override;
    virtual void OnPhaseMission_Implementation() override;

private:
    void SetupObjectives();
    void SetupRewards();

    // Tutorial step handlers
    UFUNCTION()
    void OnPlayerEnteredShip();

    UFUNCTION()
    void OnPlayerStartedFlight();

    UFUNCTION()
    void OnPlayerApproachedPlanet();

    UFUNCTION()
    void OnPlayerEnteredAtmosphere();

    UFUNCTION()
    void OnPlayerLanded();

    UFUNCTION()
    void OnPlayerExitedShip();

    UFUNCTION()
    void OnPlayerPlantedCrop();

    UFUNCTION()
    void OnPlayerReturnedToShip();

    // Tutorial state tracking
    UPROPERTY()
    bool bInShip;

    UPROPERTY()
    bool bFlightStarted;

    UPROPERTY()
    bool bPlanetApproached;

    UPROPERTY()
    bool bInAtmosphere;

    UPROPERTY()
    bool bLanded;

    UPROPERTY()
    bool bExitedShip;

    UPROPERTY()
    bool bCropPlanted;

    UPROPERTY()
    AActor* TargetPlanet;

    UPROPERTY()
    float DistanceToPlanet;
};
