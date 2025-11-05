// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClientConnection.generated.h"

// Forward declarations
class APlayerController;
class ANetworkManager;

/**
 * Connection quality metrics
 */
USTRUCT(BlueprintType)
struct FConnectionQuality
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float Ping;

	UPROPERTY(BlueprintReadOnly)
	float PacketLoss;

	UPROPERTY(BlueprintReadOnly)
	float Bandwidth;

	UPROPERTY(BlueprintReadOnly)
	float Jitter;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStable;

	UPROPERTY(BlueprintReadOnly)
	int32 QualityScore;

	FConnectionQuality()
	{
		Ping = 0.0f;
		PacketLoss = 0.0f;
		Bandwidth = 0.0f;
		Jitter = 0.0f;
		bIsStable = true;
		QualityScore = 100;
	}
};

/**
 * Network statistics
 */
USTRUCT(BlueprintType)
struct FNetworkStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PacketsSent;

	UPROPERTY(BlueprintReadOnly)
	int32 PacketsReceived;

	UPROPERTY(BlueprintReadOnly)
	int32 BytesSent;

	UPROPERTY(BlueprintReadOnly)
	int32 BytesReceived;

	UPROPERTY(BlueprintReadOnly)
	float TotalBandwidthUsed;

	UPROPERTY(BlueprintReadOnly)
	float AveragePacketSize;

	UPROPERTY(BlueprintReadOnly)
	int32 DroppedPackets;

	UPROPERTY(BlueprintReadOnly)
	int32 RetransmittedPackets;

	FNetworkStatistics()
	{
		PacketsSent = 0;
		PacketsReceived = 0;
		BytesSent = 0;
		BytesReceived = 0;
		TotalBandwidthUsed = 0.0f;
		AveragePacketSize = 0.0f;
		DroppedPackets = 0;
		RetransmittedPackets = 0;
	}
};

/**
 * Client Connection - Manages individual client connections to server
 * Handles connection state, quality monitoring, and data transmission
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API AClientConnection : public AActor
{
	GENERATED_BODY()

public:
	AClientConnection();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Connection management
	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool Connect(const FString& IPAddress, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool IsConnected() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool IsConnecting() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void Reconnect();

	// Data transmission
	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool SendData(const TArray<uint8>& Data);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool SendString(const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	TArray<uint8> ReceiveData();

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool HasPendingData() const;

	// Quality monitoring
	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	FConnectionQuality GetConnectionQuality() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void UpdateConnectionQuality();

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	float GetPing() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	float GetPacketLoss() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	bool IsConnectionStable() const;

	// Network statistics
	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	FNetworkStatistics GetNetworkStatistics() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void ResetStatistics();

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void LogStatistics();

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void SetTimeout(float Timeout);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	float GetTimeout() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void SetMaxRetries(int32 MaxRetries);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	int32 GetMaxRetries() const;

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	void SetKeepAliveInterval(float Interval);

	UFUNCTION(BlueprintCallable, Category = "Client Connection")
	float GetKeepAliveInterval() const;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, bool, bConnected);
	UPROPERTY(BlueprintAssignable, Category = "Client Connection")
	FOnConnectionStateChanged OnConnectionStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataReceived, const TArray<uint8>&, Data);
	UPROPERTY(BlueprintAssignable, Category = "Client Connection")
	FOnDataReceived OnDataReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionQualityChanged, const FConnectionQuality&, Quality);
	UPROPERTY(BlueprintAssignable, Category = "Client Connection")
	FOnConnectionQualityChanged OnConnectionQualityChanged;

protected:
	// Connection properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	bool bIsConnected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	bool bIsConnecting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	FString ServerAddress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	int32 ServerPort;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	APlayerController* OwningPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	ANetworkManager* NetworkManager;

	// Quality metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	FConnectionQuality ConnectionQuality;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Client Connection")
	FNetworkStatistics NetworkStats;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client Connection")
	float ConnectionTimeout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client Connection")
	int32 MaxConnectionRetries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client Connection")
	float KeepAliveInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client Connection")
	float QualityUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client Connection")
	float PingInterval;

	// Internal state
	UPROPERTY()
	float LastKeepAliveTime;

	UPROPERTY()
	float LastQualityUpdateTime;

	UPROPERTY()
	float LastPingTime;

	UPROPERTY()
	int32 CurrentRetryCount;

	UPROPERTY()
	TArray<float> PingHistory;

	UPROPERTY()
	TArray<float> JitterHistory;

	UPROPERTY()
	TArray<uint8> ReceiveBuffer;

	UPROPERTY()
	float ConnectionStartTime;

private:
	void ProcessConnection();
	void SendKeepAlive();
	void SendPing();
	void CalculateQualityMetrics();
	void HandleTimeout();
	void UpdateStatistics(int32 BytesSent, int32 BytesReceived);
	void ClearBuffers();
	bool ValidateConnection() const;
};