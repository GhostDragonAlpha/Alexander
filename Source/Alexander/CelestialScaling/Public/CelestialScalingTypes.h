// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CelestialScalingTypes.generated.h"

/**
 * Scaling mode for celestial bodies
 * Determines how bodies are rendered and positioned
 */
UENUM(BlueprintType)
enum class EScalingMode : uint8
{
	// Scaling is disabled - all bodies rendered at real size (not practical)
	Disabled UMETA(DisplayName = "Disabled"),
	
	// Dynamic scaling based on distance from player
	Dynamic UMETA(DisplayName = "Dynamic"),
	
	// Fixed scale factor for all bodies
	Fixed UMETA(DisplayName = "Fixed")
};

/**
 * Gravity calculation mode
 * Determines how gravitational forces are computed
 */
UENUM(BlueprintType)
enum class EGravityMode : uint8
{
	// Realistic Newtonian gravity (F = GMm/r²)
	Realistic UMETA(DisplayName = "Realistic"),
	
	// Scaled gravity for gameplay purposes
	Scaled UMETA(DisplayName = "Scaled"),
	
	// Hybrid approach - realistic within SOI, scaled outside
	Hybrid UMETA(DisplayName = "Hybrid")
};

/**
 * Priority level for celestial body updates
 * Higher priority bodies updated more frequently
 */
UENUM(BlueprintType)
enum class ECelestialUpdatePriority : uint8
{
	// Critical bodies (player ship, nearby planets)
	Critical UMETA(DisplayName = "Critical"),
	
	// High priority (bodies in same system)
	High UMETA(DisplayName = "High"),
	
	// Medium priority (nearby systems)
	Medium UMETA(DisplayName = "Medium"),
	
	// Low priority (distant bodies)
	Low UMETA(DisplayName = "Low"),
	
	// Minimal updates (very distant bodies)
	Minimal UMETA(DisplayName = "Minimal")
};

/**
 * Configuration for celestial scaling system
 * Exposed to editor for designer control
 */
USTRUCT(BlueprintType)
struct ALEXANDER_API FCelestialScalingConfig
{
	GENERATED_BODY()

	// Scaling mode to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	EScalingMode ScalingMode = EScalingMode::Dynamic;

	// Gravity calculation mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	EGravityMode GravityMode = EGravityMode::Hybrid;

	// Enable automatic origin recentering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precision")
	bool bEnableOriginRecentering = true;

	// Enable dynamic LOD for distant bodies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableDynamicLOD = true;

	// Maximum number of bodies to update per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxBodiesPerFrame = 100;

	// Distance threshold for culling distant bodies (in km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000", ClampMax = "10000000"))
	float CullingDistance = 1000000.0f;

	// Enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo = false;

	// Default constructor
	FCelestialScalingConfig()
		: ScalingMode(EScalingMode::Dynamic)
		, GravityMode(EGravityMode::Hybrid)
		, bEnableOriginRecentering(true)
		, bEnableDynamicLOD(true)
		, MaxBodiesPerFrame(100)
		, CullingDistance(1000000.0f)
		, bShowDebugInfo(false)
	{}
};

/**
 * Runtime statistics for celestial scaling system
 * Used for monitoring and debugging
 */
USTRUCT(BlueprintType)
struct ALEXANDER_API FCelestialScalingStats
{
	GENERATED_BODY()

	// Total number of registered bodies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 TotalBodies = 0;

	// Number of bodies currently visible
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 VisibleBodies = 0;

	// Number of bodies updated this frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 BodiesUpdatedThisFrame = 0;

	// Current player distance from origin (in km)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float DistanceFromOrigin = 0.0f;

	// Number of origin recentering operations performed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 RecenterCount = 0;

	// Average update time per body (in milliseconds)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float AverageUpdateTimeMs = 0.0f;

	// Default constructor
	FCelestialScalingStats()
		: TotalBodies(0)
		, VisibleBodies(0)
		, BodiesUpdatedThisFrame(0)
		, DistanceFromOrigin(0.0f)
		, RecenterCount(0)
		, AverageUpdateTimeMs(0.0f)
	{}
};

/**
 * LOD level configuration for celestial bodies
 */
USTRUCT(BlueprintType)
struct ALEXANDER_API FCelestialLODConfig
{
	GENERATED_BODY()

	// Distance threshold for this LOD level (in km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float Distance = 0.0f;

	// Mesh detail level (0 = highest, 3 = lowest)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0", ClampMax = "3"))
	int32 MeshLODLevel = 0;

	// Whether to update orbital position at this LOD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bUpdatePosition = true;

	// Whether to calculate gravity at this LOD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bCalculateGravity = true;

	// Update frequency (updates per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float UpdateFrequency = 30.0f;

	// Default constructor
	FCelestialLODConfig()
		: Distance(0.0f)
		, MeshLODLevel(0)
		, bUpdatePosition(true)
		, bCalculateGravity(true)
		, UpdateFrequency(30.0f)
	{}
};
