// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceGatheringSystem.h"
#include "SystemSelfTestInterface.h"
#include "PlanetaryMiningSystem.generated.h"

// Forward declarations
class APlanet;
class ASpaceship;
class UInventoryManager;

// Mining equipment types
UENUM(BlueprintType)
enum class EPlanetaryMiningEquipment : uint8
{
	HandDrill UMETA(DisplayName = "Hand Drill"),
	SurfaceMiner UMETA(DisplayName = "Surface Miner"),
	DeepDrill UMETA(DisplayName = "Deep Drill"),
	FrackingRig UMETA(DisplayName = "Fracking Rig"),
	Excavator UMETA(DisplayName = "Excavator")
};

// Environmental impact levels
UENUM(BlueprintType)
enum class EEnvironmentalImpact : uint8
{
	Minimal UMETA(DisplayName = "Minimal"),
	Low UMETA(DisplayName = "Low"),
	Moderate UMETA(DisplayName = "Moderate"),
	High UMETA(DisplayName = "High"),
	Severe UMETA(DisplayName = "Severe")
};

// Mining permit status
UENUM(BlueprintType)
enum class EPermitStatus : uint8
{
	NotRequired UMETA(DisplayName = "Not Required"),
	Available UMETA(DisplayName = "Available"),
	Applied UMETA(DisplayName = "Applied"),
	Granted UMETA(DisplayName = "Granted"),
	Denied UMETA(DisplayName = "Denied"),
	Revoked UMETA(DisplayName = "Revoked")
};

// Planetary scan result
USTRUCT(BlueprintType)
struct FPlanetaryScanResult
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

	// Environmental impact of scanning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scan")
	EEnvironmentalImpact EnvironmentalImpact;

	FPlanetaryScanResult()
	{
		bSuccess = false;
		ScanStrength = 0.0f;
		ScanQuality = 0.0f;
		ScanTime = 0.0f;
		EnvironmentalImpact = EEnvironmentalImpact::Minimal;
	}
};

// Mining operation parameters
USTRUCT(BlueprintType)
struct FPlanetaryMiningParams
{
	GENERATED_BODY()

	// Equipment type being used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EPlanetaryMiningEquipment EquipmentType;

	// Mining power (affects extraction rate)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float MiningPower;

	// Mining efficiency (affects resource yield)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float MiningEfficiency;

	// Environmental impact multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float EnvironmentalImpactMultiplier;

	FPlanetaryMiningParams()
	{
		EquipmentType = EPlanetaryMiningEquipment::HandDrill;
		MiningPower = 1.0f;
		MiningEfficiency = 1.0f;
		EnvironmentalImpactMultiplier = 1.0f;
	}
};

// Planetary mining operation result
USTRUCT(BlueprintType)
struct FPlanetaryMiningResult
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

	// Environmental impact caused
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EEnvironmentalImpact EnvironmentalImpact;

	// Time taken for operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float OperationTime;

	// Experience gained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float ExperienceGained;

	// Failure reason (if failed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	FString FailureReason;

	FPlanetaryMiningResult()
	{
		bSuccess = false;
		TotalValue = 0.0f;
		EnergyConsumed = 0.0f;
		EnvironmentalImpact = EEnvironmentalImpact::Minimal;
		OperationTime = 0.0f;
		ExperienceGained = 0.0f;
	}
};

// Mining permit
USTRUCT(BlueprintType)
struct FMiningPermit
{
	GENERATED_BODY()

	// Permit ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	FGuid PermitID;

	// Planet this permit is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	TWeakObjectPtr<APlanet> TargetPlanet;

	// Permit status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	EPermitStatus Status;

	// Issue date
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	FDateTime IssueDate;

	// Expiration date
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	FDateTime ExpirationDate;

	// Maximum extraction allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	int32 MaxExtraction;

	// Current extraction under this permit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	int32 CurrentExtraction;

	// Environmental restrictions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Permit")
	EEnvironmentalImpact MaxAllowedImpact;

	FMiningPermit()
	{
		Status = EPermitStatus::NotRequired;
		MaxExtraction = 1000;
		CurrentExtraction = 0;
		MaxAllowedImpact = EEnvironmentalImpact::Moderate;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlanetaryScanned, APlanet*, Planet, const FPlanetaryScanResult&, ScanResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlanetaryMiningStarted, APlanet*, Planet, const FPlanetaryMiningParams&, Params);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlanetaryMiningCompleted, APlanet*, Planet, const FPlanetaryMiningResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentalImpactChanged, APlanet*, Planet, EEnvironmentalImpact, NewImpact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiningPermitChanged, const FMiningPermit&, Permit, EPermitStatus, NewStatus);

/**
 * Planetary Mining System - Handles surface and underground mining operations
 */
UCLASS()
class ALEXANDER_API UPlanetaryMiningSystem : public UWorldSubsystem, public ISystemSelfTestInterface
{
	GENERATED_BODY()

public:
	UPlanetaryMiningSystem();

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
	// PLANETARY SCANNING
	// ============================================================================

	// Scan a planetary location for resources
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryScanResult ScanPlanetaryLocation(APlanet* Planet, FVector Location, float ScanStrength);

	// Quick surface scan
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryScanResult QuickSurfaceScan(APlanet* Planet, FVector Location);

	// Deep geological scan
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryScanResult DeepGeologicalScan(APlanet* Planet, FVector Location);

	// Get last scan result for location
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryScanResult GetLastScanResult(APlanet* Planet, FVector Location) const;

	// ============================================================================
	// MINING OPERATIONS
	// ============================================================================

	// Mine resources at a planetary location
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryMiningResult MinePlanetaryLocation(APlanet* Planet, FVector Location, const FPlanetaryMiningParams& Params);

	// Mine with current equipment
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	FPlanetaryMiningResult MineWithCurrentEquipment(APlanet* Planet, FVector Location);

	// Get current mining equipment
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	EPlanetaryMiningEquipment GetCurrentEquipment() const { return CurrentEquipment; }

	// Set current mining equipment
	UFUNCTION(BlueprintCallable, Category = "Planetary Mining")
	void SetCurrentEquipment(EPlanetaryMiningEquipment Equipment);

	// ============================================================================
	// ENVIRONMENTAL MANAGEMENT
	// ============================================================================

	// Get environmental impact at location
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	EEnvironmentalImpact GetEnvironmentalImpact(APlanet* Planet, FVector Location) const;

	// Get planetary environmental health
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	float GetPlanetaryHealth(APlanet* Planet) const;

	// Restore environmental health
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	void RestoreEnvironmentalHealth(APlanet* Planet, float Amount);

	// ============================================================================
	// PERMIT MANAGEMENT
	// ============================================================================

	// Apply for mining permit
	UFUNCTION(BlueprintCallable, Category = "Permits")
	FMiningPermit ApplyForMiningPermit(APlanet* Planet, int32 RequestedExtraction, EEnvironmentalImpact MaxImpact);

	// Get mining permit for planet
	UFUNCTION(BlueprintCallable, Category = "Permits")
	FMiningPermit GetMiningPermit(APlanet* Planet) const;

	// Check if mining is permitted at location
	UFUNCTION(BlueprintCallable, Category = "Permits")
	bool IsMiningPermitted(APlanet* Planet, FVector Location, EEnvironmentalImpact Impact) const;

	// Update permit status
	UFUNCTION(BlueprintCallable, Category = "Permits")
	void UpdatePermitStatus(APlanet* Planet, EPermitStatus NewStatus);

	// ============================================================================
	// EQUIPMENT MANAGEMENT
	// ============================================================================

	// Get equipment statistics
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	float GetEquipmentMiningPower(EPlanetaryMiningEquipment Equipment) const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	float GetEquipmentEfficiency(EPlanetaryMiningEquipment Equipment) const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	float GetEquipmentEnvironmentalImpact(EPlanetaryMiningEquipment Equipment) const;

	// Upgrade equipment
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UpgradeEquipment(EPlanetaryMiningEquipment NewEquipment);

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total mining operations
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalMiningOperations() const { return TotalMiningOperations; }

	// Get total resources extracted
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalResourcesExtracted() const { return TotalResourcesExtracted; }

	// Get total environmental impact
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalEnvironmentalImpact() const { return TotalEnvironmentalImpact; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Planetary Mining Events")
	FOnPlanetaryScanned OnPlanetaryScanned;

	UPROPERTY(BlueprintAssignable, Category = "Planetary Mining Events")
	FOnPlanetaryMiningStarted OnPlanetaryMiningStarted;

	UPROPERTY(BlueprintAssignable, Category = "Planetary Mining Events")
	FOnPlanetaryMiningCompleted OnPlanetaryMiningCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Planetary Mining Events")
	FOnEnvironmentalImpactChanged OnEnvironmentalImpactChanged;

	UPROPERTY(BlueprintAssignable, Category = "Planetary Mining Events")
	FOnMiningPermitChanged OnMiningPermitChanged;

protected:
	// Resource gathering system reference
	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	// Inventory manager reference
	UPROPERTY()
	TWeakObjectPtr<UInventoryManager> InventoryManager;

	// Current mining equipment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	EPlanetaryMiningEquipment CurrentEquipment;

	// Scan history
	UPROPERTY()
	TMap<TPair<TWeakObjectPtr<APlanet>, FVector>, FPlanetaryScanResult> ScanHistory;

	// Mining permits
	UPROPERTY()
	TMap<TWeakObjectPtr<APlanet>, FMiningPermit> MiningPermits;

	// Environmental health per planet
	UPROPERTY()
	TMap<TWeakObjectPtr<APlanet>, float> PlanetaryEnvironmentalHealth;

	// Statistics
	UPROPERTY()
	int32 TotalMiningOperations;

	UPROPERTY()
	TMap<FName, int32> TotalResourcesExtracted;

	UPROPERTY()
	float TotalEnvironmentalImpact;

	// Mining skill level (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
	float MiningSkillLevel;

	// Equipment statistics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<EPlanetaryMiningEquipment, float> EquipmentPowerMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<EPlanetaryMiningEquipment, float> EquipmentEfficiencyMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<EPlanetaryMiningEquipment, float> EquipmentImpactMap;

private:
	// Helper functions
	void InitializeEquipmentStatistics();
	float CalculateEnvironmentalImpact(const FPlanetaryMiningParams& Params, const FResourceDeposit& Deposit) const;
	bool CheckPermitRestrictions(const FMiningPermit& Permit, const FPlanetaryMiningParams& Params, int32 AmountExtracted) const;
	void UpdateEnvironmentalHealth(APlanet* Planet, EEnvironmentalImpact Impact);
};