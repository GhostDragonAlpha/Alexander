// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OrbitalBody.h"
#include "OrbitalMechanics.h"
#include "PhysicsConsensus.generated.h"

/**
 * Validation state for player position tracking
 */
UENUM(BlueprintType)
enum class EValidationState : uint8
{
	// Player is trusted, all validations passing
	TRUSTED UMETA(DisplayName = "Trusted"),

	// Player has failed some validations, under scrutiny
	SUSPECT UMETA(DisplayName = "Suspect"),

	// Player has failed consensus threshold, flagged as cheater
	FLAGGED UMETA(DisplayName = "Flagged"),

	// Player has been kicked from session
	KICKED UMETA(DisplayName = "Kicked")
};

/**
 * Position report from a player (network message data)
 */
USTRUCT(BlueprintType)
struct FPositionReport
{
	GENERATED_BODY()

	// Player ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 PlayerID = 0;

	// Timestamp (seconds since session start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	float Timestamp = 0.0f;

	// Reported position (world space, meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	FVector Position = FVector::ZeroVector;

	// Reported velocity (m/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	FVector Velocity = FVector::ZeroVector;

	// Reported thrust vector (Newtons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	FVector Thrust = FVector::ZeroVector;

	// Reported rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	FRotator Rotation = FRotator::ZeroRotator;

	// Sequence number (for packet ordering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 SequenceNumber = 0;
};

/**
 * Validation vote from a client
 */
USTRUCT(BlueprintType)
struct FValidationVote
{
	GENERATED_BODY()

	// ID of client casting vote
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 ValidatorID = 0;

	// ID of player being validated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 TargetPlayerID = 0;

	// Sequence number of report being voted on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 SequenceNumber = 0;

	// Is the position valid?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	bool bIsValid = true;

	// Position error in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	float PositionError = 0.0f;

	// Thrust error in Newtons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	float ThrustError = 0.0f;
};

/**
 * UE 5.6 Wrapper: TArray<FPositionReport> for TMap compatibility
 */
USTRUCT(BlueprintType)
struct FPositionReportArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	TArray<FPositionReport> Reports;
};

/**
 * UE 5.6 Wrapper: TArray<FValidationVote> for TMap compatibility
 */
USTRUCT(BlueprintType)
struct FValidationVoteArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	TArray<FValidationVote> Votes;
};

/**
 * UE 5.6 Wrapper: TMap<int32, TArray<FValidationVote>> for nested TMap compatibility
 * This wraps the inner TMap to allow TMap<int32, TMap<int32, TArray<FValidationVote>>>
 */
USTRUCT(BlueprintType)
struct FValidationVoteSequenceMap
{
	GENERATED_BODY()

	// Map from sequence number to array of votes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	TMap<int32, FValidationVoteArray> SequenceVotes;
};

/**
 * Consensus result for a player
 */
USTRUCT(BlueprintType)
struct FConsensusResult
{
	GENERATED_BODY()

	// Player ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 PlayerID = 0;

	// Did consensus reach agreement?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	bool bConsensusReached = false;

	// Number of valid votes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 ValidVotes = 0;

	// Number of invalid votes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	int32 InvalidVotes = 0;

	// Average position error
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consensus")
	float AveragePositionError = 0.0f;
};

/**
 * Physics Consensus System
 *
 * THE REVOLUTIONARY FEATURE: Cheat-proof multiplayer via physics validation
 *
 * Every client validates other players' positions using gravity predictions and thrust verification.
 * Position AND thrust must be consistent with physics calculations, making cheating physically impossible.
 *
 * Key Principles:
 * - "You wouldn't be able to lie either way" - Both position and thrust are validated
 * - Each client = consensus server (decentralized validation)
 * - Byzantine fault tolerance (2/3 majority voting)
 * - Distance-over-time validation for long-term consistency
 *
 * Integration:
 * - Uses existing AOrbitalBody system for gravity calculations
 * - Leverages UOrbitalMechanics for deterministic physics
 * - Works with FlightController for thrust tracking
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UPhysicsConsensus : public UObject
{
	GENERATED_BODY()

public:
	UPhysicsConsensus();

	// ============================================================================
	// CORE PREDICTION AND VALIDATION
	// ============================================================================

	/**
	 * Predict where a player SHOULD be based on physics (gravity + last thrust)
	 *
	 * @param PlayerID - ID of player to predict
	 * @param DeltaTime - Time elapsed since last known state
	 * @return Predicted position in world space (meters)
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Prediction")
	FVector PredictPosition(int32 PlayerID, float DeltaTime);

	/**
	 * Validate reported position against physics prediction
	 *
	 * @param PlayerID - ID of player reporting position
	 * @param ReportedPosition - Position claimed by player
	 * @param Timestamp - Time of report
	 * @return True if position is valid within tolerance
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Validation")
	bool ValidatePosition(int32 PlayerID, FVector ReportedPosition, float Timestamp);

	/**
	 * Validate reported thrust explains trajectory deviation from gravity
	 *
	 * @param PlayerID - ID of player reporting thrust
	 * @param ReportedThrust - Thrust vector claimed by player (Newtons)
	 * @param ReportedPosition - Current position
	 * @return True if thrust explains the deviation
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Validation")
	bool ValidateThrust(int32 PlayerID, FVector ReportedThrust, FVector ReportedPosition);

	/**
	 * Validate distance traveled over time is physically possible
	 *
	 * @param PlayerID - ID of player to validate
	 * @param TimeWindow - Time window to check (seconds)
	 * @return True if distance is within possible range
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Validation")
	bool ValidateDistanceOverTime(int32 PlayerID, float TimeWindow = 5.0f);

	// ============================================================================
	// POSITION REPORTING
	// ============================================================================

	/**
	 * Add position report from a player
	 *
	 * @param Report - Position report data
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Reports")
	void AddPositionReport(const FPositionReport& Report);

	/**
	 * Get last known position for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Last reported position
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Reports")
	FVector GetLastKnownPosition(int32 PlayerID) const;

	/**
	 * Get last known velocity for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Last reported velocity
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Reports")
	FVector GetLastKnownVelocity(int32 PlayerID) const;

	/**
	 * Get last reported thrust for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Last reported thrust
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Reports")
	FVector GetLastReportedThrust(int32 PlayerID) const;

	/**
	 * Get timestamp of last report
	 *
	 * @param PlayerID - ID of player
	 * @return Timestamp (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Reports")
	float GetLastTimestamp(int32 PlayerID) const;

	// ============================================================================
	// CONSENSUS VOTING
	// ============================================================================

	/**
	 * Submit validation vote for a player's position
	 *
	 * @param Vote - Validation vote data
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Voting")
	void SubmitValidationVote(const FValidationVote& Vote);

	/**
	 * Calculate consensus result for a player
	 *
	 * @param PlayerID - ID of player
	 * @param SequenceNumber - Sequence number of report to check
	 * @return Consensus result with vote counts
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Voting")
	FConsensusResult CalculateConsensus(int32 PlayerID, int32 SequenceNumber);

	/**
	 * Check if consensus has been reached (2/3 majority)
	 *
	 * @param PlayerID - ID of player
	 * @return True if consensus reached
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Voting")
	bool IsConsensusReached(int32 PlayerID);

	// ============================================================================
	// STATE MANAGEMENT
	// ============================================================================

	/**
	 * Get validation state for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Current validation state
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|State")
	EValidationState GetValidationState(int32 PlayerID) const;

	/**
	 * Update player validation state based on consensus result
	 *
	 * @param PlayerID - ID of player
	 * @param bValidationPassed - Did validation pass?
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|State")
	void UpdateValidationState(int32 PlayerID, bool bValidationPassed);

	/**
	 * Check if player should be kicked
	 *
	 * @param PlayerID - ID of player
	 * @return True if player has exceeded kick threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|State")
	bool ShouldKickPlayer(int32 PlayerID) const;

	/**
	 * Reset validation state for a player (e.g., on network issues)
	 *
	 * @param PlayerID - ID of player
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|State")
	void ResetValidationState(int32 PlayerID);

	// ============================================================================
	// ORBITAL BODY INTEGRATION
	// ============================================================================

	/**
	 * Register a player's orbital body for physics tracking
	 *
	 * @param PlayerID - ID of player
	 * @param OrbitalBody - Player's orbital body actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Setup")
	void RegisterPlayer(int32 PlayerID, AOrbitalBody* OrbitalBody);

	/**
	 * Unregister a player (on disconnect)
	 *
	 * @param PlayerID - ID of player
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Setup")
	void UnregisterPlayer(int32 PlayerID);

	/**
	 * Get orbital body for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Player's orbital body (may be null)
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Setup")
	AOrbitalBody* GetPlayerOrbitalBody(int32 PlayerID) const;

	// ============================================================================
	// TOLERANCE CALCULATIONS
	// ============================================================================

	/**
	 * Calculate position tolerance based on time and network conditions
	 *
	 * @param DeltaTime - Time since last report
	 * @param PlayerID - ID of player (for latency lookup)
	 * @return Tolerance in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Tolerance")
	float CalculatePositionTolerance(float DeltaTime, int32 PlayerID) const;

	/**
	 * Calculate thrust tolerance
	 *
	 * @return Tolerance in Newtons
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Tolerance")
	float CalculateThrustTolerance() const;

	/**
	 * Get network latency for a player
	 *
	 * @param PlayerID - ID of player
	 * @return Latency in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Physics Consensus|Tolerance")
	float GetNetworkLatency(int32 PlayerID) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Base position tolerance (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float BasePositionTolerance = 1.0f;

	// Time-based tolerance rate (meters per second of prediction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float TimeToleranceRate = 0.5f;

	// Base thrust tolerance (Newtons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float BaseThrustTolerance = 1000.0f;

	// Thrust tolerance as percentage of max thrust
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float ThrustTolerancePercentage = 0.05f; // 5%

	// Consensus threshold (fraction of votes needed, 0.67 = 2/3 majority)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float ConsensusThreshold = 0.67f;

	// Failures before flagging player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	int32 FlagThreshold = 3;

	// Failures before kicking player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	int32 KickThreshold = 5;

	// Time window for kick threshold (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float KickTimeWindow = 10.0f;

	// Maximum position history to store per player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	int32 MaxHistorySize = 100;

	// Maximum thrust force (Newtons) - for validation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float MaxThrustForce = 100000.0f; // 100 kN

	// Maximum speed (m/s) - for validation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float MaxSpeed = 1000.0f; // 1000 m/s

	// Player mass (kg) - used for thrust calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Config")
	float PlayerMass = 50000.0f; // 50 tons

	// Enable debug logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consensus|Debug")
	bool bEnableDebugLogging = false;

protected:
	// Position history per player (circular buffer)
	// UE 5.6: Wrapped TArray in FPositionReportArray for UPROPERTY compatibility
	UPROPERTY()
	TMap<int32, FPositionReportArray> PositionHistory;

	// Validation votes per player per sequence number
	// UE 5.6: Double-wrapped for nested TMap compatibility
	// Outer TMap: PlayerID -> FValidationVoteSequenceMap
	// Inner TMap (in wrapper): SequenceNumber -> FValidationVoteArray
	// Innermost TArray (in wrapper): TArray<FValidationVote>
	UPROPERTY()
	TMap<int32, FValidationVoteSequenceMap> ValidationVotes;

	// Validation state per player
	TMap<int32, EValidationState> ValidationStates;

	// Failure count per player (for kick threshold)
	TMap<int32, int32> FailureCounts;

	// Timestamp of first failure in current window
	TMap<int32, float> FirstFailureTimestamp;

	// Registered player orbital bodies
	TMap<int32, TWeakObjectPtr<AOrbitalBody>> PlayerOrbitalBodies;

	// Network latency per player (seconds)
	TMap<int32, float> NetworkLatencies;

	// Helper: Get position history for player (returns wrapper's Reports array)
	TArray<FPositionReport>* GetOrCreatePositionHistory(int32 PlayerID);

	// Helper: Add to circular buffer with max size
	void AddToHistory(TArray<FPositionReport>& History, const FPositionReport& Report);

	// Helper: Calculate total distance from history
	float CalculateTotalDistance(const TArray<FPositionReport>& History, float TimeWindow) const;

	// Helper: Calculate maximum possible distance
	float CalculateMaxPossibleDistance(const TArray<FPositionReport>& History, float TimeWindow) const;

	// Helper: Calculate minimum possible distance (gravity only)
	float CalculateMinPossibleDistance(const TArray<FPositionReport>& History, float TimeWindow) const;
};
