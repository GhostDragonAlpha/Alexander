// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerTimeComponent.generated.h"

// Forward declarations
class URelativitySystem;
class APlayerController;

/**
 * Player Time Component
 * Attached to player pawn/character to track their personal time progression
 * Replicates time data to all clients for UI display
 * Provides hooks for chronometer and UI elements
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlayerTimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerTimeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// REPLICATED TIME DATA
	// ============================================================================

	// Current biological age (replicated)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float BiologicalAge;

	// Current time debt (replicated)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	double TimeDebt;

	// Current time dilation factor being experienced
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float CurrentTimeDilation;

	// Total real time this player has been active (seconds)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	double TotalRealTime;

	// Total world time experienced by player (seconds)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	double TotalWorldTime;

	// Current velocity (m/s)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float CurrentVelocity;

	// Current altitude above nearest body (m)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time")
	float CurrentAltitude;

	// ============================================================================
	// TIME MANAGEMENT
	// ============================================================================

	/**
	 * Initialize the time component
	 * @param StartingAge Initial biological age
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void Initialize(float StartingAge = 25.0f);

	/**
	 * Advance biological age by delta seconds
	 * Should be called by RelativitySystem, not manually
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void AdvanceBiologicalAge(float SecondsDelta);

	/**
	 * Update time data from RelativitySystem
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void UpdateFromRelativitySystem();

	// ============================================================================
	// UI QUERIES
	// ============================================================================

	/**
	 * Get formatted time debt string
	 * @return Human-readable string like "3 days, 4 hours"
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	FString GetFormattedTimeDebt() const;

	/**
	 * Get formatted biological age
	 * @return String like "25.3 years"
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	FString GetFormattedBiologicalAge() const;

	/**
	 * Get formatted total session time
	 * @return String like "2 hours, 15 minutes"
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	FString GetFormattedSessionTime() const;

	/**
	 * Get time dilation color for UI
	 * Green = normal, Yellow = moderate, Red = extreme
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	FLinearColor GetTimeDilationColor() const;

	/**
	 * Get time debt severity (0-3)
	 * 0 = none, 1 = low, 2 = moderate, 3 = high
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	int32 GetTimeDebtSeverity() const;

	/**
	 * Is experiencing extreme time dilation?
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	bool IsExperiencingExtremeDilation() const;

	/**
	 * Get velocity as percentage of speed of light
	 */
	UFUNCTION(BlueprintPure, Category = "Time|UI")
	float GetVelocityPercentageOfLightSpeed() const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Should this component auto-register with RelativitySystem?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Config")
	bool bAutoRegister = true;

	// Update frequency (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Config", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float UpdateFrequency = 0.5f;

	// Display extreme dilation warnings?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Config")
	bool bShowDilationWarnings = true;

private:
	// Reference to RelativitySystem
	UPROPERTY()
	URelativitySystem* RelativitySystem;

	// Reference to owning player controller
	UPROPERTY()
	APlayerController* OwningPlayerController;

	// Update timer
	float UpdateTimer;

	// Has been initialized?
	bool bInitialized;

	// Helper functions
	void RegisterWithRelativitySystem();
	void UnregisterFromRelativitySystem();
	APlayerController* FindOwningPlayerController() const;
};
