// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "SunStormComponent.h"
#include "ProceduralStarSystemGenerator.h"
#include "Sun.generated.h"

/**
 * ASun - Central star of the solar system simulation
 *
 * This actor represents the Sun in a space exploration game, providing:
 * - Realistic orbital mechanics as the root of the solar system hierarchy
 * - Spectacular visual effects with multiple light sources for dramatic illumination
 * - Dynamic electromagnetic storms that affect lighting and visual effects
 * - Subtle light flickering for realism and atmosphere
 * - Niagara particle effects for corona, solar flares, and storm activity
 *
 * The Sun uses a layered lighting approach:
 * - Main SunLight: Primary illumination source for the entire solar system
 * - CoreLight: Intense inner core glow for close-range dramatic effect
 * - CoronaLight: Soft outer corona glow that illuminates distant objects
 *
 * Storm mechanics create periodic bursts of activity that:
 * - Intensify light sources during peak phases
 * - Trigger solar flare particle effects
 * - Add color variation to simulate electrical activity
 * - Build up, peak, and decay over time for organic feel
 */
UCLASS(Blueprintable)
class ALEXANDER_API ASun : public AOrbitalBody
{
	GENERATED_BODY()

public:
	ASun();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Configuration asset for customizable sun properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Configuration")
	class USunConfiguration* SunConfig;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* SunMesh; // Visual representation of the sun's surface

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPointLightComponent* SunLight; // Primary light source illuminating the solar system

	// Additional lights for spectacular layered illumination effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPointLightComponent* CoreLight; // Intense inner core light for close-range drama

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPointLightComponent* CoronaLight; // Soft outer corona glow for distant illumination

	// Niagara particle system components for visual effects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* CoronaEffect; // Constant glowing corona around the sun

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* SolarFlares; // Burst effects during storm peaks

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* ElectromagneticStorm; // Electrical activity during storms

	// Storm management component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USunStormComponent* StormComponent;

	// Visual Properties - Control the sun's appearance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Visuals")
	float SunScale; // Size multiplier for the sun mesh (default: 3.0 for dramatic effect)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Visuals")
	FLinearColor SunColor; // Base color of the sun (default: warm golden FLinearColor(1.0f, 0.85f, 0.4f))

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Visuals")
	float EmissiveIntensity; // How brightly the sun surface glows (default: 1000.0f)

	// Lighting Properties - Control illumination intensity and range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Lighting")
	float MainLightIntensity; // Brightness of primary sun light (default: 50000.0f)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Lighting")
	float CoreLightIntensity; // Brightness of inner core light (default: 100000.0f)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Lighting")
	float CoronaLightIntensity; // Brightness of outer corona light (default: 25000.0f)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Lighting")
	float LightAttenuationRadius; // How far the lights reach (default: 200000.0f for solar system scale)

	// Storm Properties - Control electromagnetic storm behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Storms")
	bool bEnableElectromagneticStorms; // Whether to enable periodic storm activity (default: true)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Storms")
	float StormIntensity; // How intense storm effects are (0.0-1.0, default: 1.0)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Storms")
	float StormFrequency; // How often storms occur in seconds (default: 2.0)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Storms")
	float FlareIntensity; // Brightness multiplier for solar flare effects (default: 1.5)

	// Surface temperature in Kelvin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties|Physics")
	float SurfaceTemperature;

private:
	// Animation state variables for runtime effects
	float FlickerTimer; // Tracks light flicker animation
	float RotationAngle; // Current rotation for visual interest
	float StormTimer; // Tracks storm timing

	// Performance tracking
	float TotalTickTime; // Accumulated tick time for performance monitoring
	int32 TickCount; // Number of ticks for averaging

	// Core update functions called every frame
	void UpdateLightFlicker(float DeltaTime); // Applies subtle realistic light variations
};
