// Copyright Epic Games, Inc. All Rights Reserved.

#include "Moon.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AMoon::AMoon()
{
	PrimaryActorTick.bCanEverTick = true;

	// Moons orbit Planets by default
	OrbitMode = EOrbitMode::Orbit;
	Mass = 7.342e22f; // Mass of Earth's Moon in kg (default)
	OrbitRadius = 3000.0f; // Default orbit distance (closer than planets)
	OrbitSpeed = 20.0f; // Faster orbital speed than planets
	OrbitInclination = 5.0f; // Slight inclination (realistic for moons)

	// Create root scene component
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create mesh component
	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonMesh"));
	MoonMesh->SetupAttachment(RootComponent);

	// Load sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MoonMesh->SetStaticMesh(SphereMesh.Object);
	}

	// Default visual properties (smaller than planets)
	MoonScale = 0.2f;
	MoonColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Default gray
}

void AMoon::BeginPlay()
{
	Super::BeginPlay();

	// Apply scale
	MoonMesh->SetWorldScale3D(FVector(MoonScale));

	// Apply color to material
	if (MoonMesh->GetStaticMesh())
	{
		UMaterialInterface* BaseMaterial = MoonMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (DynMaterial)
			{
				DynMaterial->SetVectorParameterValue(FName("Color"), MoonColor);
				MoonMesh->SetMaterial(0, DynMaterial);
			}
		}
	}
}
