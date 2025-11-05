// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropFactory.h"

TArray<UCropDefinition*> UCropFactory::CreateAllStandardCrops()
{
	TArray<UCropDefinition*> Crops;
	
	Crops.Add(CreateSpaceWheat());
	Crops.Add(CreateLunarPotato());
	Crops.Add(CreateMartianTomato());
	Crops.Add(CreateTitanCorn());
	Crops.Add(CreateVenusianPepper());
	Crops.Add(CreateEuropaKelp());
	Crops.Add(CreateGanymedeBerry());
	Crops.Add(CreateCallistoMushroom());
	Crops.Add(CreateIoCactus());
	Crops.Add(CreateEnceladusRice());
	Crops.Add(CreateTritonHerb());
	Crops.Add(CreatePlutoFlower());
	
	return Crops;
}

UCropDefinition* UCropFactory::CreateSpaceWheat()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Grassland, EBiomeType::Desert };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Space Wheat"),
		TEXT("Hardy grain crop adapted for low-gravity environments. Grows quickly and reliably in most conditions."),
		600.0f,  // 10 minutes
		10.0f, 30.0f,  // Temperature
		0.3f, 0.6f,    // Humidity
		8.0f,          // Water consumption
		0.4f,          // Soil quality
		0.8f,          // Sunlight
		Biomes,
		8, 15,         // Yield
		5.0f,          // Price
		TEXT("space_wheat")
	);
	
	Crop->FertilityDepletion = 0.05f;
	Crop->BiomeAdaptability = 0.7f;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateLunarPotato()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Grassland, EBiomeType::Tundra, EBiomeType::Desert };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Lunar Potato"),
		TEXT("Versatile tuber that grows in harsh conditions. Resistant to drought and temperature extremes."),
		900.0f,  // 15 minutes
		5.0f, 25.0f,
		0.4f, 0.7f,
		12.0f,
		0.3f,
		0.6f,
		Biomes,
		10, 20,
		8.0f,
		TEXT("lunar_potato")
	);
	
	Crop->FertilityDepletion = 0.08f;
	Crop->BiomeAdaptability = 0.8f;
	Crop->bResistantToDrought = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateMartianTomato()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Grassland, EBiomeType::Forest };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Martian Tomato"),
		TEXT("Genetically modified tomato for red planet conditions. Requires consistent watering and good soil."),
		720.0f,  // 12 minutes
		15.0f, 28.0f,
		0.5f, 0.8f,
		15.0f,
		0.6f,
		0.9f,
		Biomes,
		12, 18,
		12.0f,
		TEXT("martian_tomato")
	);
	
	Crop->FertilityDepletion = 0.06f;
	Crop->BiomeAdaptability = 0.5f;
	Crop->bRequiresIrrigation = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateTitanCorn()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Grassland, EBiomeType::Forest };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Titan Corn"),
		TEXT("Tall grain crop optimized for methane-rich atmospheres. Takes longer to grow but yields well."),
		1200.0f,  // 20 minutes
		12.0f, 26.0f,
		0.4f, 0.7f,
		18.0f,
		0.5f,
		0.85f,
		Biomes,
		15, 25,
		10.0f,
		TEXT("titan_corn")
	);
	
	Crop->FertilityDepletion = 0.1f;
	Crop->BiomeAdaptability = 0.6f;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateVenusianPepper()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Volcanic, EBiomeType::Desert };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Venusian Pepper"),
		TEXT("Heat-loving spicy crop for extreme environments. Highly valuable and pest resistant."),
		840.0f,  // 14 minutes
		25.0f, 45.0f,
		0.2f, 0.5f,
		10.0f,
		0.4f,
		0.95f,
		Biomes,
		8, 12,
		20.0f,
		TEXT("venusian_pepper")
	);
	
	Crop->FertilityDepletion = 0.04f;
	Crop->BiomeAdaptability = 0.3f;
	Crop->bResistantToPests = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateEuropaKelp()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Ocean, EBiomeType::Tundra };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Europa Kelp"),
		TEXT("Aquatic crop for underwater farming. Grows quickly in cold, wet environments."),
		480.0f,  // 8 minutes
		-5.0f, 15.0f,
		0.9f, 1.0f,
		25.0f,
		0.2f,
		0.3f,
		Biomes,
		20, 30,
		6.0f,
		TEXT("europa_kelp")
	);
	
	Crop->FertilityDepletion = 0.02f;
	Crop->BiomeAdaptability = 0.4f;
	Crop->bRequiresIrrigation = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateGanymedeBerry()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Forest, EBiomeType::Grassland };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Ganymede Berry"),
		TEXT("Sweet fruit crop with high nutritional value. Can regrow after harvest for multiple yields."),
		1080.0f,  // 18 minutes
		10.0f, 22.0f,
		0.6f, 0.8f,
		14.0f,
		0.7f,
		0.75f,
		Biomes,
		10, 16,
		18.0f,
		TEXT("ganymede_berry")
	);
	
	Crop->FertilityDepletion = 0.07f;
	Crop->BiomeAdaptability = 0.5f;
	Crop->RegrowthChance = 0.3f;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateCallistoMushroom()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Forest, EBiomeType::Tundra, EBiomeType::Volcanic };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Callisto Mushroom"),
		TEXT("Fungal crop that grows in low light. Fast growing and pest resistant."),
		360.0f,  // 6 minutes
		5.0f, 20.0f,
		0.7f, 0.95f,
		8.0f,
		0.3f,
		0.2f,
		Biomes,
		15, 22,
		15.0f,
		TEXT("callisto_mushroom")
	);
	
	Crop->FertilityDepletion = 0.03f;
	Crop->BiomeAdaptability = 0.7f;
	Crop->bResistantToPests = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateIoCactus()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Desert, EBiomeType::Volcanic };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Io Cactus"),
		TEXT("Drought-resistant succulent for arid worlds. Extremely hardy but slow growing. Very valuable."),
		1500.0f,  // 25 minutes
		20.0f, 40.0f,
		0.1f, 0.3f,
		4.0f,
		0.2f,
		1.0f,
		Biomes,
		6, 10,
		25.0f,
		TEXT("io_cactus")
	);
	
	Crop->FertilityDepletion = 0.01f;
	Crop->BiomeAdaptability = 0.2f;
	Crop->bResistantToDrought = true;
	Crop->bResistantToPests = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateEnceladusRice()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Ocean, EBiomeType::Grassland };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Enceladus Rice"),
		TEXT("Water-loving grain for wet environments. Requires irrigation but produces high yields."),
		960.0f,  // 16 minutes
		18.0f, 30.0f,
		0.8f, 1.0f,
		22.0f,
		0.5f,
		0.8f,
		Biomes,
		18, 28,
		7.0f,
		TEXT("enceladus_rice")
	);
	
	Crop->FertilityDepletion = 0.06f;
	Crop->BiomeAdaptability = 0.4f;
	Crop->bRequiresIrrigation = true;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateTritonHerb()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Tundra, EBiomeType::Grassland };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Triton Herb"),
		TEXT("Medicinal herb for cold climates. Valuable for pharmaceutical applications."),
		540.0f,  // 9 minutes
		-10.0f, 10.0f,
		0.4f, 0.6f,
		6.0f,
		0.4f,
		0.5f,
		Biomes,
		12, 18,
		22.0f,
		TEXT("triton_herb")
	);
	
	Crop->FertilityDepletion = 0.04f;
	Crop->BiomeAdaptability = 0.5f;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreatePlutoFlower()
{
	TArray<EBiomeType> Biomes = { EBiomeType::Tundra, EBiomeType::Forest };
	
	UCropDefinition* Crop = CreateCropBase(
		TEXT("Pluto Flower"),
		TEXT("Beautiful ornamental crop for extreme cold. Highly prized for decorative purposes."),
		720.0f,  // 12 minutes
		-20.0f, 5.0f,
		0.3f, 0.5f,
		5.0f,
		0.5f,
		0.6f,
		Biomes,
		8, 14,
		30.0f,
		TEXT("pluto_flower")
	);
	
	Crop->FertilityDepletion = 0.05f;
	Crop->BiomeAdaptability = 0.3f;
	Crop->RegrowthChance = 0.2f;
	
	return Crop;
}

UCropDefinition* UCropFactory::CreateCropBase(
	const FString& Name,
	const FString& Description,
	float GrowthDuration,
	float TempMin, float TempMax,
	float HumidMin, float HumidMax,
	float WaterConsumption,
	float SoilQuality,
	float Sunlight,
	const TArray<EBiomeType>& Biomes,
	int32 YieldMin, int32 YieldMax,
	float Price,
	const FString& ItemID
)
{
	UCropDefinition* Crop = NewObject<UCropDefinition>();
	
	// Identity
	Crop->CropName = Name;
	Crop->CropDescription = FText::FromString(Description);
	
	// Growth requirements
	Crop->GrowthDuration = GrowthDuration;
	Crop->OptimalTemperature = FCropFloatRange(TempMin, TempMax);
	Crop->OptimalHumidity = FCropFloatRange(HumidMin, HumidMax);
	Crop->WaterConsumption = WaterConsumption;
	Crop->SoilQualityRequirement = SoilQuality;
	Crop->SunlightRequirement = Sunlight;
	
	// Biome compatibility
	Crop->SuitableBiomes = Biomes;
	Crop->BiomeAdaptability = 0.5f; // Default, can be overridden
	
	// Harvest configuration
	Crop->YieldAmount = FCropIntRange(YieldMin, YieldMax);
	Crop->HarvestItemID = ItemID;
	Crop->SellPrice = Price;
	Crop->RegrowthChance = 0.0f; // Default, can be overridden
	
	// Special properties (defaults)
	Crop->bRequiresFertilizer = false;
	Crop->bRequiresIrrigation = false;
	Crop->bResistantToPests = false;
	Crop->bResistantToDrought = false;
	Crop->FertilityDepletion = 0.05f; // Default, can be overridden
	
	return Crop;
}
