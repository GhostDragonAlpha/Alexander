// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ReplicationManager.generated.h"

// Forward declarations
class AActor;

/**
 * Replication priority levels
 */
UENUM(BlueprintType)
enum class EAlexanderReplicationPriority : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Replication frequency settings
 */
UENUM(BlueprintType)
enum class EReplicationFrequency : uint8
{
	Never UMETA(DisplayName = "Never"),
	Rarely UMETA(DisplayName = "Rarely (5s)"),
	Occasionally UMETA(DisplayName = "Occasionally (1s)"),
	Frequently UMETA(DisplayName = "Frequently (0.1s)"),
	Always UMETA(DisplayName = "Always (Every tick)")
};

/**
 * Actor replication data
 */
USTRUCT(BlueprintType)
struct FActorReplicationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* Actor;

	UPROPERTY(BlueprintReadOnly)
	EAlexanderReplicationPriority Priority;

	UPROPERTY(BlueprintReadOnly)
	EReplicationFrequency Frequency;

	UPROPERTY(BlueprintReadOnly)
	float LastReplicationTime;

	UPROPERTY(BlueprintReadOnly)
	bool bNeedsInitialReplication;

	UPROPERTY(BlueprintReadOnly)
	bool bForceReplication;

	UPROPERTY(BlueprintReadOnly)
	float ReplicationInterval;

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> LastKnownState;

	FActorReplicationData()
	{
		Actor = nullptr;
		Priority = EAlexanderReplicationPriority::Medium;
		Frequency = EReplicationFrequency::Occasionally;
		LastReplicationTime = 0.0f;
		bNeedsInitialReplication = true;
		bForceReplication = false;
		ReplicationInterval = 1.0f;
	}
};

/**
 * Replication statistics
 */
USTRUCT(BlueprintType)
struct FReplicationStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalActorsReplicated;

	UPROPERTY(BlueprintReadOnly)
	int32 ActorsReplicatedThisFrame;

	UPROPERTY(BlueprintReadOnly)
	float BandwidthUsed;

	UPROPERTY(BlueprintReadOnly)
	float CompressionRatio;

	UPROPERTY(BlueprintReadOnly)
	int32 PacketsSent;

	UPROPERTY(BlueprintReadOnly)
	int32 PacketsReceived;

	UPROPERTY(BlueprintReadOnly)
	float AverageReplicationTime;

	FReplicationStats()
	{
		TotalActorsReplicated = 0;
		ActorsReplicatedThisFrame = 0;
		BandwidthUsed = 0.0f;
		CompressionRatio = 0.0f;
		PacketsSent = 0;
		PacketsReceived = 0;
		AverageReplicationTime = 0.0f;
	}
};

/**
 * Replication Manager - Handles actor replication across server mesh
 * Manages what data gets sent to which clients and when
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UReplicationManager : public UObject
{
	GENERATED_BODY()

public:
	UReplicationManager();

protected:
	virtual void BeginDestroy() override;

public:
	// Actor registration
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void RegisterActor(AActor* Actor, EAlexanderReplicationPriority Priority = EAlexanderReplicationPriority::Medium, EReplicationFrequency Frequency = EReplicationFrequency::Occasionally);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void UnregisterActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	bool IsActorRegistered(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetActorPriority(AActor* Actor, EAlexanderReplicationPriority Priority);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetActorFrequency(AActor* Actor, EReplicationFrequency Frequency);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void ForceActorReplication(AActor* Actor);

	// Replication updates
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void ServerUpdate(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void ClientUpdate(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void UpdateActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void UpdateActorsInRadius(FVector Center, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void UpdateCriticalActors();

	// State management
	UFUNCTION(BlueprintCallable, Category = "Replication")
	TArray<uint8> GetActorState(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	bool SetActorState(AActor* Actor, const TArray<uint8>& State);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	bool HasActorChanged(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void MarkActorDirty(AActor* Actor);

	// Relevance filtering
	UFUNCTION(BlueprintCallable, Category = "Replication")
	bool IsActorRelevantToClient(AActor* Actor, APlayerController* Client);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	TArray<AActor*> GetRelevantActorsForClient(APlayerController* Client);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetRelevanceDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void SetAlwaysRelevantActors(const TArray<AActor*>& Actors);

	// Optimization
	UFUNCTION(BlueprintCallable, Category = "Replication")
	void OptimizeReplication();

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void CompressReplicationData();

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void PrioritizeActors();

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void ThrottleReplication(float MaxBandwidth);

	// Statistics and debugging
	UFUNCTION(BlueprintCallable, Category = "Replication")
	FReplicationStats GetReplicationStats() const;

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void ResetStats();

	UFUNCTION(BlueprintCallable, Category = "Replication")
	void LogReplicationInfo();

	UFUNCTION(BlueprintCallable, Category = "Replication")
	int32 GetRegisteredActorCount() const;

	UFUNCTION(BlueprintCallable, Category = "Replication")
	TArray<AActor*> GetRegisteredActors() const;

protected:
	// Registered actors
	UPROPERTY()
	TMap<AActor*, FActorReplicationData> RegisteredActors;

	UPROPERTY()
	TArray<AActor*> AlwaysRelevantActors;

	// Replication settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float RelevanceDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float MaxBandwidthPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	bool bEnableCompression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	bool bEnableDeltaCompression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	int32 MaxActorsPerFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float ServerUpdateRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float ClientUpdateRate;

	// Internal state
	UPROPERTY()
	FReplicationStats Stats;

	UPROPERTY()
	float LastServerUpdate;

	UPROPERTY()
	float LastClientUpdate;

	UPROPERTY()
	float CurrentBandwidthUsage;

	UPROPERTY()
	TArray<AActor*> ActorsToUpdateThisFrame;

private:
	void ProcessActorReplication(AActor* Actor, FActorReplicationData& ReplicationData, float DeltaTime);
	bool ShouldReplicateActor(const FActorReplicationData& ReplicationData, float CurrentTime);
	float GetReplicationInterval(EReplicationFrequency Frequency) const;
	TArray<uint8> CompressData(const TArray<uint8>& Data);
	TArray<uint8> DecompressData(const TArray<uint8>& Data);
	void UpdateStatistics(float ReplicationTime, int32 DataSize);
	void SortActorsByPriority();
};
