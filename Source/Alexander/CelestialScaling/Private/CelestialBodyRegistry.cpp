// Copyright Epic Games, Inc. All Rights Reserved.

#include "CelestialBodyRegistry.h"
#include "CelestialBodyComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void UCelestialBodyRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisteredBodies.Empty();
	BodyLookup.Empty();
	bAutoUpdateEnabled = true;
	AutoUpdateFrequency = 0.1f; // Update every 0.1 seconds
	TimeSinceLastUpdate = 0.0f;
	MaxBodiesPerFrame = 100;
	bEnableDebugLogging = false;

	UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Initialized"));
}

void UCelestialBodyRegistry::Deinitialize()
{
	// Clear all registered bodies
	{
		FScopeLock Lock(&RegistryLock);
		RegisteredBodies.Empty();
		BodyLookup.Empty();
	}

	UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Deinitialized"));

	Super::Deinitialize();
}

void UCelestialBodyRegistry::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (bEnableDebugLogging)
	{
		LogRegistryStatistics();
	}

	UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: World begin play - %d bodies registered"),
		RegisteredBodies.Num());
}

// ========== Body Registration (Thread-Safe) ==========

void UCelestialBodyRegistry::RegisterCelestialBody(UCelestialBodyComponent* Body)
{
	if (!IsValidBodyComponent(Body))
	{
		UE_LOG(LogTemp, Warning, TEXT("CelestialBodyRegistry: Cannot register invalid body component"));
		return;
	}

	FScopeLock Lock(&RegistryLock);

	// Check if already registered
	if (RegisteredBodies.Contains(Body))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("CelestialBodyRegistry: Body '%s' already registered"),
				*Body->GetBodyName().ToString());
		}
		return;
	}

	// Add to array and lookup map
	RegisteredBodies.Add(Body);
	BodyLookup.Add(Body->GetBodyName(), Body);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Registered body '%s' (Total: %d)"),
			*Body->GetBodyName().ToString(), RegisteredBodies.Num());
	}
}

void UCelestialBodyRegistry::UnregisterCelestialBody(UCelestialBodyComponent* Body)
{
	if (!Body)
	{
		return;
	}

	FScopeLock Lock(&RegistryLock);

	// Remove from lookup map
	BodyLookup.Remove(Body->GetBodyName());

	// Remove from array
	int32 RemovedCount = RegisteredBodies.Remove(Body);

	if (bEnableDebugLogging && RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Unregistered body '%s' (Total: %d)"),
			*Body->GetBodyName().ToString(), RegisteredBodies.Num());
	}
}

// ========== Body Lookup ==========

TArray<UCelestialBodyComponent*> UCelestialBodyRegistry::GetAllCelestialBodies() const
{
	FScopeLock Lock(&RegistryLock);
	return RegisteredBodies;
}

UCelestialBodyComponent* UCelestialBodyRegistry::FindBodyByName(FName BodyName) const
{
	FScopeLock Lock(&RegistryLock);

	UCelestialBodyComponent* const* FoundBody = BodyLookup.Find(BodyName);
	return FoundBody ? *FoundBody : nullptr;
}

TArray<UCelestialBodyComponent*> UCelestialBodyRegistry::GetBodiesInRange(const FVector& ReferencePoint, float MaxDistance) const
{
	FScopeLock Lock(&RegistryLock);

	TArray<UCelestialBodyComponent*> BodiesInRange;
	float MaxDistanceSquared = MaxDistance * MaxDistance;

	for (UCelestialBodyComponent* Body : RegisteredBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		FVector BodyLocation = Body->GetOwner()->GetActorLocation();
		float DistanceSquared = FVector::DistSquared(ReferencePoint, BodyLocation);

		if (DistanceSquared <= MaxDistanceSquared)
		{
			BodiesInRange.Add(Body);
		}
	}

	return BodiesInRange;
}

TArray<UCelestialBodyComponent*> UCelestialBodyRegistry::GetNearestBodies(const FVector& ReferencePoint, int32 Count) const
{
	FScopeLock Lock(&RegistryLock);

	// Create array with distance data
	TArray<TPair<float, UCelestialBodyComponent*>> BodiesWithDistance;
	BodiesWithDistance.Reserve(RegisteredBodies.Num());

	for (UCelestialBodyComponent* Body : RegisteredBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		FVector BodyLocation = Body->GetOwner()->GetActorLocation();
		float DistanceSquared = FVector::DistSquared(ReferencePoint, BodyLocation);
		BodiesWithDistance.Add(TPair<float, UCelestialBodyComponent*>(DistanceSquared, Body));
	}

	// Sort by distance
	BodiesWithDistance.Sort([](const TPair<float, UCelestialBodyComponent*>& A, const TPair<float, UCelestialBodyComponent*>& B)
	{
		return A.Key < B.Key;
	});

	// Extract the nearest N bodies
	TArray<UCelestialBodyComponent*> NearestBodies;
	int32 NumToReturn = FMath::Min(Count, BodiesWithDistance.Num());

	for (int32 i = 0; i < NumToReturn; ++i)
	{
		NearestBodies.Add(BodiesWithDistance[i].Value);
	}

	return NearestBodies;
}

// ========== Universe Translation ==========

void UCelestialBodyRegistry::UpdateAllBodyPositions(const FVector& OffsetDelta)
{
	FScopeLock Lock(&RegistryLock);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Updating positions for %d bodies with offset %s"),
			RegisteredBodies.Num(), *OffsetDelta.ToString());
	}

	// Update all registered bodies
	int32 UpdatedCount = 0;
	for (UCelestialBodyComponent* Body : RegisteredBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		// Apply position offset
		Body->ApplyPositionOffset(OffsetDelta);
		UpdatedCount++;
	}

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("CelestialBodyRegistry: Updated %d body positions"), UpdatedCount);
	}

	// Replicate to clients if this is the server
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
	{
		MulticastUpdateBodyPositions(OffsetDelta);
	}
}

void UCelestialBodyRegistry::UpdateAllBodyScales(const FVector& PlayerPosition)
{
	FScopeLock Lock(&RegistryLock);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("CelestialBodyRegistry: Updating scales for %d bodies from player position %s"),
			RegisteredBodies.Num(), *PlayerPosition.ToString());
	}

	// Update scales based on distance from player
	int32 UpdatedCount = 0;
	for (UCelestialBodyComponent* Body : RegisteredBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		// Calculate distance to player
		FVector BodyLocation = Body->GetOwner()->GetActorLocation();
		float Distance = FVector::Dist(PlayerPosition, BodyLocation);

		// Update body scale based on distance
		Body->UpdateScaleForDistance(Distance);
		UpdatedCount++;
	}

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("CelestialBodyRegistry: Updated %d body scales"), UpdatedCount);
	}
}

// ========== Optimization ==========

void UCelestialBodyRegistry::SortBodiesByDistance(const FVector& ReferencePoint)
{
	FScopeLock Lock(&RegistryLock);

	RegisteredBodies.Sort([&ReferencePoint](const UCelestialBodyComponent& A, const UCelestialBodyComponent& B)
	{
		if (!IsValid(&A) || !IsValid(&B))
		{
			return false;
		}

		float DistA = FVector::DistSquared(ReferencePoint, A.GetOwner()->GetActorLocation());
		float DistB = FVector::DistSquared(ReferencePoint, B.GetOwner()->GetActorLocation());
		return DistA < DistB;
	});

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("CelestialBodyRegistry: Sorted %d bodies by distance from %s"),
			RegisteredBodies.Num(), *ReferencePoint.ToString());
	}
}

void UCelestialBodyRegistry::ClearAllBodies()
{
	FScopeLock Lock(&RegistryLock);

	int32 ClearedCount = RegisteredBodies.Num();
	RegisteredBodies.Empty();
	BodyLookup.Empty();

	UE_LOG(LogTemp, Warning, TEXT("CelestialBodyRegistry: Cleared %d bodies from registry"), ClearedCount);
}

// ========== Network Replication ==========

void UCelestialBodyRegistry::MulticastUpdateBodyPositions_Implementation(const FVector& OffsetDelta)
{
	// On clients, apply the position update
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		FScopeLock Lock(&RegistryLock);

		for (UCelestialBodyComponent* Body : RegisteredBodies)
		{
			if (IsValid(Body))
			{
				Body->ApplyPositionOffset(OffsetDelta);
			}
		}
	}
}

// ========== Internal Methods ==========

bool UCelestialBodyRegistry::IsValidBodyComponent(UCelestialBodyComponent* Body) const
{
	if (!Body)
	{
		return false;
	}

	if (!IsValid(Body))
	{
		return false;
	}

	if (Body->GetBodyName().IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("CelestialBodyRegistry: Body component has no name"));
		return false;
	}

	return true;
}

void UCelestialBodyRegistry::LogRegistryStatistics() const
{
	FScopeLock Lock(&RegistryLock);

	UE_LOG(LogTemp, Log, TEXT("=== CelestialBodyRegistry Statistics ==="));
	UE_LOG(LogTemp, Log, TEXT("Total Bodies: %d"), RegisteredBodies.Num());
	UE_LOG(LogTemp, Log, TEXT("Auto Update: %s"), bAutoUpdateEnabled ? TEXT("Enabled") : TEXT("Disabled"));
	UE_LOG(LogTemp, Log, TEXT("Update Frequency: %.2f seconds"), AutoUpdateFrequency);
	UE_LOG(LogTemp, Log, TEXT("Max Bodies Per Frame: %d"), MaxBodiesPerFrame);

	// List all registered bodies
	if (RegisteredBodies.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Registered Bodies:"));
		for (int32 i = 0; i < RegisteredBodies.Num(); ++i)
		{
			UCelestialBodyComponent* Body = RegisteredBodies[i];
			if (IsValid(Body))
			{
				UE_LOG(LogTemp, Log, TEXT("  [%d] %s at %s"),
					i, *Body->GetBodyName().ToString(), *Body->GetOwner()->GetActorLocation().ToString());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}
