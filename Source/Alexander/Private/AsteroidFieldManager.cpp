// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsteroidFieldManager.h"
#include "Asteroid.h"
#include "Spaceship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

AAsteroidFieldManager::AAsteroidFieldManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Set default spawn location
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AAsteroidFieldManager::BeginPlay()
{
	Super::BeginPlay();

	// Cache player ship reference
	CachePlayerShip();

	// Log initialization
	UE_LOG(LogTemp, Log, TEXT("AsteroidFieldManager initialized at %s (Field Radius: %.2f km)"),
		*GetActorLocation().ToString(), FieldRadius / 1000.0f);
}

void AAsteroidFieldManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update tier assignments periodically (not every frame)
	TimeSinceLastTierUpdate += DeltaTime;
	if (TimeSinceLastTierUpdate >= TierUpdateInterval)
	{
		UpdateAsteroidTiers();
		TimeSinceLastTierUpdate = 0.0f;
	}

	// Draw debug visualization if enabled
	if (bShowDebug)
	{
		DrawFieldDebug();
	}

	// Performance profiling
	if (bEnablePerformanceProfiling)
	{
		// Basic frame time tracking
		// More detailed profiling will be added in Phase 2
		LastFrameTimeMS = DeltaTime * 1000.0f;
	}
}

// ============================================================================
// FIELD SPAWNING
// ============================================================================

void AAsteroidFieldManager::SpawnAsteroidField(int32 AsteroidCount, float FieldRadiusOverride)
{
	// Use override radius if provided
	if (FieldRadiusOverride > 0.0f)
	{
		FieldRadius = FieldRadiusOverride;
	}

	// Clear existing field
	ClearAsteroidField();

	// Cache player ship if not already cached
	if (!CachedPlayerShip)
	{
		CachePlayerShip();
	}

	// Clamp asteroid count to max
	AsteroidCount = FMath::Min(AsteroidCount, MaxActiveAsteroids);

	UE_LOG(LogTemp, Warning, TEXT("Spawning %d asteroids in %.2f km radius field..."),
		AsteroidCount, FieldRadius / 1000.0f);

	// Spawn asteroids
	for (int32 i = 0; i < AsteroidCount; i++)
	{
		// Generate random properties
		FVector Location = GenerateRandomPositionInField();
		float Size = GenerateRandomSize();
		FVector Velocity = GenerateRandomVelocity();

		// Spawn asteroid
		AAsteroid* NewAsteroid = SpawnAsteroid(Location, Size, Velocity);

		if (NewAsteroid)
		{
			// Assign unique ID
			NewAsteroid->AsteroidID = NextAsteroidID++;

			// Set random density
			NewAsteroid->Density = GenerateRandomDensity();

			// Add to active list
			ActiveAsteroids.Add(NewAsteroid);
		}
	}

	// Initial tier assignment
	UpdateAsteroidTiers();

	UE_LOG(LogTemp, Warning, TEXT("Asteroid field spawned: %d asteroids created"), ActiveAsteroids.Num());
}

AAsteroid* AAsteroidFieldManager::SpawnAsteroid(FVector Location, float Radius, FVector InitialVelocity)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// Set spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn asteroid actor
	AAsteroid* NewAsteroid = GetWorld()->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

	if (NewAsteroid)
	{
		// Set properties
		NewAsteroid->AsteroidRadius = Radius;
		NewAsteroid->Velocity = InitialVelocity;
		NewAsteroid->bIsActive = true;

		// Recalculate mass based on size
		NewAsteroid->UpdateMassFromSize();

		UE_LOG(LogTemp, Verbose, TEXT("Spawned asteroid at %s (Radius: %.2f m, Mass: %.2e kg)"),
			*Location.ToString(), Radius, NewAsteroid->Mass);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn asteroid at %s"), *Location.ToString());
	}

	return NewAsteroid;
}

void AAsteroidFieldManager::ClearAsteroidField()
{
	UE_LOG(LogTemp, Log, TEXT("Clearing asteroid field (%d asteroids)..."), ActiveAsteroids.Num());

	// Destroy all active asteroids
	for (AAsteroid* Asteroid : ActiveAsteroids)
	{
		if (Asteroid && IsValid(Asteroid))
		{
			Asteroid->Destroy();
		}
	}

	// Clear arrays
	ActiveAsteroids.Empty();
	Tier0Asteroids.Empty();
	Tier1Asteroids.Empty();
	Tier2Asteroids.Empty();
	Tier3Asteroids.Empty();
	Tier4Asteroids.Empty();

	// Reset ID counter
	NextAsteroidID = 0;

	UE_LOG(LogTemp, Log, TEXT("Asteroid field cleared"));
}

void AAsteroidFieldManager::SpawnTestGrid(int32 GridSize, float Spacing)
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning test grid: %dx%dx%d (Spacing: %.2f m)"), GridSize, GridSize, GridSize, Spacing);

	// Clear existing field
	ClearAsteroidField();

	// Calculate grid center offset
	float HalfExtent = (GridSize - 1) * Spacing * 0.5f;
	FVector GridCenter = FieldCenter;

	int32 AsteroidsSpawned = 0;

	// Spawn grid
	for (int32 x = 0; x < GridSize; x++)
	{
		for (int32 y = 0; y < GridSize; y++)
		{
			for (int32 z = 0; z < GridSize; z++)
			{
				// Calculate position
				FVector Position = GridCenter + FVector(
					(x * Spacing) - HalfExtent,
					(y * Spacing) - HalfExtent,
					(z * Spacing) - HalfExtent
				);

				// Fixed size for test grid
				float Size = 50.0f; // 50m radius

				// Small random velocity
				FVector Velocity = FVector(
					FMath::FRandRange(-10.0f, 10.0f),
					FMath::FRandRange(-10.0f, 10.0f),
					FMath::FRandRange(-10.0f, 10.0f)
				);

				// Spawn asteroid
				AAsteroid* NewAsteroid = SpawnAsteroid(Position, Size, Velocity);

				if (NewAsteroid)
				{
					// Assign unique ID
					NewAsteroid->AsteroidID = NextAsteroidID++;
					NewAsteroid->Density = 2500.0f; // Standard rocky density

					// Add to active list
					ActiveAsteroids.Add(NewAsteroid);
					AsteroidsSpawned++;
				}
			}
		}
	}

	// Initial tier assignment
	UpdateAsteroidTiers();

	UE_LOG(LogTemp, Warning, TEXT("Test grid complete: %d asteroids spawned"), AsteroidsSpawned);
}

// ============================================================================
// TIER MANAGEMENT
// ============================================================================

void AAsteroidFieldManager::UpdateAsteroidTiers()
{
	if (!CachedPlayerShip)
	{
		CachePlayerShip();
		if (!CachedPlayerShip)
		{
			// No player ship found - skip tier updates
			return;
		}
	}

	FVector PlayerLocation = CachedPlayerShip->GetActorLocation();

	// Update each asteroid's tier based on distance to player
	for (AAsteroid* Asteroid : ActiveAsteroids)
	{
		if (!Asteroid || !IsValid(Asteroid) || !Asteroid->bIsActive)
		{
			continue;
		}

		// Calculate distance to player
		float Distance = FVector::Dist(Asteroid->GetActorLocation(), PlayerLocation);

		// Update tier based on distance
		Asteroid->UpdateTierBasedOnDistance(Distance);
	}

	// Reorganize tier arrays for efficient iteration
	UpdateTierArrays();
}

void AAsteroidFieldManager::UpdateTierArrays()
{
	// Clear tier arrays
	Tier0Asteroids.Empty();
	Tier1Asteroids.Empty();
	Tier2Asteroids.Empty();
	Tier3Asteroids.Empty();
	Tier4Asteroids.Empty();

	// Reorganize asteroids by tier
	for (AAsteroid* Asteroid : ActiveAsteroids)
	{
		if (!Asteroid || !IsValid(Asteroid) || !Asteroid->bIsActive)
		{
			continue;
		}

		switch (Asteroid->CurrentTier)
		{
			case 0:
				Tier0Asteroids.Add(Asteroid);
				break;
			case 1:
				Tier1Asteroids.Add(Asteroid);
				break;
			case 2:
				Tier2Asteroids.Add(Asteroid);
				break;
			case 3:
				Tier3Asteroids.Add(Asteroid);
				break;
			case 4:
				Tier4Asteroids.Add(Asteroid);
				break;
		}
	}
}

TArray<AAsteroid*> AAsteroidFieldManager::GetAsteroidsInTier(int32 Tier) const
{
	switch (Tier)
	{
		case 0: return Tier0Asteroids;
		case 1: return Tier1Asteroids;
		case 2: return Tier2Asteroids;
		case 3: return Tier3Asteroids;
		case 4: return Tier4Asteroids;
		default: return TArray<AAsteroid*>();
	}
}

void AAsteroidFieldManager::GetTierStats(int32& OutTier0, int32& OutTier1, int32& OutTier2, int32& OutTier3, int32& OutTier4) const
{
	OutTier0 = Tier0Asteroids.Num();
	OutTier1 = Tier1Asteroids.Num();
	OutTier2 = Tier2Asteroids.Num();
	OutTier3 = Tier3Asteroids.Num();
	OutTier4 = Tier4Asteroids.Num();
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FVector AAsteroidFieldManager::GenerateRandomPositionInField() const
{
	// Generate random point in sphere using rejection sampling
	// This ensures uniform distribution within sphere volume
	FVector RandomPoint;
	do
	{
		RandomPoint.X = FMath::FRandRange(-1.0f, 1.0f);
		RandomPoint.Y = FMath::FRandRange(-1.0f, 1.0f);
		RandomPoint.Z = FMath::FRandRange(-1.0f, 1.0f);
	} while (RandomPoint.SizeSquared() > 1.0f);

	// Scale to field radius and offset by field center
	return FieldCenter + (RandomPoint * FieldRadius);
}

FVector AAsteroidFieldManager::GenerateRandomVelocity() const
{
	// Generate random velocity vector
	FVector RandomDirection = FVector(
		FMath::FRandRange(-1.0f, 1.0f),
		FMath::FRandRange(-1.0f, 1.0f),
		FMath::FRandRange(-1.0f, 1.0f)
	).GetSafeNormal();

	// Random speed within range
	float Speed = FMath::FRandRange(MinInitialVelocity, MaxInitialVelocity);

	return RandomDirection * Speed;
}

float AAsteroidFieldManager::GenerateRandomSize() const
{
	// Use power law distribution (biased toward smaller asteroids)
	// P(r)  r^(-2.5) - realistic size distribution for asteroids
	float RandomValue = FMath::FRand();
	float SizeRatio = FMath::Pow(RandomValue, 2.5f); // Exponential bias

	// Map to size range
	return FMath::Lerp(MinAsteroidRadius, MaxAsteroidRadius, SizeRatio);
}

float AAsteroidFieldManager::GenerateRandomDensity() const
{
	// Uniform distribution within density range
	return FMath::FRandRange(MinDensity, MaxDensity);
}

void AAsteroidFieldManager::CachePlayerShip()
{
	if (!GetWorld())
	{
		return;
	}

	// Find player pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		CachedPlayerShip = Cast<ASpaceship>(PlayerPawn);

		if (CachedPlayerShip)
		{
			UE_LOG(LogTemp, Log, TEXT("AsteroidFieldManager: Cached player ship reference"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AsteroidFieldManager: Player pawn is not a Spaceship"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AsteroidFieldManager: No player pawn found"));
	}
}

// ============================================================================
// DEBUG VISUALIZATION
// ============================================================================

void AAsteroidFieldManager::DrawFieldDebug() const
{
	if (!GetWorld())
	{
		return;
	}

	// Draw field boundaries
	if (bShowTierBoundaries)
	{
		if (CachedPlayerShip)
		{
			FVector PlayerLocation = CachedPlayerShip->GetActorLocation();

			// Draw tier boundary spheres
			DrawDebugSphere(GetWorld(), PlayerLocation, Tier0Threshold, 32, FColor::Red, false, -1.0f, 0, 20.0f);
			DrawDebugSphere(GetWorld(), PlayerLocation, Tier1Threshold, 32, FColor::Orange, false, -1.0f, 0, 20.0f);
			DrawDebugSphere(GetWorld(), PlayerLocation, Tier2Threshold, 32, FColor::Yellow, false, -1.0f, 0, 20.0f);
			DrawDebugSphere(GetWorld(), PlayerLocation, Tier3Threshold, 32, FColor::Green, false, -1.0f, 0, 20.0f);
		}

		// Draw field center sphere
		DrawDebugSphere(GetWorld(), FieldCenter, FieldRadius, 64, FColor::Cyan, false, -1.0f, 0, 50.0f);
	}

	// Draw tier statistics
	if (CachedPlayerShip)
	{
		FVector PlayerLocation = CachedPlayerShip->GetActorLocation();
		FVector LabelLocation = PlayerLocation + FVector(0, 0, 50000.0f); // 50km above player

		FString StatsText = FString::Printf(
			TEXT("Asteroid Field Stats\nTotal: %d / %d\nT0: %d | T1: %d | T2: %d | T3: %d | T4: %d\nFrame: %.2fms"),
			ActiveAsteroids.Num(), MaxActiveAsteroids,
			Tier0Asteroids.Num(), Tier1Asteroids.Num(), Tier2Asteroids.Num(),
			Tier3Asteroids.Num(), Tier4Asteroids.Num(),
			LastFrameTimeMS
		);

		DrawDebugString(GetWorld(), LabelLocation, StatsText, nullptr, FColor::White, 0.0f, true, 1.5f);
	}
}
