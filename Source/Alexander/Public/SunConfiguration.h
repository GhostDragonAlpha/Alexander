// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SunConfiguration.generated.h"

/**
 * Configuration asset for Sun actor properties
 *
 * This data-only asset allows designers to tweak sun appearance and behavior
 * without modifying code. Can be shared across multiple sun instances or
 * easily swapped for different solar system themes.
 */
UCLASS(BlueprintType)
class ALEXANDER_API USunConfiguration : public UDataAsset
{
	GENERATED_BODY()

public:
	USunConfiguration();

	// === VISUAL PROPERTIES ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SunScale = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.4f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0"))
	float EmissiveIntensity = 1000.0f;

	// === LIGHTING PROPERTIES ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
	float MainLightIntensity = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
	float CoreLightIntensity = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0"))
	float CoronaLightIntensity = 25000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "1000.0"))
	float LightAttenuationRadius = 200000.0f;

	// === STORM PROPERTIES ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms")
	bool bEnableElectromagneticStorms = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float StormIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float StormFrequency = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms", meta = (ClampMin = "0.0"))
	float FlareIntensity = 1.5f;

	// === PHYSICAL PROPERTIES ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Mass = 1.989e30f; // Mass of our Sun in kg

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0"))
	float RotationSpeed = 5.0f; // Degrees per second

	// === LIGHT COLORS ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Colors")
	FLinearColor CoreLightColor = FLinearColor(1.0f, 0.95f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Colors")
	FLinearColor CoronaLightColor = FLinearColor(1.0f, 0.7f, 0.3f);

	// === STORM COLORS ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms|Colors")
	FLinearColor StormColorBase = FLinearColor(1.0f, 0.95f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storms|Colors", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float StormColorVariation = 0.1f;
};