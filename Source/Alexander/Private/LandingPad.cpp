// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandingPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "NavigationMarkerComponent.h"
#include "NavigationMarkerSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ALandingPad::ALandingPad()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create pad mesh
	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	PadMesh->SetupAttachment(RootComponent);
	PadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PadMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	PadMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	// Create landing zone collision
	LandingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("LandingZone"));
	LandingZone->SetupAttachment(RootComponent);
	LandingZone->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f)); // 20m x 20m x 10m
	LandingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LandingZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	LandingZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	LandingZone->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f)); // Center above pad

	// Create approach vector indicator
	ApproachVector = CreateDefaultSubobject<UArrowComponent>(TEXT("ApproachVector"));
	ApproachVector->SetupAttachment(RootComponent);
	ApproachVector->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	ApproachVector->SetArrowColor(FLinearColor::Green);
	ApproachVector->SetArrowSize(2.0f);
	ApproachVector->SetHiddenInGame(false);

	// Create navigation marker
	NavigationMarker = CreateDefaultSubobject<UNavigationMarkerComponent>(TEXT("NavigationMarker"));
	NavigationMarker->SetupAttachment(RootComponent);
	NavigationMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f)); // 5m above pad
	NavigationMarker->MarkerType = EMarkerType::LandingPad;
	NavigationMarker->VisibilityMode = EMarkerVisibility::Always;
	NavigationMarker->MarkerColor = FLinearColor::Green;
	NavigationMarker->MinVisibilityDistance = 100.0f * 100.0f; // 100m
	NavigationMarker->MaxVisibilityDistance = 100000.0f * 100.0f; // 100km
	NavigationMarker->OrbitalAltitudeThreshold = 100000.0f * 100.0f; // 100km
	NavigationMarker->AtmosphericAltitudeRange = FVector2D(10000.0f * 100.0f, 100000.0f * 100.0f); // 10-100km

	// Set default values
	PadStatus = ELandingPadStatus::Available;
	ReservedBy = nullptr;
	ReservationExpireTime = 0.0f;
}

void ALandingPad::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	if (LandingZone)
	{
		LandingZone->OnComponentBeginOverlap.AddDynamic(this, &ALandingPad::OnLandingZoneBeginOverlap);
		LandingZone->OnComponentEndOverlap.AddDynamic(this, &ALandingPad::OnLandingZoneEndOverlap);
	}

	// Flatten terrain if requested
	if (bFlattenTerrain)
	{
		FlattenTerrainUnderPad();
	}

	// Check terrain suitability
	float AverageSlope = 0.0f;
	if (!IsTerrainSuitable(AverageSlope))
	{
		UE_LOG(LogTemp, Warning, TEXT("Landing Pad '%s' placed on unsuitable terrain (slope: %.1f degrees)"),
			*PadName, AverageSlope);
	}

	// Update initial visuals
	UpdateVisuals();

	// Register navigation marker with subsystem
	if (NavigationMarker)
	{
		NavigationMarker->MarkerName = PadName;
		
		UWorld* World = GetWorld();
		if (World)
		{
			UNavigationMarkerSubsystem* MarkerSubsystem = World->GetSubsystem<UNavigationMarkerSubsystem>();
			if (MarkerSubsystem)
			{
				MarkerSubsystem->RegisterMarker(NavigationMarker);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' (ID: %d) initialized at %s"),
		*PadName, PadID, *GetActorLocation().ToString());
}

void ALandingPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update reservation timer
	UpdateReservation(DeltaTime);

	// Update visuals if status changed
	UpdateVisuals();

	// Debug visualization
	if (bShowApproachMarkers)
	{
		FVector ApproachPos = GetApproachPosition();
		DrawDebugSphere(GetWorld(), ApproachPos, 50.0f, 12, GetStatusColor().ToFColor(true), false, -1.0f, 0, 2.0f);
		DrawDebugLine(GetWorld(), ApproachPos, GetActorLocation(), GetStatusColor().ToFColor(true), false, -1.0f, 0, 2.0f);
	}
}

// ============================================================================
// RESERVATION SYSTEM
// ============================================================================

bool ALandingPad::ReservePad(AActor* Actor, float Duration)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("LandingPad::ReservePad - Invalid actor"));
		return false;
	}

	// Check if already reserved or occupied
	if (PadStatus == ELandingPadStatus::Reserved || PadStatus == ELandingPadStatus::Occupied)
	{
		if (ReservedBy == Actor)
		{
			// Extend reservation
			float ReserveDuration = Duration > 0.0f ? Duration : DefaultReservationDuration;
			ReservationExpireTime = GetWorld()->GetTimeSeconds() + ReserveDuration;
			UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' reservation extended for %s"),
				*PadName, *Actor->GetName());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Landing Pad '%s' already reserved by %s"),
				*PadName, ReservedBy ? *ReservedBy->GetName() : TEXT("Unknown"));
			return false;
		}
	}

	// Check if disabled
	if (PadStatus == ELandingPadStatus::Disabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landing Pad '%s' is disabled"), *PadName);
		return false;
	}

	// Reserve the pad
	PadStatus = ELandingPadStatus::Reserved;
	ReservedBy = Actor;
	float ReserveDuration = Duration > 0.0f ? Duration : DefaultReservationDuration;
	ReservationExpireTime = GetWorld()->GetTimeSeconds() + ReserveDuration;

	UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' reserved by %s for %.1f seconds"),
		*PadName, *Actor->GetName(), ReserveDuration);

	// Trigger blueprint event
	OnPadReserved(Actor);

	return true;
}

bool ALandingPad::ReleasePad(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("LandingPad::ReleasePad - Invalid actor"));
		return false;
	}

	// Check if this actor has the reservation
	if (ReservedBy != Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landing Pad '%s' - Actor %s does not have reservation"),
			*PadName, *Actor->GetName());
		return false;
	}

	// Release the pad
	PadStatus = ELandingPadStatus::Available;
	ReservedBy = nullptr;
	ReservationExpireTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' released by %s"), *PadName, *Actor->GetName());

	// Trigger blueprint event
	OnPadReleased(Actor);

	return true;
}

bool ALandingPad::IsAvailable() const
{
	return PadStatus == ELandingPadStatus::Available;
}

void ALandingPad::ForceRelease()
{
	AActor* PreviousReserver = ReservedBy;
	
	PadStatus = ELandingPadStatus::Available;
	ReservedBy = nullptr;
	ReservationExpireTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' force released"), *PadName);

	if (PreviousReserver)
	{
		OnPadReleased(PreviousReserver);
	}
}

void ALandingPad::UpdateReservation(float DeltaTime)
{
	// Check if reservation has expired
	if (PadStatus == ELandingPadStatus::Reserved && ReservationExpireTime > 0.0f)
	{
		if (GetWorld()->GetTimeSeconds() >= ReservationExpireTime)
		{
			UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' reservation expired"), *PadName);
			ForceRelease();
		}
	}
}

// ============================================================================
// APPROACH GUIDANCE
// ============================================================================

FVector ALandingPad::GetApproachPosition() const
{
	FVector PadLocation = GetActorLocation();
	FRotator PadRotation = GetActorRotation();

	// Calculate approach position based on approach angle and distance
	FRotator ApproachRotation = PadRotation;
	ApproachRotation.Yaw += ApproachAngle;

	FVector ApproachDirection = ApproachRotation.Vector();
	FVector ApproachOffset = ApproachDirection * ApproachDistance;
	ApproachOffset.Z = ApproachAltitude * 100.0f; // Convert meters to cm

	return PadLocation + ApproachOffset;
}

FVector ALandingPad::GetApproachDirection() const
{
	FVector ApproachPos = GetApproachPosition();
	FVector PadLocation = GetActorLocation();
	
	FVector Direction = (PadLocation - ApproachPos).GetSafeNormal();
	return Direction;
}

bool ALandingPad::IsInApproachCorridor(AActor* Actor, float& OutDistance, float& OutAlignment) const
{
	if (!Actor)
	{
		OutDistance = 0.0f;
		OutAlignment = 0.0f;
		return false;
	}

	FVector ActorLocation = Actor->GetActorLocation();
	FVector IdealApproachPos = GetApproachPosition();
	FVector IdealApproachDir = GetApproachDirection();

	// Calculate distance from ideal approach position
	OutDistance = FVector::Dist(ActorLocation, IdealApproachPos);

	// Calculate alignment with approach vector
	FVector ActorToPad = (GetActorLocation() - ActorLocation).GetSafeNormal();
	OutAlignment = FVector::DotProduct(ActorToPad, IdealApproachDir);
	OutAlignment = FMath::Clamp((OutAlignment + 1.0f) * 0.5f, 0.0f, 1.0f); // Normalize to 0-1

	// Check if within acceptable parameters
	const float MaxApproachDeviation = 100.0f * 100.0f; // 100 meters in cm
	const float MinAlignment = 0.7f; // 70% alignment required

	return (OutDistance <= MaxApproachDeviation) && (OutAlignment >= MinAlignment);
}

// ============================================================================
// TERRAIN INTEGRATION
// ============================================================================

void ALandingPad::FlattenTerrainUnderPad()
{
	// This is a placeholder for terrain flattening
	// In a full implementation, this would:
	// 1. Get reference to the planet's terrain system
	// 2. Calculate the area to flatten based on FlattenRadius
	// 3. Modify the heightmap to create a flat surface
	// 4. Blend the edges using FlattenBlendDistance
	// 5. Update the terrain mesh

	UE_LOG(LogTemp, Log, TEXT("Landing Pad '%s' - Terrain flattening requested (radius: %.1fm, blend: %.1fm)"),
		*PadName, FlattenRadius, FlattenBlendDistance);

	// For now, we'll just ensure the pad is positioned correctly
	// The actual terrain modification would be implemented when the terrain system is complete
}

bool ALandingPad::IsTerrainSuitable(float& OutAverageSlope) const
{
	// Sample terrain at multiple points around the pad
	const int32 NumSamples = 8;
	const float SampleRadius = PadSize.X * 50.0f; // Convert to cm
	
	TArray<FVector> SamplePoints;
	TArray<float> Heights;

	FVector PadLocation = GetActorLocation();

	// Sample in a circle around the pad
	for (int32 i = 0; i < NumSamples; i++)
	{
		float Angle = (2.0f * PI * i) / NumSamples;
		FVector Offset(FMath::Cos(Angle) * SampleRadius, FMath::Sin(Angle) * SampleRadius, 0.0f);
		FVector SamplePoint = PadLocation + Offset;

		// Trace down to find terrain height
		FHitResult HitResult;
		FVector TraceStart = SamplePoint + FVector(0.0f, 0.0f, 1000.0f);
		FVector TraceEnd = SamplePoint - FVector(0.0f, 0.0f, 10000.0f);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			Heights.Add(HitResult.Location.Z);
			SamplePoints.Add(HitResult.Location);
		}
	}

	// Calculate average slope
	if (Heights.Num() < 3)
	{
		OutAverageSlope = 0.0f;
		return true; // Can't determine, assume suitable
	}

	// Find min and max heights
	float MinHeight = Heights[0];
	float MaxHeight = Heights[0];
	for (float Height : Heights)
	{
		MinHeight = FMath::Min(MinHeight, Height);
		MaxHeight = FMath::Max(MaxHeight, Height);
	}

	// Calculate slope angle
	float HeightDifference = MaxHeight - MinHeight;
	float SlopeRadians = FMath::Atan2(HeightDifference, SampleRadius);
	OutAverageSlope = FMath::RadiansToDegrees(SlopeRadians);

	return OutAverageSlope <= MaxSlopeAngle;
}

// ============================================================================
// VISUAL MARKERS
// ============================================================================

void ALandingPad::UpdateVisuals()
{
	FLinearColor StatusColor = GetStatusColor();

	// Update approach vector color
	if (ApproachVector)
	{
		ApproachVector->SetArrowColor(StatusColor);
		ApproachVector->SetHiddenInGame(!bShowApproachMarkers);
	}

	// Update navigation marker color
	if (NavigationMarker)
	{
		NavigationMarker->SetMarkerColor(StatusColor);
	}

	// Update pad material if it has an emissive parameter
	if (PadMesh && bShowStatusLights)
	{
		UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(PadMesh->GetMaterial(0));
		if (!DynMaterial)
		{
			UMaterialInterface* BaseMaterial = PadMesh->GetMaterial(0);
			if (BaseMaterial)
			{
				DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				PadMesh->SetMaterial(0, DynMaterial);
			}
		}

		if (DynMaterial)
		{
			DynMaterial->SetVectorParameterValue(FName("StatusColor"), StatusColor);
			DynMaterial->SetVectorParameterValue(FName("EmissiveColor"), StatusColor);
		}
	}
}

FLinearColor ALandingPad::GetStatusColor() const
{
	switch (PadStatus)
	{
	case ELandingPadStatus::Available:
		return AvailableColor;
	case ELandingPadStatus::Reserved:
		return ReservedColor;
	case ELandingPadStatus::Occupied:
		return OccupiedColor;
	case ELandingPadStatus::Disabled:
		return DisabledColor;
	default:
		return FLinearColor::White;
	}
}

// ============================================================================
// EVENTS
// ============================================================================

void ALandingPad::OnLandingZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidShip(OtherActor))
	{
		return;
	}

	// Check if this ship has a reservation
	if (ReservedBy == OtherActor)
	{
		// Ship is landing
		PadStatus = ELandingPadStatus::Occupied;
		UE_LOG(LogTemp, Log, TEXT("Ship %s landed on pad '%s'"), *OtherActor->GetName(), *PadName);
		
		// Trigger blueprint event
		OnShipLanded(OtherActor);
	}
	else if (PadStatus == ELandingPadStatus::Available)
	{
		// Auto-reserve for landing ship
		ReservePad(OtherActor, DefaultReservationDuration);
		PadStatus = ELandingPadStatus::Occupied;
		UE_LOG(LogTemp, Log, TEXT("Ship %s landed on available pad '%s'"), *OtherActor->GetName(), *PadName);
		
		// Trigger blueprint event
		OnShipLanded(OtherActor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ship %s attempted to land on unavailable pad '%s'"),
			*OtherActor->GetName(), *PadName);
	}
}

void ALandingPad::OnLandingZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidShip(OtherActor))
	{
		return;
	}

	// Check if this is the ship that was occupying the pad
	if (ReservedBy == OtherActor && PadStatus == ELandingPadStatus::Occupied)
	{
		// Ship is taking off
		ReleasePad(OtherActor);
		UE_LOG(LogTemp, Log, TEXT("Ship %s departed from pad '%s'"), *OtherActor->GetName(), *PadName);
		
		// Trigger blueprint event
		OnShipDeparted(OtherActor);
	}
}

bool ALandingPad::IsValidShip(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Check if actor is a pawn (player ship) or has specific ship tag
	// This is a simple check - could be expanded with ship interface or component
	return Actor->IsA(APawn::StaticClass()) || Actor->ActorHasTag(FName("Ship"));
}
