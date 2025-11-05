// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CelestialScalingSafetySystem.generated.h"

// Forward declarations
class UPlayerOriginManager;
class ASpaceship;
class AActor;

/**
 * Safety error severity levels
 */
UENUM(BlueprintType)
enum class ESafetyErrorSeverity : uint8
{
	/** Informational only, no action needed */
	Info UMETA(DisplayName = "Info"),

	/** Warning, monitor situation */
	Warning UMETA(DisplayName = "Warning"),

	/** Error, requires correction */
	Error UMETA(DisplayName = "Error"),

	/** Critical, immediate action required */
	Critical UMETA(DisplayName = "Critical"),

	/** Fatal, system shutdown required */
	Fatal UMETA(DisplayName = "Fatal")
};

/**
 * Safety event log entry
 */
USTRUCT(BlueprintType)
struct FSafetyEventLog
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Safety")
	ESafetyErrorSeverity Severity;

	UPROPERTY(BlueprintReadOnly, Category = "Safety")
	FString EventMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Safety")
	FString ActorName;

	UPROPERTY(BlueprintReadOnly, Category = "Safety")
	float Timestamp;

	FSafetyEventLog()
		: Severity(ESafetyErrorSeverity::Info)
		, Timestamp(0.0f)
	{
	}

	FSafetyEventLog(ESafetyErrorSeverity InSeverity, const FString& InMessage, const FString& InActorName, float InTimestamp)
		: Severity(InSeverity)
		, EventMessage(InMessage)
		, ActorName(InActorName)
		, Timestamp(InTimestamp)
	{
	}
};

/**
 * Celestial Scaling Safety System (Agent 4)
 *
 * Game instance subsystem for monitoring and enforcing safety limits
 * across the celestial scaling system
 *
 * This system:
 * - Validates scale factors to prevent rendering/physics issues
 * - Clamps gravitational forces to prevent extreme G-forces
 * - Detects floating-point precision errors
 * - Provides automatic recovery mechanisms
 * - Logs safety events for debugging
 * - Monitors network desync conditions
 *
 * The safety system acts as a final safeguard layer, ensuring that
 * all values remain within physically and computationally reasonable bounds
 */
UCLASS()
class ALEXANDER_API UCelestialScalingSafetySystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================================
	// SUBSYSTEM LIFECYCLE
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================================
	// SCALE FACTOR SAFETY
	// ============================================================================

	/**
	 * Clamp scale factor to safe rendering/physics bounds
	 * @param Factor - Scale factor to clamp
	 * @return Clamped scale factor within safe range
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float ClampScaleFactor(float Factor) const;

	/**
	 * Check if scale factor is within valid range
	 * @param Factor - Scale factor to validate
	 * @return True if factor is safe to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool IsValidScaleFactor(float Factor) const;

	/**
	 * Get recommended scale factor based on distance
	 * Applies logarithmic scaling with safety bounds
	 * @param Distance - Distance to object in Unreal units
	 * @return Safe scale factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float GetSafeScaleFactor(float Distance) const;

	// ============================================================================
	// GRAVITATIONAL FORCE SAFETY
	// ============================================================================

	/**
	 * Clamp gravitational force to prevent extreme G-forces
	 * @param Force - Gravitational force vector in Newtons
	 * @param TargetMass - Mass of the object experiencing force (kg)
	 * @param MaxGForce - Maximum allowed G-force (default: 50G)
	 * @return Clamped force vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	FVector ClampGravitationalForce(const FVector& Force, float TargetMass, float MaxGForce = 50.0f) const;

	/**
	 * Check if gravitational force is within safe limits
	 * @param Force - Force vector to check
	 * @param TargetMass - Mass of the object
	 * @return True if force is safe
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool IsForceWithinSafeLimits(const FVector& Force, float TargetMass) const;

	/**
	 * Calculate G-force from acceleration
	 * @param Acceleration - Acceleration vector in m/s²
	 * @return G-force magnitude
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float CalculateGForce(const FVector& Acceleration) const;

	/**
	 * Get maximum safe velocity based on current scale factor
	 * @param ScaleFactor - Active scale factor
	 * @return Maximum safe velocity in m/s
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float GetMaxSafeVelocity(float ScaleFactor) const;

	// ============================================================================
	// PRECISION ERROR DETECTION
	// ============================================================================

	/**
	 * Detect floating-point precision errors in position
	 * @param Position - Position vector to check
	 * @return True if precision degradation detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool DetectPrecisionError(const FVector& Position) const;

	/**
	 * Calculate floating-point precision error magnitude
	 * @param OriginalValue - Original double precision value
	 * @param ConvertedValue - Converted single precision value
	 * @return Error magnitude (percentage)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float CalculateFloatingPointError(double OriginalValue, float ConvertedValue) const;

	/**
	 * Check if position is within safe precision range
	 * @param Position - Position to check
	 * @return True if position maintains acceptable precision
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool IsPositionPrecisionSafe(const FVector& Position) const;

	/**
	 * Get recommended precision threshold based on distance from origin
	 * @param DistanceFromOrigin - Distance from world origin
	 * @return Recommended precision threshold in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float GetPrecisionThreshold(float DistanceFromOrigin) const;

	// ============================================================================
	// AUTOMATIC RECOVERY
	// ============================================================================

	/**
	 * Trigger emergency universe recenter
	 * Forces immediate recentering regardless of thresholds
	 * @param OriginManager - Origin manager to recenter
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void TriggerEmergencyRecenter(UPlayerOriginManager* OriginManager);

	/**
	 * Fallback to world-space mode (disable scaling)
	 * Used when origin-centered physics fails critically
	 * @param Ship - Spaceship to revert to world-space mode
	 * @param Reason - Reason for fallback (for logging)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void FallbackToWorldSpaceMode(ASpaceship* Ship, const FString& Reason);

	/**
	 * Reset actor to safe state
	 * Clears velocity, resets position to origin, clears errors
	 * @param Actor - Actor to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void ResetActorToSafeState(AActor* Actor);

	/**
	 * Validate and repair actor physics state
	 * Checks for NaN/Inf, out-of-bounds values, etc.
	 * @param Actor - Actor to validate
	 * @return True if repairs were needed and applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool ValidateAndRepairPhysicsState(AActor* Actor);

	// ============================================================================
	// NETWORK SAFETY
	// ============================================================================

	/**
	 * Log network desynchronization event
	 * @param Actor - Actor experiencing desync
	 * @param ClientPosition - Client's position
	 * @param ServerPosition - Server's authoritative position
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void LogNetworkDesync(AActor* Actor, const FVector& ClientPosition, const FVector& ServerPosition);

	/**
	 * Check if network desync is within acceptable bounds
	 * @param ErrorDistance - Distance between client and server positions
	 * @return True if within acceptable tolerance
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool IsDesyncAcceptable(float ErrorDistance) const;

	/**
	 * Get network correction strength based on error
	 * Returns 0.0-1.0 blend factor for smooth correction
	 * @param ErrorDistance - Distance error in meters
	 * @return Correction strength (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	float GetNetworkCorrectionStrength(float ErrorDistance) const;

	// ============================================================================
	// LOGGING AND MONITORING
	// ============================================================================

	/**
	 * Log a safety warning event
	 * @param Message - Warning message
	 * @param Severity - Event severity level
	 * @param Actor - Actor related to event (optional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void LogSafetyEvent(const FString& Message, ESafetyErrorSeverity Severity, AActor* Actor = nullptr);

	/**
	 * Get recent safety events
	 * @param MaxEvents - Maximum number of events to return
	 * @return Array of recent safety events
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	TArray<FSafetyEventLog> GetRecentSafetyEvents(int32 MaxEvents = 10) const;

	/**
	 * Clear safety event log
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void ClearEventLog();

	/**
	 * Get safety statistics
	 * @param OutTotalEvents - Total events logged
	 * @param OutCriticalEvents - Number of critical events
	 * @param OutRecoveryOperations - Number of automatic recoveries
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void GetSafetyStatistics(int32& OutTotalEvents, int32& OutCriticalEvents, int32& OutRecoveryOperations) const;

	/**
	 * Enable or disable safety system
	 * WARNING: Disabling safety system can lead to crashes!
	 * @param bEnabled - Whether to enable safety checks
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	void SetSafetySystemEnabled(bool bEnabled) { bSafetySystemEnabled = bEnabled; }

	/**
	 * Check if safety system is enabled
	 * @return True if safety checks are active
	 */
	UFUNCTION(BlueprintCallable, Category = "Celestial|Safety")
	bool IsSafetySystemEnabled() const { return bSafetySystemEnabled; }

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Enable/disable safety system */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|General")
	bool bSafetySystemEnabled;

	/** Enable automatic recovery on errors */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|General")
	bool bAutoRecoveryEnabled;

	/** Enable detailed safety logging */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|General")
	bool bVerboseLogging;

	/** Minimum allowed scale factor */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Scale", meta = (ClampMin = "0.0001"))
	float MinScaleFactor;

	/** Maximum allowed scale factor */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Scale", meta = (ClampMin = "1.0"))
	float MaxScaleFactor;

	/** Maximum allowed G-force */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Physics", meta = (ClampMin = "1.0"))
	float MaxGForce;

	/** Maximum safe velocity (m/s) */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Physics", meta = (ClampMin = "100.0"))
	float MaxSafeVelocityMS;

	/** Distance threshold for precision warnings (meters) */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Precision", meta = (ClampMin = "10000.0"))
	float PrecisionWarningThreshold;

	/** Distance threshold for emergency recentering (meters) */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Precision", meta = (ClampMin = "100000.0"))
	float EmergencyRecenterThreshold;

	/** Network desync tolerance (meters) */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Network", meta = (ClampMin = "0.1"))
	float NetworkDesyncTolerance;

	/** Maximum event log size */
	UPROPERTY(EditDefaultsOnly, Category = "Safety|Logging", meta = (ClampMin = "10"))
	int32 MaxEventLogSize;

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Safety event log (circular buffer) */
	TArray<FSafetyEventLog> EventLog;

	/** Statistics */
	int32 TotalEventCount;
	int32 CriticalEventCount;
	int32 RecoveryOperationCount;
	int32 DesyncEventCount;

	/** Last event timestamp */
	float LastEventTime;

	// ============================================================================
	// CONSTANTS
	// ============================================================================

	/** Standard gravity (m/s²) */
	static constexpr float STANDARD_GRAVITY = 9.81f;

	/** Minimum distance for gravity calculations (prevents singularities) */
	static constexpr float MIN_GRAVITY_DISTANCE = 1.0f; // 1 meter

	/** Floating-point epsilon for comparisons */
	static constexpr float FLOAT_EPSILON = 1e-6f;

	/** Conversion: Unreal units (cm) to meters */
	static constexpr float CM_TO_METERS = 0.01f;

	/** Conversion: Meters to kilometers */
	static constexpr float M_TO_KM = 0.001f;

	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	/** Add event to log with automatic pruning */
	void AddEventToLog(const FSafetyEventLog& Event);

	/** Check if value is NaN or Inf */
	bool IsValueInvalid(float Value) const;

	/** Check if vector contains NaN or Inf */
	bool IsVectorInvalid(const FVector& Vector) const;

	/** Format safety message with context */
	FString FormatSafetyMessage(const FString& Message, AActor* Actor) const;
};
