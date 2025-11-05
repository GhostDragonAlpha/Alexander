// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "PlanetFarmingComponent.generated.h"

// Forward declaration - AAlexanderFarmPlot will be implemented in farming tasks
UCLASS()
class AAlexanderFarmPlot : public AActor
{
	GENERATED_BODY()
};

/**
 * Component for managing farming systems on the planet
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetFarmingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetFarmingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Active farm plots on this planet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	TArray<AAlexanderFarmPlot*> ActiveFarms;

	// Create new farm plot at location
	UFUNCTION(BlueprintCallable, Category = "Farming")
	AAlexanderFarmPlot* CreateFarmPlot(FVector Location, FVector2D Size);

	// Check if location is suitable for farming
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool IsSuitableForFarming(FVector Location) const;

	// Update all active farms
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdateFarms(float DeltaTime);

	// Get base soil quality at location (biome-dependent)
	UFUNCTION(BlueprintCallable, Category = "Farming")
	float GetBaseSoilQuality(FVector Location) const;

	// Initialize farming system for this planet
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void InitializeFarming(class APlanet* Planet);

private:
	UPROPERTY()
	class APlanet* OwningPlanet;
};
