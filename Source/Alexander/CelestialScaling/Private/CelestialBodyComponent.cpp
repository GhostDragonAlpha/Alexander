// Copyright Epic Games, Inc. All Rights Reserved.

#include "CelestialBodyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CelestialBodyRegistry.h"

UCelestialBodyComponent::UCelestialBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	Mass = 5.972e24f;
	Radius = 6371.0f;
	CurrentScaleFactor = 1.0f;
	TargetScaleFactor = 1.0f;
	DistanceToPlayer = 0.0f;

	bEnableDynamicScaling = true;
	ScaleTransitionSpeed = 0.1f;
	MinScaleFactor = 0.001f;
	MaxScaleFactor = 1000.0f;

	bEnableGravity = true;
	GravityMultiplier = 1.0f;

	CurrentLODLevel = 0;
	bShowDebugInfo = false;
	bIsRegistered = false;

	BodyID = NAME_None;
	BodyType = TEXT("Planet");
	VisualMesh = nullptr;
	OriginalScale = FVector::OneVector;
}

void UCelestialBodyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCelestialBodyComponent, Mass);
	DOREPLIFETIME(UCelestialBodyComponent, Radius);
	DOREPLIFETIME(UCelestialBodyComponent, CurrentScaleFactor);
	DOREPLIFETIME(UCelestialBodyComponent, TargetScaleFactor);
	DOREPLIFETIME(UCelestialBodyComponent, DistanceToPlayer);
}

void UCelestialBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheVisualMeshComponent();

	if (AActor* Owner = GetOwner())
	{
		OriginalScale = Owner->GetActorScale3D();
	}

	RegisterWithSubsystem();
}

void UCelestialBodyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromSubsystem();
	Super::EndPlay(EndPlayReason);
}

void UCelestialBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableDynamicScaling && FMath::Abs(CurrentScaleFactor - TargetScaleFactor) > KINDA_SMALL_NUMBER)
	{
		SmoothScaleTransition(DeltaTime);
	}

	if (bShowDebugInfo)
	{
		DrawDebugVisualization();
	}
}

void UCelestialBodyComponent::UpdateScale(const FVector& PlayerPosition)
{
	if (!bEnableDynamicScaling) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector BodyPosition = Owner->GetActorLocation();
	float DistanceCm = FVector::Dist(BodyPosition, PlayerPosition);
	DistanceToPlayer = DistanceCm / KilometersToCm;

	TargetScaleFactor = CalculateScaleFactorForDistance(DistanceToPlayer);
	TargetScaleFactor = FMath::Clamp(TargetScaleFactor, MinScaleFactor, MaxScaleFactor);
}

void UCelestialBodyComponent::ApplyScaleImmediate(float NewScale)
{
	NewScale = FMath::Clamp(NewScale, MinScaleFactor, MaxScaleFactor);
	CurrentScaleFactor = NewScale;
	TargetScaleFactor = NewScale;
	ApplyScaleToActor(NewScale);
}

void UCelestialBodyComponent::SmoothScaleTransition(float DeltaTime)
{
	float InterpSpeed = ScaleTransitionSpeed * 10.0f;
	CurrentScaleFactor = FMath::FInterpTo(CurrentScaleFactor, TargetScaleFactor, DeltaTime, InterpSpeed);
	ApplyScaleToActor(CurrentScaleFactor);
}

FVector UCelestialBodyComponent::CalculateGravitationalForce(const FVector& TargetPosition, float TargetMass) const
{
	if (!bEnableGravity || Mass <= 0.0f || TargetMass <= 0.0f) return FVector::ZeroVector;

	AActor* Owner = GetOwner();
	if (!Owner) return FVector::ZeroVector;

	FVector Direction = Owner->GetActorLocation() - TargetPosition;
	float DistanceCm = Direction.Size();
	if (DistanceCm < KINDA_SMALL_NUMBER) return FVector::ZeroVector;

	Direction.Normalize();
	float DistanceMeters = DistanceCm / 100.0f;

	double ForceMagnitude = (GravitationalConstant * Mass * TargetMass) / (DistanceMeters * DistanceMeters);
	ForceMagnitude *= GravityMultiplier;

	return Direction * static_cast<float>(ForceMagnitude);
}

FVector UCelestialBodyComponent::CalculateGravitationalAcceleration(const FVector& Position) const
{
	if (!bEnableGravity || Mass <= 0.0f) return FVector::ZeroVector;

	AActor* Owner = GetOwner();
	if (!Owner) return FVector::ZeroVector;

	FVector Direction = Owner->GetActorLocation() - Position;
	float DistanceCm = Direction.Size();
	if (DistanceCm < KINDA_SMALL_NUMBER) return FVector::ZeroVector;

	Direction.Normalize();
	float DistanceMeters = DistanceCm / 100.0f;

	double AccelerationMagnitude = (GravitationalConstant * Mass) / (DistanceMeters * DistanceMeters);
	AccelerationMagnitude *= GravityMultiplier;

	return Direction * static_cast<float>(AccelerationMagnitude);
}

void UCelestialBodyComponent::UpdateLODSystem()
{
	if (DistanceToPlayer < 1000.0f) CurrentLODLevel = 0;
	else if (DistanceToPlayer < 10000.0f) CurrentLODLevel = 1;
	else if (DistanceToPlayer < 100000.0f) CurrentLODLevel = 2;
	else CurrentLODLevel = 3;
}

void UCelestialBodyComponent::ApplyPositionOffset(const FVector& Offset)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->AddActorWorldOffset(Offset, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UCelestialBodyComponent::UpdateScaleForDistance(float Distance)
{
	DistanceToPlayer = Distance;
	TargetScaleFactor = CalculateScaleFactorForDistance(Distance);
	TargetScaleFactor = FMath::Clamp(TargetScaleFactor, MinScaleFactor, MaxScaleFactor);
}

void UCelestialBodyComponent::DrawDebugVisualization()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector BodyPosition = Owner->GetActorLocation();
	float DebugRadius = Radius * 100.0f * CurrentScaleFactor;
	DrawDebugSphere(World, BodyPosition, DebugRadius, 16, FColor::Cyan, false, -1.0f, 0, 2.0f);

	FString DebugText = FString::Printf(TEXT("%s\nScale: %.3f\nDist: %.0f km"), *BodyID.ToString(), CurrentScaleFactor, DistanceToPlayer);
	DrawDebugString(World, BodyPosition + FVector(0, 0, DebugRadius + 100.0f), DebugText, nullptr, FColor::White, 0.0f, true);
}

FString UCelestialBodyComponent::GetStatusInfo() const
{
	return FString::Printf(TEXT("CelestialBody: %s (%s)\nMass: %.2e kg\nRadius: %.2f km\nScale: %.3f\nDistance: %.2f km"),
		*BodyID.ToString(), *BodyType, Mass, Radius, CurrentScaleFactor, DistanceToPlayer);
}

void UCelestialBodyComponent::RegisterWithSubsystem()
{
	UCelestialBodyRegistry* Registry = GetWorld()->GetSubsystem<UCelestialBodyRegistry>();
	if (Registry)
	{
		Registry->RegisterCelestialBody(this);
		bIsRegistered = true;
	}
}

void UCelestialBodyComponent::UnregisterFromSubsystem()
{
	if (!bIsRegistered) return;

	UCelestialBodyRegistry* Registry = GetWorld()->GetSubsystem<UCelestialBodyRegistry>();
	if (Registry)
	{
		Registry->UnregisterCelestialBody(this);
	}
	bIsRegistered = false;
}

void UCelestialBodyComponent::CacheVisualMeshComponent()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		VisualMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
	}
}

float UCelestialBodyComponent::CalculateScaleFactorForDistance(float Distance) const
{
	if (Distance < 1.0f) Distance = 1.0f;
	float ScaleFactor = FMath::LogX(10.0f, Distance) / 10.0f;
	return FMath::Clamp(ScaleFactor, MinScaleFactor, MaxScaleFactor);
}

void UCelestialBodyComponent::ApplyScaleToActor(float Scale)
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector NewScale = OriginalScale * Scale;
		Owner->SetActorScale3D(NewScale);
	}
}
