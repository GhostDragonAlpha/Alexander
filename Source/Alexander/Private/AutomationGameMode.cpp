// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutomationGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AAutomationGameMode::AAutomationGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f; // Update stats every second

	// Load default ship class
	static ConstructorHelpers::FClassFinder<APawn> ShipClassFinder(TEXT("/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"));
	if (ShipClassFinder.Succeeded())
	{
		DefaultPawnClass = ShipClassFinder.Class;
		DefaultShipClass = ShipClassFinder.Class;
	}

	// Multiplayer settings
	bAllowSpectators = true;

	// Automation settings
	bEnableAutomationServer = true;
	AutomationPort = 8080;
	bVerboseLogging = true;
}

void AAutomationGameMode::BeginPlay()
{
	Super::BeginPlay();

	ServerStartTime = FPlatformTime::Seconds();

	// Parse command line parameters
	int32 PortOverride = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("HTTPPort="), PortOverride))
	{
		AutomationPort = PortOverride;
		UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: HTTP port overridden to %d from command line"), AutomationPort);
	}

	// Setup automation server
	if (bEnableAutomationServer)
	{
		SetupAutomationServer();
	}

	// Setup multiplayer
	SetupMultiplayerConfig();

	UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Server started"));
	LogServerStatus();
}

void AAutomationGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update performance metrics
	TickCount++;
	AverageTickRate = TickCount / (FPlatformTime::Seconds() - ServerStartTime);

	// Log status periodically (every 60 ticks)
	if (TickCount % 60 == 0)
	{
		if (bVerboseLogging)
		{
			LogServerStatus();
		}
	}
}

void AAutomationGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup automation server
	if (AutomationServer)
	{
		AutomationServer->StopServer();
		AutomationServer = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Server stopped"));

	Super::EndPlay(EndPlayReason);
}

void AAutomationGameMode::SetupAutomationServer()
{
	// Create automation server component
	AutomationServer = NewObject<UAutomationAPIServer>(this, TEXT("AutomationAPIServer"));
	if (AutomationServer)
	{
		AutomationServer->RegisterComponent();
		AutomationServer->ListenPort = AutomationPort;
		AutomationServer->bVerboseLogging = bVerboseLogging;
		AutomationServer->StartServer();

		UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Automation server started on port %d"), AutomationPort);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AutomationGameMode: Failed to create automation server"));
	}
}

void AAutomationGameMode::SetupMultiplayerConfig()
{
	// Set max players
	if (GameState)
	{
		// Configure game state for multiplayer
		UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Configured for up to %d players"), MaxPlayers);
	}

	// Set default pawn class
	if (DefaultShipClass)
	{
		DefaultPawnClass = DefaultShipClass;
		UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Default pawn set to %s"), *DefaultShipClass->GetName());
	}
}

void AAutomationGameMode::LogServerStatus()
{
	double Uptime = FPlatformTime::Seconds() - ServerStartTime;

	UE_LOG(LogTemp, Log, TEXT("===== Server Status ====="));
	UE_LOG(LogTemp, Log, TEXT("Uptime: %.1f seconds"), Uptime);
	UE_LOG(LogTemp, Log, TEXT("Active Players: %d / %d"), ActivePlayers, MaxPlayers);
	UE_LOG(LogTemp, Log, TEXT("Total Connections: %d"), TotalConnections);
	UE_LOG(LogTemp, Log, TEXT("Average Tick Rate: %.1f Hz"), AverageTickRate);

	if (AutomationServer)
	{
		UE_LOG(LogTemp, Log, TEXT("Automation Server: %s"), AutomationServer->IsServerRunning() ? TEXT("Running") : TEXT("Stopped"));
		UE_LOG(LogTemp, Log, TEXT("Tracked Ships: %d"), AutomationServer->GetAllShips().Num());
	}

	UE_LOG(LogTemp, Log, TEXT("========================"));
}

TArray<AActor*> AAutomationGameMode::GetAllActiveShips()
{
	TArray<AActor*> Ships;

	// Get ships from automation server
	if (AutomationServer)
	{
		Ships = AutomationServer->GetAllShips();
	}

	// Also get all player-controlled pawns
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			Ships.AddUnique(PC->GetPawn());
		}
	}

	return Ships;
}

FString AAutomationGameMode::GetServerStatistics()
{
	double Uptime = FPlatformTime::Seconds() - ServerStartTime;

	FString Stats = FString::Printf(
		TEXT("Server Statistics:\n")
		TEXT("Uptime: %.1f seconds\n")
		TEXT("Active Players: %d / %d\n")
		TEXT("Total Connections: %d\n")
		TEXT("Average Tick Rate: %.1f Hz\n")
		TEXT("Active Ships: %d\n")
		TEXT("Automation Server: %s\n"),
		Uptime,
		ActivePlayers,
		MaxPlayers,
		TotalConnections,
		AverageTickRate,
		GetAllActiveShips().Num(),
		(AutomationServer && AutomationServer->IsServerRunning()) ? TEXT("Running") : TEXT("Stopped")
	);

	return Stats;
}

bool AAutomationGameMode::IsDedicatedServer() const
{
	return GetNetMode() == NM_DedicatedServer;
}

AActor* AAutomationGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Find all player starts
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationGameMode: No player starts found, using world origin"));
		return nullptr;
	}

	// Find least recently used player start
	AActor* BestStart = nullptr;
	float BestScore = -1.0f;

	for (AActor* Start : PlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Start);
		if (!PlayerStart)
			continue;

		// Calculate score based on distance to other players
		float Score = 0.0f;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC && PC->GetPawn())
			{
				float Distance = FVector::Dist(PlayerStart->GetActorLocation(), PC->GetPawn()->GetActorLocation());
				Score += Distance;
			}
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestStart = PlayerStart;
		}
	}

	return BestStart ? BestStart : PlayerStarts[0];
}

void AAutomationGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	TotalConnections++;
	ActivePlayers++;

	UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Player connected - Total: %d"), ActivePlayers);

	// Spawn player at appropriate location
	if (NewPlayer && !NewPlayer->GetPawn())
	{
		FVector SpawnLocation = CalculateSpawnLocation(ActivePlayers - 1);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		if (DefaultPawnClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (NewPawn)
			{
				NewPlayer->Possess(NewPawn);
				UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Spawned ship for player at %s"), *SpawnLocation.ToString());

				// Register with automation server
				if (AutomationServer)
				{
					FString ShipID = FString::Printf(TEXT("player_%d"), ActivePlayers);
					AutomationServer->RegisterShip(NewPawn, ShipID);
				}
			}
		}
	}
}

void AAutomationGameMode::Logout(AController* Exiting)
{
	ActivePlayers = FMath::Max(0, ActivePlayers - 1);

	UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: Player disconnected - Remaining: %d"), ActivePlayers);

	Super::Logout(Exiting);
}

FVector AAutomationGameMode::CalculateSpawnLocation(int32 PlayerIndex)
{
	// Spawn players in a circle around the origin
	float Radius = 500.0f;
	float Angle = (PlayerIndex * 360.0f / MaxPlayers) * (PI / 180.0f);

	FVector SpawnLocation;
	SpawnLocation.X = FMath::Cos(Angle) * Radius;
	SpawnLocation.Y = FMath::Sin(Angle) * Radius;
	SpawnLocation.Z = 300.0f; // Spawn 300 units above ground

	return SpawnLocation;
}
