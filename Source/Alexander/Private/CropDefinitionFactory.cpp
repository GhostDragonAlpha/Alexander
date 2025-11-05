// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropDefinitionFactory.h"

TArray<UCropDefinition*> UCropDefinitionFactory::CreateAllCropDefinitions(UObject* Outer)
{
	TArray<UCropDefinition*> Crops;

	Crops.Add(CreateWheat(Outer));
	Crops.Add(CreateCorn(Outer));
	Crops.Add(CreateRice(Outer));
	Crops.Add(CreateTomatoes(Outer));
	Crops.Add(CreatePotatoes(Outer));
	Crops.Add(CreateCarrots(Outer));
	Crops.Add(CreateSoybeans(Outer));
	Crops.Add(CreateBeans(Outer));
	Crops.Add(CreateCotton(Outer));
	Crops.Add(CreateCoffee(Outer));
	Crops.Add(CreateCacao(Outer));
	Crops.Add(CreateBarley(Outer));
	Crops.Add(CreateTurnips(Outer));

	return Crops;
}

UCropDefinition* UCropDefinitionFactory::CreateCropByName(const FString& CropName, UObject* Outer)
{
	if (CropName.Equals(TEXT("Wheat"), ESearchCase::IgnoreCase))
		return CreateWheat(Outer);
	if (CropName.Equals(TEXT("Corn"), ESearchCase::IgnoreCase))
		return CreateCorn(Outer);
	if (CropName.Equals(TEXT("Rice"), ESearchCase::IgnoreCase))
		return CreateRice(Outer);
	if (CropName.Equals(TEXT("Tomatoes"), ESearchCase::IgnoreCase))
		return CreateTomatoes(Outer);
	if (CropName.Equals(TEXT("Potatoes"), ESearchCase::IgnoreCase))
		return CreatePotatoes(Outer);
	if (CropName.Equals(TEXT("Carrots"), ESearchCase::IgnoreCase))
		return CreateCarrots(Outer);
	if (CropName.Equals(TEXT("Soybeans"), ESearchCase::IgnoreCase))
		return CreateSoybeans(Outer);
	if (CropName.Equals(TEXT("Beans"), ESearchCase::IgnoreCase))
		return CreateBeans(Outer);
	if (CropName.Equals(TEXT("Cotton"), ESearchCase::IgnoreCase))
		return CreateCotton(Outer);
	if (CropName.Equals(TEXT("Coffee"), ESearchCase::IgnoreCase))
		return CreateCoffee(Outer);
	if (CropName.Equals(TEXT("Cacao"), ESearchCase::IgnoreCase))
		return CreateCacao(Outer);
	if (CropName.Equals(TEXT("Barley"), ESearchCase::IgnoreCase))
		return CreateBarley(Outer);
	if (CropName.Equals(TEXT("Turnips"), ESearchCase::IgnoreCase))
		return CreateTurnips(Outer);

	return nullptr;
}

void UCropDefinitionFactory::SetupCropBase(UCropDefinition* Crop, UObject* Outer)
{
	if (!Crop)
	{
		return;
	}

	// Base setup is handled by UCropDefinition constructor
	// This function can be extended for common initialization
}

UCropDefinition* UCropDefinitionFactory::CreateWheat(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Wheat");
	Crop->CropDescription = FText::FromString(TEXT("A versatile staple grain crop that grows in most temperate climates. Essential for food production."));
	Crop->HarvestItemID = TEXT("wheat");

	// Growth Requirements
	Crop->GrowthDuration = 900.0f; // 15 minutes
	Crop->OptimalTemperature = FCropFloatRange(15.0f, 25.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.4f, 0.6f);
	Crop->WaterConsumption = 15.0f;
	Crop->SoilQualityRequirement = 0.5f;
	Crop->SunlightRequirement = 0.8f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Desert);
	Crop->SuitableBiomes.Add(EBiomeType::Tundra);
	Crop->BiomeAdaptability = 0.6f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(8, 15);
	Crop->SellPrice = 12.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = true;
	Crop->FertilityDepletion = 0.08f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateCorn(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Corn");
	Crop->CropDescription = FText::FromString(TEXT("High-yield grain crop that thrives in warm, sunny conditions. Excellent for food and biofuel."));
	Crop->HarvestItemID = TEXT("corn");

	// Growth Requirements
	Crop->GrowthDuration = 1200.0f; // 20 minutes
	Crop->OptimalTemperature = FCropFloatRange(20.0f, 30.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 25.0f;
	Crop->SoilQualityRequirement = 0.6f;
	Crop->SunlightRequirement = 0.9f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.4f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(10, 20);
	Crop->SellPrice = 15.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = true;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.12f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateRice(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Rice");
	Crop->CropDescription = FText::FromString(TEXT("Water-intensive grain crop requiring flooded fields. Staple food for many cultures."));
	Crop->HarvestItemID = TEXT("rice");

	// Growth Requirements
	Crop->GrowthDuration = 1500.0f; // 25 minutes
	Crop->OptimalTemperature = FCropFloatRange(22.0f, 32.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.7f, 0.9f);
	Crop->WaterConsumption = 50.0f;
	Crop->SoilQualityRequirement = 0.5f;
	Crop->SunlightRequirement = 0.8f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Ocean);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.3f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(12, 18);
	Crop->SellPrice = 18.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.06f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateTomatoes(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Tomatoes");
	Crop->CropDescription = FText::FromString(TEXT("Versatile fruit vegetable with high nutritional value. Grows well in moderate climates."));
	Crop->HarvestItemID = TEXT("tomatoes");

	// Growth Requirements
	Crop->GrowthDuration = 600.0f; // 10 minutes
	Crop->OptimalTemperature = FCropFloatRange(18.0f, 26.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 20.0f;
	Crop->SoilQualityRequirement = 0.6f;
	Crop->SunlightRequirement = 0.9f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.5f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(15, 25);
	Crop->SellPrice = 8.0f;
	Crop->RegrowthChance = 0.3f; // 30% regrowth

	// Special Properties
	Crop->bRequiresFertilizer = true;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.10f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreatePotatoes(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Potatoes");
	Crop->CropDescription = FText::FromString(TEXT("Hardy root vegetable that grows in various climates. Reliable food source."));
	Crop->HarvestItemID = TEXT("potatoes");

	// Growth Requirements
	Crop->GrowthDuration = 800.0f; // 13.3 minutes
	Crop->OptimalTemperature = FCropFloatRange(12.0f, 22.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.4f, 0.6f);
	Crop->WaterConsumption = 18.0f;
	Crop->SoilQualityRequirement = 0.4f;
	Crop->SunlightRequirement = 0.6f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Tundra);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.7f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(10, 18);
	Crop->SellPrice = 10.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = true;
	Crop->FertilityDepletion = 0.07f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateCarrots(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Carrots");
	Crop->CropDescription = FText::FromString(TEXT("Fast-growing root crop with excellent storage properties. Easy to cultivate."));
	Crop->HarvestItemID = TEXT("carrots");

	// Growth Requirements
	Crop->GrowthDuration = 480.0f; // 8 minutes
	Crop->OptimalTemperature = FCropFloatRange(15.0f, 20.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 12.0f;
	Crop->SoilQualityRequirement = 0.5f;
	Crop->SunlightRequirement = 0.7f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->SuitableBiomes.Add(EBiomeType::Tundra);
	Crop->BiomeAdaptability = 0.6f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(12, 20);
	Crop->SellPrice = 6.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = true;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.05f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateSoybeans(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Soybeans");
	Crop->CropDescription = FText::FromString(TEXT("Nitrogen-fixing legume that improves soil quality. High protein content."));
	Crop->HarvestItemID = TEXT("soybeans");

	// Growth Requirements
	Crop->GrowthDuration = 1000.0f; // 16.7 minutes
	Crop->OptimalTemperature = FCropFloatRange(20.0f, 30.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 22.0f;
	Crop->SoilQualityRequirement = 0.4f;
	Crop->SunlightRequirement = 0.8f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.5f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(8, 14);
	Crop->SellPrice = 20.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = -0.05f; // Improves soil!

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateBeans(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Beans");
	Crop->CropDescription = FText::FromString(TEXT("Quick-growing legume with multiple harvests. Good protein source."));
	Crop->HarvestItemID = TEXT("beans");

	// Growth Requirements
	Crop->GrowthDuration = 540.0f; // 9 minutes
	Crop->OptimalTemperature = FCropFloatRange(18.0f, 28.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 18.0f;
	Crop->SoilQualityRequirement = 0.4f;
	Crop->SunlightRequirement = 0.8f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->SuitableBiomes.Add(EBiomeType::Desert);
	Crop->BiomeAdaptability = 0.6f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(10, 16);
	Crop->SellPrice = 14.0f;
	Crop->RegrowthChance = 0.4f; // 40% regrowth

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = -0.03f; // Improves soil!

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateCotton(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Cotton");
	Crop->CropDescription = FText::FromString(TEXT("Fiber crop for textile production. Requires warm climate and good soil."));
	Crop->HarvestItemID = TEXT("cotton");

	// Growth Requirements
	Crop->GrowthDuration = 1800.0f; // 30 minutes
	Crop->OptimalTemperature = FCropFloatRange(25.0f, 35.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.4f, 0.6f);
	Crop->WaterConsumption = 30.0f;
	Crop->SoilQualityRequirement = 0.7f;
	Crop->SunlightRequirement = 0.95f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Desert);
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->BiomeAdaptability = 0.3f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(5, 10);
	Crop->SellPrice = 35.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = true;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = true;
	Crop->FertilityDepletion = 0.15f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateCoffee(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Coffee");
	Crop->CropDescription = FText::FromString(TEXT("High-value cash crop requiring specific conditions. Produces valuable beans."));
	Crop->HarvestItemID = TEXT("coffee");

	// Growth Requirements
	Crop->GrowthDuration = 2400.0f; // 40 minutes
	Crop->OptimalTemperature = FCropFloatRange(18.0f, 24.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.6f, 0.8f);
	Crop->WaterConsumption = 28.0f;
	Crop->SoilQualityRequirement = 0.8f;
	Crop->SunlightRequirement = 0.6f; // Shade-tolerant

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.2f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(3, 8);
	Crop->SellPrice = 60.0f;
	Crop->RegrowthChance = 0.8f; // 80% regrowth - perennial

	// Special Properties
	Crop->bRequiresFertilizer = true;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.10f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateCacao(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Cacao");
	Crop->CropDescription = FText::FromString(TEXT("Luxury crop for chocolate production. Requires tropical forest conditions."));
	Crop->HarvestItemID = TEXT("cacao");

	// Growth Requirements
	Crop->GrowthDuration = 2700.0f; // 45 minutes
	Crop->OptimalTemperature = FCropFloatRange(22.0f, 30.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.7f, 0.9f);
	Crop->WaterConsumption = 35.0f;
	Crop->SoilQualityRequirement = 0.8f;
	Crop->SunlightRequirement = 0.5f; // Shade-loving

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.1f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(2, 6);
	Crop->SellPrice = 80.0f;
	Crop->RegrowthChance = 0.9f; // 90% regrowth - perennial tree

	// Special Properties
	Crop->bRequiresFertilizer = true;
	Crop->bRequiresIrrigation = true;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.08f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateBarley(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Barley");
	Crop->CropDescription = FText::FromString(TEXT("Cold-resistant grain crop. Excellent for harsh climates and brewing."));
	Crop->HarvestItemID = TEXT("barley");

	// Growth Requirements
	Crop->GrowthDuration = 840.0f; // 14 minutes
	Crop->OptimalTemperature = FCropFloatRange(8.0f, 18.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.4f, 0.6f);
	Crop->WaterConsumption = 14.0f;
	Crop->SoilQualityRequirement = 0.4f;
	Crop->SunlightRequirement = 0.7f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Tundra);
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Desert);
	Crop->BiomeAdaptability = 0.6f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(8, 14);
	Crop->SellPrice = 13.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = true;
	Crop->bResistantToDrought = true;
	Crop->FertilityDepletion = 0.06f;

	return Crop;
}

UCropDefinition* UCropDefinitionFactory::CreateTurnips(UObject* Outer)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>(Outer);
	
	// Identity
	Crop->CropName = TEXT("Turnips");
	Crop->CropDescription = FText::FromString(TEXT("Frost-tolerant root vegetable. Grows quickly in cool climates."));
	Crop->HarvestItemID = TEXT("turnips");

	// Growth Requirements
	Crop->GrowthDuration = 420.0f; // 7 minutes
	Crop->OptimalTemperature = FCropFloatRange(10.0f, 18.0f);
	Crop->OptimalHumidity = FCropFloatRange(0.5f, 0.7f);
	Crop->WaterConsumption = 10.0f;
	Crop->SoilQualityRequirement = 0.3f;
	Crop->SunlightRequirement = 0.6f;

	// Biome Compatibility
	Crop->SuitableBiomes.Empty();
	Crop->SuitableBiomes.Add(EBiomeType::Tundra);
	Crop->SuitableBiomes.Add(EBiomeType::Grassland);
	Crop->SuitableBiomes.Add(EBiomeType::Forest);
	Crop->BiomeAdaptability = 0.7f;

	// Harvest
	Crop->YieldAmount = FCropIntRange(10, 16);
	Crop->SellPrice = 7.0f;
	Crop->RegrowthChance = 0.0f;

	// Special Properties
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = true;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.04f;

	return Crop;
}

