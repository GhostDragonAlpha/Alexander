// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClientConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "NetworkManager.h"

AClientConnection::AClientConnection()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize connection state
	bIsConnected = false;
	bIsConnecting = false;
	ServerAddress = TEXT("");
	ServerPort = 7777;
	OwningPlayer = nullptr;
	NetworkManager = nullptr;

	// Initialize configuration
	ConnectionTimeout = 10.0f;
	MaxConnectionRetries = 3;
	KeepAliveInterval = 5.0f;
	QualityUpdateInterval = 1.0f;
	PingInterval = 2.0f;

	// Initialize internal state
	LastKeepAliveTime = 0.0f;
	LastQualityUpdateTime = 0.0f;
	LastPingTime = 0.0f;
	CurrentRetryCount = 0;
	ConnectionStartTime = 0.0f;
}

void AClientConnection::BeginPlay()
{
	Super::BeginPlay();

	// Get owning player controller
	OwningPlayer = GetWorld()->GetFirstPlayerController();
	
	// Find network manager
	TArray<AActor*> NetworkManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANetworkManager::StaticClass(), NetworkManagers);
	if (NetworkManagers.Num() > 0)
	{
		NetworkManager = Cast<ANetworkManager>(NetworkManagers[0]);
	}

	UE_LOG(LogTemp, Warning, TEXT("ClientConnection initialized"));
}

void AClientConnection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Process connection if connecting
	if (bIsConnecting)
	{
		ProcessConnection();
	}

	// Send keep alive if connected
	if (bIsConnected && CurrentTime - LastKeepAliveTime >= KeepAliveInterval)
	{
		SendKeepAlive();
		LastKeepAliveTime = CurrentTime;
	}

	// Send ping if connected
	if (bIsConnected && CurrentTime - LastPingTime >= PingInterval)
	{
		SendPing();
		LastPingTime = CurrentTime;
	}

	// Update connection quality
	if (bIsConnected && CurrentTime - LastQualityUpdateTime >= QualityUpdateInterval)
	{
		UpdateConnectionQuality();
		LastQualityUpdateTime = CurrentTime;
	}

	// Handle timeout
	if (bIsConnecting && CurrentTime - ConnectionStartTime >= ConnectionTimeout)
	{
		HandleTimeout();
	}
}

void AClientConnection::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Disconnect if connected
	if (bIsConnected)
	{
		Disconnect();
	}

	Super::EndPlay(EndPlayReason);
}

bool AClientConnection::Connect(const FString& IPAddress, int32 Port)
{
	if (bIsConnected || bIsConnecting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already connected or connecting"));
		return false;
	}

	ServerAddress = IPAddress;
	ServerPort = Port;
	bIsConnecting = true;
	CurrentRetryCount = 0;
	ConnectionStartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Warning, TEXT("Connecting to server %s:%d"), *IPAddress, Port);

	// This would use Unreal's networking system to connect
	// For now, simulate connection
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (bIsConnecting)
		{
			bIsConnected = true;
			bIsConnecting = false;
			
			UE_LOG(LogTemp, Warning, TEXT("Connected to server"));
			
			// Broadcast connection state change
			OnConnectionStateChanged.Broadcast(true);
		}
	}, 2.0f, false);

	return true;
}

void AClientConnection::Disconnect()
{
	if (!bIsConnected && !bIsConnecting)
	{
		return;
	}

	bool bWasConnected = bIsConnected;
	
	bIsConnected = false;
	bIsConnecting = false;
	CurrentRetryCount = 0;

	// Clear buffers and reset state
	ClearBuffers();
	ResetStatistics();

	UE_LOG(LogTemp, Warning, TEXT("Disconnected from server"));

	// Broadcast connection state change
	if (bWasConnected)
	{
		OnConnectionStateChanged.Broadcast(false);
	}
}

bool AClientConnection::IsConnected() const
{
	return bIsConnected;
}

bool AClientConnection::IsConnecting() const
{
	return bIsConnecting;
}

void AClientConnection::Reconnect()
{
	Disconnect();
	
	if (!ServerAddress.IsEmpty())
	{
		Connect(ServerAddress, ServerPort);
	}
}

bool AClientConnection::SendData(const TArray<uint8>& Data)
{
	if (!bIsConnected || Data.Num() == 0)
	{
		return false;
	}

	// This would use Unreal's networking system to send data
	// For now, just simulate sending
	NetworkStats.PacketsSent++;
	NetworkStats.BytesSent += Data.Num();
	NetworkStats.TotalBandwidthUsed += Data.Num();

	UE_LOG(LogTemp, Warning, TEXT("Sent %d bytes to server"), Data.Num());

	return true;
}

bool AClientConnection::SendString(const FString& Data)
{
	if (Data.IsEmpty())
	{
		return false;
	}

	// Convert string to bytes
	TArray<uint8> ByteArray;
	ByteArray.Append((uint8*)TCHAR_TO_UTF8(*Data), Data.Len());

	return SendData(ByteArray);
}

TArray<uint8> AClientConnection::ReceiveData()
{
	TArray<uint8> Data;
	
	if (ReceiveBuffer.Num() > 0)
	{
		Data = ReceiveBuffer;
		ReceiveBuffer.Empty();
		
		NetworkStats.PacketsReceived++;
		NetworkStats.BytesReceived += Data.Num();
	}

	return Data;
}

bool AClientConnection::HasPendingData() const
{
	return ReceiveBuffer.Num() > 0;
}

FConnectionQuality AClientConnection::GetConnectionQuality() const
{
	return ConnectionQuality;
}

void AClientConnection::UpdateConnectionQuality()
{
	if (!bIsConnected)
	{
		return;
	}

	CalculateQualityMetrics();

	// Determine if connection is stable
	ConnectionQuality.bIsStable = (ConnectionQuality.Ping < 200.0f) && 
								  (ConnectionQuality.PacketLoss < 5.0f) && 
								  (ConnectionQuality.Jitter < 50.0f);

	// Calculate quality score (0-100)
	int32 PingScore = FMath::Clamp(100 - (int32)(ConnectionQuality.Ping / 2.0f), 0, 100);
	int32 LossScore = FMath::Clamp(100 - (int32)(ConnectionQuality.PacketLoss * 10), 0, 100);
	int32 JitterScore = FMath::Clamp(100 - (int32)(ConnectionQuality.Jitter / 2.0f), 0, 100);
	
	ConnectionQuality.QualityScore = (PingScore + LossScore + JitterScore) / 3;

	// Broadcast quality change
	OnConnectionQualityChanged.Broadcast(ConnectionQuality);
}

float AClientConnection::GetPing() const
{
	return ConnectionQuality.Ping;
}

float AClientConnection::GetPacketLoss() const
{
	return ConnectionQuality.PacketLoss;
}

bool AClientConnection::IsConnectionStable() const
{
	return ConnectionQuality.bIsStable;
}

FNetworkStatistics AClientConnection::GetNetworkStatistics() const
{
	return NetworkStats;
}

void AClientConnection::ResetStatistics()
{
	NetworkStats = FNetworkStatistics();
	PingHistory.Empty();
	JitterHistory.Empty();
}

void AClientConnection::LogStatistics()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Client Connection Statistics ==="));
	UE_LOG(LogTemp, Warning, TEXT("Connected: %s"), bIsConnected ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Warning, TEXT("Server: %s:%d"), *ServerAddress, ServerPort);
	UE_LOG(LogTemp, Warning, TEXT("Ping: %.1f ms"), ConnectionQuality.Ping);
	UE_LOG(LogTemp, Warning, TEXT("Packet Loss: %.1f%%"), ConnectionQuality.PacketLoss);
	UE_LOG(LogTemp, Warning, TEXT("Bandwidth: %.2f KB/s"), ConnectionQuality.Bandwidth / 1024.0f);
	UE_LOG(LogTemp, Warning, TEXT("Jitter: %.1f ms"), ConnectionQuality.Jitter);
	UE_LOG(LogTemp, Warning, TEXT("Quality Score: %d"), ConnectionQuality.QualityScore);
	UE_LOG(LogTemp, Warning, TEXT("Packets Sent: %d"), NetworkStats.PacketsSent);
	UE_LOG(LogTemp, Warning, TEXT("Packets Received: %d"), NetworkStats.PacketsReceived);
	UE_LOG(LogTemp, Warning, TEXT("Bytes Sent: %d"), NetworkStats.BytesSent);
	UE_LOG(LogTemp, Warning, TEXT("Bytes Received: %d"), NetworkStats.BytesReceived);
	UE_LOG(LogTemp, Warning, TEXT("Total Bandwidth: %.2f MB"), NetworkStats.TotalBandwidthUsed / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Warning, TEXT("====================================="));
}

void AClientConnection::SetTimeout(float Timeout)
{
	ConnectionTimeout = FMath::Max(1.0f, Timeout);
}

float AClientConnection::GetTimeout() const
{
	return ConnectionTimeout;
}

void AClientConnection::SetMaxRetries(int32 MaxRetries)
{
	MaxConnectionRetries = FMath::Max(0, MaxRetries);
}

int32 AClientConnection::GetMaxRetries() const
{
	return MaxConnectionRetries;
}

void AClientConnection::SetKeepAliveInterval(float Interval)
{
	KeepAliveInterval = FMath::Max(1.0f, Interval);
}

float AClientConnection::GetKeepAliveInterval() const
{
	return KeepAliveInterval;
}

void AClientConnection::ProcessConnection()
{
	// This would process the actual connection attempt
	// For now, the connection is handled in Connect() with a timer
}

void AClientConnection::SendKeepAlive()
{
	if (!bIsConnected)
	{
		return;
	}

	// Send keep alive packet
	FString KeepAliveData = TEXT("KEEP_ALIVE");
	SendString(KeepAliveData);

	UE_LOG(LogTemp, Warning, TEXT("Sent keep alive to server"));
}

void AClientConnection::SendPing()
{
	if (!bIsConnected)
	{
		return;
	}

	float PingTime = GetWorld()->GetTimeSeconds();
	FString PingData = FString::Printf(TEXT("PING_%.3f"), PingTime);
	SendString(PingData);

	UE_LOG(LogTemp, Warning, TEXT("Sent ping to server"));
}

void AClientConnection::CalculateQualityMetrics()
{
	// Calculate ping (average of recent pings)
	if (PingHistory.Num() > 0)
	{
		float TotalPing = 0.0f;
		for (float Ping : PingHistory)
		{
			TotalPing += Ping;
		}
		ConnectionQuality.Ping = TotalPing / PingHistory.Num();
	}

	// Calculate jitter (variation in ping)
	if (PingHistory.Num() > 1)
	{
		float TotalJitter = 0.0f;
		for (int32 i = 1; i < PingHistory.Num(); i++)
		{
			TotalJitter += FMath::Abs(PingHistory[i] - PingHistory[i - 1]);
		}
		ConnectionQuality.Jitter = TotalJitter / (PingHistory.Num() - 1);
	}

	// Calculate packet loss
	if (NetworkStats.PacketsSent > 0)
	{
		int32 LostPackets = NetworkStats.PacketsSent - NetworkStats.PacketsReceived;
		ConnectionQuality.PacketLoss = (float)LostPackets / NetworkStats.PacketsSent * 100.0f;
	}

	// Calculate bandwidth (bytes per second)
	static float LastBandwidthUpdate = 0.0f;
	static int32 LastBytesReceived = 0;
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastBandwidthUpdate >= 1.0f)
	{
		int32 BytesThisSecond = NetworkStats.BytesReceived - LastBytesReceived;
		ConnectionQuality.Bandwidth = BytesThisSecond / (CurrentTime - LastBandwidthUpdate);
		
		LastBandwidthUpdate = CurrentTime;
		LastBytesReceived = NetworkStats.BytesReceived;
	}
}

void AClientConnection::HandleTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("Connection timeout"));
	
	if (CurrentRetryCount < MaxConnectionRetries)
	{
		CurrentRetryCount++;
		UE_LOG(LogTemp, Warning, TEXT("Retrying connection (%d/%d)"), CurrentRetryCount, MaxConnectionRetries);
		
		// Retry connection
		Connect(ServerAddress, ServerPort);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Max retries reached, giving up"));
		Disconnect();
	}
}

void AClientConnection::UpdateStatistics(int32 BytesSent, int32 BytesReceived)
{
	if (BytesSent > 0)
	{
		NetworkStats.BytesSent += BytesSent;
		NetworkStats.PacketsSent++;
	}

	if (BytesReceived > 0)
	{
		NetworkStats.BytesReceived += BytesReceived;
		NetworkStats.PacketsReceived++;
	}

	// Update average packet size
	int32 TotalPackets = NetworkStats.PacketsSent + NetworkStats.PacketsReceived;
	if (TotalPackets > 0)
	{
		NetworkStats.AveragePacketSize = (float)(NetworkStats.BytesSent + NetworkStats.BytesReceived) / TotalPackets;
	}

	// Update total bandwidth
	NetworkStats.TotalBandwidthUsed += BytesSent + BytesReceived;
}

void AClientConnection::ClearBuffers()
{
	ReceiveBuffer.Empty();
}

bool AClientConnection::ValidateConnection() const
{
	return bIsConnected && !ServerAddress.IsEmpty() && ServerPort > 0;
}