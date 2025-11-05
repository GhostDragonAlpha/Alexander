// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRWateringCan.h"
#include "FarmPlot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AVRWateringCan::AVRWateringCan()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);

	// Create watering can mesh
	CanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CanMesh"));
	CanMesh->SetupAttachment(RootComp);
	CanMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CanMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CanMesh->SetSimulatePhysics(true);
	CanMesh->SetMassOverrideInKg(NAME_None, 0.5f); // Light when empty

	// Create grab collision sphere
	GrabCollision = CreateDefaultSubobject<USphereComponent>(TEXT("GrabCollision"));
	GrabCollision->SetupAttachment(CanMesh);
	GrabCollision->SetSphereRadius(15.0f);
	GrabCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GrabCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrabCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create spout location (where water comes out)
	SpoutLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpoutLocation"));
	SpoutLocation->SetupAttachment(CanMesh);
	SpoutLocation->SetRelativeLocation(FVector(20.0f, 0.0f, 10.0f)); // Adjust based on mesh

	// Create water stream Niagara component
	WaterStreamEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WaterStreamEffect"));
	WaterStreamEffect->SetupAttachment(SpoutLocation);
	WaterStreamEffect->SetAutoActivate(false);

	// Initialize water properties
	WaterCapacity = 10.0f; // 10 liters
	CurrentWaterAmount = WaterCapacity;
	WaterFlowRate = 0.5f; // 0.5 liters per second
	MinTiltAngleForPouring = 45.0f; // Degrees
	MaxWateringDistance = 200.0f; // cm

	// Initialize state
	bIsGrabbed = false;
	GrabbingHand = nullptr;
	bIsPouring = false;
	bWaterHittingSurface = false;
	WaterHitLocation = FVector::ZeroVector;
	LastHapticTime = 0.0f;
	HapticFeedbackInterval = 0.1f; // 10 Hz
	HapticIntensityMultiplier = 0.4f;
}

void AVRWateringCan::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic material instance for water level visualization
	if (CanMesh && CanMesh->GetMaterial(0))
	{
		CanMaterialInstance = CanMesh->CreateDynamicMaterialInstance(0);
		UpdateVisualFeedback();
	}

	// Set initial water stream system
	if (WaterStreamNiagaraSystem && WaterStreamEffect)
	{
		WaterStreamEffect->SetAsset(WaterStreamNiagaraSystem);
	}
}

void AVRWateringCan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGrabbed)
	{
		// Update pouring state based on orientation
		UpdatePouringState();

		if (bIsPouring && CurrentWaterAmount > 0.0f)
		{
			// Update water stream
			UpdateWaterStream(DeltaTime);

			// Perform watering raycast
			PerformWateringRaycast();

			// Apply water to farm plots
			ApplyWaterToFarmPlots(DeltaTime);

			// Update haptic feedback
			UpdateHapticFeedback();

			// Consume water
			CurrentWaterAmount = FMath::Max(0.0f, CurrentWaterAmount - (WaterFlowRate * DeltaTime));

			// Update visual feedback
			UpdateVisualFeedback();

			// Stop pouring if out of water
			if (CurrentWaterAmount <= 0.0f)
			{
				StopPouring();
				
				// Play empty sound
				if (EmptySound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
				}
			}
		}
	}

	// Update audio
	UpdateAudio();
}

void AVRWateringCan::OnGrabbed(AActor* GrabbingActor)
{
	bIsGrabbed = true;
	GrabbingHand = GrabbingActor;

	// Disable physics while grabbed
	if (CanMesh)
	{
		CanMesh->SetSimulatePhysics(false);
	}

	UE_LOG(LogTemp, Log, TEXT("VRWateringCan: Grabbed by %s"), *GrabbingActor->GetName());
}

void AVRWateringCan::OnReleased()
{
	bIsGrabbed = false;

	// Stop pouring
	if (bIsPouring)
	{
		StopPouring();
	}

	// Re-enable physics
	if (CanMesh)
	{
		CanMesh->SetSimulatePhysics(true);
	}

	GrabbingHand = nullptr;

	UE_LOG(LogTemp, Log, TEXT("VRWateringCan: Released"));
}

float AVRWateringCan::GetWaterPercentage() const
{
	if (WaterCapacity <= 0.0f)
	{
		return 0.0f;
	}
	return CurrentWaterAmount / WaterCapacity;
}

void AVRWateringCan::RefillWater()
{
	CurrentWaterAmount = WaterCapacity;
	UpdateVisualFeedback();
	
	UE_LOG(LogTemp, Log, TEXT("VRWateringCan: Refilled to %.2f liters"), CurrentWaterAmount);
}

void AVRWateringCan::AddWater(float Amount)
{
	CurrentWaterAmount = FMath::Clamp(CurrentWaterAmount + Amount, 0.0f, WaterCapacity);
	UpdateVisualFeedback();
}

void AVRWateringCan::UpdatePouringState()
{
	float TiltAngle = CalculateTiltAngle();
	bool bShouldPour = TiltAngle >= MinTiltAngleForPouring && CurrentWaterAmount > 0.0f;

	if (bShouldPour && !bIsPouring)
	{
		StartPouring();
	}
	else if (!bShouldPour && bIsPouring)
	{
		StopPouring();
	}
}

float AVRWateringCan::CalculateTiltAngle() const
{
	// Get the up vector of the spout in world space
	FVector SpoutUp = SpoutLocation->GetUpVector();
	
	// Calculate angle from vertical (world up)
	FVector WorldUp = FVector::UpVector;
	float DotProduct = FVector::DotProduct(SpoutUp, WorldUp);
	float AngleRadians = FMath::Acos(DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	return AngleDegrees;
}

void AVRWateringCan::StartPouring()
{
	bIsPouring = true;

	// Activate water stream effect
	if (WaterStreamEffect)
	{
		WaterStreamEffect->Activate(true);
	}

	UE_LOG(LogTemp, Log, TEXT("VRWateringCan: Started pouring"));
}

void AVRWateringCan::StopPouring()
{
	bIsPouring = false;

	// Deactivate water stream effect
	if (WaterStreamEffect)
	{
		WaterStreamEffect->Deactivate();
	}

	// Clear affected farm plots
	AffectedFarmPlots.Empty();
	WateredCells.Empty();
	bWaterHittingSurface = false;

	UE_LOG(LogTemp, Log, TEXT("VRWateringCan: Stopped pouring"));
}

void AVRWateringCan::UpdateWaterStream(float DeltaTime)
{
	if (!WaterStreamEffect)
	{
		return;
	}

	// Calculate water stream parameters based on tilt angle and water amount
	float TiltAngle = CalculateTiltAngle();
	float TiltFactor = FMath::Clamp((TiltAngle - MinTiltAngleForPouring) / 45.0f, 0.0f, 1.0f);
	
	// Update Niagara parameters
	float StreamIntensity = TiltFactor * GetWaterPercentage();
	WaterStreamEffect->SetFloatParameter(TEXT("StreamIntensity"), StreamIntensity);
	WaterStreamEffect->SetFloatParameter(TEXT("FlowRate"), WaterFlowRate * TiltFactor);
	
	// Update stream velocity based on tilt
	FVector StreamVelocity = SpoutLocation->GetForwardVector() * 300.0f * TiltFactor;
	WaterStreamEffect->SetVectorParameter(TEXT("StreamVelocity"), StreamVelocity);
}

void AVRWateringCan::PerformWateringRaycast()
{
	if (!SpoutLocation)
	{
		return;
	}

	// Perform line trace from spout downward
	FVector StartLocation = SpoutLocation->GetComponentLocation();
	FVector Direction = -FVector::UpVector; // Gravity direction
	
	// Add some forward component based on spout direction
	FVector SpoutForward = SpoutLocation->GetForwardVector();
	Direction = (Direction + SpoutForward * 0.3f).GetSafeNormal();
	
	FVector EndLocation = StartLocation + Direction * MaxWateringDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// Debug visualization
	#if WITH_EDITOR
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 0.1f);
	#endif

	if (bHit)
	{
		bWaterHittingSurface = true;
		WaterHitLocation = HitResult.Location;

		// Check if we hit a farm plot
		AFarmPlot* FarmPlot = Cast<AFarmPlot>(HitResult.GetActor());
		if (!FarmPlot && HitResult.GetComponent())
		{
			FarmPlot = Cast<AFarmPlot>(HitResult.GetComponent()->GetOwner());
		}

		if (FarmPlot)
		{
			AffectedFarmPlots.AddUnique(FarmPlot);
		}
	}
	else
	{
		bWaterHittingSurface = false;
		AffectedFarmPlots.Empty();
	}
}

void AVRWateringCan::ApplyWaterToFarmPlots(float DeltaTime)
{
	if (AffectedFarmPlots.Num() == 0)
	{
		return;
	}

	// Calculate water amount to apply this frame
	float WaterToApply = WaterFlowRate * DeltaTime;

	// Apply water to each affected farm plot
	for (AFarmPlot* FarmPlot : AffectedFarmPlots)
	{
		if (FarmPlot)
		{
			// Water the plot
			FarmPlot->WaterPlot(WaterToApply);

			UE_LOG(LogTemp, Verbose, TEXT("VRWateringCan: Watering farm plot with %.3f liters"), WaterToApply);
		}
	}
}

void AVRWateringCan::UpdateHapticFeedback()
{
	if (!GrabbingHand)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// Only update haptics at specified interval
	if (CurrentTime - LastHapticTime < HapticFeedbackInterval)
	{
		return;
	}

	LastHapticTime = CurrentTime;

	// Calculate haptic intensity based on water flow and surface hit
	float Intensity = 0.3f; // Base intensity for pouring
	
	if (bWaterHittingSurface)
	{
		// Stronger feedback when water hits surface
		Intensity = 0.5f;
	}

	// Modulate by water flow rate
	float TiltAngle = CalculateTiltAngle();
	float TiltFactor = FMath::Clamp((TiltAngle - MinTiltAngleForPouring) / 45.0f, 0.0f, 1.0f);
	Intensity *= TiltFactor;

	// Apply intensity multiplier
	Intensity *= HapticIntensityMultiplier;

	// Play haptic feedback
	PlayWateringHaptics(Intensity);
}

void AVRWateringCan::PlayWateringHaptics(float Intensity)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// Play haptic pulse
		// In a full implementation, this would determine which hand is grabbing
		PC->PlayHapticEffect(
			nullptr, // Haptic effect asset (would be set in Blueprint)
			EControllerHand::Right, // Would be determined by which hand grabbed
			Intensity,
			false // Don't loop
		);
	}
}

void AVRWateringCan::UpdateVisualFeedback()
{
	if (!CanMaterialInstance)
	{
		return;
	}

	// Update water level parameter
	float WaterLevel = GetWaterPercentage();
	CanMaterialInstance->SetScalarParameterValue(TEXT("WaterLevel"), WaterLevel);

	// Update water color based on amount
	FLinearColor WaterColor = FLinearColor::Blue;
	if (WaterLevel < 0.2f)
	{
		// Reddish tint when low on water
		WaterColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);
	}
	CanMaterialInstance->SetVectorParameterValue(TEXT("WaterColor"), WaterColor);

	// Update mass based on water amount
	if (CanMesh)
	{
		float TotalMass = 0.5f + (CurrentWaterAmount * 0.1f); // Base mass + water mass
		CanMesh->SetMassOverrideInKg(NAME_None, TotalMass);
	}
}

void AVRWateringCan::UpdateAudio()
{
	if (!bIsPouring)
	{
		// Stop pouring sound if playing
		if (PouringSoundComponent && PouringSoundComponent->IsPlaying())
		{
			PouringSoundComponent->Stop();
		}
		return;
	}

	// Start or update pouring sound
	if (PouringSound)
	{
		if (!PouringSoundComponent || !PouringSoundComponent->IsPlaying())
		{
			// Create and play pouring sound
			PouringSoundComponent = UGameplayStatics::SpawnSoundAttached(
				PouringSound,
				SpoutLocation,
				NAME_None,
				FVector::ZeroVector,
				EAttachLocation::KeepRelativeOffset,
				false, // Don't stop when owner is destroyed
				1.0f, // Volume
				1.0f, // Pitch
				0.0f, // Start time
				nullptr, // Attenuation settings
				nullptr, // Concurrency settings
				false // Auto destroy
			);
		}

		// Modulate sound based on water flow
		if (PouringSoundComponent)
		{
			float TiltAngle = CalculateTiltAngle();
			float TiltFactor = FMath::Clamp((TiltAngle - MinTiltAngleForPouring) / 45.0f, 0.0f, 1.0f);
			
			// Adjust volume and pitch based on flow
			float Volume = 0.5f + (TiltFactor * 0.5f);
			float Pitch = 0.8f + (TiltFactor * 0.4f);
			
			PouringSoundComponent->SetVolumeMultiplier(Volume);
			PouringSoundComponent->SetPitchMultiplier(Pitch);
		}
	}
}
