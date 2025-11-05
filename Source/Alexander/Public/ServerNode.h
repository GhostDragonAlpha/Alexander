// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ServerNode.generated.h"

/**
 * Server load balancing status
 */
UENUM(BlueprintType)
enum class EServerLoadStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
	Overloaded UMETA(DisplayName = "Overloaded"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Server Node - Individual server instance in mesh network
 * Handles player connections and load balancing
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API AServerNode : public AActor
{
	GENERATED_BODY()

public:
	AServerNode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// Server properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	FString ServerID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	FVector NodeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	int32 MaxPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	int32 CurrentPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	float ServerLoad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	EServerLoadStatus LoadStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	bool bIsActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	float BandwidthUsage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	float CPUUsage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Server")
	float MemoryUsage;

	// Player management
	UFUNCTION(BlueprintCallable, Category = "Server")
	bool AddPlayer(APlayerController* Player);

	UFUNCTION(BlueprintCallable, Category = "Server")
	bool RemovePlayer(APlayerController* Player);

	UFUNCTION(BlueprintCallable, Category = "Server")
	TArray<APlayerController*> GetConnectedPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	bool CanAcceptPlayer() const;

	// Load management
	UFUNCTION(BlueprintCallable, Category = "Server")
	float GetLoadPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	void UpdateLoadMetrics();

	UFUNCTION(BlueprintCallable, Category = "Server")
	void OptimizePerformance();

	// Load balancing
	UFUNCTION(BlueprintCallable, Category = "Server")
	void TransferPlayersTo(AServerNode* TargetNode, float Percentage);

	UFUNCTION(BlueprintCallable, Category = "Server")
	bool ShouldTransferPlayers() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	AServerNode* FindBestTargetNode() const;

	// Server operations
	UFUNCTION(BlueprintCallable, Category = "Server")
	void StartServer();

	UFUNCTION(BlueprintCallable, Category = "Server")
	void StopServer();

	UFUNCTION(BlueprintCallable, Category = "Server")
	void RestartServer();

	UFUNCTION(BlueprintCallable, Category = "Server")
	void UpdateServerStatus();

	// Region management
	UFUNCTION(BlueprintCallable, Category = "Server")
	FString GetRegion() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	void SetRegion(const FString& NewRegion);

	UFUNCTION(BlueprintCallable, Category = "Server")
	bool IsInRegion(const FVector& PlayerLocation) const;

	// Performance monitoring
	UFUNCTION(BlueprintCallable, Category = "Server")
	float GetAveragePing() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	float GetPacketLoss() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	int32 GetTotalPackets() const;

	UFUNCTION(BlueprintCallable, Category = "Server")
	void ResetMetrics();

protected:
	// Internal state
	UPROPERTY()
	TArray<APlayerController*> ConnectedPlayers;

	UPROPERTY()
	float LastLoadUpdate;

	UPROPERTY()
	float PerformanceTimer;

	UPROPERTY()
	int32 PacketsSent;

	UPROPERTY()
	int32 PacketsReceived;

	UPROPERTY()
	float TotalPing;

	UPROPERTY()
	int32 PingCount;

private:
	void CalculateServerLoad();
	void UpdateLoadStatus();
	void LogServerMetrics();
};