// Copyright Epic Games, Inc. All Rights Reserved.

#include "Asteroid.h"
#include "Spaceship.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Define tier colors for debug visualization
const FColor AAsteroid::TierColors[5] = {
	FColor::Red,      // Tier 0: Critical (full n-body)
	FColor::Orange,   // Tier 1: Active (simplified physics)
	FColor::Yellow,   // Tier 2: Distant (kinematic)
	FColor::Green,    // Tier 3: Far (minimal updates)
	FColor::Blue      // Tier 4: Procedural (GPU instanced)
};

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// Default to ballistic mode (tumbling through space)
	OrbitMode = EOrbitMode::Ballistic;

	// Initialize physics settings for typical asteroid
	Mass = 1.0e12f; // Will be recalculated from size and density
	Velocity = FVector::ZeroVector;
	bEnablePhysics = true;
	bEnableGravity = true;
	bEnableCollision = true;

	// Set up collision on mesh component (inherited from AOrbitalBody)
	if (MeshComponent)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComponent->SetNotifyRigidBodyCollision(true);
		MeshComponent->SetSimulatePhysics(false); // We handle physics in Tick
	}
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	// Calculate mass from size and density
	UpdateMassFromSize();

	// Find and cache player ship reference
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	CachedPlayerShip = Cast<ASpaceship>(PlayerPawn);

	// Bind collision event if mesh component exists
	if (MeshComponent)
	{
		MeshComponent->OnComponentHit.AddDynamic(this, &AAsteroid::OnAsteroidHit);
	}

	// Apply initial tier physics settings
	ApplyTierPhysicsSettings();

	// Mark as active
	bIsActive = true;

	UE_LOG(LogTemp, Log, TEXT("Asteroid %d spawned at %s with mass %.2e kg (Radius: %.2f m, Density: %.2f kg/m³)"),
		AsteroidID, *GetActorLocation().ToString(), Mass, AsteroidRadius, Density);
}

void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only tick if active
	if (!bIsActive)
	{
		return;
	}

	// Update based on current physics tier
	switch (CurrentTier)
	{
		case 0:
			UpdateTier0_Critical(DeltaTime);
			break;
		case 1:
			UpdateTier1_Active(DeltaTime);
			break;
		case 2:
			UpdateTier2_Distant(DeltaTime);
			break;
		case 3:
			UpdateTier3_Far(DeltaTime);
			break;
		case 4:
			// Tier 4 is GPU instanced - no per-object tick
			break;
		default:
			// Fallback to simplified physics
			UpdateTier1_Active(DeltaTime);
			break;
	}

	// Draw debug visualization if enabled
	if (bShowTierDebug)
	{
		DrawAsteroidDebug();
	}
}

void AAsteroid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteroid, AsteroidID);
}

// ============================================================================
// MASS CALCULATION
// ============================================================================

void AAsteroid::UpdateMassFromSize()
{
	// Calculate volume of sphere: V = (4/3) * π * r³
	// Convert radius from meters to cm for consistency with Unreal units
	const float RadiusInCm = AsteroidRadius * 100.0f; // m to cm
	const float Volume = (4.0f / 3.0f) * PI * FMath::Pow(RadiusInCm, 3.0f); // cm³

	// Calculate mass: M = ρ * V
	// Density is in kg/m³, Volume is in cm³, so convert
	const float VolumeInM3 = Volume / 1000000.0f; // cm³ to m³
	Mass = Density * VolumeInM3; // kg

	// Update parent's Radius property (in km for consistency with celestial bodies)
	Radius = AsteroidRadius / 1000.0f; // meters to km

	// Update standard gravitational parameter (μ = G * M)
	const float G = 6.67430e-11f; // m³/(kg·s²)
	StandardGravitationalParameter = G * Mass;

	UE_LOG(LogTemp, Verbose, TEXT("Asteroid mass calculated: %.2e kg (Radius: %.2f m, Density: %.2f kg/m³)"),
		Mass, AsteroidRadius, Density);
}

// ============================================================================
// TIER MANAGEMENT
// ============================================================================

void AAsteroid::SetPhysicsTier(int32 NewTier)
{
	// Clamp tier to valid range [0-4]
	NewTier = FMath::Clamp(NewTier, 0, 4);

	if (CurrentTier != NewTier)
	{
		int32 OldTier = CurrentTier;
		CurrentTier = NewTier;

		// Apply new tier physics settings
		ApplyTierPhysicsSettings();

		UE_LOG(LogTemp, Verbose, TEXT("Asteroid %d tier changed: %d -> %d (Distance: %.2f m)"),
			AsteroidID, OldTier, NewTier, CachedDistanceToPlayer);
	}
}

float AAsteroid::GetDistanceToPlayer() const
{
	if (!CachedPlayerShip)
	{
		return CachedDistanceToPlayer;
	}

	return FVector::Dist(GetActorLocation(), CachedPlayerShip->GetActorLocation());
}

void AAsteroid::UpdateTierBasedOnDistance(float DistanceToPlayer)
{
	CachedDistanceToPlayer = DistanceToPlayer;

	// Tier thresholds (in meters)
	// These will eventually be driven by AsteroidFieldManager
	const float Tier0Threshold = 1000.0f;    // 0-1km: Critical
	const float Tier1Threshold = 50000.0f;   // 1-50km: Active
	const float Tier2Threshold = 200000.0f;  // 50-200km: Distant
	const float Tier3Threshold = 500000.0f;  // 200-500km: Far
	// Beyond 500km: Tier 4 (Procedural)

	// Determine new tier with hysteresis (10% buffer to prevent oscillation)
	const float Hysteresis = 1.1f;
	int32 NewTier = CurrentTier;

	if (DistanceToPlayer < Tier0Threshold)
	{
		NewTier = 0;
	}
	else if (DistanceToPlayer < Tier1Threshold * (CurrentTier == 0 ? Hysteresis : 1.0f))
	{
		NewTier = 1;
	}
	else if (DistanceToPlayer < Tier2Threshold * (CurrentTier == 1 ? Hysteresis : 1.0f))
	{
		NewTier = 2;
	}
	else if (DistanceToPlayer < Tier3Threshold * (CurrentTier == 2 ? Hysteresis : 1.0f))
	{
		NewTier = 3;
	}
	else
	{
		NewTier = 4;
	}

	SetPhysicsTier(NewTier);
}

void AAsteroid::ApplyTierPhysicsSettings()
{
	// PHASE 1: Basic settings - complex physics deferred to Phase 2
	switch (CurrentTier)
	{
		case 0: // Critical - Full physics
			bEnablePhysics = true;
			bEnableGravity = true;
			bEnableCollision = true;
			PrimaryActorTick.bCanEverTick = true;
			break;

		case 1: // Active - Simplified physics
			bEnablePhysics = true;
			bEnableGravity = true;
			bEnableCollision = true;
			PrimaryActorTick.bCanEverTick = true;
			break;

		case 2: // Distant - Kinematic only
			bEnablePhysics = true;
			bEnableGravity = false;
			bEnableCollision = false;
			PrimaryActorTick.bCanEverTick = true;
			break;

		case 3: // Far - Minimal updates
			bEnablePhysics = false;
			bEnableGravity = false;
			bEnableCollision = false;
			PrimaryActorTick.bCanEverTick = true;
			break;

		case 4: // Procedural - No updates (GPU instanced)
			bEnablePhysics = false;
			bEnableGravity = false;
			bEnableCollision = false;
			PrimaryActorTick.bCanEverTick = false;
			break;
	}
}

// ============================================================================
// TIER-SPECIFIC PHYSICS UPDATES
// ============================================================================

void AAsteroid::UpdateTier0_Critical(float DeltaTime)
{
	// PHASE 1: Stub - use parent class physics
	// PHASE 2: Implement full n-body gravity with nearby asteroids
	// For now, let AOrbitalBody::Tick handle physics
}

void AAsteroid::UpdateTier1_Active(float DeltaTime)
{
	// PHASE 1: Stub - use parent class physics
	// PHASE 2: Implement simplified gravity (only from major bodies)
	// For now, let AOrbitalBody::Tick handle physics
}

void AAsteroid::UpdateTier2_Distant(float DeltaTime)
{
	// PHASE 1: Simple kinematic motion
	// PHASE 2: Pre-calculated orbit paths

	if (OrbitTarget.IsValid())
	{
		// Simple circular orbit approximation
		FVector ToTarget = OrbitTarget->GetActorLocation() - GetActorLocation();
		float Distance = ToTarget.Size();
		FVector OrbitDirection = ToTarget.GetSafeNormal();
		FVector Tangent = FVector::CrossProduct(OrbitDirection, FVector::UpVector).GetSafeNormal();

		// Move along tangent (simplified orbit)
		FVector NewLocation = GetActorLocation() + Tangent * OrbitSpeed * DeltaTime;
		SetActorLocation(NewLocation);
	}
}

void AAsteroid::UpdateTier3_Far(float DeltaTime)
{
	// PHASE 1: No physics updates - just maintain position
	// PHASE 2: Occasional position snap to orbit path
	// Minimal work - just exist for visibility
}

// ============================================================================
// COLLISION & FRAGMENTATION
// ============================================================================

void AAsteroid::OnAsteroidHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsActive || !OtherActor)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Asteroid %d hit by %s at %s (Impulse: %s)"),
		AsteroidID, *OtherActor->GetName(), *Hit.ImpactPoint.ToString(), *NormalImpulse.ToString());

	// Check if we should fragment
	const float ImpactSpeed = NormalImpulse.Size() / Mass; // v = impulse / mass
	const float FragmentThreshold = 100.0f; // m/s

	if (CanFragment() && ImpactSpeed > FragmentThreshold)
	{
		Fragment(Hit.ImpactPoint, NormalImpulse / Mass);
	}
}

bool AAsteroid::CanFragment() const
{
	return AsteroidRadius >= MinFragmentRadius;
}

void AAsteroid::Fragment(FVector ImpactPoint, FVector ImpactVelocity)
{
	UE_LOG(LogTemp, Warning, TEXT("Asteroid %d fragmenting at %s (Radius: %.2f m)"),
		AsteroidID, *ImpactPoint.ToString(), AsteroidRadius);

	// PHASE 1: Simple stub - just destroy asteroid
	// PHASE 2: Spawn fragment children with shared velocity

	// For now, just deactivate
	DeactivateToPool();

	// TODO Phase 2: Spawn FragmentCount smaller asteroids
	// Each fragment should have radius = AsteroidRadius / 2
	// Distribute momentum among fragments
}

// ============================================================================
// POOLING SUPPORT
// ============================================================================

void AAsteroid::ActivateFromPool(FVector SpawnLocation, FVector InitialVelocity, float Size)
{
	// Reset state
	AsteroidRadius = Size;
	UpdateMassFromSize();

	// Set position and velocity
	SetActorLocation(SpawnLocation);
	Velocity = InitialVelocity;

	// Reset tier
	CurrentTier = 1;
	ApplyTierPhysicsSettings();

	// Show mesh
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Mark active
	bIsActive = true;

	UE_LOG(LogTemp, Log, TEXT("Asteroid %d activated from pool at %s (Radius: %.2f m)"),
		AsteroidID, *SpawnLocation.ToString(), AsteroidRadius);
}

void AAsteroid::DeactivateToPool()
{
	// Hide mesh
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Reset velocity
	Velocity = FVector::ZeroVector;

	// Mark inactive
	bIsActive = false;

	// Move far away (outside play area)
	SetActorLocation(FVector(0, 0, -1000000.0f));

	UE_LOG(LogTemp, Verbose, TEXT("Asteroid %d deactivated to pool"), AsteroidID);
}

// ============================================================================
// DEBUG VISUALIZATION
// ============================================================================

void AAsteroid::DrawAsteroidDebug() const
{
	if (!GetWorld() || !bIsActive)
	{
		return;
	}

	FVector Location = GetActorLocation();

	// Draw sphere representing asteroid
	FColor TierColor = TierColors[FMath::Clamp(CurrentTier, 0, 4)];
	DrawDebugSphere(GetWorld(), Location, AsteroidRadius * 100.0f, 12, TierColor, false, -1.0f, 0, 2.0f);

	// Draw tier label
	FString TierLabel = FString::Printf(TEXT("T%d: %.1fkm"), CurrentTier, CachedDistanceToPlayer / 1000.0f);
	DrawDebugString(GetWorld(), Location + FVector(0, 0, AsteroidRadius * 120.0f), TierLabel, nullptr, TierColor, 0.0f, true);

	// Draw velocity vector
	if (!Velocity.IsNearlyZero())
	{
		DrawDebugLine(GetWorld(), Location, Location + Velocity * 10.0f, FColor::Cyan, false, -1.0f, 0, 3.0f);
	}
}
