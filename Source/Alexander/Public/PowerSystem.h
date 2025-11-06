// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSystem.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UBoxComponent;

/**
 * Power source type
 */
UENUM(BlueprintType)
enum class EPowerSourceType : uint8
{
	Solar,
	Generator,
	Battery,
	Grid
};

/**
 * Power consumer interface
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPowerConsumer : public UInterface
{
	GENERATED_BODY()
};

class IPowerConsumer
{
	GENERATED_BODY()

public:
	/**
	 * Get power requirement in watts
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	float GetPowerRequirement() const;

	/**
	 * Set whether this consumer is receiving power
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	void SetPowered(bool bIsPowered);

	/**
	 * Check if this consumer is currently powered
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	bool IsPowered() const;
};

// Inline implementations of interface Execute functions to avoid ODR violations
FORCEINLINE float IPowerConsumer::Execute_GetPowerRequirement(const UObject* O)
{
	return 0.0f;
}

FORCEINLINE void IPowerConsumer::Execute_SetPowered(UObject* O, bool bIsPowered)
{
	// Default implementation does nothing
}

FORCEINLINE bool IPowerConsumer::Execute_IsPowered(const UObject* O)
{
	return false;
}

/**
 * Power provider interface
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPowerProvider : public UInterface
{
	GENERATED_BODY()
};

class IPowerProvider
{
	GENERATED_BODY()

public:
	/**
	 * Get available power output in watts
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	float GetAvailablePower() const;

	/**
	 * Get maximum power capacity in watts
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	float GetMaxPowerCapacity() const;

	/**
	 * Get power source type
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Power")
	EPowerSourceType GetPowerSourceType() const;
};

// Inline implementations of interface Execute functions
FORCEINLINE float IPowerProvider::Execute_GetAvailablePower(const UObject* O)
{
	return 0.0f;
}

FORCEINLINE float IPowerProvider::Execute_GetMaxPowerCapacity(const UObject* O)
{
	return 0.0f;
}

FORCEINLINE EPowerSourceType IPowerProvider::Execute_GetPowerSourceType(const UObject* O)
{
	return EPowerSourceType::Grid;
}

/**
 * Power network node - connects power providers and consumers
 */
UCLASS()
class ALEXANDER_API APowerNetworkNode : public AActor
{
	GENERATED_BODY()

public:
	APowerNetworkNode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* NodeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ConnectionRange;

	// ============================================================================
	// NETWORK PROPERTIES
	// ============================================================================

	/** Network ID for grouping connected nodes */
	UPROPERTY(BlueprintReadOnly, Category = "Power")
	int32 NetworkID;

	/** Connected power providers */
	UPROPERTY(BlueprintReadOnly, Category = "Power")
	TArray<TScriptInterface<IPowerProvider>> ConnectedProviders;

	/** Connected power consumers */
	UPROPERTY(BlueprintReadOnly, Category = "Power")
	TArray<TScriptInterface<IPowerConsumer>> ConnectedConsumers;

	/** Adjacent network nodes */
	UPROPERTY(BlueprintReadOnly, Category = "Power")
	TArray<APowerNetworkNode*> ConnectedNodes;

	/** Maximum connection range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float ConnectionRangeCm;

	/** Update interval for network recalculation (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	float UpdateInterval;

	/** Time since last update */
	UPROPERTY()
	float TimeSinceLastUpdate;

	// ============================================================================
	// NETWORK OPERATIONS
	// ============================================================================

	/**
	 * Update network connections
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void UpdateNetworkConnections();

	/**
	 * Calculate total available power in network
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetTotalAvailablePower() const;

	/**
	 * Calculate total power demand in network
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetTotalPowerDemand() const;

	/**
	 * Check if network has sufficient power
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	bool HasSufficientPower() const;

	/**
	 * Distribute power to consumers
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void DistributePower();

	/**
	 * Find all actors implementing power interfaces in range
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void FindPowerActorsInRange();

	/**
	 * Connect to another network node
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void ConnectToNode(APowerNetworkNode* OtherNode);

	/**
	 * Disconnect from another network node
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void DisconnectFromNode(APowerNetworkNode* OtherNode);

protected:
	/**
	 * Find nearby network nodes
	 */
	TArray<APowerNetworkNode*> FindNearbyNodes() const;

	/**
	 * Propagate network ID to connected nodes
	 */
	void PropagateNetworkID(int32 NewNetworkID);
};
