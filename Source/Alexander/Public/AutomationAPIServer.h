// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TriangulationValidator.h"
#include "AutomationAPIServer.generated.h"

// Forward declarations for socket types
class FSocket;
struct FIPv4Endpoint;

/**
 * Wrapper struct for TArray<FObserverMeasurement> to work in TMap
 */
USTRUCT()
struct FObserverMeasurementArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FObserverMeasurement> Measurements;
};

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

	// GET /get_player_pawn - Get the default player pawn
	FString HandleGetPlayerPawn();

	// POST /apply_thrust - Apply thrust forces to a ship
	FString HandleApplyThrust(const FString& RequestBody);

	// POST /submit_observation - Submit sphere-based observation (direction, distance, scale_factor)
	FString HandleSubmitObservation(const FString& RequestBody);

	// POST /validate_position - Validate position using multi-sphere triangulation
	FString HandleValidatePosition(const FString& RequestBody);

	// POST /apply_ship_customization - Apply customization stats to ship
	FString HandleApplyShipCustomization(const FString& RequestBody);

	// GET /get_ship_customization/{ship_id} - Get ship customization stats
	FString HandleGetShipCustomization(const FString& ShipID);

	// POST /equip_ship_part - Equip a part to a ship
	FString HandleEquipShipPart(const FString& RequestBody);

	// GET /get_ship_loadout/{ship_id} - Get complete ship loadout
	FString HandleGetShipLoadout(const FString& ShipID);

#if WITH_EDITOR
	// ============================================================================
	// EDITOR CONTROL (Editor-only)
	// ============================================================================

	// POST /play - Start Play In Editor (PIE)
	FString HandlePlayInEditor(const FString& RequestBody);

	// POST /stop - Stop Play In Editor (PIE)
	FString HandleStopPlaying();

	// POST /pause - Pause/unpause the game
	FString HandlePauseGame();

	// GET /editor_state - Get current editor/PIE state
	FString HandleGetEditorState();

	// ============================================================================
	// MAP/LEVEL CONTROL (Editor-only)
	// ============================================================================

	// GET /list_maps - List all available maps in Content/Maps
	FString HandleListMaps();

	// POST /load_map - Load a specific map in editor
	FString HandleLoadMap(const FString& RequestBody);

	// GET /get_current_map - Get currently loaded map name
	FString HandleGetCurrentMap();

	// POST /save_map - Save current map
	FString HandleSaveMap();

	// POST /save_map_as - Save map with new name
	FString HandleSaveMapAs(const FString& RequestBody);

	// ============================================================================
	// CONSOLE COMMANDS (Editor-only)
	// ============================================================================

	// POST /execute_console_command - Execute any console command
	FString HandleExecuteConsoleCommand(const FString& RequestBody);

	// POST /set_cvar - Set console variable
	FString HandleSetCVar(const FString& RequestBody);

	// GET /get_cvar/{name} - Get console variable value
	FString HandleGetCVar(const FString& CVarName);

	// ============================================================================
	// ASSET MANAGEMENT (Editor-only)
	// ============================================================================

	// GET /list_assets - List assets in content browser (optional JSON filter)
	FString HandleListAssets(const FString& RequestBody);

	// GET /validate_asset/{path} - Validate asset integrity
	FString HandleValidateAsset(const FString& AssetPath);

	// POST /reload_asset - Reload asset from disk
	FString HandleReloadAsset(const FString& RequestBody);

	// GET /get_asset_dependencies/{path} - Get asset dependencies
	FString HandleGetAssetDependencies(const FString& AssetPath);

	// ============================================================================
	// TEST AUTOMATION (Editor-only)
	// ============================================================================

	// POST /run_automation_test - Run specific test
	FString HandleRunAutomationTest(const FString& RequestBody);

	// GET /list_automation_tests - List all available automation tests
	FString HandleListAutomationTests();

	// GET /get_test_results - Get results of last test run
	FString HandleGetTestResults();

	// ============================================================================
	// BUILD & COMPILE (Editor-only)
	// ============================================================================

	// POST /recompile_shaders - Recompile all shaders
	FString HandleRecompileShaders(const FString& RequestBody);

	// POST /build_lighting - Build static lighting for current level
	FString HandleBuildLighting(const FString& RequestBody);

	// POST /refresh_content_browser - Refresh/rescan content browser
	FString HandleRefreshContentBrowser(const FString& RequestBody);

	// ============================================================================
	// ACTOR/LEVEL INSPECTION (Editor-only)
	// ============================================================================

	// GET /list_actors - List all actors in current level (with optional filter)
	FString HandleListActors(const FString& RequestBody);

	// GET /get_actor_info/{name} - Get detailed info about specific actor
	FString HandleGetActorInfo(const FString& ActorName);

	// POST /delete_actor - Delete actor by name
	FString HandleDeleteActor(const FString& RequestBody);

	// POST /clear_level - Delete all non-essential actors
	FString HandleClearLevel(const FString& RequestBody);

	// POST /spawn_actor - Spawn actor at location
	FString HandleSpawnActor(const FString& RequestBody);

	// POST /move_actor - Set actor transform
	FString HandleMoveActor(const FString& RequestBody);

	// GET /get_components/{actor_name} - List actor components
	FString HandleGetComponents(const FString& ActorName);

	// ============================================================================
	// MATERIAL & TEXTURE QUERY (Editor-only)
	// ============================================================================

	// GET /materials/list - List all materials in project
	FString HandleMaterialsList(const FString& RequestBody);

	// GET /materials/get_properties/{material_path} - Get material properties
	FString HandleGetMaterialProperties(const FString& MaterialPath);

	// GET /textures/list - List all textures in project
	FString HandleTexturesList(const FString& RequestBody);

	// ============================================================================
	// BLUEPRINT/CODE INTERACTION (Editor-only)
	// ============================================================================

	// POST /compile_blueprint - Compile a blueprint asset
	FString HandleCompileBlueprint(const FString& RequestBody);

	// GET /get_blueprint_errors/{blueprint_path} - Get compilation errors for a blueprint
	FString HandleGetBlueprintErrors(const FString& BlueprintPath);

	// POST /call_blueprint_function - Execute a blueprint function on an actor
	FString HandleCallBlueprintFunction(const FString& RequestBody);

	// ============================================================================
	// ADVANCED PIE CONTROL (Editor-only)
	// ============================================================================

	// POST /set_viewport_position - Position viewport camera
	FString HandleSetViewportPosition(const FString& RequestBody);

	// GET /get_viewport_info - Get viewport details
	FString HandleGetViewportInfo();

	// POST /simulate_input - Send game input commands
	FString HandleSimulateInput(const FString& RequestBody);

	// ============================================================================
	// SEQUENCER CONTROL (Editor-only)
	// ============================================================================

	// GET /list_sequences - List all level sequences in project
	FString HandleListSequences();

	// POST /play_sequence - Play a cinematic sequence
	FString HandlePlaySequence(const FString& RequestBody);

	// GET /get_sequence_state - Query sequence playback state
	FString HandleGetSequenceState();

	// ============================================================================
	// PERFORMANCE PROFILING (Editor-only)
	// ============================================================================

	// POST /start_profiling - Begin profiler capture
	FString HandleStartProfiling(const FString& RequestBody);

	// POST /stop_profiling - End and export capture
	FString HandleStopProfiling(const FString& RequestBody);

	// GET /get_profiling_data - Retrieve profiling statistics
	FString HandleGetProfilingData();

	// ============================================================================
	// LIVE CODING INTEGRATION (Editor-only)
	// ============================================================================

	// POST /trigger_live_coding - Trigger Live Coding compilation and hot reload
	FString HandleTriggerLiveCoding(const FString& RequestBody);

	// GET /get_live_coding_status - Get current Live Coding state and compilation results
	FString HandleGetLiveCodingStatus();
#endif // WITH_EDITOR

	// ============================================================================
	// LOCALIZATION MODULE
	// ============================================================================

	// GET /localization/get_cultures - List available cultures
	FString HandleLocalizationGetCultures();

	// GET /localization/get_current_culture - Get current language
	FString HandleLocalizationGetCurrentCulture();

	// POST /localization/set_culture - Change language
	FString HandleLocalizationSetCulture(const FString& RequestBody);

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

	// Thread safety for TrackedShips access
	mutable FCriticalSection TrackedShipsLock;

	// Request rate limiting
	float LastRequestTime = 0.0f;
	int32 RequestsThisSecond = 0;

	// Performance metrics
	int32 TotalRequestsProcessed = 0;
	float TotalProcessingTime = 0.0f;

	// Observation tracking (target_id -> array of observations)
	UPROPERTY()
	TMap<int32, FObserverMeasurementArray> StoredObservations;

	// Next observation ID counter
	int32 NextObservationID = 1;

#if WITH_EDITORONLY_DATA
	// Sequencer state tracking
	UPROPERTY()
	class ALevelSequenceActor* CurrentSequenceActor = nullptr;
#endif

#if WITH_EDITOR
	// Test automation results storage
	TArray<TSharedPtr<FJsonObject>> LastTestResults;
	bool bTestsRunning = false;
	FString LastTestName;

	// Profiling state tracking
	bool bProfilingActive = false;
	FString ProfilingType;
	FString ProfilingOutputPath;
	double ProfilingStartTime = 0.0;
	TArray<float> CapturedFrameTimes;
	float TotalCapturedTime = 0.0f;
	int32 CapturedFrameCount = 0;
#endif

	// ============================================================================
	// TCP SOCKET LISTENER
	// ============================================================================

	// TCP listener for HTTP server
	class FTcpListener* TcpListener = nullptr;

	// Handle incoming connection
	bool OnIncomingConnection(class FSocket* Socket, const FIPv4Endpoint& Endpoint);

	// Process HTTP request from socket
	void ProcessSocketRequest(class FSocket* Socket);

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

#if WITH_EDITOR
	// PIE state change callback - clears tracked ships when PIE ends
	void OnPIEEnded(bool bIsSimulating);
#endif
};
