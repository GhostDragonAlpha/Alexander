// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseModule.generated.h"

// Forward declarations
class ABaseBuildingManager;

UCLASS()
class ALEXANDER_API ABaseModule : public AActor
{
	GENERATED_BODY()
	
public:
	ABaseModule();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Module identification
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	FGuid GetModuleID() const { return ModuleID; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	EBaseModuleType GetModuleType() const { return ModuleType; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	FString GetModuleName() const { return ModuleName; }

	// Module status
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	EModuleStatus GetModuleStatus() const { return Status; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetModuleStatus(EModuleStatus NewStatus) { Status = NewStatus; }

	// Health management
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetHealth(float NewHealth) { Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth); }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void DamageModule(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void RepairModule(float RepairAmount);

	// Power management
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	float GetPowerConsumption() const { return PowerConsumption; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	float GetPowerGeneration() const { return PowerGeneration; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	bool IsPowered() const { return bIsPowered; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetPowered(bool bPowered) { bIsPowered = bPowered; }

	// Grid position
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	FIntPoint GetGridPosition() const { return GridPosition; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetGridPosition(FIntPoint NewPosition) { GridPosition = NewPosition; }

	// Module size
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	EModuleSize GetModuleSize() const { return ModuleSize; }

	// Connections
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	TArray<FModuleConnection> GetConnections() const { return Connections; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void AddConnection(const FModuleConnection& Connection) { Connections.Add(Connection); }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void RemoveConnection(const FGuid& ConnectedModuleID);

	// Resources
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	TMap<FName, int32> GetStoredResources() const { return StoredResources; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	bool HasResource(FName ResourceID, int32 Quantity) const;

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void AddResource(FName ResourceID, int32 Quantity) { StoredResources.FindOrAdd(ResourceID) += Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	bool RemoveResource(FName ResourceID, int32 Quantity);

	// Upgrade level
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	int32 GetUpgradeLevel() const { return UpgradeLevel; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetUpgradeLevel(int32 NewLevel) { UpgradeLevel = FMath::Clamp(NewLevel, 1, MaxUpgradeLevel); }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	int32 GetMaxUpgradeLevel() const { return MaxUpgradeLevel; }

	// Efficiency
	UFUNCTION(BlueprintCallable, Category = "Base Module")
	float GetEfficiency() const { return Efficiency; }

	UFUNCTION(BlueprintCallable, Category = "Base Module")
	void SetEfficiency(float NewEfficiency) { Efficiency = FMath::Clamp(NewEfficiency, 0.0f, 1.0f); }

protected:
	// Module identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FGuid ModuleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	EBaseModuleType ModuleType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	FString ModuleName;

	// Grid placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FIntPoint GridPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	EModuleSize ModuleSize;

	// Status and health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	EModuleStatus Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHealth;

	// Power
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float PowerGeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	bool bIsPowered;

	// Atmosphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bHasAtmosphere;

	// Storage capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	int32 StorageCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	TMap<FName, int32> StoredResources;

	// Population capacity (for habitat modules)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitat")
	int32 PopulationCapacity;

	// Defense
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float DefenseRating;

	// Connections to other modules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FModuleConnection> Connections;

	// Upgrade level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 UpgradeLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 MaxUpgradeLevel;

	// Efficiency (affected by damage, upgrades, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float Efficiency;

	// Building manager reference
	UPROPERTY()
	TWeakObjectPtr<ABaseBuildingManager> BuildingManager;

	// Land claim this module belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Claim")
	FGuid ClaimID;
};