// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipPhysicsConfig.generated.h"

/**
 * Ship Physics Configuration Data Asset
 * Defines preset physics parameters for different ship types
 * Used by ShipCustomizationComponent to apply physics profiles
 */
UCLASS(BlueprintType)
class ALEXANDER_API UShipPhysicsConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Configuration name (e.g., "Light Fighter", "Heavy Freighter")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FText ConfigName;

	// Description of this configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FText Description;

	// ============================================================================
	// PHYSICS PARAMETERS
	// ============================================================================

	// Mass in kilograms (affects inertia and gravity pull)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass = 1000.0f;

	// Thrust power in Newtons (affects acceleration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float ThrustPower = 100000.0f;

	// Maximum velocity in m/s (safety limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float MaxVelocity = 1000.0f;

	// Angular thrust power (affects rotation speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AngularThrustPower = 50000.0f;

	// Fuel capacity (future feature)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float FuelCapacity = 1000.0f;

	// Fuel consumption rate per second (future feature)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float FuelConsumptionRate = 1.0f;

	// ============================================================================
	// FLIGHT CHARACTERISTICS
	// ============================================================================

	// Acceleration multiplier (how quickly thrust changes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float AccelerationMultiplier = 1.0f;

	// Rotation speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float RotationSpeedMultiplier = 1.0f;

	// Stability assist strength (0 = none, 1 = maximum)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StabilityAssistStrength = 0.5f;
};
