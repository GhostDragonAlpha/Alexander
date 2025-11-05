// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RelativitySystem.generated.h"

// Forward declarations
class APlayerController;
class UPlayerTimeComponent;

/**
 * Player time tracking data
 */
USTRUCT(BlueprintType)
struct FPlayerTimeData
{
	GENERATED_BODY()

	// Starting age of player (years)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float BiologicalAge = 25.0f;

	// Total real-world time elapsed (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double TotalRealTimeSeconds = 0.0;

	// Total world time experienced by this player (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double TotalWorldTimeSeconds = 0.0;

	// Time debt: how much time passed in universe vs player experience (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double TimeDebt = 0.0;

	// Total accumulated velocity-based time dilation factor
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double CumulativeVelocityDilation = 0.0;

	// Total accumulated gravitational time dilation factor
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double CumulativeGravityDilation = 0.0;

	// Current velocity (m/s)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float CurrentVelocity = 0.0f;

	// Current altitude above nearest massive body (m)
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float CurrentAltitude = 0.0f;

	// Session start time
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	double SessionStartTime = 0.0;
};

/**
 * Time warp prediction data
 */
USTRUCT(BlueprintType)
struct FTimeWarpPrediction
{
	GENERATED_BODY()

	// Requested time warp factor
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	float TimeWarpFactor = 1.0f;

	// Duration in real seconds
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	float RealDurationSeconds = 0.0f;

	// Predicted universe time advancement (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	double UniverseTimeAdvancement = 0.0;

	// Predicted player biological time advancement (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	double PlayerBiologicalTimeAdvancement = 0.0;

	// Predicted time debt increase (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	double TimeDebtIncrease = 0.0;

	// Warning level (0=safe, 1=moderate, 2=high, 3=extreme)
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	int32 WarningLevel = 0;

	// Human-readable time debt string (e.g., "3 days, 4 hours")
	UPROPERTY(BlueprintReadOnly, Category = "Prediction")
	FString FormattedTimeDebt;
};

/**
 * Relativity event delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeDebtChanged, APlayerController*, Player, double, NewTimeDebt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExtremeDilationEntered, APlayerController*, Player, float, DilationFactor, FString, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeWarpActivated, float, WarpFactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeWarpDeactivated);

/**
 * Relativity System
 * Manages time dilation effects from velocity and gravity
 * Tracks player biological aging vs universal time
 * Integrates with time warp mechanics for meaningful gameplay consequences
 */
UCLASS()
class ALEXANDER_API URelativitySystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URelativitySystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function (called from GameInstance or manually)
	void Tick(float DeltaTime);

	// ============================================================================
	// TIME DILATION CALCULATIONS (Special & General Relativity)
	// ============================================================================

	/**
	 * Calculate velocity-based time dilation (Special Relativity)
	 * Formula: t' = t * sqrt(1 - v²/c²)
	 * @param VelocityMetersPerSec Velocity in m/s
	 * @return Time dilation factor (1.0 = normal, < 1.0 = time slows)
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Calculations")
	float CalculateVelocityTimeDilation(float VelocityMetersPerSec) const;

	/**
	 * Calculate gravitational time dilation (General Relativity)
	 * Formula: t' = t * sqrt(1 - (2GM)/(rc²))
	 * @param Altitude Distance from center of massive body (m)
	 * @param BodyMass Mass of the body (kg)
	 * @param BodyRadius Radius of the body (m)
	 * @return Time dilation factor (1.0 = normal, < 1.0 = time slows)
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Calculations")
	float CalculateGravitationalTimeDilation(float Altitude, float BodyMass, float BodyRadius) const;

	/**
	 * Get combined time dilation for an actor
	 * Combines velocity and gravitational effects
	 * @param Actor Actor to calculate for
	 * @return Combined time dilation factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Calculations")
	float GetCombinedTimeDilation(AActor* Actor) const;

	/**
	 * Calculate effective time dilation including time warp factor
	 * @param Actor Actor experiencing dilation
	 * @param TimeWarpFactor Current time warp multiplier
	 * @return Effective time dilation considering both relativity and time warp
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Calculations")
	float GetEffectiveTimeDilation(AActor* Actor, float TimeWarpFactor) const;

	// ============================================================================
	// PLAYER TIME TRACKING
	// ============================================================================

	/**
	 * Register a player for time tracking
	 * @param Player Player controller to track
	 * @param StartingAge Initial biological age (default 25 years)
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Tracking")
	void RegisterPlayer(APlayerController* Player, float StartingAge = 25.0f);

	/**
	 * Unregister a player from time tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Tracking")
	void UnregisterPlayer(APlayerController* Player);

	/**
	 * Update player's biological age based on time dilation
	 * Called each tick with real delta time
	 * @param Player Player controller
	 * @param RealDeltaTime Real-world time elapsed (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Tracking")
	void UpdatePlayerAge(APlayerController* Player, float RealDeltaTime);

	/**
	 * Get player's current biological age
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Tracking")
	float GetPlayerBiologicalAge(APlayerController* Player) const;

	/**
	 * Get player's time debt (universe time - player time)
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Tracking")
	double GetTimeDebt(APlayerController* Player) const;

	/**
	 * Get formatted time debt string
	 * @param Player Player controller
	 * @return Human-readable string like "3 days, 4 hours, 12 minutes"
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Tracking")
	FString GetFormattedTimeDebt(APlayerController* Player) const;

	/**
	 * Get player time data
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Tracking")
	FPlayerTimeData GetPlayerTimeData(APlayerController* Player) const;

	// ============================================================================
	// UNIVERSAL TIME
	// ============================================================================

	/**
	 * Get current universal time (seconds since game start)
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Universal")
	double GetUniversalTime() const { return UniversalTime; }

	/**
	 * Get formatted universal time
	 * @return String like "Year 1, Day 145, 14:32:18"
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Universal")
	FString GetFormattedUniversalTime() const;

	/**
	 * Advance universal time (called by time warp or normal tick)
	 * @param DeltaSeconds Time to advance in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Universal")
	void AdvanceUniversalTime(float DeltaSeconds);

	// ============================================================================
	// TIME WARP INTERACTION
	// ============================================================================

	/**
	 * Apply time warp factor
	 * Universe advances faster, player ages slower due to relativistic protection
	 * @param Factor Time warp multiplier (1.0 = normal, 100.0 = 100x speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|TimeWarp")
	void ApplyTimeWarpFactor(float Factor);

	/**
	 * Get current time warp factor
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|TimeWarp")
	float GetCurrentTimeWarpFactor() const { return CurrentTimeWarpFactor; }

	/**
	 * Predict consequences of activating time warp
	 * @param WarpFactor Desired time warp factor
	 * @param DurationSeconds How long warp will be active (real time)
	 * @param Player Player to predict for
	 * @return Prediction data with warnings
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|TimeWarp")
	FTimeWarpPrediction PredictTimeWarpConsequences(float WarpFactor, float DurationSeconds, APlayerController* Player) const;

	/**
	 * Check if time warp requires warning
	 * @param WarpFactor Desired warp factor
	 * @param DurationSeconds Duration in real seconds
	 * @return True if player should be warned
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|TimeWarp")
	bool ShouldWarnAboutTimeWarp(float WarpFactor, float DurationSeconds) const;

	// ============================================================================
	// GAMEPLAY CONSEQUENCES
	// ============================================================================

	/**
	 * Check if player has accumulated significant time debt
	 * Used for achievements and story triggers
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Consequences")
	bool HasSignificantTimeDebt(APlayerController* Player, float ThresholdHours = 24.0f) const;

	/**
	 * Check if player is experiencing extreme time dilation
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Consequences")
	bool IsExperiencingExtremeDilation(APlayerController* Player, float Threshold = 0.5f) const;

	/**
	 * Get time dilation color for UI (green=normal, yellow=moderate, red=extreme)
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|UI")
	FLinearColor GetTimeDilationColor(float DilationFactor) const;

	/**
	 * Get time debt severity (0-3: none, low, moderate, high)
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Consequences")
	int32 GetTimeDebtSeverity(APlayerController* Player) const;

	// ============================================================================
	// NETWORK REPLICATION
	// ============================================================================

	/**
	 * Sync time data for late-joining players
	 */
	UFUNCTION(BlueprintCallable, Category = "Relativity|Network")
	void SyncPlayerOnJoin(APlayerController* Player);

	/**
	 * Get all tracked players
	 */
	UFUNCTION(BlueprintPure, Category = "Relativity|Network")
	TArray<APlayerController*> GetTrackedPlayers() const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Enable/disable relativistic effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity|Config")
	bool bEnableRelativisticEffects = true;

	// Enable/disable time debt accumulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity|Config")
	bool bEnableTimeDebt = true;

	// Scale factor for relativity effects (1.0 = realistic, higher = more dramatic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity|Config", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float RelativityScaleFactor = 1.0f;

	// Minimum time warp factor that requires warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity|Config", meta = (ClampMin = "1.0"))
	float TimeWarpWarningThreshold = 10.0f;

	// Time debt threshold for achievements (hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity|Config")
	float AchievementTimeDebtThreshold = 168.0f; // 1 week

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Relativity|Events")
	FOnTimeDebtChanged OnTimeDebtChanged;

	UPROPERTY(BlueprintAssignable, Category = "Relativity|Events")
	FOnExtremeDilationEntered OnExtremeDilationEntered;

	UPROPERTY(BlueprintAssignable, Category = "Relativity|Events")
	FOnTimeWarpActivated OnTimeWarpActivated;

	UPROPERTY(BlueprintAssignable, Category = "Relativity|Events")
	FOnTimeWarpDeactivated OnTimeWarpDeactivated;

private:
	// ============================================================================
	// PHYSICAL CONSTANTS
	// ============================================================================

	// Speed of light (m/s)
	static constexpr double SPEED_OF_LIGHT = 299792458.0;

	// Gravitational constant (m³/kg/s²)
	static constexpr double GRAV_CONSTANT = 6.67430e-11;

	// ============================================================================
	// TIME TRACKING DATA
	// ============================================================================

	// Universal time (seconds since game start)
	UPROPERTY()
	double UniversalTime = 0.0;

	// Current time warp factor
	UPROPERTY()
	float CurrentTimeWarpFactor = 1.0f;

	// Player time data map
	UPROPERTY()
	TMap<APlayerController*, FPlayerTimeData> PlayerTimeData;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Calculate nearest massive body for gravitational dilation
	void FindNearestMassiveBody(AActor* Actor, float& OutMass, float& OutRadius, float& OutDistance) const;

	// Format time duration to human-readable string
	FString FormatTimeDuration(double Seconds) const;

	// Calculate warning level for time warp
	int32 CalculateWarningLevel(double TimeDebtIncrease) const;

	// Update time debt tracking
	void UpdateTimeDebt(APlayerController* Player, double UniverseTimeDelta, double PlayerTimeDelta);
};
