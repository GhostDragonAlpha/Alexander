// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerOriginManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "CelestialBodyComponent.h"

// Forward declaration - will be implemented by Agent 2
// #include "CelestialScaling/Public/CelestialBodyRegistry.h"

UPlayerOriginManager::UPlayerOriginManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Enable replication
	SetIsReplicatedByDefault(true);

	// Default values
	VirtualPosition = FPlayerOriginVirtualPosition();
	LastRecenterPosition = FVector::ZeroVector;
	TotalDistanceTraveled = 0.0f;

	PrecisionThreshold = 1000000.0f; // 10 km
	bAutoRecenterUniverse = true;
	PrecisionCheckInterval = 1.0f; // Check every second

	bShowDebugInfo = false;
	BodyRegistry = nullptr;
	PrecisionCheckTimer = 0.0f;
	RecenterCount = 0;
	bIsLocalPlayer = false;
}

void UPlayerOriginManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate virtual position (server authoritative)
	DOREPLIFETIME(UPlayerOriginManager, VirtualPosition);
	DOREPLIFETIME(UPlayerOriginManager, TotalDistanceTraveled);
}

void UPlayerOriginManager::BeginPlay()
{
	Super::BeginPlay();

	// Determine if this is the local player
	DetermineLocalPlayer();

	// Initialize virtual position from current world position
	if (AActor* Owner = GetOwner())
	{
		VirtualPosition = FPlayerOriginVirtualPosition::FromWorldPosition(Owner->GetActorLocation());
		LastRecenterPosition = Owner->GetActorLocation();
	}

	// Get reference to body registry (when Agent 2's work is complete)
	// BodyRegistry = GetWorld()->GetSubsystem<UCelestialBodyRegistry>();

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerOriginManager: Initialized for %s (Local: %s)"),
			*GetOwner()->GetName(),
			bIsLocalPlayer ? TEXT("Yes") : TEXT("No"));
	}
}

void UPlayerOriginManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UPlayerOriginManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update precision monitoring
	if (bAutoRecenterUniverse)
	{
		UpdatePrecisionMonitoring(DeltaTime);
	}

	// Debug visualization
	if (bShowDebugInfo)
	{
		DrawDebugVisualization();
	}
}

// ============================================================================
// MOVEMENT FUNCTIONS
// ============================================================================

void UPlayerOriginManager::ServerMovePlayer_Implementation(const FVector& MovementDelta)
{
	// Server executes the movement
	MovePlayer(MovementDelta);
}

bool UPlayerOriginManager::ServerMovePlayer_Validate(const FVector& MovementDelta)
{
	// Validate movement delta isn't impossibly large
	// This prevents cheating/exploits
	const float MaxMovementPerCall = 1000000.0f; // 10 km max per call
	return MovementDelta.SizeSquared() <= (MaxMovementPerCall * MaxMovementPerCall);
}

void UPlayerOriginManager::MovePlayer(const FVector& MovementDelta)
{
	if (MovementDelta.IsNearlyZero())
	{
		return;
	}

	// Update virtual position
	VirtualPosition.AddDelta(MovementDelta);

	// Update total distance traveled
	TotalDistanceTraveled += MovementDelta.Size() * CmToKm;

	// Check if universe needs recentering
	if (bAutoRecenterUniverse && ShouldRecenterUniverse())
	{
		RecenterUniverse();
	}

	// Notify subsystems of position change
	NotifyPositionChange();

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("PlayerOriginManager: Moved by %.2f cm, Total: %.2f km"),
			MovementDelta.Size(), TotalDistanceTraveled);
	}
}

void UPlayerOriginManager::SetVirtualPosition(const FPlayerOriginVirtualPosition& NewPosition)
{
	// Server only
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	VirtualPosition = NewPosition;
	NotifyPositionChange();
}

// ============================================================================
// UNIVERSE TRANSLATION
// ============================================================================

void UPlayerOriginManager::TranslateUniverse(const FVector& OffsetDelta)
{
	if (OffsetDelta.IsNearlyZero())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Gather all actors that need translation
	TArray<AActor*> ActorsToTranslate;
	GatherActorsForTranslation(ActorsToTranslate);

	// Translate each actor
	for (AActor* Actor : ActorsToTranslate)
	{
		if (Actor)
		{
			FVector NewLocation = Actor->GetActorLocation() + OffsetDelta;
			Actor->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// Update last recenter position
	LastRecenterPosition = GetOwner()->GetActorLocation();

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerOriginManager: Translated universe by %.2f cm (%d actors)"),
			OffsetDelta.Size(), ActorsToTranslate.Num());
	}
}

bool UPlayerOriginManager::ShouldRecenterUniverse() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Check distance from world origin
	float DistanceFromOrigin = Owner->GetActorLocation().Size();
	return DistanceFromOrigin > PrecisionThreshold;
}

void UPlayerOriginManager::RecenterUniverse()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Calculate offset to move player back to origin
	FVector PlayerLocation = Owner->GetActorLocation();
	FVector OffsetDelta = -PlayerLocation; // Move everything so player is at origin

	// Perform translation
	TranslateUniverse(OffsetDelta);

	// Increment recenter count
	RecenterCount++;

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerOriginManager: Universe recentered (#%d) - Player was %.2f km from origin"),
			RecenterCount, PlayerLocation.Size() * CmToKm);
	}
}

float UPlayerOriginManager::CalculatePrecisionError() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}

	// Calculate approximate precision loss based on distance from origin
	float DistanceFromOrigin = Owner->GetActorLocation().Size();

	// IEEE 754 single precision has ~7 decimal digits of precision
	// Error increases with magnitude of the number
	float ApproximateError = FMath::Max(1.0f, DistanceFromOrigin / 1000000.0f); // Error in cm

	return ApproximateError;
}

// ============================================================================
// BLUEPRINT ACCESS
// ============================================================================

float UPlayerOriginManager::GetDistanceFromOriginKm() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}

	return Owner->GetActorLocation().Size() * CmToKm;
}

// ============================================================================
// DEBUG & UTILITIES
// ============================================================================

void UPlayerOriginManager::DrawDebugVisualization()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector PlayerLocation = Owner->GetActorLocation();

	// Draw precision threshold sphere at origin
	DrawDebugSphere(World, FVector::ZeroVector, PrecisionThreshold, 32, FColor::Yellow, false, -1.0f, 0, 10.0f);

	// Draw line from origin to player
	DrawDebugLine(World, FVector::ZeroVector, PlayerLocation, FColor::Green, false, -1.0f, 0, 5.0f);

	// Draw debug text
	FString DebugText = FString::Printf(
		TEXT("Virtual Position:\nSector: (%d, %d, %d)\nLocal: (%.1f, %.1f, %.1f)\nDistance: %.2f km\nPrecision Error: %.2f cm\nRecenters: %d"),
		VirtualPosition.SectorCoords.X, VirtualPosition.SectorCoords.Y, VirtualPosition.SectorCoords.Z,
		VirtualPosition.LocalOffset.X, VirtualPosition.LocalOffset.Y, VirtualPosition.LocalOffset.Z,
		GetDistanceFromOriginKm(),
		CalculatePrecisionError(),
		RecenterCount
	);

	DrawDebugString(World, PlayerLocation + FVector(0, 0, 200.0f), DebugText, nullptr, FColor::White, 0.0f, true);
}

FString UPlayerOriginManager::GetStatusInfo() const
{
	AActor* Owner = GetOwner();
	FString OwnerName = Owner ? Owner->GetName() : TEXT("None");

	return FString::Printf(
		TEXT("PlayerOriginManager: %s\nSector: (%d, %d, %d)\nLocal Offset: (%.2f, %.2f, %.2f)\nDistance from Origin: %.2f km\nTotal Traveled: %.2f km\nPrecision Error: %.2f cm\nRecenter Count: %d"),
		*OwnerName,
		VirtualPosition.SectorCoords.X, VirtualPosition.SectorCoords.Y, VirtualPosition.SectorCoords.Z,
		VirtualPosition.LocalOffset.X, VirtualPosition.LocalOffset.Y, VirtualPosition.LocalOffset.Z,
		GetDistanceFromOriginKm(),
		TotalDistanceTraveled,
		CalculatePrecisionError(),
		RecenterCount
	);
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

void UPlayerOriginManager::DetermineLocalPlayer()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		bIsLocalPlayer = false;
		return;
	}

	// Check if owner is possessed by local player controller
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
		bIsLocalPlayer = (PC && PC->IsLocalController());
	}
	else
	{
		bIsLocalPlayer = false;
	}
}

void UPlayerOriginManager::UpdatePrecisionMonitoring(float DeltaTime)
{
	PrecisionCheckTimer += DeltaTime;

	if (PrecisionCheckTimer >= PrecisionCheckInterval)
	{
		PrecisionCheckTimer = 0.0f;

		// Check if recentering is needed
		if (ShouldRecenterUniverse())
		{
			// Only recenter if we're the server/authority
			if (GetOwnerRole() == ROLE_Authority)
			{
				RecenterUniverse();
			}
		}
	}
}

void UPlayerOriginManager::NotifyPositionChange()
{
	// TODO: Notify CelestialBodyRegistry of position change (Agent 2's implementation)
	// if (BodyRegistry)
	// {
	//     BodyRegistry->OnPlayerPositionChanged(VirtualPosition);
	// }

	// Update celestial body scaling based on new position
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector WorldPosition = VirtualPosition.ToWorldPosition();

	// Update all celestial bodies with new player position
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			UCelestialBodyComponent* CelestialBody = Actor->FindComponentByClass<UCelestialBodyComponent>();
			if (CelestialBody)
			{
				CelestialBody->UpdateScale(WorldPosition);
			}
		}
	}
}

void UPlayerOriginManager::GatherActorsForTranslation(TArray<AActor*>& OutActors)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	OutActors.Empty();

	// Get all actors in the world
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		// Skip certain actor types that shouldn't be translated
		// (e.g., world settings, persistent level actors, etc.)
		if (Actor->IsA<AWorldSettings>())
		{
			continue;
		}

		// Skip the player itself (we're centering on them)
		if (Actor == GetOwner())
		{
			continue;
		}

		// Include all movable actors
		if (Actor->GetRootComponent() && Actor->GetRootComponent()->Mobility == EComponentMobility::Movable)
		{
			OutActors.Add(Actor);
		}

		// Include actors with celestial body components
		if (Actor->FindComponentByClass<UCelestialBodyComponent>())
		{
			OutActors.AddUnique(Actor);
		}
	}
}
