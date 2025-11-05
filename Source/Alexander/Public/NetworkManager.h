// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "NetworkManager.generated.h"

// Forward declarations
class AServerNode;
class AClientConnection;
class UReplicationManager;
class UNetworkPhysics;

/**
 * Network roles for actors
 */
UENUM(BlueprintType)
enum class ENetworkRole : uint8
{
	None UMETA(DisplayName = "None"),
	AutonomousProxy UMETA(DisplayName = "Autonomous Proxy"),
	SimulatedProxy UMETA(DisplayName = "Simulated Proxy"),
	Authority UMETA(DisplayName = "Authority")
};

/**
 * Connection states
 */
UENUM(BlueprintType)
enum class EAlexanderConnectionState : uint8
{
	Disconnected UMETA(DisplayName = "Disconnected"),
	Connecting UMETA(DisplayName = "Connecting"),
	Connected UMETA(DisplayName = "Connected"),
	Loading UMETA(DisplayName = "Loading"),
	Ready UMETA(DisplayName = "Ready"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Network message types
 */
UENUM(BlueprintType)
enum class ENetworkMessageType : uint8
{
	Handshake UMETA(DisplayName = "Handshake"),
	PositionUpdate UMETA(DisplayName = "Position Update"),
	RotationUpdate UMETA(DisplayName = "Rotation Update"),
	VelocityUpdate UMETA(DisplayName = "Velocity Update"),
	PhysicsState UMETA(DisplayName = "Physics State"),
	ChatMessage UMETA(DisplayName = "Chat Message"),
	FarmUpdate UMETA(DisplayName = "Farm Update"),
	CropUpdate UMETA(DisplayName = "Crop Update"),
	WeatherUpdate UMETA(DisplayName = "Weather Update"),
	AIUpdate UMETA(DisplayName = "AI Update"),
	VoiceData UMETA(DisplayName = "Voice Data"),
	CustomEvent UMETA(DisplayName = "Custom Event")
};

/**
 * Network Manager - Core networking system for Alexander
 * Handles server meshing, client connections, and data replication
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API ANetworkManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetworkManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	// Server management
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	bool bIsServer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	bool bIsDedicatedServer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float ServerTickRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float ClientUpdateRate;

	// Server meshing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	TArray<AServerNode*> ServerNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxServerNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float ServerMeshRadius;

	// Client connections
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	TArray<AClientConnection*> ConnectedClients;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	TMap<FString, APlayerController*> PlayerControllers;

	// Network configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 Port;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString ServerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString ServerRegion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableVoiceChat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableVRNetworking;

	// Server operations
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool StartServer();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StopServer();

	UFUNCTION(BlueprintCallable, Category = "Network")
	bool AddServerNode(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void RemoveServerNode(AServerNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void BalanceServerLoad();

	// Client operations
	UFUNCTION(BlueprintCallable, Category = "Network")
	bool ConnectToServer(const FString& IPAddress);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DisconnectFromServer();

	UFUNCTION(BlueprintCallable, Category = "Network")
	EAlexanderConnectionState GetConnectionState() const;

	// Message handling
	UFUNCTION(BlueprintCallable, Category = "Network")
	void BroadcastMessage(ENetworkMessageType MessageType, const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendToClient(APlayerController* Client, ENetworkMessageType MessageType, const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendToServer(ENetworkMessageType MessageType, const FString& Data);

	// Replication
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RegisterForReplication(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void UnregisterFromReplication(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdateReplication();

	// Voice chat
	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartVoiceChat();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StopVoiceChat();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendVoiceData(const TArray<uint8>& VoiceData);

	// Network statistics
	UFUNCTION(BlueprintCallable, Category = "Network")
	float GetPing() const;

	UFUNCTION(BlueprintCallable, Category = "Network")
	float GetPacketLoss() const;

	UFUNCTION(BlueprintCallable, Category = "Network")
	int32 GetConnectedPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Network")
	float GetServerLoad() const;

protected:
	// Internal systems
	UPROPERTY()
	UReplicationManager* ReplicationManager;

	UPROPERTY()
	UNetworkPhysics* NetworkPhysics;

	// Network state
	UPROPERTY()
	EAlexanderConnectionState ConnectionState;

	UPROPERTY()
	float LastUpdateTime;

	UPROPERTY()
	int32 TotalPacketsSent;

	UPROPERTY()
	int32 TotalPacketsReceived;

	UPROPERTY()
	float TotalBandwidthUsed;

private:
	void ProcessNetworkMessages();
	void HandleClientMessage(APlayerController* Client, ENetworkMessageType MessageType, const FString& Data);
	void HandleServerMessage(ENetworkMessageType MessageType, const FString& Data);
	void UpdateServerMeshing();
	void OptimizeNetworkTraffic();
	void CompressNetworkData();
};