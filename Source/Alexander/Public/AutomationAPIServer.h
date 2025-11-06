// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "AutomationAPIServer.generated.h"

/**
 * HTTP-based Automation API Server
 * Allows Python scripts to control the game for automated testing
 * Runs in dedicated server mode for autonomous development
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UAutomationAPIServer : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutomationAPIServer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// SERVER CONFIGURATION
	// ============================================================================

	// Port to listen on for HTTP requests
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation Server")
	int32 ListenPort = 8080;

	// Enable/disable the server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation Server")
	bool bServerEnabled = true;

	// Enable verbose logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation Server")
	bool bVerboseLogging = true;

	// Maximum requests per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation Server")
	int32 MaxRequestsPerSecond = 60;

	// ============================================================================
	// API ENDPOINTS
	// ============================================================================

	// Start the HTTP server
	UFUNCTION(BlueprintCallable, Category = "Automation Server")
	bool StartServer();

	// Stop the HTTP server
	UFUNCTION(BlueprintCallable, Category = "Automation Server")
	void StopServer();

	// Is server running?
	UFUNCTION(BlueprintCallable, Category = "Automation Server")
	bool IsServerRunning() const { return bIsRunning; }

	// Get server status
	UFUNCTION(BlueprintCallable, Category = "Automation Server")
	FString GetServerStatus() const;

	// ============================================================================
	// REQUEST HANDLERS
	// ============================================================================

	// Handle incoming HTTP request
	void HandleHTTPRequest(const FString& Endpoint, const FString& Method, const FString& Body, FString& OutResponse);

	// POST /spawn_ship - Spawn a ship at location
	FString HandleSpawnShip(const FString& RequestBody);

	// POST /set_input - Set input for a ship
	FString HandleSetInput(const FString& RequestBody);

	// GET /get_position/{ship_id} - Get ship position
	FString HandleGetPosition(const FString& ShipID);

	// GET /get_velocity/{ship_id} - Get ship velocity
	FString HandleGetVelocity(const FString& ShipID);

	// POST /screenshot - Take a screenshot
	FString HandleScreenshot(const FString& RequestBody);

	// GET /status - Get server status
	FString HandleStatus();

	// GET /list_ships - List all active ships
	FString HandleListShips();

	// DELETE /destroy_ship/{ship_id} - Destroy a ship
	FString HandleDestroyShip(const FString& ShipID);

	// ============================================================================
	// SHIP TRACKING
	// ============================================================================

	// Register a spawned ship
	void RegisterShip(AActor* Ship, const FString& ShipID);

	// Unregister a ship
	void UnregisterShip(const FString& ShipID);

	// Get ship by ID
	AActor* GetShipByID(const FString& ShipID);

	// Get all ships
	TArray<AActor*> GetAllShips();

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Is server running?
	bool bIsRunning = false;

	// Next ship ID counter
	int32 NextShipID = 1;

	// Map of ship IDs to actors
	UPROPERTY()
	TMap<FString, AActor*> TrackedShips;

	// Request rate limiting
	float LastRequestTime = 0.0f;
	int32 RequestsThisSecond = 0;

	// Performance metrics
	int32 TotalRequestsProcessed = 0;
	float TotalProcessingTime = 0.0f;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Parse JSON string
	TSharedPtr<FJsonObject> ParseJSON(const FString& JSONString);

	// Create JSON response
	FString CreateJSONResponse(bool bSuccess, const FString& Message, const TSharedPtr<FJsonObject>& Data = nullptr);

	// Rate limit check
	bool CheckRateLimit();

	// Log request
	void LogRequest(const FString& Endpoint, const FString& Method, float ProcessingTime);

	// Generate unique ship ID
	FString GenerateShipID();

	// Validate ship class
	bool ValidateShipClass(UClass* ShipClass);
};
