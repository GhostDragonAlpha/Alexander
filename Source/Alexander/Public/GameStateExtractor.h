// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"
#include "GameStateExtractor.generated.h"

// Forward declarations for JSON types
class FJsonObject;

// Forward declarations for other types
class UFlightController;
class UShipCustomizationComponent;

/**
 * Game State Extractor Component
 * Extracts comprehensive game state information for autonomous playtesting
 * Provides real-time data on player, world, performance, inventory, missions, and UI state
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UGameStateExtractor : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameStateExtractor();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Enable verbose logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State Extractor")
	bool bVerboseLogging;

	// State update interval in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State Extractor")
	float StateUpdateInterval;

	// Radius for nearby actor detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State Extractor")
	float NearbyActorDetectionRadius;

	// Enable performance tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State Extractor")
	bool bTrackPerformance;

	// ============================================================================
	// MAIN API
	// ============================================================================

	// Extract complete game state as JSON object
	TSharedPtr<FJsonObject, ESPMode::ThreadSafe> ExtractGameState();

	// Get game state as JSON string
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	FString GetGameStateJSON();

	// ============================================================================
	// STATE EXTRACTION METHODS
	// ============================================================================

	// Extract player state (position, rotation, health, inventory, etc.)
	TSharedPtr<FJsonObject> ExtractPlayerState();

	// Extract world state (time, actors, game mode, etc.)
	TSharedPtr<FJsonObject> ExtractWorldState();

	// Extract performance metrics (FPS, memory, hardware)
	TSharedPtr<FJsonObject> ExtractPerformanceMetrics();

	// Extract inventory state (equipped parts, loadout, etc.)
	TSharedPtr<FJsonObject> ExtractInventoryState();

	// Extract mission/objective status
	TSharedPtr<FJsonObject> ExtractMissionState();

	// Extract nearby actors and objects
	TSharedPtr<FJsonObject> ExtractNearbyActors(float Radius);

	// Extract UI state (menus, dialogs, HUD elements)
	TSharedPtr<FJsonObject> ExtractUIState();

	// ============================================================================
	// HELPER METHODS
	// ============================================================================

	// Get game world (handles both editor and packaged builds)
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	UWorld* GetGameWorld();

	// Get player controller
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	APlayerController* GetPlayerController();

	// Get player pawn
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	APawn* GetPlayerPawn();

	// Get flight controller component
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	class UFlightController* GetFlightController();

	// Get ship customization component
	UFUNCTION(BlueprintCallable, Category = "Game State Extractor")
	class UShipCustomizationComponent* GetShipCustomizationComponent();

	// Convert FVector to JSON object
	TSharedPtr<FJsonObject> VectorToJSON(const FVector& Vector);

	// Convert FRotator to JSON object
	TSharedPtr<FJsonObject> RotatorToJSON(const FRotator& Rotator);

	// Convert FTransform to JSON object
	TSharedPtr<FJsonObject> TransformToJSON(const FTransform& Transform);

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Last time state was updated
	float LastUpdateTime;

	// Cached game state
	TSharedPtr<FJsonObject, ESPMode::ThreadSafe> CachedGameState;

	// Performance tracking
	float AverageFPS;
	float MinFPS;
	float MaxFPS;
	int32 FrameCount;

#if WITH_EDITORONLY_DATA
	// Editor-specific data
#endif
};