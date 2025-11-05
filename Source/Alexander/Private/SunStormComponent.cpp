// Copyright Epic Games, Inc. All Rights Reserved.

#include "SunStormComponent.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"

USunStormComponent::USunStormComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USunStormComponent::InitializeStorm(class ASun* OwningSun, class UNiagaraComponent* InStormEffect, class UPointLightComponent* InCoreLight, class UNiagaraComponent* InFlareEffect, bool bInEnableStorms, float InIntensity, float InFrequency)
{
	StormEffect = InStormEffect;
	CoreLight = InCoreLight;
	FlareEffect = InFlareEffect;
	bEnableStorms = bInEnableStorms;
	StormIntensity = InIntensity;
	StormFrequency = InFrequency;
}

void USunStormComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableStorms)
	{
		UpdateStorm(DeltaTime);
	}
}

void USunStormComponent::UpdateStorm(float DeltaTime)
{
	// Increment storm timer to track cycle progress
	StormTimer += DeltaTime;

	// Calculate storm cycle timing based on frequency setting
	float StormCycle = 1.0f / FMath::Max(StormFrequency, 0.1f);
	float StormPhase = FMath::Fmod(StormTimer, StormCycle) / StormCycle;

	// Create organic storm intensity wave that builds up, peaks, and decays
	float StormWave = 0.0f;
	if (StormPhase < 0.3f)
	{
		// Build-up phase: gradual increase using sine wave
		StormWave = FMath::Sin(StormPhase / 0.3f * PI * 0.5f);
	}
	else if (StormPhase < 0.5f)
	{
		// Peak burst phase: intense activity with oscillation
		StormWave = 1.0f + FMath::Sin((StormPhase - 0.3f) / 0.2f * PI) * 0.5f;
	}
	else
	{
		// Decay phase: exponential falloff for natural dissipation
		StormWave = FMath::Exp(-(StormPhase - 0.5f) * 5.0f);
	}

	CurrentStormIntensity = StormWave;

	// Apply storm intensity to core light for dramatic effect
	float StormBoost = StormWave * StormIntensity * 20000.0f;

	if (CoreLight)
	{
		CoreLight->SetIntensity(CoreLight->Intensity + StormBoost);

		// Add color variation during intense storms for electrical activity effect
		if (StormWave > 0.7f)
		{
			FLinearColor StormColor = FLinearColor(1.0f, 0.95f + FMath::RandRange(-0.1f, 0.1f), 0.8f);
			CoreLight->SetLightColor(StormColor);
		}
	}

	// Update Niagara storm effect intensity parameter
	if (StormEffect && StormEffect->GetAsset())
	{
		StormEffect->SetFloatParameter(FName("CurrentIntensity"), StormWave * StormIntensity);
	}

	// Trigger solar flare bursts at storm peak
	if (StormPhase >= 0.3f && StormPhase < 0.35f && FlareEffect && FlareEffect->GetAsset())
	{
		// Burst event trigger for particle system
		FlareEffect->SetFloatParameter(FName("BurstTrigger"), 1.0f);
	}
	else if (FlareEffect && FlareEffect->GetAsset())
	{
		FlareEffect->SetFloatParameter(FName("BurstTrigger"), 0.0f);
	}
}