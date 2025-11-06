// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AutomationAPIServer.h"
#include "AutomationGameMode.generated.h"

/**
 * Automation Game Mode
 * Dedicated server game mode with automation API for AI-driven testing
 * Supports multiplayer VR spaceship flight + autonomous development
 */
UCLASS()
class ALEXANDER_API AAutomationGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAutomationGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ============================================================================
	// AUTOMATION SERVER
	// ============================================================================

	// Automation API server instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Automation")
	UAutomationAPIServer* AutomationServer;

	// Enable automation server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
	bool bEnableAutomationServer = true;

	// Automation server listen port
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
	int32 AutomationPort = 8080;

	// Enable verbose logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
	bool bVerboseLogging = true;

	// ============================================================================
	// MULTIPLAYER CONFIGURATION
	// ============================================================================

	// Maximum players
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	int32 MaxPlayers = 8;

	// Allow spectators
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	bool bAllowSpectators = true;

	// Default ship class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	TSubclassOf<APawn> DefaultShipClass;

	// ============================================================================
	// GAME STATE
	// ============================================================================

	// Get all active ships
	UFUNCTION(BlueprintCallable, Category = "Game State")
	TArray<AActor*> GetAllActiveShips();

	// Get server statistics
	UFUNCTION(BlueprintCallable, Category = "Game State")
	FString GetServerStatistics();

	// Is running as dedicated server?
	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsDedicatedServer() const;

	// ============================================================================
	// PLAYER SPAWNING
	// ============================================================================

	// Override spawn logic for multiplayer
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Server start time
	double ServerStartTime = 0.0;

	// Total connections
	int32 TotalConnections = 0;

	// Active player count
	int32 ActivePlayers = 0;

	// Performance metrics
	float AverageTickRate = 0.0f;
	int32 TickCount = 0;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Setup automation server
	void SetupAutomationServer();

	// Setup multiplayer configuration
	void SetupMultiplayerConfig();

	// Log server status
	void LogServerStatus();

	// Calculate spawn locations for multiplayer
	FVector CalculateSpawnLocation(int32 PlayerIndex);
};
