// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CropTypes.generated.h"

// Extended crop types (20 total)
UENUM(BlueprintType)
enum class ECropTypeExtended : uint8
{
	// Food Crops (10 types)
	Wheat UMETA(DisplayName = "Wheat"),
	Corn UMETA(DisplayName = "Corn"),
	Tomatoes UMETA(DisplayName = "Tomatoes"),
	Potatoes UMETA(DisplayName = "Potatoes"),
	Carrots UMETA(DisplayName = "Carrots"),
	Lettuce UMETA(DisplayName = "Lettuce"),
	Strawberries UMETA(DisplayName = "Strawberries"),
	Rice UMETA(DisplayName = "Rice"),
	Beans UMETA(DisplayName = "Beans"),
	Onions UMETA(DisplayName = "Onions"),

	// Medical Crops (4 types)
	MedicalHerbs UMETA(DisplayName = "Medical Herbs"),
	PharmaceuticalPlants UMETA(DisplayName = "Pharmaceutical Plants"),
	AloeVera UMETA(DisplayName = "Aloe Vera"),
	Lavender UMETA(DisplayName = "Lavender"),

	// Industrial Crops (4 types)
	Cotton UMETA(DisplayName = "Cotton"),
	BiofuelCrops UMETA(DisplayName = "Biofuel Crops"),
	RubberPlants UMETA(DisplayName = "Rubber Plants"),
	Bamboo UMETA(DisplayName = "Bamboo"),

	// Illegal/Special Crops (2 types)
	SpaceWeed UMETA(DisplayName = "Space Weed"),
	QuantumPlants UMETA(DisplayName = "Quantum Plants")
};