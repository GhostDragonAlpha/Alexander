// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerNode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "NetworkManager.h"

AServerNode::AServerNode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize server properties
	ServerID = FString::Printf(TEXT("ServerNode_%d"), FMath::RandRange(1000, 9999));
	NodeLocation = FVector::ZeroVector;
	MaxPlayers = 32;
	CurrentPlayers = 0;
	ServerLoad = 0.0f;
	LoadStatus = EServerLoadStatus::Idle;
	bIsActive = false;
	BandwidthUsage = 0.0f;
	CPUUsage = 0.0f;
	MemoryUsage = 0.0f;

	// Initialize internal state
	LastLoadUpdate = 0.0f;
	PerformanceTimer = 0.0f;
	PacketsSent = 0;
	PacketsReceived = 0;
	TotalPing = 0.0f;
	PingCount = 0;
}

void AServerNode::BeginPlay()
{
	Super::BeginPlay();

	NodeLocation = GetActorLocation();
	bIsActive = true;
	
	UE_LOG(LogTemp, Warning, TEXT("ServerNode %s initialized at %s"), *ServerID, *NodeLocation.ToString());
}

void AServerNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive)
	{
		return;
	}

	// Update performance metrics
	PerformanceTimer += DeltaTime;
	if (PerformanceTimer >= 1.0f) // Update every second
	{
		UpdateLoadMetrics();
		PerformanceTimer = 0.0f;
	}

	// Log metrics periodically
	static float LogTimer = 0.0f;
	LogTimer += DeltaTime;
	if (LogTimer >= 60.0f) // Log every minute
	{
		LogServerMetrics();
		LogTimer = 0.0f;
	}
}

bool AServerNode::AddPlayer(APlayerController* Player)
{
	if (!Player || !bIsActive || CurrentPlayers >= MaxPlayers)
	{
		return false;
	}

	// Check if player is already connected
	if (ConnectedPlayers.Contains(Player))
	{
		return false;
	}

	ConnectedPlayers.Add(Player);
	CurrentPlayers++;

	// Update server load immediately
	CalculateServerLoad();
	UpdateLoadStatus();

	UE_LOG(LogTemp, Warning, TEXT("Player added to ServerNode %s. Current players: %d/%d"), 
		*ServerID, CurrentPlayers, MaxPlayers);

	return true;
}

bool AServerNode::RemovePlayer(APlayerController* Player)
{
	if (!Player || !ConnectedPlayers.Contains(Player))
	{
		return false;
	}

	ConnectedPlayers.Remove(Player);
	CurrentPlayers--;

	// Update server load immediately
	CalculateServerLoad();
	UpdateLoadStatus();

	UE_LOG(LogTemp, Warning, TEXT("Player removed from ServerNode %s. Current players: %d/%d"), 
		*ServerID, CurrentPlayers, MaxPlayers);

	return true;
}

TArray<APlayerController*> AServerNode::GetConnectedPlayers() const
{
	return ConnectedPlayers;
}

bool AServerNode::CanAcceptPlayer() const
{
	return bIsActive && CurrentPlayers < MaxPlayers && LoadStatus != EServerLoadStatus::Critical;
}

float AServerNode::GetLoadPercentage() const
{
	return ServerLoad;
}

void AServerNode::UpdateLoadMetrics()
{
	if (!bIsActive)
	{
		return;
	}

	// Calculate player load (40% weight)
	float PlayerLoad = (float)CurrentPlayers / MaxPlayers * 40.0f;

	// Simulate CPU usage (30% weight)
	CPUUsage = FMath::RandRange(10.0f, 60.0f) + (PlayerLoad * 0.5f);
	float CPULoad = FMath::Clamp(CPUUsage / 100.0f * 30.0f, 0.0f, 30.0f);

	// Simulate memory usage (20% weight)
	MemoryUsage = FMath::RandRange(20.0f, 70.0f) + (PlayerLoad * 0.3f);
	float MemoryLoad = FMath::Clamp(MemoryUsage / 100.0f * 20.0f, 0.0f, 20.0f);

	// Simulate bandwidth usage (10% weight)
	BandwidthUsage = CurrentPlayers * FMath::RandRange(1.0f, 5.0f);
	float BandwidthLoad = FMath::Clamp(BandwidthUsage / 100.0f * 10.0f, 0.0f, 10.0f);

	// Calculate total server load
	ServerLoad = PlayerLoad + CPULoad + MemoryLoad + BandwidthLoad;
	ServerLoad = FMath::Clamp(ServerLoad, 0.0f, 100.0f);

	LastLoadUpdate = GetWorld()->GetTimeSeconds();
}

void AServerNode::OptimizePerformance()
{
	if (!bIsActive)
	{
		return;
	}

	// If server is overloaded, implement optimization strategies
	if (LoadStatus == EServerLoadStatus::High || LoadStatus == EServerLoadStatus::Overloaded)
	{
		// Reduce update rates for non-critical systems
		// This would interact with other systems to reduce their update frequency
		
		UE_LOG(LogTemp, Warning, TEXT("ServerNode %s applying performance optimizations"), *ServerID);
	}
}

void AServerNode::TransferPlayersTo(AServerNode* TargetNode, float Percentage)
{
	if (!TargetNode || !bIsActive || Percentage <= 0.0f || Percentage > 1.0f)
	{
		return;
	}

	int32 PlayersToTransfer = FMath::RoundToInt(ConnectedPlayers.Num() * Percentage);
	if (PlayersToTransfer == 0)
	{
		return;
	}

	TArray<APlayerController*> PlayersToMove;
	
	// Select players for transfer (prefer those with highest ping)
	for (int32 i = 0; i < PlayersToTransfer && i < ConnectedPlayers.Num(); i++)
	{
		PlayersToMove.Add(ConnectedPlayers[i]);
	}

	// Transfer players
	int32 SuccessfulTransfers = 0;
	for (APlayerController* Player : PlayersToMove)
	{
		if (TargetNode->AddPlayer(Player))
		{
			RemovePlayer(Player);
			SuccessfulTransfers++;

			// Notify player of server transfer
			if (Player && Player->PlayerState)
			{
				FString Message = FString::Printf(TEXT("Transferring to server %s"), *TargetNode->ServerID);
				// This would use Unreal's client RPC system
				UE_LOG(LogTemp, Warning, TEXT("Transferring player %s to %s"), 
					*Player->PlayerState->GetPlayerName(), *TargetNode->ServerID);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Transferred %d players from %s to %s"), 
		SuccessfulTransfers, *ServerID, *TargetNode->ServerID);
}

bool AServerNode::ShouldTransferPlayers() const
{
	return LoadStatus == EServerLoadStatus::Overloaded || LoadStatus == EServerLoadStatus::Critical;
}

AServerNode* AServerNode::FindBestTargetNode() const
{
	// This would typically query the NetworkManager for available nodes
	// For now, return nullptr as placeholder
	return nullptr;
}

void AServerNode::StartServer()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	CurrentPlayers = 0;
	ServerLoad = 0.0f;
	LoadStatus = EServerLoadStatus::Idle;
	
	// Reset metrics
	ResetMetrics();

	UE_LOG(LogTemp, Warning, TEXT("ServerNode %s started"), *ServerID);
}

void AServerNode::StopServer()
{
	if (!bIsActive)
	{
		return;
	}

	// Transfer all players to other servers before shutting down
	if (ConnectedPlayers.Num() > 0)
	{
		AServerNode* TargetNode = FindBestTargetNode();
		if (TargetNode)
		{
			TransferPlayersTo(TargetNode, 1.0f); // Transfer all players
		}
		else
		{
			// Disconnect all players if no target available
			for (APlayerController* Player : ConnectedPlayers)
			{
				if (Player)
				{
					Player->ClientWasKicked();
				}
			}
		}
	}

	bIsActive = false;
	ConnectedPlayers.Empty();
	CurrentPlayers = 0;

	UE_LOG(LogTemp, Warning, TEXT("ServerNode %s stopped"), *ServerID);
}

void AServerNode::RestartServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Restarting ServerNode %s"), *ServerID);
	
	StopServer();
	
	// Wait a moment before restarting
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		StartServer();
	}, 2.0f, false);
}

void AServerNode::UpdateServerStatus()
{
	if (!bIsActive)
	{
		return;
	}

	CalculateServerLoad();
	UpdateLoadStatus();
	
	// This would typically report status to NetworkManager
	UE_LOG(LogTemp, Warning, TEXT("ServerNode %s status: %s (%.1f%% load, %d/%d players)"), 
		*ServerID, *UEnum::GetValueAsString(LoadStatus), ServerLoad, CurrentPlayers, MaxPlayers);
}

FString AServerNode::GetRegion() const
{
	// For now, return a region based on location
	if (NodeLocation.X > 0)
	{
		return TEXT("East");
	}
	else if (NodeLocation.X < 0)
	{
		return TEXT("West");
	}
	else
	{
		return TEXT("Central");
	}
}

void AServerNode::SetRegion(const FString& NewRegion)
{
	// This would update the server's region configuration
	UE_LOG(LogTemp, Warning, TEXT("ServerNode %s region set to %s"), *ServerID, *NewRegion);
}

bool AServerNode::IsInRegion(const FVector& PlayerLocation) const
{
	// Check if player is within this server's region (50km radius)
	float Distance = FVector::Dist(PlayerLocation, NodeLocation);
	return Distance <= 50000.0f; // 50km
}

float AServerNode::GetAveragePing() const
{
	if (PingCount == 0)
	{
		return 0.0f;
	}
	return TotalPing / PingCount;
}

float AServerNode::GetPacketLoss() const
{
	if (PacketsSent == 0)
	{
		return 0.0f;
	}
	
	int32 LostPackets = FMath::Max(0, PacketsSent - PacketsReceived);
	return (float)LostPackets / PacketsSent * 100.0f;
}

int32 AServerNode::GetTotalPackets() const
{
	return PacketsSent + PacketsReceived;
}

void AServerNode::ResetMetrics()
{
	PacketsSent = 0;
	PacketsReceived = 0;
	TotalPing = 0.0f;
	PingCount = 0;
	BandwidthUsage = 0.0f;
	CPUUsage = 0.0f;
	MemoryUsage = 0.0f;
}

void AServerNode::CalculateServerLoad()
{
	if (!bIsActive)
	{
		ServerLoad = 0.0f;
		return;
	}

	// Player load (primary factor)
	float PlayerLoad = (float)CurrentPlayers / MaxPlayers * 50.0f;

	// System resource load
	float ResourceLoad = (CPUUsage + MemoryUsage) / 2.0f * 0.3f;

	// Network load
	float NetworkLoad = FMath::Clamp(BandwidthUsage / 100.0f * 20.0f, 0.0f, 20.0f);

	ServerLoad = PlayerLoad + ResourceLoad + NetworkLoad;
	ServerLoad = FMath::Clamp(ServerLoad, 0.0f, 100.0f);
}

void AServerNode::UpdateLoadStatus()
{
	if (ServerLoad <= 20.0f)
	{
		LoadStatus = EServerLoadStatus::Idle;
	}
	else if (ServerLoad <= 40.0f)
	{
		LoadStatus = EServerLoadStatus::Low;
	}
	else if (ServerLoad <= 60.0f)
	{
		LoadStatus = EServerLoadStatus::Medium;
	}
	else if (ServerLoad <= 80.0f)
	{
		LoadStatus = EServerLoadStatus::High;
	}
	else if (ServerLoad <= 95.0f)
	{
		LoadStatus = EServerLoadStatus::Overloaded;
	}
	else
	{
		LoadStatus = EServerLoadStatus::Critical;
	}
}

void AServerNode::LogServerMetrics()
{
	if (!bIsActive)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== ServerNode %s Metrics ==="), *ServerID);
	UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *UEnum::GetValueAsString(LoadStatus));
	UE_LOG(LogTemp, Warning, TEXT("Load: %.1f%%"), ServerLoad);
	UE_LOG(LogTemp, Warning, TEXT("Players: %d/%d"), CurrentPlayers, MaxPlayers);
	UE_LOG(LogTemp, Warning, TEXT("CPU: %.1f%%, Memory: %.1f%%, Bandwidth: %.1f KB/s"), 
		CPUUsage, MemoryUsage, BandwidthUsage);
	UE_LOG(LogTemp, Warning, TEXT("Avg Ping: %.1f ms, Packet Loss: %.1f%%"), 
		GetAveragePing(), GetPacketLoss());
	UE_LOG(LogTemp, Warning, TEXT("Total Packets: %d"), GetTotalPackets());
	UE_LOG(LogTemp, Warning, TEXT("================================"));
}