// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
// #include "Components/VRPawn.h" // Removed - VRPawn not available in UE5.6
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Spaceship.generated.h"

// Forward declarations
class UCockpitComponent;
class UEngineComponent;
class UFlightController;
class UShipSystemsManager;
class AVRCharacter;
class UPlayerOriginManager;
class UGravitySimulator;

// ============================================================================
// NETWORK REPLICATION STRUCTURES
// ============================================================================

/**
 * High-precision virtual position for spaceship origin-centered physics
 * Stores position as double precision but compresses for network transmission
 */
USTRUCT(BlueprintType)
struct FSpaceshipVirtualPosition
{
	GENERATED_BODY()

	// High-precision absolute position in universe (meters)
	// Not replicated directly - compressed to int64 for bandwidth
	FVector AbsolutePosition;

	// Sector/chunk coordinates for extreme distances
	UPROPERTY(BlueprintReadOnly)
	FIntVector SectorCoordinates;

	// Local position within sector (single precision is fine here)
	UPROPERTY(BlueprintReadOnly)
	FVector LocalPosition;

	FSpaceshipVirtualPosition()
	{
		AbsolutePosition = FVector::ZeroVector;
		SectorCoordinates = FIntVector::ZeroValue;
		LocalPosition = FVector::ZeroVector;
	}

	// Custom network serialization for bandwidth optimization
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	// Compress double precision to int64 (cm precision up to light-years)
	int64 CompressCoordinate(double Coordinate) const;
	double DecompressCoordinate(int64 Compressed) const;
};

template<>
struct TStructOpsTypeTraits<FSpaceshipVirtualPosition> : public TStructOpsTypeTraitsBase2<FSpaceshipVirtualPosition>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * Spaceship network state for replication
 * Optimized for bandwidth with delta compression
 */
USTRUCT(BlueprintType)
struct FSpaceshipNetworkState
{
	GENERATED_BODY()

	// Virtual position in universe
	UPROPERTY(BlueprintReadOnly)
	FSpaceshipVirtualPosition VirtualPos;

	// Velocity (compressed to 16-bit per axis)
	UPROPERTY(BlueprintReadOnly)
	FVector CompressedVelocity;

	// Rotation (quantized)
	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	// Angular velocity (compressed)
	UPROPERTY(BlueprintReadOnly)
	FVector CompressedAngularVelocity;

	// Timestamp for lag compensation
	UPROPERTY(BlueprintReadOnly)
	float Timestamp;

	// Sequence number for packet ordering (not exposed to blueprint)
	uint32 SequenceNumber;

	// Active scale factor (current celestial body influence)
	UPROPERTY(BlueprintReadOnly)
	float ActiveScaleFactor;

	// Input state hash (for prediction reconciliation - not exposed to blueprint)
	uint32 InputStateHash;

	// ============================================================================
	// SHIP CUSTOMIZATION STATS (Phase 3 - Network Replication)
	// ============================================================================

	// Replicated ship mass (affects gravity and inertia)
	float ReplicatedMass = 1000.0f;

	// Replicated thrust power
	float ReplicatedThrustPower = 100000.0f;

	// Replicated max velocity
	float ReplicatedMaxVelocity = 1000.0f;

	FSpaceshipNetworkState()
	{
		CompressedVelocity = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
		CompressedAngularVelocity = FVector::ZeroVector;
		Timestamp = 0.0f;
		SequenceNumber = 0;
		ActiveScaleFactor = 1.0f;
		InputStateHash = 0;
		ReplicatedMass = 1000.0f;
		ReplicatedThrustPower = 100000.0f;
		ReplicatedMaxVelocity = 1000.0f;
	}

	// Compress velocity for network transmission (16-bit per axis)
	static FVector CompressVelocity(const FVector& Velocity);
	static FVector DecompressVelocity(const FVector& Compressed);
};

/**
 * Client prediction data for smooth multiplayer
 */
USTRUCT()
struct FClientPredictionData
{
	GENERATED_BODY()

	// Predicted position
	FSpaceshipVirtualPosition PredictedPosition;

	// Predicted velocity
	FVector PredictedVelocity;

	// Last server-confirmed state
	FSpaceshipNetworkState LastServerState;

	// Prediction error accumulator
	FVector PredictionError;

	// Input history for reconciliation
	TArray<uint32> InputHistory;

	// Timestamp history
	TArray<float> TimestampHistory;

	FClientPredictionData()
	{
		PredictedVelocity = FVector::ZeroVector;
		PredictionError = FVector::ZeroVector;
	}
};

/**
 * Spaceship flight modes
 */
UENUM(BlueprintType)
enum class EFlightMode : uint8
{
	// Standard flight mode with full control
	Normal UMETA(DisplayName = "Normal"),
	
	// Assisted flight with stability
	Assisted UMETA(DisplayName = "Assisted"),
	
	// Newtonian physics only
	Newtonian UMETA(DisplayName = "Newtonian"),
	
	// Docking mode with precision control
	Docking UMETA(DisplayName = "Docking"),
	
	// Combat mode with enhanced targeting
	Combat UMETA(DisplayName = "Combat")
};

/**
 * Ship damage states
 */
UENUM(BlueprintType)
enum class EShipDamageState : uint8
{
	Pristine UMETA(DisplayName = "Pristine"),
	MinorDamage UMETA(DisplayName = "Minor Damage"),
	ModerateDamage UMETA(DisplayName = "Moderate Damage"),
	HeavyDamage UMETA(DisplayName = "Heavy Damage"),
	Critical UMETA(DisplayName = "Critical"),
	Destroyed UMETA(DisplayName = "Destroyed")
};

/**
 * Base Spaceship class - inherits from OrbitalBody for realistic space physics
 * Supports VR interaction and 6DOF movement
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API ASpaceship : public AOrbitalBody
{
	GENERATED_BODY()

public:
	ASpaceship();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

public:
	// ============================================================================
	// CORE COMPONENTS
	// ============================================================================

	// Root component for ship
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* ShipRoot;

	// Main ship mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ShipMesh;

	// VR cockpit component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCockpitComponent* CockpitComponent;

	// Engine component for thrust
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UEngineComponent* EngineComponent;

	// Flight controller for input handling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFlightController* FlightController;

	// Ship systems manager
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShipSystemsManager* SystemsManager;

	// ============================================================================
	// ORIGIN-CENTERED PHYSICS SYSTEM (Agent 3 Integration)
	// ============================================================================

	// Player origin manager (Agent 3 component) - handles universe translation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Origin System")
	UPlayerOriginManager* OriginManager;

	// Enable origin-centered physics (feature flag for backward compatibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin System")
	bool bUseOriginCenteredPhysics = true;

	// Virtual position in universe (high precision)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Origin System")
	FSpaceshipVirtualPosition VirtualPosition;

	// Current active scale factor (from nearest celestial body)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Origin System")
	float CurrentScaleFactor = 1.0f;

	// Maximum allowed distance from origin before auto-recenter (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin System")
	float MaxOriginDistance = 10000.0f; // 10km default

	// Precision error threshold for auto-recovery (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin System")
	float PrecisionErrorThreshold = 0.01f; // 1cm

	// ============================================================================
	// NETWORK REPLICATION
	// ============================================================================

	// Replicated network state (optimized for bandwidth)
	UPROPERTY(ReplicatedUsing = OnRep_NetworkState)
	FSpaceshipNetworkState ReplicatedNetworkState;

	// Client prediction data (local only, not replicated)
	FClientPredictionData ClientPrediction;

	// Enable client-side prediction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableClientPrediction = true;

	// Enable server reconciliation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableServerReconciliation = true;

	// Network update rate (Hz)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ClampMin = "1", ClampMax = "60"))
	float NetworkUpdateRate = 20.0f;

	// Relevance radius for network culling (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float NetworkRelevanceRadius = 100000.0f; // 100km

	// Sequence counter for network packets
	uint32 NetworkSequenceCounter = 0;

	// Last network update time
	float LastNetworkUpdateTime = 0.0f;

	// OnRep function for network state
	UFUNCTION()
	void OnRep_NetworkState();

	// ============================================================================
	// VR INTEGRATION
	// ============================================================================

	// VR camera component (will be attached to cockpit)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UCameraComponent* VRCamera;

	// VR pawn reference (for hand tracking, etc.)
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	// AVRCharacter* VRPawn;

	// Is this a VR-enabled ship?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	bool bIsVREnabled = true;

	// ============================================================================
	// FLIGHT PROPERTIES
	// ============================================================================

	// Current flight mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	EFlightMode FlightMode = EFlightMode::Assisted;

	// Maximum thrust force (Newtons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float MaxThrust = 100000.0f;

	// Maximum angular velocity (degrees/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float MaxAngularVelocity = 90.0f;

	// Thrust sensitivity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float ThrustSensitivity = 0.5f;

	// Rotation sensitivity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float RotationSensitivity = 0.5f;

	// Current damage state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	EShipDamageState DamageState = EShipDamageState::Pristine;

	// Hull integrity (0-100)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	float HullIntegrity = 100.0f;

	// ============================================================================
	// INPUT HANDLING
	// ============================================================================

	// Thrust input (-1 to 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FVector ThrustInput = FVector::ZeroVector;

	// Rotation input (-1 to 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FVector RotationInput = FVector::ZeroVector;

	// ============================================================================
	// FLIGHT CONTROL
	// ============================================================================

	// Apply thrust in local space
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ApplyThrust(FVector LocalThrust);

	// Apply rotation
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ApplyRotation(FVector LocalRotation);

	// Set flight mode
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void SetFlightMode(EFlightMode NewMode);

	// Toggle between normal and assisted flight
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void ToggleAssistedFlight();

	// Emergency stop (kill all velocity)
	UFUNCTION(BlueprintCallable, Category = "Flight")
	void EmergencyStop();

	// ============================================================================
	// SHIP SYSTEMS
	// ============================================================================

// 	// Take damage
// 	UFUNCTION(BlueprintCallable, Category = "Ship")
// 	void TakeDamage(float DamageAmount, FVector HitLocation, FVector HitNormal);

	// Repair ship systems
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void RepairSystems(float RepairAmount);

	// Check if ship is operational
	UFUNCTION(BlueprintCallable, Category = "Ship")
	bool IsOperational() const;

	// Get ship status as text
	UFUNCTION(BlueprintCallable, Category = "Ship")
	FString GetShipStatus() const;

	// ============================================================================
	// VR INTERACTION
	// ============================================================================

	// Initialize VR systems
	UFUNCTION(BlueprintCallable, Category = "VR")
	void InitializeVR();

	// Get cockpit transform for VR positioning
	UFUNCTION(BlueprintCallable, Category = "VR")
	FTransform GetCockpitTransform() const;

	// Handle VR controller interaction
	UFUNCTION(BlueprintCallable, Category = "VR")
	void HandleVRControllerInteraction(class UMotionControllerComponent* Controller, bool bIsRightHand);

	// ============================================================================
	// ORIGIN-CENTERED PHYSICS FUNCTIONS
	// ============================================================================

	// Initialize origin manager (called in BeginPlay if Agent 3 components exist)
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	void InitializeOriginSystem();

	// Calculate movement for this frame (returns intended world-space movement)
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	FVector CalculateMovementThisFrame(float DeltaTime);

	// Apply movement in world-space (fallback mode)
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	void ApplyMovementWorldSpace(float DeltaTime);

	// Check if origin recentering is needed
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	bool ShouldRecenterOrigin() const;

	// Trigger origin recenter
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	void RecenterOrigin();

	// Detect and recover from precision errors
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	void DetectAndRecoverPrecisionErrors();

	// Update virtual position from world position
	UFUNCTION(BlueprintCallable, Category = "Origin System")
	void UpdateVirtualPosition();

	// ============================================================================
	// NETWORK RPC METHODS
	// ============================================================================

	// Server RPC: Client sends movement input to server
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveSpaceship(FVector Movement, FVector AngularMovement, float DeltaTime, uint32 InputHash, float Timestamp);

	// Multicast RPC: Server broadcasts universe position update to all clients
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastUpdateUniversePosition(FVector OriginOffset);

	// Client RPC: Server sends correction to specific client
	UFUNCTION(Client, Unreliable)
	void ClientCorrectPrediction(FSpaceshipNetworkState CorrectState);

	// Server RPC: Request full state sync (for late-joining clients)
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFullStateSync();

	// Client RPC: Send full state to late-joining client
	UFUNCTION(Client, Reliable)
	void ClientReceiveFullState(FSpaceshipNetworkState FullState, FSpaceshipVirtualPosition Position);

	// ============================================================================
	// SHIP CUSTOMIZATION NETWORK RPCs (Phase 3)
	// ============================================================================

	// Server RPC to apply ship customization (authority)
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerApplyCustomization(float NewMass, float NewThrustPower, float NewMaxVelocity);

	// ============================================================================
	// NETWORK PREDICTION & RECONCILIATION
	// ============================================================================

	// Predict movement locally (client-side)
	UFUNCTION(BlueprintCallable, Category = "Network")
	void PredictMovementLocal(float DeltaTime);

	// Reconcile with server state
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ReconcileWithServerState(const FSpaceshipNetworkState& ServerState);

	// Calculate prediction error
	UFUNCTION(BlueprintCallable, Category = "Network")
	float CalculatePredictionError() const;

	// Smooth correction toward server state
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ApplySmoothCorrection(float DeltaTime);

	// Update network state for replication
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdateNetworkState();

	// Check if should send network update
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool ShouldSendNetworkUpdate() const;

	// ============================================================================
	// ERROR HANDLING & SAFETY
	// ============================================================================

	// Clamp all physics values to safe ranges
	UFUNCTION(BlueprintCallable, Category = "Safety")
	void ClampPhysicsValues();

	// Limit gravitational forces (prevent extreme G-forces)
	UFUNCTION(BlueprintCallable, Category = "Safety")
	void LimitGravitationalForces(FVector& GravityForce, float DeltaTime);

	// Check for network desync and recover
	UFUNCTION(BlueprintCallable, Category = "Safety")
	void DetectAndRecoverNetworkDesync();

	// Fallback to world-space mode on critical error
	UFUNCTION(BlueprintCallable, Category = "Safety")
	void FallbackToWorldSpaceMode(const FString& Reason);

	// Validate origin manager reference
	UFUNCTION(BlueprintCallable, Category = "Safety")
	bool ValidateOriginManager() const;

	// ============================================================================
	// DEBUG & UTILITIES
	// ============================================================================

	// Show flight debug info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowFlightDebug = false;

	// Show ship status in HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowShipStatus = true;

protected:
	// Update flight physics
	void UpdateFlightPhysics(float DeltaTime);

	// Update ship systems
	void UpdateShipSystems(float DeltaTime);

	// Handle input processing
	void ProcessFlightInput(float DeltaTime);

	// Apply flight mode specific behavior
	void ApplyFlightModeBehavior(float DeltaTime);

	// Update damage state based on hull integrity
	void UpdateDamageState();

	// Draw debug information
	void DrawDebugInfo();

	// ============================================================================
	// ORIGIN-CENTERED PHYSICS INTERNAL
	// ============================================================================

	// Update origin-centered physics
	void UpdateOriginCenteredPhysics(float DeltaTime);

	// Apply gravitational forces from GravitySimulator (Agent 3)
	void ApplyGravitationalForces(float DeltaTime);

	// ============================================================================
	// NETWORK INTERNAL
	// ============================================================================

	// Network replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Send network update to server/clients
	void SendNetworkUpdate();

	// Process received network state
	void ProcessNetworkState(const FSpaceshipNetworkState& State);

	// Build current network state
	FSpaceshipNetworkState BuildNetworkState() const;

	// Calculate input hash for prediction
	uint32 CalculateInputHash() const;

private:
	// Internal state
	bool bIsThrusting = false;
	bool bIsRotating = false;
	float CurrentThrustLevel = 0.0f;
	FVector CurrentAngularVelocity = FVector::ZeroVector;

	// Performance tracking
	float LastPhysicsUpdateTime = 0.0f;
	int32 PhysicsUpdateCount = 0;

	// Network tracking
	float TimeSinceLastNetworkUpdate = 0.0f;
	FVector LastReplicatedPosition = FVector::ZeroVector;
	FVector LastReplicatedVelocity = FVector::ZeroVector;
	float AccumulatedPredictionError = 0.0f;
	int32 NetworkDesyncCount = 0;

	// Origin-centered tracking
	FVector LastOriginPosition = FVector::ZeroVector;
	float DistanceFromOrigin = 0.0f;
	bool bOriginSystemInitialized = false;
	int32 PrecisionErrorCount = 0;

	// Safety limits
	static constexpr float MAX_VELOCITY = 1000000.0f; // 1000 km/s max
	static constexpr float MAX_ANGULAR_VELOCITY_RAD = 10.0f; // ~572 deg/s max
	static constexpr float MAX_G_FORCE = 50.0f; // 50G max acceleration
	static constexpr float NETWORK_CORRECTION_THRESHOLD = 10.0f; // 10m error threshold
};