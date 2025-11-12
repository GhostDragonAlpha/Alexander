// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReplicationManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"  // For replication functionality and DOREPLIFETIME macros

UReplicationManager::UReplicationManager()
{
	// Initialize replication settings
	RelevanceDistance = 10000.0f; // 10km default relevance distance
	MaxBandwidthPerSecond = 1024.0f * 1024.0f; // 1MB/s default
	bEnableCompression = true;
	bEnableDeltaCompression = true;
	MaxActorsPerFrame = 50;
	ServerUpdateRate = 60.0f;
	ClientUpdateRate = 30.0f;

	// Initialize internal state
	LastServerUpdate = 0.0f;
	LastClientUpdate = 0.0f;
	CurrentBandwidthUsage = 0.0f;
}

void UReplicationManager::BeginDestroy()
{
	// Clean up registered actors
	RegisteredActors.Empty();
	AlwaysRelevantActors.Empty();
	
	Super::BeginDestroy();
}

void UReplicationManager::RegisterActor(AActor* Actor, EAlexanderReplicationPriority Priority, EReplicationFrequency Frequency)
{
	if (!Actor)
	{
		return;
	}

	// Check if already registered
	if (RegisteredActors.Contains(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor %s is already registered for replication"), *Actor->GetName());
		return;
	}

	// Create replication data
	FActorReplicationData ReplicationData;
	ReplicationData.Actor = Actor;
	ReplicationData.Priority = Priority;
	ReplicationData.Frequency = Frequency;
	ReplicationData.ReplicationInterval = GetReplicationInterval(Frequency);
	ReplicationData.LastReplicationTime = 0.0f;
	ReplicationData.bNeedsInitialReplication = true;
	ReplicationData.bForceReplication = false;

	// Register the actor
	RegisteredActors.Add(Actor, ReplicationData);

	UE_LOG(LogTemp, Warning, TEXT("Registered actor %s for replication (Priority: %s, Frequency: %s)"), 
		*Actor->GetName(), 
		*UEnum::GetValueAsString(Priority), 
		*UEnum::GetValueAsString(Frequency));
}

void UReplicationManager::UnregisterActor(AActor* Actor)
{
	if (!Actor || !RegisteredActors.Contains(Actor))
	{
		return;
	}

	RegisteredActors.Remove(Actor);
	AlwaysRelevantActors.Remove(Actor);

	UE_LOG(LogTemp, Warning, TEXT("Unregistered actor %s from replication"), *Actor->GetName());
}

bool UReplicationManager::IsActorRegistered(AActor* Actor) const
{
	return RegisteredActors.Contains(Actor);
}

void UReplicationManager::SetActorPriority(AActor* Actor, EAlexanderReplicationPriority Priority)
{
	if (FActorReplicationData* ReplicationData = RegisteredActors.Find(Actor))
	{
		ReplicationData->Priority = Priority;
	}
}

void UReplicationManager::SetActorFrequency(AActor* Actor, EReplicationFrequency Frequency)
{
	if (FActorReplicationData* ReplicationData = RegisteredActors.Find(Actor))
	{
		ReplicationData->Frequency = Frequency;
		ReplicationData->ReplicationInterval = GetReplicationInterval(Frequency);
	}
}

void UReplicationManager::ForceActorReplication(AActor* Actor)
{
	if (FActorReplicationData* ReplicationData = RegisteredActors.Find(Actor))
	{
		ReplicationData->bForceReplication = true;
	}
}

void UReplicationManager::ServerUpdate(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// Check if we should update based on server rate
	if (CurrentTime - LastServerUpdate < 1.0f / ServerUpdateRate)
	{
		return;
	}

	LastServerUpdate = CurrentTime;
	Stats.ActorsReplicatedThisFrame = 0;

	// Sort actors by priority
	SortActorsByPriority();

	// Process each registered actor
	for (auto& ActorPair : RegisteredActors)
	{
		AActor* Actor = ActorPair.Key;
		FActorReplicationData& ReplicationData = ActorPair.Value;

		if (ShouldReplicateActor(ReplicationData, CurrentTime))
		{
			ProcessActorReplication(Actor, ReplicationData, DeltaTime);
		}
	}

	// Update statistics
	Stats.TotalActorsReplicated += Stats.ActorsReplicatedThisFrame;
	LastServerUpdate = CurrentTime;
}

void UReplicationManager::ClientUpdate(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// Check if we should update based on client rate
	if (CurrentTime - LastClientUpdate < 1.0f / ClientUpdateRate)
	{
		return;
	}

	LastClientUpdate = CurrentTime;

	// Process incoming replication data
	// This would typically handle data received from the server
	// For now, simulate client-side updates
}

void UReplicationManager::UpdateActor(AActor* Actor)
{
	if (!Actor || !RegisteredActors.Contains(Actor))
	{
		return;
	}

	FActorReplicationData& ReplicationData = RegisteredActors[Actor];
	ReplicationData.bForceReplication = true;
}

void UReplicationManager::UpdateActorsInRadius(FVector Center, float Radius)
{
	for (auto& ActorPair : RegisteredActors)
	{
		AActor* Actor = ActorPair.Key;
		if (Actor)
		{
			float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
			if (Distance <= Radius)
			{
				UpdateActor(Actor);
			}
		}
	}
}

void UReplicationManager::UpdateCriticalActors()
{
	for (auto& ActorPair : RegisteredActors)
	{
		const FActorReplicationData& ReplicationData = ActorPair.Value;
		if (ReplicationData.Priority == EAlexanderReplicationPriority::Critical)
		{
			UpdateActor(ActorPair.Key);
		}
	}
}

TArray<uint8> UReplicationManager::GetActorState(AActor* Actor)
{
	if (!Actor)
	{
		return TArray<uint8>();
	}

	// Serialize actor state
	TArray<uint8> ActorState;
	
	// Basic transform data
	FVector Location = Actor->GetActorLocation();
	FRotator Rotation = Actor->GetActorRotation();
	FVector Scale = Actor->GetActorScale3D();

	// Serialize to byte array
	ActorState.Append((uint8*)&Location, sizeof(Location));
	ActorState.Append((uint8*)&Rotation, sizeof(Rotation));
	ActorState.Append((uint8*)&Scale, sizeof(Scale));

	// Add component data if needed
	// This would be expanded to include all relevant actor properties

	return ActorState;
}

bool UReplicationManager::SetActorState(AActor* Actor, const TArray<uint8>& State)
{
	if (!Actor || State.Num() == 0)
	{
		return false;
	}

	// Deserialize actor state
	if (State.Num() < sizeof(FVector) * 3)
	{
		return false;
	}

	int32 Offset = 0;
	
	// Extract transform data
	FVector Location;
	FMemory::Memcpy(&Location, State.GetData() + Offset, sizeof(Location));
	Offset += sizeof(Location);

	FRotator Rotation;
	FMemory::Memcpy(&Rotation, State.GetData() + Offset, sizeof(Rotation));
	Offset += sizeof(Rotation);

	FVector Scale;
	FMemory::Memcpy(&Scale, State.GetData() + Offset, sizeof(Scale));

	// Apply to actor
	Actor->SetActorLocationAndRotation(Location, Rotation);
	Actor->SetActorScale3D(Scale);

	return true;
}

bool UReplicationManager::HasActorChanged(AActor* Actor)
{
	if (!Actor || !RegisteredActors.Contains(Actor))
	{
		return false;
	}

	const FActorReplicationData& ReplicationData = RegisteredActors[Actor];
	TArray<uint8> CurrentState = GetActorState(Actor);

	// Compare with last known state
	if (ReplicationData.LastKnownState.Num() != CurrentState.Num())
	{
		return true;
	}

	return !FMemory::Memcmp(ReplicationData.LastKnownState.GetData(), CurrentState.GetData(), CurrentState.Num());
}

void UReplicationManager::MarkActorDirty(AActor* Actor)
{
	if (RegisteredActors.Contains(Actor))
	{
		RegisteredActors[Actor].bForceReplication = true;
	}
}

bool UReplicationManager::IsActorRelevantToClient(AActor* Actor, APlayerController* Client)
{
	if (!Actor || !Client)
	{
		return false;
	}

	// Always relevant actors
	if (AlwaysRelevantActors.Contains(Actor))
	{
		return true;
	}

	// Check distance-based relevance
	if (Client->GetPawn())
	{
		float Distance = FVector::Dist(Actor->GetActorLocation(), Client->GetPawn()->GetActorLocation());
		return Distance <= RelevanceDistance;
	}

	return false;
}

TArray<AActor*> UReplicationManager::GetRelevantActorsForClient(APlayerController* Client)
{
	TArray<AActor*> RelevantActors;

	for (const auto& ActorPair : RegisteredActors)
	{
		AActor* Actor = ActorPair.Key;
		if (IsActorRelevantToClient(Actor, Client))
		{
			RelevantActors.Add(Actor);
		}
	}

	return RelevantActors;
}

void UReplicationManager::SetRelevanceDistance(float Distance)
{
	RelevanceDistance = FMath::Max(0.0f, Distance);
}

void UReplicationManager::SetAlwaysRelevantActors(const TArray<AActor*>& Actors)
{
	AlwaysRelevantActors = Actors;
}

void UReplicationManager::OptimizeReplication()
{
	// Remove destroyed actors
	TArray<AActor*> ActorsToRemove;
	for (const auto& ActorPair : RegisteredActors)
	{
		AActor* Actor = ActorPair.Key;
		// Use IsValid() instead of IsValidLowLevel() which is deprecated
		if (!IsValid(Actor))
		{
			ActorsToRemove.Add(Actor);
		}
	}

	for (AActor* Actor : ActorsToRemove)
	{
		UnregisterActor(Actor);
	}

	// Prioritize actors based on relevance and priority
	PrioritizeActors();
}

void UReplicationManager::CompressReplicationData()
{
	if (!bEnableCompression)
	{
		return;
	}

	// This would implement data compression
	// For now, just log that compression is enabled
	UE_LOG(LogTemp, Warning, TEXT("Replication data compression enabled"));
}

void UReplicationManager::PrioritizeActors()
{
	// Sort actors by priority and relevance
	SortActorsByPriority();
}

void UReplicationManager::ThrottleReplication(float MaxBandwidth)
{
	MaxBandwidthPerSecond = MaxBandwidth;
}

FReplicationStats UReplicationManager::GetReplicationStats() const
{
	return Stats;
}

void UReplicationManager::ResetStats()
{
	Stats = FReplicationStats();
	CurrentBandwidthUsage = 0.0f;
}

void UReplicationManager::LogReplicationInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Replication Manager Statistics ==="));
	UE_LOG(LogTemp, Warning, TEXT("Registered Actors: %d"), RegisteredActors.Num());
	UE_LOG(LogTemp, Warning, TEXT("Total Replicated: %d"), Stats.TotalActorsReplicated);
	UE_LOG(LogTemp, Warning, TEXT("This Frame: %d"), Stats.ActorsReplicatedThisFrame);
	UE_LOG(LogTemp, Warning, TEXT("Bandwidth Used: %.2f KB"), Stats.BandwidthUsed / 1024.0f);
	UE_LOG(LogTemp, Warning, TEXT("Packets Sent: %d"), Stats.PacketsSent);
	UE_LOG(LogTemp, Warning, TEXT("Compression Ratio: %.2f%%"), Stats.CompressionRatio * 100.0f);
	UE_LOG(LogTemp, Warning, TEXT("====================================="));
}

int32 UReplicationManager::GetRegisteredActorCount() const
{
	return RegisteredActors.Num();
}

TArray<AActor*> UReplicationManager::GetRegisteredActors() const
{
	TArray<AActor*> Actors;
	RegisteredActors.GetKeys(Actors);
	return Actors;
}

void UReplicationManager::ProcessActorReplication(AActor* Actor, FActorReplicationData& ReplicationData, float DeltaTime)
{
	if (!Actor)
	{
		return;
	}

	float StartTime = GetWorld()->GetTimeSeconds();

	// Get current actor state
	TArray<uint8> CurrentState = GetActorState(Actor);

	// Check if state has changed or if forced replication is needed
	bool bStateChanged = HasActorChanged(Actor);
	bool bShouldReplicate = ReplicationData.bForceReplication || ReplicationData.bNeedsInitialReplication || bStateChanged;

	if (bShouldReplicate)
	{
		// Compress data if enabled
		TArray<uint8> DataToSend = CurrentState;
		if (bEnableCompression)
		{
			DataToSend = CompressData(CurrentState);
		}

		// Calculate data size
		int32 DataSize = DataToSend.Num();

		// Check bandwidth throttling
		if (CurrentBandwidthUsage + DataSize > MaxBandwidthPerSecond)
		{
			// Skip this actor due to bandwidth throttling
			return;
		}

		// This would send the data to relevant clients
		// For now, just simulate the send
		Stats.PacketsSent++;
		CurrentBandwidthUsage += DataSize;
		Stats.BandwidthUsed += DataSize;

		// Update replication data
		ReplicationData.LastReplicationTime = GetWorld()->GetTimeSeconds();
		ReplicationData.LastKnownState = CurrentState;
		ReplicationData.bNeedsInitialReplication = false;
		ReplicationData.bForceReplication = false;

		Stats.ActorsReplicatedThisFrame++;

		UE_LOG(LogTemp, Warning, TEXT("Replicated actor %s (%d bytes)"), *Actor->GetName(), DataSize);
	}

	// Update statistics
	float ReplicationTime = GetWorld()->GetTimeSeconds() - StartTime;
	UpdateStatistics(ReplicationTime, CurrentState.Num());
}

bool UReplicationManager::ShouldReplicateActor(const FActorReplicationData& ReplicationData, float CurrentTime)
{
	// Always replicate if forced
	if (ReplicationData.bForceReplication)
	{
		return true;
	}

	// Always replicate initial state
	if (ReplicationData.bNeedsInitialReplication)
	{
		return true;
	}

	// Check time-based replication
	float TimeSinceLastReplication = CurrentTime - ReplicationData.LastReplicationTime;
	return TimeSinceLastReplication >= ReplicationData.ReplicationInterval;
}

float UReplicationManager::GetReplicationInterval(EReplicationFrequency Frequency) const
{
	switch (Frequency)
	{
		case EReplicationFrequency::Never:
			return FLT_MAX;
		case EReplicationFrequency::Rarely:
			return 5.0f;
		case EReplicationFrequency::Occasionally:
			return 1.0f;
		case EReplicationFrequency::Frequently:
			return 0.1f;
		case EReplicationFrequency::Always:
			return 0.0f; // Every tick
		default:
			return 1.0f;
	}
}

TArray<uint8> UReplicationManager::CompressData(const TArray<uint8>& Data)
{
	// This would implement actual data compression
	// For now, just return the data as-is
	return Data;
}

TArray<uint8> UReplicationManager::DecompressData(const TArray<uint8>& Data)
{
	// This would implement actual data decompression
	// For now, just return the data as-is
	return Data;
}

void UReplicationManager::UpdateStatistics(float ReplicationTime, int32 DataSize)
{
	// Update average replication time
	static int32 UpdateCount = 0;
	UpdateCount++;
	
	Stats.AverageReplicationTime = (Stats.AverageReplicationTime * (UpdateCount - 1) + ReplicationTime) / UpdateCount;

	// Update compression ratio if compression is enabled
	if (bEnableCompression && DataSize > 0)
	{
		// This would calculate actual compression ratio
		Stats.CompressionRatio = 0.3f; // Placeholder: 30% compression
	}
}

void UReplicationManager::SortActorsByPriority()
{
	// This would sort the RegisteredActors map by priority
	// For now, the priority is handled during processing
}
