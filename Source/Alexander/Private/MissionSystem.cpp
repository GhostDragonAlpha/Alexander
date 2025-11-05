#include "MissionSystem.h"

UMissionSystem::UMissionSystem()
{
}

void UMissionSystem::Initialize()
{
    // Initialize mission system
}

void UMissionSystem::Update(float DeltaTime)
{
    // Update mission system
}

bool UMissionSystem::IsMissionActive(const FName& MissionID) const
{
    return ActiveMissions.Contains(MissionID);
}

void UMissionSystem::StartMission(const FName& MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        ActiveMissions.Add(MissionID, 0.0f);
    }
}

void UMissionSystem::CompleteMission(const FName& MissionID)
{
    ActiveMissions.Remove(MissionID);
}

float UMissionSystem::GetMissionProgress(const FName& MissionID) const
{
    const float* Progress = ActiveMissions.Find(MissionID);
    return Progress ? *Progress : 0.0f;
}