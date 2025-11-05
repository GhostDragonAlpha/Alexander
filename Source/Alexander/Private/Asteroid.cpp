// Copyright Epic Games, Inc. All Rights Reserved.

#include "Asteroid.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/Vector.h"

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	// Asteroids can orbit anything - OrbitTarget set manually in editor
	// Default to ballistic mode (tumbling through space)
	OrbitMode = EOrbitMode::Ballistic;
	Mass = 1.0e12f; // 1 trillion kg (small asteroid default)
	OrbitRadius = 15000.0f; // Variable orbit distance
	OrbitSpeed = 30.0f; // Fast, erratic orbits
	OrbitInclination = 15.0f; // Often highly inclined
	Velocity = FVector::ZeroVector; // Start stationary, will be affected by gravity

	// Create root scene component
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create mesh component
	AsteroidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	AsteroidMesh->SetupAttachment(RootComponent);

	// Load sphere mesh (in a real implementation, would use irregular asteroid meshes)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		AsteroidMesh->SetStaticMesh(SphereMesh.Object);
	}

	// Default visual properties (small, rocky)
	AsteroidScale = 0.1f;
	AsteroidColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Default brown/rocky
	ShapeIrregularity = 0.5f; // Medium irregularity
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	// Apply scale
	AsteroidMesh->SetWorldScale3D(FVector(AsteroidScale));

	// Apply color to material
	if (AsteroidMesh->GetStaticMesh())
	{
		UMaterialInterface* BaseMaterial = AsteroidMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (DynMaterial)
			{
				DynMaterial->SetVectorParameterValue(FName("Color"), AsteroidColor);
				// Could add ShapeIrregularity parameter to material here
				DynMaterial->SetScalarParameterValue(FName("Roughness"), 0.9f); // Rocky surface
				AsteroidMesh->SetMaterial(0, DynMaterial);
			}
		}
	}

	// If in ballistic mode with an orbit target, start with some initial velocity
	// This simulates the asteroid tumbling through space
	if (OrbitMode == EOrbitMode::Ballistic && OrbitTarget.IsValid())
	{
		// Give it a small random velocity perpendicular to the direction to target
		FVector ToTarget = (OrbitTarget.IsValid() ? OrbitTarget->GetActorLocation() : FVector::ZeroVector - GetActorLocation()).GetSafeNormal();
		FVector Perpendicular = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
		Velocity = Perpendicular * 100.0f; // Small initial velocity
	}
}
