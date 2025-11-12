// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameLauncher.generated.h"

// Game launch configuration
USTRUCT(BlueprintType)
struct FGameLaunchConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString Resolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FString QualityPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 HTTPPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnableConsole;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnableDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxRestarts;

	FGameLaunchConfig()
	{
		MapName = TEXT("VRTemplateMap");
		GameMode = TEXT("/Script/Alexander.AutomationGameMode");
		Resolution = TEXT("1280x720");
		QualityPreset = TEXT("medium");
		HTTPPort = 8080;
		bEnableConsole = true;
		bEnableDebug = true;
		MaxRestarts = 3;
	}
};

// Game process information
USTRUCT(BlueprintType)
struct FGameProcessInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	int32 ProcessId;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	FString StartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	FString MapName;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	int32 HTTPPort;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Process")
	float Uptime;

	FGameProcessInfo()
	{
		ProcessId = 0;
		HTTPPort = 8080;
		bIsRunning = false;
		Uptime = 0.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UGameLauncher : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGameLauncher();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Initialize the launcher
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	void Initialize();

	// Launch the game
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	bool LaunchGame(const FGameLaunchConfig& Config);

	// Launch game with default configuration
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	bool LaunchGameWithDefaultConfig();

	// Check if game is running
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	bool IsGameRunning() const;

	// Check if game is ready (HTTP API responding)
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	bool IsGameReady() const;

	// Get game process information
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	FGameProcessInfo GetProcessInfo() const;

	// Shutdown the game
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	void ShutdownGame();

	// Restart the game
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	bool RestartGame();

	// Get default launch configuration
	UFUNCTION(BlueprintCallable, Category = "Game Launcher")
	FGameLaunchConfig GetDefaultConfig() const;

protected:
	// Launch process
	bool LaunchProcess(const FGameLaunchConfig& Config);

	// Wait for game to be ready
	bool WaitForGameReady(float Timeout);

	// Save process information
	void SaveProcessInfo(const FGameLaunchConfig& Config);

	// Load process information
	bool LoadProcessInfo();

	// Check process health
	bool CheckProcessHealth();

	// Kill existing process
	void KillExistingProcess();

private:
	// Default configuration
	FGameLaunchConfig DefaultConfig;

	// Current configuration
	FGameLaunchConfig CurrentConfig;

	// Process information
	FGameProcessInfo ProcessInfo;

	// Process handle (platform specific)
	void* ProcessHandle;

	// Start time
	float StartTime;

	// Restart count
	int32 RestartCount;

	// Maximum restart attempts
	int32 MaxRestartAttempts;
};