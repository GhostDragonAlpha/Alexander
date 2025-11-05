// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SunStormComponent.generated.h"

/**
 * Component for managing sun storm effects
 * Handles electromagnetic storm simulation, particle effects, and light modulation
 */
UCLASS(ClassGroup=(Sun), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API USunStormComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USunStormComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Storm properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm Properties")
	bool bEnableStorms = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm Properties", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float StormIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm Properties", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float StormFrequency = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm Properties", meta = (ClampMin = "0.0"))
	float FlareIntensity = 1.5f;

	// Component references (set by owning Sun actor)
	class UPointLightComponent* CoreLight = nullptr;
	class UNiagaraComponent* StormEffect = nullptr;
	class UNiagaraComponent* FlareEffect = nullptr;

private:
	// Current storm state
	float CurrentStormIntensity = 0.0f;
	float StormTimer = 0.0f;

public:
	void InitializeStorm(class ASun* OwningSun, class UNiagaraComponent* InStormEffect, class UPointLightComponent* InCoreLight, class UNiagaraComponent* InFlareEffect, bool bInEnableStorms, float InIntensity, float InFrequency);

	// Get current storm intensity (0.0 to 1.0)
	float GetCurrentStormIntensity() const { return CurrentStormIntensity; }

	// Update storm simulation
	void UpdateStorm(float DeltaTime);
};