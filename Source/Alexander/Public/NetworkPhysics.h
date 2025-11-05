// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetworkPhysics.generated.h"

// Forward declarations
class AActor;
class UPrimitiveComponent;

/**
 * Network physics synchronization modes
 */
UENUM(BlueprintType)
enum class ENetworkPhysicsMode : uint8
{
	None UMETA(DisplayName = "None"),
	Authority UMETA(DisplayName = "Authority"),
	Autonomous UMETA(DisplayName = "Autonomous"),
	Simulated UMETA(DisplayName = "Simulated"),
	Replicated UMETA(DisplayName = "Replicated")
};

/**
 * Physics state data for network replication
 */
USTRUCT(BlueprintType)
struct FNetworkPhysicsState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
	FVector LinearVelocity;

	UPROPERTY(BlueprintReadOnly)
	FVector AngularVelocity;

	UPROPERTY(BlueprintReadOnly)
	bool bSimulatingPhysics;

	UPROPERTY(BlueprintReadOnly)
	float Timestamp;

	UPROPERTY(BlueprintReadOnly)
	int32 SequenceNumber;

	FNetworkPhysicsState()
	{
		Location = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
		LinearVelocity = FVector::ZeroVector;
		AngularVelocity = FVector::ZeroVector;
		bSimulatingPhysics = false;
		Timestamp = 0.0f;
		SequenceNumber = 0;
	}
};

/**
 * Physics interpolation data
 */
USTRUCT(BlueprintType)
struct FPhysicsInterpolationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FNetworkPhysicsState StartState;

	UPROPERTY(BlueprintReadOnly)
	FNetworkPhysicsState TargetState;

	UPROPERTY(BlueprintReadOnly)
	float InterpolationTime;

	UPROPERTY(BlueprintReadOnly)
	float InterpolationDuration;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInterpolating;

	FPhysicsInterpolationData()
	{
		InterpolationTime = 0.0f;
		InterpolationDuration = 0.1f;
		bIsInterpolating = false;
	}
};

/**
 * Network Physics - Handles client-side prediction and server reconciliation
 * Manages physics synchronization across the network
 */

/**
 * Wrapper for historical physics states array
 */
USTRUCT()
struct FHistoricalPhysicsStates
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FNetworkPhysicsState> States;
};

/**
 * Wrapper for input history array
 */
USTRUCT()
struct FInputHistoryData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint8> InputData;
};

UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UNetworkPhysics : public UObject
{
	GENERATED_BODY()

public:
	UNetworkPhysics();

protected:
	virtual void BeginDestroy() override;

public:
	// Actor registration
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void RegisterActor(AActor* Actor, ENetworkPhysicsMode Mode = ENetworkPhysicsMode::Replicated);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void UnregisterActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	bool IsActorRegistered(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void SetPhysicsMode(AActor* Actor, ENetworkPhysicsMode Mode);

	// State management
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	FNetworkPhysicsState GetPhysicsState(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void SetPhysicsState(AActor* Actor, const FNetworkPhysicsState& State);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void ApplyPhysicsState(AActor* Actor, const FNetworkPhysicsState& State);

	// Client-side prediction
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void EnableClientPrediction(AActor* Actor, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	bool IsClientPredictionEnabled(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void StoreInputState(AActor* Actor, uint8 Input, float Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void PredictMovement(AActor* Actor, float DeltaTime);

	// Server reconciliation
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void ReconcileWithServer(AActor* Actor, const FNetworkPhysicsState& ServerState);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void CorrectPredictionError(AActor* Actor, const FNetworkPhysicsState& CorrectState);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	float GetPredictionError(AActor* Actor) const;

	// Interpolation
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void EnableInterpolation(AActor* Actor, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	bool IsInterpolationEnabled(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void StartInterpolation(AActor* Actor, const FNetworkPhysicsState& TargetState, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void UpdateInterpolation(AActor* Actor, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void StopInterpolation(AActor* Actor);

	// Extrapolation
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void EnableExtrapolation(AActor* Actor, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	bool IsExtrapolationEnabled(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	FNetworkPhysicsState ExtrapolateState(AActor* Actor, float FutureTime);

	// Lag compensation
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void EnableLagCompensation(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	bool IsLagCompensationEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	FNetworkPhysicsState GetHistoricalState(AActor* Actor, float PastTime);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void StoreHistoricalState(AActor* Actor, const FNetworkPhysicsState& State);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void CleanupOldStates(float MaxAge);

	// Optimization
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void SetUpdateRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	float GetUpdateRate() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void SetInterpolationDelay(float Delay);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	float GetInterpolationDelay() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void OptimizePhysics();

	// Updates
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void ServerUpdate(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void ClientUpdate(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void UpdatePhysics(float DeltaTime);

	// Statistics
	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	float GetAveragePing() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	float GetAveragePredictionError() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	int32 GetReconciliationCount() const;

	UFUNCTION(BlueprintCallable, Category = "Network Physics")
	void ResetStatistics();

protected:
	// Registered actors
	UPROPERTY()
	TMap<AActor*, ENetworkPhysicsMode> RegisteredActors;

	UPROPERTY()
	TMap<AActor*, FNetworkPhysicsState> CurrentStates;

	UPROPERTY()
	TMap<AActor*, FPhysicsInterpolationData> InterpolationData;

	UPROPERTY()
	TMap<AActor*, bool> ClientPredictionEnabled;

	UPROPERTY()
	TMap<AActor*, FHistoricalPhysicsStates> HistoricalStates;

	UPROPERTY()
	TMap<AActor*, FInputHistoryData> InputHistory;

	// Physics settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	float UpdateRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	float InterpolationDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	float MaxExtrapolationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	float PredictionErrorThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	bool bEnableLagCompensation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	bool bEnableInterpolation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	bool bEnableExtrapolation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Physics")
	bool bEnableClientPrediction;

	// Internal state
	UPROPERTY()
	int32 SequenceCounter;

	UPROPERTY()
	float LastUpdateTime;

	UPROPERTY()
	float AveragePing;

	UPROPERTY()
	float AveragePredictionError;

	UPROPERTY()
	int32 ReconciliationCount;

	UPROPERTY()
	int32 TotalSamples;

private:
	FNetworkPhysicsState ExtractPhysicsState(AActor* Actor);
	void ApplyPhysicsStateInternal(AActor* Actor, const FNetworkPhysicsState& State);
	void SmoothCorrection(AActor* Actor, const FNetworkPhysicsState& TargetState);
	float CalculateInterpolationAlpha(float CurrentTime, float StartTime, float Duration) const;
	bool ShouldReconcile(AActor* Actor, const FNetworkPhysicsState& ServerState) const;
};