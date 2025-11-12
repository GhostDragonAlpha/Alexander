// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "ResourceGatheringSystem.h"
#include "Core/EventBus.h"
#include "Core/SystemRegistry.h"
#include "Engine/World.h"

void UResourceGatheringSystem::InitializeModule()
{
	Super::InitializeModule();
	
	// Reset mining state
	CurrentMiningTarget = nullptr;
	bIsCurrentlyMining = false;
	MiningProgress = 0.0f;
	CurrentYield = 0.0f;
	MiningEfficiency = 1.0f;
	
	LogSystemMessage(TEXT("ResourceGatheringSystem: Mining systems initialized"));
}

void UResourceGatheringSystem::ShutdownModule()
{
	// Stop any active mining
	StopMining();
	
	LogSystemMessage(TEXT("ResourceGatheringSystem: Mining systems shut down"));
	
	Super::ShutdownModule();
}

void UResourceGatheringSystem::UpdateModule(float DeltaTime)
{
	if (!IsSystemHealthy())
	{
		return;
	}
	
	if (bIsCurrentlyMining && CurrentMiningTarget)
	{
		CollectResources(DeltaTime);
	}
}

FString UResourceGatheringSystem::GetSystemName() const
{
	return TEXT("ResourceGatheringSystem");
}

bool UResourceGatheringSystem::IsSystemHealthy() const
{
	if (!Super::IsSystemHealthy())
	{
		return false;
	}
	
	// Additional health checks
	if (bIsCurrentlyMining && !CurrentMiningTarget)
	{
		return false;
	}
	
	return true;
}

void UResourceGatheringSystem::StartMining(AActor* Target)
{
	if (!Target)
	{
		LogSystemMessage(TEXT("Cannot start mining: Target is null"), true);
		return;
	}
	
	CurrentMiningTarget = Target;
	bIsCurrentlyMining = true;
	MiningProgress = 0.0f;
	CurrentYield = 0.0f;
	
	LogSystemMessage(FString::Printf(TEXT("Started mining target: %s"), *Target->GetName()));
	
	// Publish mining started event
	FSystemEvent MiningStartedEvent;
	MiningStartedEvent.EventType = TEXT("MiningStarted");
	MiningStartedEvent.SourceSystem = GetSystemName();
	MiningStartedEvent.EventData = FString::Printf(TEXT("{\"target\":\"%s\"}"), *Target->GetName());
	PublishEvent(MiningStartedEvent);
}

void UResourceGatheringSystem::StopMining()
{
	if (bIsCurrentlyMining)
	{
		bIsCurrentlyMining = false;
		CompleteMining();
	}
}

void UResourceGatheringSystem::HandleShipMoved(const FSystemEvent& Event)
{
	// Could implement range checks here
}

void UResourceGatheringSystem::HandleMiningLaserActivated(const FSystemEvent& Event)
{
	// Could implement laser mining logic here
}

void UResourceGatheringSystem::ScanForResources()
{
	// In a real implementation, this would scan for nearby asteroids
	LogSystemMessage(TEXT("Scanning for resources..."));
}

void UResourceGatheringSystem::CollectResources(float DeltaTime)
{
	if (!CurrentMiningTarget)
	{
		return;
	}
	
	// Simulate mining progress
	MiningProgress += MiningSpeed * DeltaTime;
	
	// Calculate yield
	float YieldThisFrame = BaseYieldPerSecond * MiningEfficiency * DeltaTime;
	CurrentYield += YieldThisFrame;
	
	// Publish resource collected event periodically
	if (FMath::Frac(MiningProgress) < 0.1f)
	{
		FSystemEvent ResourceEvent;
		ResourceEvent.EventType = TEXT("ResourceCollected");
		ResourceEvent.SourceSystem = GetSystemName();
		ResourceEvent.EventData = FString::Printf(TEXT("{\"yield\":%f}"), YieldThisFrame);
		PublishEvent(ResourceEvent);
	}
	
	// Check if mining is complete
	if (MiningProgress >= 100.0f)
	{
		CompleteMining();
	}
}

void UResourceGatheringSystem::CompleteMining()
{
	bIsCurrentlyMining = false;
	
	// Publish mining complete event
	FSystemEvent CompleteEvent;
	CompleteEvent.EventType = TEXT("MiningComplete");
	CompleteEvent.SourceSystem = GetSystemName();
	CompleteEvent.EventData = FString::Printf(TEXT("{\"totalYield\":%f}"), CurrentYield);
	PublishEvent(CompleteEvent);
	
	LogSystemMessage(FString::Printf(TEXT("Mining complete. Total yield: %f"), CurrentYield));
	
	// Reset for next mining operation
	CurrentMiningTarget = nullptr;
	MiningProgress = 0.0f;
	CurrentYield = 0.0f;
}