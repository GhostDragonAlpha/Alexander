// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ANetworkManager::ANetworkManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize network settings
	bIsServer = false;
	bIsDedicatedServer = false;
	MaxPlayers = 64;
	ServerTickRate = 60.0f;
	ClientUpdateRate = 30.0f;

	// Server meshing settings
	MaxServerNodes = 10;
	ServerMeshRadius = 50000.0f; // 50km

	// Network configuration
	Port = 7777;
	ServerName = TEXT("Alexander Server");
	ServerRegion = TEXT("Global");
	bEnableVoiceChat = true;
	bEnableVRNetworking = true;

	// Initialize state
	ConnectionState = EAlexanderConnectionState::Disconnected;
	LastUpdateTime = 0.0f;
	TotalPacketsSent = 0;
	TotalPacketsReceived = 0;
	TotalBandwidthUsed = 0.0f;
}

void ANetworkManager::BeginPlay()
{
	Super::BeginPlay();

	// Determine if this is a server
	bIsServer = HasAuthority();
	
	if (bIsServer)
	{
		// Initialize server systems
		ReplicationManager = NewObject<UReplicationManager>(this);
		NetworkPhysics = NewObject<UNetworkPhysics>(this);
		
		// Start server if not already running
		if (!IsRunningDedicatedServer())
		{
			StartServer();
		}
	}
	else
	{
		// Initialize client systems
		ReplicationManager = NewObject<UReplicationManager>(this);
		NetworkPhysics = NewObject<UNetworkPhysics>(this);
	}
}

void ANetworkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsServer)
	{
		// Server tick
		UpdateServerMeshing();
		UpdateReplication();
		ProcessNetworkMessages();
		OptimizeNetworkTraffic();
	}
	else
	{
		// Client tick
		ProcessNetworkMessages();
		UpdateReplication();
	}

	LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void ANetworkManager::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		FString PlayerID = NewPlayer->PlayerState ? NewPlayer->PlayerState->GetPlayerName() : FString::Printf(TEXT("Player_%d"), ConnectedClients.Num());
		PlayerControllers.Add(PlayerID, NewPlayer);
		
		// Send initial game state to new player
		SendToClient(NewPlayer, ENetworkMessageType::Handshake, FString::Printf(TEXT("Welcome to %s"), *ServerName));
		
		UE_LOG(LogTemp, Warning, TEXT("Player %s joined the server"), *PlayerID);
	}
}

void ANetworkManager::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* PlayerController = Cast<APlayerController>(Exiting);
	if (PlayerController)
	{
		// Remove from connected clients
		for (auto& PlayerPair : PlayerControllers)
		{
			if (PlayerPair.Value == PlayerController)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player %s left the server"), *PlayerPair.Key);
				PlayerControllers.Remove(PlayerPair.Key);
				break;
			}
		}
	}
}

bool ANetworkManager::StartServer()
{
	if (!bIsServer)
	{
		return false;
	}

	// Initialize server nodes
	ServerNodes.Empty();
	
	// Create central server node
	AddServerNode(FVector::ZeroVector);
	
	// Create additional server nodes for load balancing
	for (int32 i = 1; i < MaxServerNodes; i++)
	{
		float Angle = (float)i / MaxServerNodes * 2.0f * PI;
		FVector Location = FVector(FMath::Cos(Angle) * ServerMeshRadius, FMath::Sin(Angle) * ServerMeshRadius, 0.0f);
		AddServerNode(Location);
	}

	UE_LOG(LogTemp, Warning, TEXT("Server %s started on port %d with %d nodes"), *ServerName, Port, ServerNodes.Num());
	return true;
}

void ANetworkManager::StopServer()
{
	if (!bIsServer)
	{
		return;
	}

	// Disconnect all clients
	for (auto& PlayerPair : PlayerControllers)
	{
		if (PlayerPair.Value)
		{
			PlayerPair.Value->ClientWasKicked();
		}
	}
	
	PlayerControllers.Empty();
	ServerNodes.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("Server stopped"));
}

bool ANetworkManager::AddServerNode(FVector Location)
{
	if (!bIsServer || ServerNodes.Num() >= MaxServerNodes)
	{
		return false;
	}

	// Create server node actor
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	
	AServerNode* NewNode = World->SpawnActor<AServerNode>(AServerNode::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (NewNode)
	{
		ServerNodes.Add(NewNode);
		UE_LOG(LogTemp, Warning, TEXT("Added server node at %s"), *Location.ToString());
		return true;
	}

	return false;
}

void ANetworkManager::RemoveServerNode(AServerNode* Node)
{
	if (!bIsServer || !Node)
	{
		return;
	}

	ServerNodes.Remove(Node);
	Node->Destroy();
	
	UE_LOG(LogTemp, Warning, TEXT("Removed server node"));
}

void ANetworkManager::BalanceServerLoad()
{
	if (!bIsServer || ServerNodes.Num() == 0)
	{
		return;
	}

	// Calculate load on each server node
	for (AServerNode* Node : ServerNodes)
	{
		if (Node)
		{
			float NodeLoad = Node->GetLoadPercentage();
			
			// If node is overloaded, redistribute players
			if (NodeLoad > 80.0f)
			{
				// Find least loaded node
				AServerNode* LeastLoadedNode = nullptr;
				float MinLoad = 100.0f;
				
				for (AServerNode* OtherNode : ServerNodes)
				{
					if (OtherNode != Node && OtherNode->GetLoadPercentage() < MinLoad)
					{
						MinLoad = OtherNode->GetLoadPercentage();
						LeastLoadedNode = OtherNode;
					}
				}
				
				// Transfer some players to least loaded node
				if (LeastLoadedNode && MinLoad < 60.0f)
				{
					Node->TransferPlayersTo(LeastLoadedNode, 0.2f); // Transfer 20% of players
				}
			}
		}
	}
}

bool ANetworkManager::ConnectToServer(const FString& IPAddress)
{
	if (bIsServer)
	{
		return false;
	}

	ConnectionState = EAlexanderConnectionState::Connecting;
	
	// This would use Unreal's networking system to connect
	// For now, simulate connection
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		ConnectionState = EAlexanderConnectionState::Connected;
		UE_LOG(LogTemp, Warning, TEXT("Connected to server"));
	}, 2.0f, false);
	
	return true;
}

void ANetworkManager::DisconnectFromServer()
{
	if (bIsServer)
	{
		return;
	}

	ConnectionState = EAlexanderConnectionState::Disconnected;
	PlayerControllers.Empty();
	
	UE_LOG(LogTemp, Warning, TEXT("Disconnected from server"));
}

EAlexanderConnectionState ANetworkManager::GetConnectionState() const
{
	return ConnectionState;
}

void ANetworkManager::BroadcastMessage(ENetworkMessageType MessageType, const FString& Data)
{
	if (!bIsServer)
	{
		return;
	}

	for (auto& PlayerPair : PlayerControllers)
	{
		SendToClient(PlayerPair.Value, MessageType, Data);
	}
	
	TotalPacketsSent += PlayerControllers.Num();
}

void ANetworkManager::SendToClient(APlayerController* Client, ENetworkMessageType MessageType, const FString& Data)
{
	if (!Client || !bIsServer)
	{
		return;
	}

	// This would use Unreal's RPC system
	// For now, just log the message
	UE_LOG(LogTemp, Warning, TEXT("Sending message to client: %s"), *Data);
	
	TotalPacketsSent++;
	TotalBandwidthUsed += Data.Len();
}

void ANetworkManager::SendToServer(ENetworkMessageType MessageType, const FString& Data)
{
	if (bIsServer)
	{
		return;
	}

	// This would use Unreal's RPC system
	// For now, just log the message
	UE_LOG(LogTemp, Warning, TEXT("Sending message to server: %s"), *Data);
	
	TotalPacketsSent++;
	TotalBandwidthUsed += Data.Len();
}

void ANetworkManager::RegisterForReplication(AActor* Actor)
{
	if (!Actor || !ReplicationManager)
	{
		return;
	}

	ReplicationManager->RegisterActor(Actor);
}

void ANetworkManager::UnregisterFromReplication(AActor* Actor)
{
	if (!Actor || !ReplicationManager)
	{
		return;
	}

	ReplicationManager->UnregisterActor(Actor);
}

void ANetworkManager::UpdateReplication()
{
	if (!ReplicationManager)
	{
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if (bIsServer)
	{
		// Server replicates to all clients
		ReplicationManager->ServerUpdate(DeltaTime);
	}
	else
	{
		// Client receives updates
		ReplicationManager->ClientUpdate(DeltaTime);
	}
}

void ANetworkManager::StartVoiceChat()
{
	if (!bEnableVoiceChat)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Voice chat started"));
}

void ANetworkManager::StopVoiceChat()
{
	UE_LOG(LogTemp, Warning, TEXT("Voice chat stopped"));
}

void ANetworkManager::SendVoiceData(const TArray<uint8>& VoiceData)
{
	if (!bEnableVoiceChat || VoiceData.Num() == 0)
	{
		return;
	}

	// Compress and send voice data
	FString CompressedData = FString::FromBlob(VoiceData.GetData(), VoiceData.Num());
	
	if (bIsServer)
	{
		BroadcastMessage(ENetworkMessageType::VoiceData, CompressedData);
	}
	else
	{
		SendToServer(ENetworkMessageType::VoiceData, CompressedData);
	}
}

float ANetworkManager::GetPing() const
{
	// This would return actual ping from network subsystem
	return 50.0f; // Placeholder
}

float ANetworkManager::GetPacketLoss() const
{
	// This would return actual packet loss
	return 0.5f; // Placeholder
}

int32 ANetworkManager::GetConnectedPlayerCount() const
{
	return PlayerControllers.Num();
}

float ANetworkManager::GetServerLoad() const
{
	if (ServerNodes.Num() == 0)
	{
		return 0.0f;
	}

	float TotalLoad = 0.0f;
	for (AServerNode* Node : ServerNodes)
	{
		if (Node)
		{
			TotalLoad += Node->GetLoadPercentage();
		}
	}
	
	return TotalLoad / ServerNodes.Num();
}

void ANetworkManager::ProcessNetworkMessages()
{
	// This would process incoming network messages
	// For now, simulate message processing
}

void ANetworkManager::HandleClientMessage(APlayerController* Client, ENetworkMessageType MessageType, const FString& Data)
{
	switch (MessageType)
	{
		case ENetworkMessageType::PositionUpdate:
			// Handle position update
			break;
		case ENetworkMessageType::ChatMessage:
			// Broadcast chat to all players
			BroadcastMessage(ENetworkMessageType::ChatMessage, Data);
			break;
		case ENetworkMessageType::FarmUpdate:
			// Handle farming update
			BroadcastMessage(ENetworkMessageType::FarmUpdate, Data);
			break;
		default:
			break;
	}
	
	TotalPacketsReceived++;
}

void ANetworkManager::HandleServerMessage(ENetworkMessageType MessageType, const FString& Data)
{
	switch (MessageType)
	{
		case ENetworkMessageType::Handshake:
			// Handle server handshake
			ConnectionState = EAlexanderConnectionState::Ready;
			break;
		case ENetworkMessageType::PositionUpdate:
			// Handle position update from other players
			break;
		case ENetworkMessageType::FarmUpdate:
			// Handle farming update from server
			break;
		default:
			break;
	}
	
	TotalPacketsReceived++;
}

void ANetworkManager::UpdateServerMeshing()
{
	if (!bIsServer || ServerNodes.Num() == 0)
	{
		return;
	}

	// Balance server load periodically
	static float LoadBalanceTimer = 0.0f;
	LoadBalanceTimer += GetWorld()->GetDeltaSeconds();
	
	if (LoadBalanceTimer >= 30.0f) // Every 30 seconds
	{
		BalanceServerLoad();
		LoadBalanceTimer = 0.0f;
	}
}

void ANetworkManager::OptimizeNetworkTraffic()
{
	// Compress network data
	CompressNetworkData();
	
	// Adjust update rates based on server load
	float ServerLoad = GetServerLoad();
	if (ServerLoad > 80.0f)
	{
		ClientUpdateRate = FMath::Max(15.0f, ClientUpdateRate * 0.9f);
	}
	else if (ServerLoad < 50.0f)
	{
		ClientUpdateRate = FMath::Min(60.0f, ClientUpdateRate * 1.1f);
	}
}

void ANetworkManager::CompressNetworkData()
{
	// This would implement data compression
	// For now, just track bandwidth usage
	static float BandwidthTimer = 0.0f;
	BandwidthTimer += GetWorld()->GetDeltaSeconds();
	
	if (BandwidthTimer >= 1.0f) // Every second
	{
		UE_LOG(LogTemp, Warning, TEXT("Network bandwidth: %.2f KB/s"), TotalBandwidthUsed / 1024.0f);
		TotalBandwidthUsed = 0.0f;
		BandwidthTimer = 0.0f;
	}
}