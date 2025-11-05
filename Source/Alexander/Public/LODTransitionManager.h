// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions in inline code
#include "TerrainTile.h"
#include "LODTransitionManager.generated.h"

/**
 * LOD transition state
 */
UENUM(BlueprintType)
enum class ELODTransitionState : uint8
{
	Idle,           // No transition
	FadingOut,      // Old LOD fading out
	FadingIn,       // New LOD fading in
	Geomorphing     // Vertex geomorphing between LODs
};

/**
 * LOD transition data for a tile
 */
USTRUCT()
struct FLODTransition
{
	GENERATED_BODY()

	// Tile identification
	UPROPERTY()
	FVector2D TilePosition;

	UPROPERTY()
	int32 OldLODLevel = 0;

	UPROPERTY()
	int32 NewLODLevel = 0;

	// Transition state
	UPROPERTY()
	ELODTransitionState State = ELODTransitionState::Idle;

	// Timing
	UPROPERTY()
	float TransitionDuration = 0.5f; // seconds

	UPROPERTY()
	float ElapsedTime = 0.0f;

	UPROPERTY()
	float TransitionProgress = 0.0f; // 0-1

	// Tile data
	UPROPERTY()
	FTerrainTileData OldTileData;

	UPROPERTY()
	FTerrainTileData NewTileData;

	// Geomorphing data
	TArray<FVector> GeomorphedVertices;
	TArray<FVector> GeomorphedNormals;

	// Distance-based fade
	UPROPERTY()
	float FadeDistance = 0.0f;

	UPROPERTY()
	float CurrentDistance = 0.0f;

	// Constructor
	FLODTransition()
		: TilePosition(FVector2D::ZeroVector)
		, OldLODLevel(0)
		, NewLODLevel(0)
		, State(ELODTransitionState::Idle)
		, TransitionDuration(0.5f)
		, ElapsedTime(0.0f)
		, TransitionProgress(0.0f)
		, FadeDistance(0.0f)
		, CurrentDistance(0.0f)
	{
	}

	// Check if transition is complete
	bool IsComplete() const
	{
		return State == ELODTransitionState::Idle || TransitionProgress >= 1.0f;
	}

	// Get interpolated alpha value with easing
	float GetEasedAlpha() const
	{
		// Smooth step easing (cubic)
		float T = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);
		return T * T * (3.0f - 2.0f * T);
	}
};

/**
 * LOD transition configuration
 */
USTRUCT(BlueprintType)
struct FLODTransitionConfig
{
	GENERATED_BODY()

	// Enable vertex geomorphing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bEnableGeomorphing = true;

	// Enable distance-based fading
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bEnableDistanceFade = true;

	// Transition duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionDuration = 0.5f;

	// Fade distance range (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float FadeDistanceRange = 100.0f;

	// Maximum concurrent transitions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 MaxConcurrentTransitions = 20;

	// Enable smooth normal interpolation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	bool bInterpolateNormals = true;

	// Use cubic easing for smoother transitions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	bool bUseSmoothEasing = true;
};

/**
 * LOD transition manager
 * Handles smooth transitions between LOD levels using vertex geomorphing and fading
 */
UCLASS(BlueprintType)
class ALEXANDER_API ULODTransitionManager : public UObject
{
	GENERATED_BODY()

public:
	ULODTransitionManager();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize transition manager
	 * @param Config Transition configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	void Initialize(const FLODTransitionConfig& Config);

	/**
	 * Shutdown and cleanup
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	void Shutdown();

	// ============================================================================
	// TRANSITION MANAGEMENT
	// ============================================================================

	/**
	 * Start LOD transition for a tile
	 * @param TilePosition Tile position
	 * @param OldLOD Old LOD level
	 * @param NewLOD New LOD level
	 * @param OldTileData Old tile data
	 * @param NewTileData New tile data
	 * @param ViewerPosition Current viewer position
	 * @return True if transition was started
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	bool StartTransition(
		FVector2D TilePosition,
		int32 OldLOD,
		int32 NewLOD,
		const FTerrainTileData& OldTileData,
		const FTerrainTileData& NewTileData,
		FVector2D ViewerPosition
	);

	/**
	 * Cancel transition for a tile
	 * @param TilePosition Tile position
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	void CancelTransition(FVector2D TilePosition);

	/**
	 * Check if tile is transitioning
	 * @param TilePosition Tile position
	 * @return True if tile is currently transitioning
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	bool IsTransitioning(FVector2D TilePosition) const;

	/**
	 * Get transition progress for a tile
	 * @param TilePosition Tile position
	 * @return Progress value 0-1, or -1 if not transitioning
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	float GetTransitionProgress(FVector2D TilePosition) const;

	// ============================================================================
	// UPDATE
	// ============================================================================

	/**
	 * Update all active transitions
	 * @param DeltaTime Time since last update
	 * @param ViewerPosition Current viewer position
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	void UpdateTransitions(float DeltaTime, FVector2D ViewerPosition);

	/**
	 * Get geomorphed tile data for rendering
	 * @param TilePosition Tile position
	 * @param OutTileData Output geomorphed tile data
	 * @return True if tile is transitioning and data was retrieved
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	bool GetGeomorphedTileData(FVector2D TilePosition, FTerrainTileData& OutTileData);

	// ============================================================================
	// GEOMORPHING
	// ============================================================================

	/**
	 * Perform vertex geomorphing between two LOD levels
	 * @param Transition Transition data
	 * @param Alpha Interpolation factor (0-1)
	 */
	void GeomorphVertices(FLODTransition& Transition, float Alpha);

	/**
	 * Interpolate vertex positions between LODs
	 * @param OldVertices Old LOD vertices
	 * @param NewVertices New LOD vertices
	 * @param Alpha Interpolation factor
	 * @return Interpolated vertices
	 */
	static TArray<FVector> InterpolateVertices(
		const TArray<FVector>& OldVertices,
		const TArray<FVector>& NewVertices,
		float Alpha
	);

	/**
	 * Interpolate normals between LODs
	 * @param OldNormals Old LOD normals
	 * @param NewNormals New LOD normals
	 * @param Alpha Interpolation factor
	 * @return Interpolated normals
	 */
	static TArray<FVector> InterpolateNormals(
		const TArray<FVector>& OldNormals,
		const TArray<FVector>& NewNormals,
		float Alpha
	);

	/**
	 * Sample vertex from higher resolution mesh
	 * @param HighResMesh High resolution mesh data
	 * @param LocalPosition Local position to sample (0-1 range)
	 * @return Sampled vertex position
	 */
	static FVector SampleVertexFromMesh(
		const FTerrainTileData& HighResMesh,
		FVector2D LocalPosition
	);

	// ============================================================================
	// DISTANCE FADE
	// ============================================================================

	/**
	 * Calculate distance-based fade factor
	 * @param TilePosition Tile position
	 * @param ViewerPosition Viewer position
	 * @param FadeRange Fade distance range
	 * @return Fade factor (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	static float CalculateDistanceFade(
		FVector2D TilePosition,
		FVector2D ViewerPosition,
		float FadeRange
	);

	// ============================================================================
	// QUERIES
	// ============================================================================

	/**
	 * Get number of active transitions
	 * @return Count of active transitions
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition")
	int32 GetActiveTransitionCount() const;

	/**
	 * Get all active transitions
	 * @return Array of active transition data
	 */
	TArray<FLODTransition> GetActiveTransitions() const;

	// ============================================================================
	// DEBUG
	// ============================================================================

	/**
	 * Draw debug visualization of transitions
	 * @param World World context
	 * @param Duration Display duration
	 */
	UFUNCTION(BlueprintCallable, Category = "LODTransition|Debug")
	void DrawDebugTransitions(UWorld* World, float Duration = 0.0f) const;

	// ============================================================================
	// PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FLODTransitionConfig Config;

private:
	// Active transitions
	TMap<FString, FLODTransition> ActiveTransitions;

	// Helper: Get transition key
	FString GetTransitionKey(FVector2D Position) const;

	// Helper: Update single transition
	void UpdateTransition(FLODTransition& Transition, float DeltaTime, FVector2D ViewerPosition);

	// Helper: Complete transition
	void CompleteTransition(const FString& Key);

	// Helper: Find corresponding vertex in different LOD
	FVector FindCorrespondingVertex(
		const FTerrainTileData& SourceMesh,
		const FTerrainTileData& TargetMesh,
		int32 SourceVertexIndex
	) const;
};
