// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Asteroid.generated.h"

/**
 * Asteroid - Most flexible orbital body
 * Can orbit any other body (Sun, Planet, Moon, or even other Asteroids)
 * Supports fractal depth: Asteroid -> Sub-asteroid -> Sub-sub-asteroid...
 */
UCLASS(Blueprintable)
class ALEXANDER_API AAsteroid : public AOrbitalBody
{
	GENERATED_BODY()

public:
	AAsteroid();

protected:
	virtual void BeginPlay() override;

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* AsteroidMesh;

	// Visual Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Properties")
	float AsteroidScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Properties")
	FLinearColor AsteroidColor;

	// Shape variation (0 = smooth sphere, 1 = very irregular)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asteroid Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShapeIrregularity;
};
