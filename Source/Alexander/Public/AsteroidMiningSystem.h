// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceGatheringSystem.h"
#include "SystemSelfTestInterface.h"
#include "AsteroidMiningSystem.generated.h"

// Forward declarations
class AAsteroid;
class ASpaceship;
class UInventoryManager;

// Mining laser types
UENUM(BlueprintType)
enum class EMiningLaserType : uint8
{
	Basic UMETA(DisplayName = "Basic Mining Laser"),
	Advanced UMETA(DisplayName = "Advanced Mining Laser"),
	Industrial UMETA(DisplayName = "Industrial Mining Laser"),
	Quantum UMETA(DisplayName = "Quantum Mining Laser")
};

// Mining efficiency tiers
UENUM(BlueprintType)
enum class EMiningEfficiencyTier : uint8
{
	Tier1_Basic UMETA(DisplayName = "Tier 1 - Basic"),
	Tier2_Standard UMETA(DisplayName = "Tier 2 - Standard"),
	Tier3_Advanced UMETA(DisplayName = "Tier 3 - Advanced"),
	Tier4_Masterwork UMETA(DisplayName = "Tier 4 - Masterwork")
};

// Asteroid scan result
USTRUCT(BlueprintType)
struct FAsteroidScanResult
{
	GENERATED_BODY()

	// Was scan successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	bool bSuccess;

	// Scan strength used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	float ScanStrength;

	// Resources detected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	TArray<FResourceDeposit> DetectedResources;

	// Scan quality (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	float ScanQuality;

	// Time taken for scan
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	float ScanTime;

	FAsteroidScanResult()
	{
		bSuccess = false;
		ScanStrength = 0.0f;
		ScanQuality = 0.0f;
		ScanTime = 0.0f;
	}
};

// Mining operation result
USTRUCT(BlueprintType)
struct FMiningOperationResult
{
	GENERATED_BODY()

	// Was mining successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	bool bSuccess;

	// Resources extracted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	TArray<FResourceQuantity> ResourcesExtracted;

	// Total value of extracted resources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float TotalValue;

	// Energy consumed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float EnergyConsumed;

	// Time taken for operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float OperationTime;

	// Experience gained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float ExperienceGained;

	// Failure reason (if failed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	FString FailureReason;

	FMiningOperationResult()
	{
		bSuccess = false;
		TotalValue = 0.0f;
		EnergyConsumed = 0.0f;
		OperationTime = 0.0f;
		ExperienceGained = 0.0f;
	}
};

// Active mining operation
USTRUCT(BlueprintType)
struct FActiveMiningOperation
{
	GENERATED_BODY()

	// Asteroid being mined
	UPROPERTY()
	TWeakObjectPtr<AAsteroid> TargetAsteroid;

	// Mining laser type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EMiningLaserType LaserType;

	// Operation start time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float StartTime;

	// Operation duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float Duration;

	// Is operation active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	bool bIsActive;

	FActiveMiningOperation()
	{
		LaserType = EMiningLaserType::Basic;
		StartTime = 0.0f;
		Duration = 0.0f;
		bIsActive = false;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAsteroidScanned, AAsteroid*, Asteroid, const FAsteroidScanResult&, ScanResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiningStarted, AAsteroid*, Asteroid, EMiningLaserType, LaserType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiningCompleted, AAsteroid*, Asteroid, const FMiningOperationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiningLaserUpgraded, EMiningLaserType, NewLaserType);

/**
 * Asteroid Mining System - Handles asteroid scanning and mining operations
 */
UCLASS()
class ALEXANDER_API UAsteroidMiningSystem : public UWorldSubsystem, public ISystemSelfTestInterface
{
	GENERATED_BODY()

public:
	UAsteroidMiningSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	//~ Begin ISystemSelfTestInterface interface
	virtual bool RunSelfTest_Implementation(FSystemTestResult& OutResult) override;
	virtual FString GetSystemName_Implementation() const override;
	virtual FString GetTestDescription_Implementation() const override;
	virtual bool IsReadyForTesting_Implementation() const override;
	//~ End ISystemSelfTestInterface interface

	// ============================================================================
	// ASTEROID SCANNING
	// ============================================================================

	// Scan an asteroid for resources
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	FAsteroidScanResult ScanAsteroid(AAsteroid* Asteroid, float ScanStrength);

	// Quick scan (low power, fast)
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	FAsteroidScanResult QuickScanAsteroid(AAsteroid* Asteroid);

	// Deep scan (high power, slow, more accurate)
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	FAsteroidScanResult DeepScanAsteroid(AAsteroid* Asteroid);

	// Get last scan result for asteroid
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	FAsteroidScanResult GetLastScanResult(AAsteroid* Asteroid) const;

	// ============================================================================
	// MINING OPERATIONS
	// ============================================================================

	// Start mining an asteroid
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	bool StartMining(AAsteroid* Asteroid, EMiningLaserType LaserType);

	// Stop current mining operation
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	void StopMining();

	// Is currently mining?
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	bool IsMining() const { return CurrentOperation.bIsActive; }

	// Get current mining target
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	AAsteroid* GetCurrentMiningTarget() const;

	// Get mining progress (0-1)
	UFUNCTION(BlueprintCallable, Category = "Asteroid Mining")
	float GetMiningProgress() const;

	// ============================================================================
	// MINING LASER MANAGEMENT
	// ============================================================================

	// Get current mining laser type
	UFUNCTION(BlueprintCallable, Category = "Mining Laser")
	EMiningLaserType GetCurrentMiningLaser() const { return CurrentLaserType; }

	// Upgrade mining laser
	UFUNCTION(BlueprintCallable, Category = "Mining Laser")
	bool UpgradeMiningLaser(EMiningLaserType NewLaserType);

	// Get mining laser statistics
	UFUNCTION(BlueprintCallable, Category = "Mining Laser")
	float GetLaserMiningPower(EMiningLaserType LaserType) const;

	UFUNCTION(BlueprintCallable, Category = "Mining Laser")
	float GetLaserEnergyConsumption(EMiningLaserType LaserType) const;

	UFUNCTION(BlueprintCallable, Category = "Mining Laser")
	float GetLaserEfficiency(EMiningLaserType LaserType) const;

	// ============================================================================
	// MINING EFFICIENCY
	// ============================================================================

	// Get current mining efficiency tier
	UFUNCTION(BlueprintCallable, Category = "Mining Efficiency")
	EMiningEfficiencyTier GetMiningEfficiencyTier() const { return CurrentEfficiencyTier; }

	// Upgrade mining efficiency
	UFUNCTION(BlueprintCallable, Category = "Mining Efficiency")
	bool UpgradeMiningEfficiency(EMiningEfficiencyTier NewTier);

	// Get efficiency multiplier
	UFUNCTION(BlueprintCallable, Category = "Mining Efficiency")
	float GetEfficiencyMultiplier(EMiningEfficiencyTier Tier) const;

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total asteroids mined
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalAsteroidsMined() const { return TotalAsteroidsMined; }

	// Get total resources extracted from asteroids
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalResourcesExtracted() const { return TotalResourcesExtracted; }

	// Get total mining time
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalMiningTime() const { return TotalMiningTime; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Asteroid Mining Events")
	FOnAsteroidScanned OnAsteroidScanned;

	UPROPERTY(BlueprintAssignable, Category = "Asteroid Mining Events")
	FOnMiningStarted OnMiningStarted;

	UPROPERTY(BlueprintAssignable, Category = "Asteroid Mining Events")
	FOnMiningCompleted OnMiningCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Asteroid Mining Events")
	FOnMiningLaserUpgraded OnMiningLaserUpgraded;

protected:
	// Resource gathering system reference
	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	// Inventory manager reference
	UPROPERTY()
	TWeakObjectPtr<UInventoryManager> InventoryManager;

	// Current mining laser type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EMiningLaserType CurrentLaserType;

	// Current mining efficiency tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EMiningEfficiencyTier CurrentEfficiencyTier;

	// Current mining operation
	UPROPERTY()
	FActiveMiningOperation CurrentOperation;

	// Scan history
	UPROPERTY()
	TMap<AAsteroid*, FAsteroidScanResult> ScanHistory;

	// Statistics
	UPROPERTY()
	int32 TotalAsteroidsMined;

	UPROPERTY()
	TMap<FName, int32> TotalResourcesExtracted;

	UPROPERTY()
	float TotalMiningTime;

	// Mining skill level (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float MiningSkillLevel;

	// Laser statistics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	TMap<EMiningLaserType, float> LaserPowerMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	TMap<EMiningLaserType, float> LaserEnergyConsumptionMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	TMap<EMiningLaserType, float> LaserEfficiencyMap;

private:
	// Helper functions
	void InitializeLaserStatistics();
	void InitializeEfficiencyTiers();
	FMiningOperationResult CompleteMiningOperation();
	float CalculateMiningDuration(AAsteroid* Asteroid, EMiningLaserType LaserType) const;
	float CalculateEnergyConsumption(float Duration, EMiningLaserType LaserType) const;
};