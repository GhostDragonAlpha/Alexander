#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MissionSystem.generated.h"

/**
 * Mission system for managing player missions and objectives
 */
UCLASS()
class ALEXANDER_API UMissionSystem : public UObject
{
    GENERATED_BODY()

public:
    UMissionSystem();

    /** Initialize the mission system */
    void Initialize();

    /** Update mission system */
    void Update(float DeltaTime);

    /** Check if a mission is active */
    bool IsMissionActive(const FName& MissionID) const;

    /** Start a mission */
    void StartMission(const FName& MissionID);

    /** Complete a mission */
    void CompleteMission(const FName& MissionID);

    /** Get mission progress */
    float GetMissionProgress(const FName& MissionID) const;

private:
    /** Active missions */
    TMap<FName, float> ActiveMissions;
};