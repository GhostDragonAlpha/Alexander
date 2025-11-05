// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerOriginManager.generated.h"

// Forward declarations
class UCelestialBodyRegistry;

/**
 * Player origin-specific virtual position for high-precision space coordinates
 * Separates position into large-scale (sectors) and local offsets
 * to avoid floating-point precision issues at large distances
 * NOTE: Using PlayerOrigin prefix to avoid collision with Agent 1's FVirtualPosition
 */
USTRUCT(BlueprintType)
struct FPlayerOriginVirtualPosition
{
	GENERATED_BODY()

	/** Sector coordinates (large scale, integer-based) */
	UPROPERTY(BlueprintReadWrite, Category = "Virtual Position")
	FIntVector SectorCoords;

	/** Local offset within the sector (high precision) */
	UPROPERTY(BlueprintReadWrite, Category = "Virtual Position")
	FVector LocalOffset;

	/** Size of each sector in Unreal units (default: 10 million cm = 100 km) */
	static constexpr float SectorSize = 10000000.0f;

	FPlayerOriginVirtualPosition()
		: SectorCoords(0, 0, 0)
		, LocalOffset(FVector::ZeroVector)
	{
	}

	FPlayerOriginVirtualPosition(const FIntVector& InSector, const FVector& InOffset)
		: SectorCoords(InSector)
		, LocalOffset(InOffset)
	{
	}

	/** Convert to world position (may lose precision at large distances) */
	FVector ToWorldPosition() const
	{
		return FVector(
			SectorCoords.X * SectorSize + LocalOffset.X,
			SectorCoords.Y * SectorSize + LocalOffset.Y,
			SectorCoords.Z * SectorSize + LocalOffset.Z
		);
	}

	/** Create from world position */
	static FPlayerOriginVirtualPosition FromWorldPosition(const FVector& WorldPos)
	{
		FPlayerOriginVirtualPosition Result;
		Result.SectorCoords.X = FMath::FloorToInt(WorldPos.X / SectorSize);
		Result.SectorCoords.Y = FMath::FloorToInt(WorldPos.Y / SectorSize);
		Result.SectorCoords.Z = FMath::FloorToInt(WorldPos.Z / SectorSize);

		Result.LocalOffset.X = WorldPos.X - (Result.SectorCoords.X * SectorSize);
		Result.LocalOffset.Y = WorldPos.Y - (Result.SectorCoords.Y * SectorSize);
		Result.LocalOffset.Z = WorldPos.Z - (Result.SectorCoords.Z * SectorSize);

		return Result;
	}

	/** Add a delta to the virtual position */
	void AddDelta(const FVector& Delta)
	{
		LocalOffset += Delta;
		NormalizeOffset();
	}

	/** Normalize offset to keep it within sector bounds */
	void NormalizeOffset()
	{
		// Move overflow to sector coordinates
		if (FMath::Abs(LocalOffset.X) > SectorSize)
		{
			int32 Sectors = FMath::FloorToInt(LocalOffset.X / SectorSize);
			SectorCoords.X += Sectors;
			LocalOffset.X -= Sectors * SectorSize;
		}

		if (FMath::Abs(LocalOffset.Y) > SectorSize)
		{
			int32 Sectors = FMath::FloorToInt(LocalOffset.Y / SectorSize);
			SectorCoords.Y += Sectors;
			LocalOffset.Y -= Sectors * SectorSize;
		}

		if (FMath::Abs(LocalOffset.Z) > SectorSize)
		{
			int32 Sectors = FMath::FloorToInt(LocalOffset.Z / SectorSize);
			SectorCoords.Z += Sectors;
			LocalOffset.Z -= Sectors * SectorSize;
		}
	}

	/** Calculate distance to another virtual position */
	float DistanceTo(const FPlayerOriginVirtualPosition& Other) const
	{
		// Calculate sector difference
		FIntVector SectorDiff = Other.SectorCoords - SectorCoords;
		FVector SectorOffset(
			SectorDiff.X * SectorSize,
			SectorDiff.Y * SectorSize,
			SectorDiff.Z * SectorSize
		);

		// Add local offset difference
		FVector TotalOffset = SectorOffset + (Other.LocalOffset - LocalOffset);

		return TotalOffset.Size();
	}
};

/**
 * Player Origin Manager Component
 *
 * Manages high-precision player position tracking for space exploration
 * This component:
 * - Tracks virtual position using sector-based coordinates
 * - Handles universe recentering to maintain floating-point precision
 * - Integrates with multiplayer replication (server-authoritative)
 * - Coordinates with celestial body scaling system
 *
 * Attach this to the player pawn/spaceship to enable precise space navigation
 */
UCLASS(ClassGroup=(CelestialScaling), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlayerOriginManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerOriginManager();

	// ============================================================================
	// COMPONENT LIFECYCLE
	// ============================================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// VIRTUAL POSITION (Server Authoritative)
	// ============================================================================

	/** Current virtual position in space (replicated) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Position")
	FPlayerOriginVirtualPosition VirtualPosition;

	/** Last recentering position */
	UPROPERTY(BlueprintReadOnly, Category = "Position")
	FVector LastRecenterPosition;

	/** Total distance traveled since game start */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Position")
	float TotalDistanceTraveled;

	// ============================================================================
	// MOVEMENT FUNCTIONS
	// ============================================================================

	/**
	 * Server RPC: Move player by delta
	 * Called by client, validated and executed on server
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMovePlayer(const FVector& MovementDelta);

	/**
	 * Move player in virtual space
	 * Updates virtual position and handles universe translation if needed
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	void MovePlayer(const FVector& MovementDelta);

	/**
	 * Set virtual position directly (server only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	void SetVirtualPosition(const FPlayerOriginVirtualPosition& NewPosition);

	// ============================================================================
	// UNIVERSE TRANSLATION
	// ============================================================================

	/**
	 * Translate the entire universe by an offset
	 * This keeps the player near world origin for precision
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	void TranslateUniverse(const FVector& OffsetDelta);

	/**
	 * Check if universe should be recentered
	 * Returns true when player is too far from origin
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	bool ShouldRecenterUniverse() const;

	/**
	 * Recenter the universe around the player
	 * Moves all actors to keep player at/near origin
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	void RecenterUniverse();

	/**
	 * Calculate current floating-point precision error
	 * Higher values = more precision loss
	 */
	UFUNCTION(BlueprintCallable, Category = "Position")
	float CalculatePrecisionError() const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Distance threshold for universe recentering (default: 1,000,000 cm = 10 km) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "100000.0"))
	float PrecisionThreshold;

	/** Enable automatic universe recentering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision")
	bool bAutoRecenterUniverse;

	/** Frequency of precision checks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision", meta = (ClampMin = "0.1"))
	float PrecisionCheckInterval;

	// ============================================================================
	// BLUEPRINT ACCESS
	// ============================================================================

	/** Get current virtual position */
	UFUNCTION(BlueprintCallable, Category = "Position")
	FPlayerOriginVirtualPosition GetVirtualPosition() const { return VirtualPosition; }

	/** Get virtual position as world coordinates (may lose precision) */
	UFUNCTION(BlueprintCallable, Category = "Position")
	FVector GetVirtualPositionAsWorld() const { return VirtualPosition.ToWorldPosition(); }

	/** Get local position relative to current sector */
	UFUNCTION(BlueprintCallable, Category = "Position")
	FVector GetLocalPosition() const { return VirtualPosition.LocalOffset; }

	/** Get current sector coordinates */
	UFUNCTION(BlueprintCallable, Category = "Position")
	FIntVector GetSectorCoordinates() const { return VirtualPosition.SectorCoords; }

	/** Get distance from origin in kilometers */
	UFUNCTION(BlueprintCallable, Category = "Position")
	float GetDistanceFromOriginKm() const;

	// ============================================================================
	// DEBUG & UTILITIES
	// ============================================================================

	/** Show debug information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

	/** Draw debug visualization */
	void DrawDebugVisualization();

	/** Get status information as string */
	UFUNCTION(BlueprintCallable, Category = "Position")
	FString GetStatusInfo() const;

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Reference to celestial body registry (when Agent 2's work is complete) */
	UPROPERTY()
	UCelestialBodyRegistry* BodyRegistry;

	/** Timer for precision checks */
	float PrecisionCheckTimer;

	/** Number of recentering operations performed */
	int32 RecenterCount;

	/** Is this the local player? */
	bool bIsLocalPlayer;

	// ============================================================================
	// INTERNAL FUNCTIONS
	// ============================================================================

	/** Check if this is the local player's component */
	void DetermineLocalPlayer();

	/** Update precision monitoring */
	void UpdatePrecisionMonitoring(float DeltaTime);

	/** Notify subsystems of position change */
	void NotifyPositionChange();

	/** Find all actors that need to be translated during recentering */
	void GatherActorsForTranslation(TArray<AActor*>& OutActors);

	// ============================================================================
	// CONSTANTS
	// ============================================================================

	/** Conversion: Unreal units (cm) to kilometers */
	static constexpr float CmToKm = 0.00001f;

	/** Maximum safe distance from origin before precision degrades significantly */
	static constexpr float MaxSafeDistance = 5000000.0f; // 50 km
};
