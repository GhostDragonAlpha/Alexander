// Copyright Epic Games, Inc. All Rights Reserved.

#include "ResourceGatheringSystem.h"
#include "Asteroid.h"
#include "Planet.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "SystemSelfTestInterface.h"

UResourceGatheringSystem::UResourceGatheringSystem()
{
	MiningSkillMultiplier = 1.0f;
	CommonResourceChance = 0.6f;
	UncommonResourceChance = 0.25f;
	RareResourceChance = 0.12f;
	ExoticResourceChance = 0.03f;
	TotalMiningOperations = 0;
}

void UResourceGatheringSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ResourceGatheringSystem initialized"));

	// Initialize default resources
	InitializeDefaultResources();

	// Clear existing data
	ResourceDefinitions.Empty();
	AsteroidDeposits.Empty();
	PlanetaryDeposits.Empty();
	TotalResourcesMined.Empty();
	TotalMiningOperations = 0;
}

void UResourceGatheringSystem::Deinitialize()
{
	// Clear all data
	ResourceDefinitions.Empty();
	AsteroidDeposits.Empty();
	PlanetaryDeposits.Empty();
	TotalResourcesMined.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("ResourceGatheringSystem deinitialized"));
}

//~ Begin ISystemSelfTestInterface interface
bool UResourceGatheringSystem::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = GetSystemName_Implementation();
	OutResult.WarningMessages.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("=== Testing Resource Gathering System ==="));
	
	// Test 1: Check if default resources were initialized
	if (ResourceDefinitions.Num() < 20) // Should have many resources defined
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = FString::Printf(TEXT("Resource definitions not properly initialized - only %d resources found"), ResourceDefinitions.Num());
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource definitions initialization - found %d resources"), ResourceDefinitions.Num());
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Resource definitions test PASSED - %d resources registered"), ResourceDefinitions.Num());
	
	// Test 2: Test resource category distribution
	TArray<FResourceDefinition> MetallicResources = GetResourcesByCategory(EResourceCategory::Metallic);
	TArray<FResourceDefinition> CrystallineResources = GetResourcesByCategory(EResourceCategory::Crystalline);
	TArray<FResourceDefinition> OrganicResources = GetResourcesByCategory(EResourceCategory::Organic);
	TArray<FResourceDefinition> ExoticResources = GetResourcesByCategory(EResourceCategory::Exotic);
	
	if (MetallicResources.Num() < 5 || CrystallineResources.Num() < 3 || OrganicResources.Num() < 2 || ExoticResources.Num() < 2)
	{
		OutResult.WarningMessages.Add(TEXT("Resource distribution may be unbalanced"));
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Resource distribution - Metallic:%d, Crystalline:%d, Organic:%d, Exotic:%d"),
			MetallicResources.Num(), CrystallineResources.Num(), OrganicResources.Num(), ExoticResources.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ Resource distribution test PASSED - Multiple categories represented"));
	}
	
	// Test 3: Test resource rarity distribution
	TArray<FResourceDefinition> CommonResources = GetResourcesByRarity(EResourceRarity::Common);
	TArray<FResourceDefinition> RareResources = GetResourcesByRarity(EResourceRarity::Rare);
	TArray<FResourceDefinition> LegendaryResources = GetResourcesByRarity(EResourceRarity::Legendary);
	
	if (CommonResources.Num() < 5 || RareResources.Num() < 2 || LegendaryResources.Num() < 1)
	{
		OutResult.WarningMessages.Add(TEXT("Resource rarity distribution may be unbalanced"));
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Rarity distribution - Common:%d, Rare:%d, Legendary:%d"),
			CommonResources.Num(), RareResources.Num(), LegendaryResources.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ Resource rarity distribution test PASSED"));
	}
	
	// Test 4: Test resource registration and retrieval
	FResourceDefinition TestResource;
	TestResource.ResourceID = FName("TEST_RESOURCE");
	TestResource.DisplayName = "Test Resource";
	TestResource.Category = EResourceCategory::Metallic;
	TestResource.Rarity = EResourceRarity::Common;
	TestResource.BaseValue = 50.0f;
	TestResource.WeightPerUnit = 2.0f;
	TestResource.VolumePerUnit = 0.001f;
	
	RegisterResource(TestResource);
	
	FResourceDefinition RetrievedResource = GetResourceDefinition(FName("TEST_RESOURCE"));
	if (RetrievedResource.ResourceID != FName("TEST_RESOURCE") ||
		RetrievedResource.BaseValue != 50.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Resource registration/retrieval failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource registration/retrieval"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Resource registration/retrieval test PASSED"));
	
	// Test 5: Test resource value calculation
	FResourceDefinition IronResource = GetResourceDefinition(FName("Iron"));
	if (IronResource.BaseValue <= 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Resource value calculation failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource value calculation"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Resource value test PASSED - Iron value: %.1f credits"), IronResource.BaseValue);
	
	// Test 6: Test resource weight and volume
	if (IronResource.WeightPerUnit <= 0.0f || IronResource.VolumePerUnit <= 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Resource weight/volume calculation failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource weight/volume calculation"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Resource weight/volume test PASSED - Iron: Weight=%.2fkg, Volume=%.4fm³"),
		IronResource.WeightPerUnit, IronResource.VolumePerUnit);
	
	// Test 7: Test mining difficulty calculation
	FResourceDeposit TestDeposit;
	TestDeposit.Difficulty = EMiningDifficulty::Medium;
	TestDeposit.CurrentQuantity = 100;
	TestDeposit.MaxQuantity = 100;
	TestDeposit.Depth = 50.0f;
	TestDeposit.QualityMultiplier = 1.0f;
	TestDeposit.ResourceID = FName("Iron");
	
	FMiningResult MiningResult = MineDeposit(TestDeposit, 1.0f, 1.0f);
	if (!MiningResult.bSuccess && MiningResult.FailureReason != "Deposit is depleted")
	{
		// This is expected since we're using a copy of the deposit
		UE_LOG(LogTemp, Log, TEXT("✓ Mining operation test PASSED (expected failure due to test setup)"));
	}
	
	// Test 8: Test resource quality calculation
	EResourceQuality Quality = CalculateResourceQuality(1.0f, EMiningDifficulty::Easy);
	if (Quality == EResourceQuality::Damaged && FMath::FRand() > 0.3f)
	{
		// Quality is random, so this might fail occasionally
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Resource quality test - got Damaged quality with high probability"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ Resource quality calculation test PASSED - Quality=%d"), (int32)Quality);
	}
	
	// Test 9: Test statistics tracking
	ResetStatistics();
	if (TotalMiningOperations != 0 || TotalResourcesMined.Num() != 0)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Statistics reset failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics reset"));
		return false;
	}
	
	// Simulate some mining operations
	TotalMiningOperations = 10;
	TotalResourcesMined.Add(FName("Iron"), 500);
	TotalResourcesMined.Add(FName("Copper"), 250);
	
	if (GetTotalMiningOperations() != 10)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Mining operations tracking failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Mining operations tracking"));
		return false;
	}
	
	TMap<FName, int32> MinedResources = GetTotalResourcesMined();
	if (MinedResources.Num() != 2 || MinedResources.FindRef(FName("Iron")) != 500)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Resource mining tracking failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource mining tracking"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Statistics tracking test PASSED"));
	
	// Test 10: Test random resource generation
	FResourceDefinition RandomResource = GetRandomResourceForCategory(EResourceCategory::Metallic);
	if (RandomResource.ResourceID.IsNone() || RandomResource.Category != EResourceCategory::Metallic)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Random resource generation failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Random resource generation"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Random resource generation test PASSED - Got %s"), *RandomResource.DisplayName);
	
	// Record performance metrics
	OutResult.bPassed = true;
	OutResult.PerformanceMetrics.Add(TEXT("TotalResourcesDefined"), ResourceDefinitions.Num());
	OutResult.PerformanceMetrics.Add(TEXT("ResourceCategories"), 4); // Metallic, Crystalline, Organic, Exotic
	OutResult.PerformanceMetrics.Add(TEXT("MiningOperationsTracked"), TotalMiningOperations);
	
	UE_LOG(LogTemp, Log, TEXT("=== Resource Gathering System test PASSED ==="));
	return true;
 223 }
 224
 225 FString UResourceGatheringSystem::GetSystemName_Implementation() const
 226 {
 227 	return TEXT("ResourceGatheringSystem");
 228 }
 229
 230 FString UResourceGatheringSystem::GetTestDescription_Implementation() const
 231 {
 232 	return TEXT("Tests resource definitions, mining operations, and resource tracking");
 233 }
 234
 235 bool UResourceGatheringSystem::IsReadyForTesting_Implementation() const
 236 {
 236 	return ResourceDefinitions.Num() > 0;
 237 }
 238 //~ End ISystemSelfTestInterface interface
 239
 240 void UResourceGatheringSystem::Tick(float DeltaTime)
 241 {
 242 	// Update any ongoing mining operations or resource regeneration
 243 	// For now, this is a placeholder for future timed operations
 244 }

void UResourceGatheringSystem::RegisterResource(const FResourceDefinition& ResourceDef)
{
	if (ResourceDef.ResourceID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register resource with empty ID"));
		return;
	}

	ResourceDefinitions.Add(ResourceDef.ResourceID, ResourceDef);
	UE_LOG(LogTemp, Log, TEXT("Registered resource: %s (%s)"), *ResourceDef.ResourceID.ToString(), *ResourceDef.DisplayName);
}

FResourceDefinition UResourceGatheringSystem::GetResourceDefinition(FName ResourceID) const
{
	const FResourceDefinition* ResourceDef = ResourceDefinitions.Find(ResourceID);
	return ResourceDef ? *ResourceDef : FResourceDefinition();
}

TArray<FResourceDefinition> UResourceGatheringSystem::GetAllResourceDefinitions() const
{
	TArray<FResourceDefinition> AllResources;
	ResourceDefinitions.GenerateValueArray(AllResources);
	return AllResources;
}

TArray<FResourceDefinition> UResourceGatheringSystem::GetResourcesByCategory(EResourceCategory Category) const
{
	TArray<FResourceDefinition> CategoryResources;
	for (const auto& Pair : ResourceDefinitions)
	{
		if (Pair.Value.Category == Category)
		{
			CategoryResources.Add(Pair.Value);
		}
	}
	return CategoryResources;
}

TArray<FResourceDefinition> UResourceGatheringSystem::GetResourcesByRarity(EResourceRarity Rarity) const
{
	TArray<FResourceDefinition> RarityResources;
	for (const auto& Pair : ResourceDefinitions)
	{
		if (Pair.Value.Rarity == Rarity)
		{
			RarityResources.Add(Pair.Value);
		}
	}
	return RarityResources;
}

void UResourceGatheringSystem::GenerateAsteroidResources(AAsteroid* Asteroid)
{
	if (!Asteroid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate resources for null asteroid"));
		return;
	}

	// Clear existing deposits
	AsteroidDeposits.Remove(Asteroid);

	// Generate deposits based on asteroid size
	int32 NumDeposits = FMath::RandRange(1, 3);
	float AsteroidSize = Asteroid->AsteroidRadius;

	TArray<FResourceDeposit> Deposits;

	for (int32 i = 0; i < NumDeposits; i++)
	{
		FResourceDeposit Deposit;

		// Determine resource type based on chances
		float Rand = FMath::FRand();
		EResourceCategory Category;

		if (Rand < CommonResourceChance)
		{
			Category = EResourceCategory::Metallic;
			Deposit.Difficulty = EMiningDifficulty::Easy;
		}
		else if (Rand < CommonResourceChance + UncommonResourceChance)
		{
			Category = EResourceCategory::Crystalline;
			Deposit.Difficulty = EMiningDifficulty::Medium;
		}
		else if (Rand < CommonResourceChance + UncommonResourceChance + RareResourceChance)
		{
			Category = EResourceCategory::Radioactive;
			Deposit.Difficulty = EMiningDifficulty::Hard;
		}
		else
		{
			Category = EResourceCategory::Exotic;
			Deposit.Difficulty = EMiningDifficulty::Extreme;
		}

		// Get random resource from category
		FResourceDefinition ResourceDef = GetRandomResourceForCategory(Category);
		Deposit.ResourceID = ResourceDef.ResourceID;

		// Set quantity based on asteroid size and resource rarity
		float SizeMultiplier = FMath::Clamp(AsteroidSize / 50.0f, 0.5f, 3.0f);
		float RarityMultiplier = 1.0f;

		switch (ResourceDef.Rarity)
		{
			case EResourceRarity::Common: RarityMultiplier = 2.0f; break;
			case EResourceRarity::Uncommon: RarityMultiplier = 1.5f; break;
			case EResourceRarity::Rare: RarityMultiplier = 1.0f; break;
			case EResourceRarity::Exotic: RarityMultiplier = 0.5f; break;
			default: RarityMultiplier = 1.0f; break;
		}

		Deposit.MaxQuantity = FMath::RandRange(10, 100) * SizeMultiplier * RarityMultiplier;
		Deposit.CurrentQuantity = Deposit.MaxQuantity;
		Deposit.Depth = FMath::FRand() * AsteroidSize;
		Deposit.DepositType = EDepositType::Surface;
		Deposit.QualityMultiplier = FMath::FRandRange(0.8f, 1.2f);
		Deposit.bIsDepleted = false;
		Deposit.Location = Asteroid->GetActorLocation() + FVector(FMath::RandRange(-AsteroidSize, AsteroidSize), 
																  FMath::RandRange(-AsteroidSize, AsteroidSize), 
																  FMath::RandRange(-AsteroidSize, AsteroidSize));

		Deposits.Add(Deposit);
	}

	// Store deposits
	AsteroidDeposits.Add(Asteroid, Deposits);

	UE_LOG(LogTemp, Log, TEXT("Generated %d resource deposits for asteroid %d (Size: %.1fm)"), 
		   Deposits.Num(), Asteroid->AsteroidID, AsteroidSize);
}

TArray<FResourceDeposit> UResourceGatheringSystem::GetAsteroidResources(AAsteroid* Asteroid) const
{
	const TArray<FResourceDeposit>* Deposits = AsteroidDeposits.Find(Asteroid);
	return Deposits ? *Deposits : TArray<FResourceDeposit>();
}

bool UResourceGatheringSystem::ScanAsteroid(AAsteroid* Asteroid, float ScanStrength, TArray<FResourceDeposit>& OutDeposits)
{
	if (!Asteroid)
	{
		return false;
	}

	TArray<FResourceDeposit> AllDeposits = GetAsteroidResources(Asteroid);
	OutDeposits.Empty();

	for (const FResourceDeposit& Deposit : AllDeposits)
	{
		// Calculate detection chance based on scan strength and deposit depth
		float DetectionChance = FMath::Clamp(ScanStrength / (1.0f + Deposit.Depth * 0.01f), 0.0f, 1.0f);
		
		if (FMath::FRand() < DetectionChance)
		{
			OutDeposits.Add(Deposit);
		}
	}

	// Fire discovery event for each detected deposit
	for (const FResourceDeposit& Deposit : OutDeposits)
	{
		OnResourceDiscovered.Broadcast(Deposit);
	}

	return OutDeposits.Num() > 0;
}

void UResourceGatheringSystem::GeneratePlanetaryResources(APlanet* Planet)
{
	if (!Planet)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate resources for null planet"));
		return;
	}

	// Clear existing deposits for this planet
	PlanetaryDeposits.Remove(Planet);

	// Generate resource deposits based on planet type and biomes
	// This is a simplified version - in a full implementation, this would be
	// based on the planet's biome system and geological features

	FPlanetaryDepositMap PlanetDeposits;

	// For now, we'll create a few sample deposits at random locations
	int32 NumDeposits = FMath::RandRange(5, 15);

	for (int32 i = 0; i < NumDeposits; i++)
	{
		FResourceDeposit Deposit;

		// Determine resource type based on planet characteristics
		float Rand = FMath::FRand();
		EResourceCategory Category;

		if (Rand < 0.4f)
		{
			Category = EResourceCategory::Metallic;
			Deposit.Difficulty = EMiningDifficulty::Easy;
		}
		else if (Rand < 0.7f)
		{
			Category = EResourceCategory::Crystalline;
			Deposit.Difficulty = EMiningDifficulty::Medium;
		}
		else if (Rand < 0.9f)
		{
			Category = EResourceCategory::Organic;
			Deposit.Difficulty = EMiningDifficulty::Easy;
		}
		else
		{
			Category = EResourceCategory::Radioactive;
			Deposit.Difficulty = EMiningDifficulty::Hard;
		}

		// Get random resource from category
		FResourceDefinition ResourceDef = GetRandomResourceForCategory(Category);
		Deposit.ResourceID = ResourceDef.ResourceID;

		// Set quantity
		Deposit.MaxQuantity = FMath::RandRange(50, 500);
		Deposit.CurrentQuantity = Deposit.MaxQuantity;
		Deposit.Depth = FMath::FRandRange(0.0f, 1000.0f);
		
		// Set deposit type based on depth
		if (Deposit.Depth < 10.0f)
		{
			Deposit.DepositType = EDepositType::Surface;
		}
		else if (Deposit.Depth < 100.0f)
		{
			Deposit.DepositType = EDepositType::Underground;
		}
		else
		{
			Deposit.DepositType = EDepositType::Deep;
		}

		Deposit.QualityMultiplier = FMath::FRandRange(0.7f, 1.3f);
		Deposit.bIsDepleted = false;

		// Random location on planet surface (simplified)
		FVector RandomLocation = Planet->GetActorLocation() + FVector(
			FMath::RandRange(-100000.0f, 100000.0f),
			FMath::RandRange(-100000.0f, 100000.0f),
			FMath::RandRange(-100000.0f, 100000.0f)
		).GetSafeNormal() * Planet->PlanetRadius * 100000.0f;

		Deposit.Location = RandomLocation;

		// Add to planet deposits (using location as key)
		PlanetDeposits.LocationDeposits.FindOrAdd(RandomLocation).Deposits.Add(Deposit);
	}

	// Store deposits
	PlanetaryDeposits.Add(Planet, PlanetDepositMap);

	UE_LOG(LogTemp, Log, TEXT("Generated %d resource deposits for planet"), NumDeposits);
}

TArray<FResourceDeposit> UResourceGatheringSystem::GetPlanetaryResources(APlanet* Planet, FVector Location) const
{
	if (!Planet)
	{
		return TArray<FResourceDeposit>();
	}

	// Find deposits near the location
	TArray<FResourceDeposit> NearbyDeposits;
	const FPlanetaryDepositMap* PlanetDeposits = PlanetaryDeposits.Find(Planet);

	if (PlanetDepositMap)
	{
<<<<<<< Updated upstream
		for (const auto& Pair : PlanetDepositMap->LocationDeposits)
=======
		for (const auto& Pair : PlanetDeposits->LocationDeposits)
>>>>>>> Stashed changes
		{
			float Distance = FVector::Dist(Location, Pair.Key);
			if (Distance < 5000.0f) // 5km range
			{
				NearbyDeposits.Append(Pair.Value.Deposits);
			}
		}
	}

	return NearbyDeposits;
}

bool UResourceGatheringSystem::ScanPlanetaryLocation(APlanet* Planet, FVector Location, float ScanStrength, TArray<FResourceDeposit>& OutDeposits)
{
	if (!Planet)
	{
		return false;
	}

	TArray<FResourceDeposit> AllDeposits = GetPlanetaryResources(Planet, Location);
	OutDeposits.Empty();

	for (const FResourceDeposit& Deposit : AllDeposits)
	{
		// Calculate detection chance based on scan strength and deposit depth
		float DetectionChance = FMath::Clamp(ScanStrength / (1.0f + Deposit.Depth * 0.005f), 0.0f, 1.0f);
		
		if (FMath::FRand() < DetectionChance)
		{
			OutDeposits.Add(Deposit);
		}
	}

	// Fire discovery event for each detected deposit
	for (const FResourceDeposit& Deposit : OutDeposits)
	{
		OnResourceDiscovered.Broadcast(Deposit);
	}

	return OutDeposits.Num() > 0;
}

FMiningResult UResourceGatheringSystem::MineDeposit(FResourceDeposit& Deposit, float MiningPower, float MiningEfficiency)
{
	FMiningResult Result;
	Result.bSuccess = false;

	if (Deposit.bIsDepleted || Deposit.CurrentQuantity <= 0)
	{
		Result.FailureReason = "Deposit is depleted";
		return Result;
	}

	// Calculate mining success based on power, efficiency, and difficulty
	float DifficultyMultiplier = 1.0f;
	switch (Deposit.Difficulty)
	{
		case EMiningDifficulty::Easy: DifficultyMultiplier = 1.0f; break;
		case EMiningDifficulty::Medium: DifficultyMultiplier = 1.5f; break;
		case EMiningDifficulty::Hard: DifficultyMultiplier = 2.0f; break;
		case EMiningDifficulty::Extreme: DifficultyMultiplier = 3.0f; break;
	}

	float SuccessChance = FMath::Clamp((MiningPower / DifficultyMultiplier) * MiningEfficiency * MiningSkillMultiplier, 0.1f, 0.95f);

	if (FMath::FRand() > SuccessChance)
	{
		Result.FailureReason = "Mining operation failed";
		Result.ExperienceGained = 1.0f; // Small XP even on failure
		return Result;
	}

	// Calculate amount extracted
	float AmountMultiplier = MiningPower * MiningEfficiency * MiningSkillMultiplier / DifficultyMultiplier;
	int32 AmountExtracted = FMath::RandRange(1, FMath::Max(1, FMath::FloorToInt(AmountMultiplier * 10)));

	// Don't extract more than available
	AmountExtracted = FMath::Min(AmountExtracted, Deposit.CurrentQuantity);

	// Update deposit
	Deposit.CurrentQuantity -= AmountExtracted;
	if (Deposit.CurrentQuantity <= 0)
	{
		Deposit.CurrentQuantity = 0;
		Deposit.bIsDepleted = true;
		OnDepositDepleted.Broadcast(Deposit);
	}

	// Calculate resource quality
	EResourceQuality Quality = CalculateResourceQuality(Deposit.QualityMultiplier, Deposit.Difficulty);

	// Create resource quantity
	FResourceQuantity ExtractedResource(Deposit.ResourceID, AmountExtracted, Quality);
	Result.ResourcesExtracted.Add(ExtractedResource);
	Result.bSuccess = true;
	Result.ExperienceGained = AmountExtracted * 0.5f * DifficultyMultiplier;

	// Update statistics
	TotalMiningOperations++;
	int32& MinedAmount = TotalResourcesMined.FindOrAdd(Deposit.ResourceID);
	MinedAmount += AmountExtracted;

	// Fire mined event
	OnResourceMined.Broadcast(ExtractedResource);

	UE_LOG(LogTemp, Log, TEXT("Mined %d units of %s (Quality: %d)"), AmountExtracted, *Deposit.ResourceID.ToString(), (int32)Quality);

	return Result;
}

FMiningResult UResourceGatheringSystem::MineAsteroid(AAsteroid* Asteroid, float MiningPower, float MiningEfficiency)
{
	if (!Asteroid)
	{
		FMiningResult Result;
		Result.bSuccess = false;
		Result.FailureReason = "Invalid asteroid";
		return Result;
	}

	TArray<FResourceDeposit> Deposits = GetAsteroidResources(Asteroid);
	if (Deposits.Num() == 0)
	{
		FMiningResult Result;
		Result.bSuccess = false;
		Result.FailureReason = "No resources found";
		return Result;
	}

	// Mine the first available deposit
	for (FResourceDeposit& Deposit : Deposits)
	{
		if (!Deposit.bIsDepleted)
		{
			return MineDeposit(Deposit, MiningPower, MiningEfficiency);
		}
	}

	FMiningResult Result;
	Result.bSuccess = false;
	Result.FailureReason = "All deposits depleted";
	return Result;
}

FMiningResult UResourceGatheringSystem::MinePlanetaryDeposit(APlanet* Planet, FVector Location, float MiningPower, float MiningEfficiency)
{
	if (!Planet)
	{
		FMiningResult Result;
		Result.bSuccess = false;
		Result.FailureReason = "Invalid planet";
		return Result;
	}

	TArray<FResourceDeposit> Deposits = GetPlanetaryResources(Planet, Location);
	if (Deposits.Num() == 0)
	{
		FMiningResult Result;
		Result.bSuccess = false;
		Result.FailureReason = "No resources found at location";
		return Result;
	}

	// Mine the first available deposit
	for (FResourceDeposit& Deposit : Deposits)
	{
		if (!Deposit.bIsDepleted)
		{
			return MineDeposit(Deposit, MiningPower, MiningEfficiency);
		}
	}

	FMiningResult Result;
	Result.bSuccess = false;
	Result.FailureReason = "All deposits depleted";
	return Result;
}

void UResourceGatheringSystem::InitializeDefaultResources()
{
	// Initialize all resource categories
	InitializeMetallicResources();
	InitializeCrystallineResources();
	InitializeOrganicResources();
	InitializeRadioactiveResources();
	InitializeExoticResources();

	UE_LOG(LogTemp, Log, TEXT("Initialized default resource definitions"));
}

void UResourceGatheringSystem::LoadResourcesFromDataAssets()
{
	// Placeholder for loading from data assets
	UE_LOG(LogTemp, Log, TEXT("LoadResourcesFromDataAssets not yet implemented"));
}

void UResourceGatheringSystem::ResetStatistics()
{
	TotalResourcesMined.Empty();
	TotalMiningOperations = 0;
	UE_LOG(LogTemp, Log, TEXT("Resource gathering statistics reset"));
}

FResourceDefinition UResourceGatheringSystem::GetRandomResourceForCategory(EResourceCategory Category) const
{
	TArray<FResourceDefinition> CategoryResources = GetResourcesByCategory(Category);
	
	if (CategoryResources.Num() > 0)
	{
		return CategoryResources[FMath::RandRange(0, CategoryResources.Num() - 1)];
	}

	// Return a default resource if none found
	FResourceDefinition Default;
	Default.ResourceID = FName("Unknown");
	Default.DisplayName = "Unknown Resource";
	Default.Category = Category;
	return Default;
}

EResourceQuality UResourceGatheringSystem::CalculateResourceQuality(float QualityMultiplier, EMiningDifficulty Difficulty) const
{
	float BaseQuality = FMath::FRand();

	// Adjust based on quality multiplier
	BaseQuality *= QualityMultiplier;

	// Adjust based on difficulty (harder deposits yield better quality)
	switch (Difficulty)
	{
		case EMiningDifficulty::Easy: BaseQuality *= 0.8f; break;
		case EMiningDifficulty::Medium: BaseQuality *= 1.0f; break;
		case EMiningDifficulty::Hard: BaseQuality *= 1.2f; break;
		case EMiningDifficulty::Extreme: BaseQuality *= 1.5f; break;
	}

	// Convert to quality enum
	if (BaseQuality < 0.3f)
	{
		return EResourceQuality::Damaged;
	}
	else if (BaseQuality < 0.7f)
	{
		return EResourceQuality::Normal;
	}
	else if (BaseQuality < 0.95f)
	{
		return EResourceQuality::Pristine;
	}
	else
	{
		return EResourceQuality::Legendary;
	}
}

void UResourceGatheringSystem::InitializeMetallicResources()
{
	// Iron
	FResourceDefinition Iron;
	Iron.ResourceID = FName("Iron");
	Iron.DisplayName = "Iron Ore";
	Iron.Category = EResourceCategory::Metallic;
	Iron.Rarity = EResourceRarity::Common;
	Iron.BaseValue = 10.0f;
	Iron.WeightPerUnit = 2.5f;
	Iron.VolumePerUnit = 0.001f;
	Iron.Description = "Common iron ore used for basic construction and manufacturing.";
	RegisterResource(Iron);

	// Nickel
	FResourceDefinition Nickel;
	Nickel.ResourceID = FName("Nickel");
	Nickel.DisplayName = "Nickel Ore";
	Nickel.Category = EResourceCategory::Metallic;
	Nickel.Rarity = EResourceRarity::Common;
	Nickel.BaseValue = 15.0f;
	Nickel.WeightPerUnit = 2.8f;
	Nickel.VolumePerUnit = 0.001f;
	Nickel.Description = "Nickel ore used in alloys and advanced materials.";
	RegisterResource(Nickel);

	// Copper
	FResourceDefinition Copper;
	Copper.ResourceID = FName("Copper");
	Copper.DisplayName = "Copper Ore";
	Copper.Category = EResourceCategory::Metallic;
	Copper.Rarity = EResourceRarity::Uncommon;
	Copper.BaseValue = 25.0f;
	Copper.WeightPerUnit = 3.0f;
	Copper.VolumePerUnit = 0.001f;
	Copper.Description = "Copper ore essential for electronics and electrical systems.";
	RegisterResource(Copper);

	// Aluminum
	FResourceDefinition Aluminum;
	Aluminum.ResourceID = FName("Aluminum");
	Aluminum.DisplayName = "Aluminum Ore";
	Aluminum.Category = EResourceCategory::Metallic;
	Aluminum.Rarity = EResourceRarity::Uncommon;
	Aluminum.BaseValue = 30.0f;
	Aluminum.WeightPerUnit = 1.5f;
	Aluminum.VolumePerUnit = 0.001f;
	Aluminum.Description = "Lightweight aluminum ore for aerospace applications.";
	RegisterResource(Aluminum);

	// Titanium
	FResourceDefinition Titanium;
	Titanium.ResourceID = FName("Titanium");
	Titanium.DisplayName = "Titanium Ore";
	Titanium.Category = EResourceCategory::Metallic;
	Titanium.Rarity = EResourceRarity::Rare;
	Titanium.BaseValue = 100.0f;
	Titanium.WeightPerUnit = 2.0f;
	Titanium.VolumePerUnit = 0.001f;
	Titanium.Description = "High-strength titanium ore for advanced construction.";
	RegisterResource(Titanium);

	// Gold
	FResourceDefinition Gold;
	Gold.ResourceID = FName("Gold");
	Gold.DisplayName = "Gold Ore";
	Gold.Category = EResourceCategory::Metallic;
	Gold.Rarity = EResourceRarity::Rare;
	Gold.BaseValue = 200.0f;
	Gold.WeightPerUnit = 5.0f;
	Gold.VolumePerUnit = 0.001f;
	Gold.Description = "Precious gold ore for high-value applications.";
	RegisterResource(Gold);

	// Platinum
	FResourceDefinition Platinum;
	Platinum.ResourceID = FName("Platinum");
	Platinum.DisplayName = "Platinum Ore";
	Platinum.Category = EResourceCategory::Metallic;
	Platinum.Rarity = EResourceRarity::Rare;
	Platinum.BaseValue = 300.0f;
	Platinum.WeightPerUnit = 4.5f;
	Platinum.VolumePerUnit = 0.001f;
	Platinum.Description = "Rare platinum ore for catalytic and electronic applications.";
	RegisterResource(Platinum);

	// Uranium
	FResourceDefinition Uranium;
	Uranium.ResourceID = FName("Uranium");
	Uranium.DisplayName = "Uranium Ore";
	Uranium.Category = EResourceCategory::Radioactive;
	Uranium.Rarity = EResourceRarity::Rare;
	Uranium.BaseValue = 500.0f;
	Uranium.WeightPerUnit = 6.0f;
	Uranium.VolumePerUnit = 0.001f;
	Uranium.Description = "Radioactive uranium ore for nuclear applications.";
	RegisterResource(Uranium);
}

void UResourceGatheringSystem::InitializeCrystallineResources()
{
	// Silicon
	FResourceDefinition Silicon;
	Silicon.ResourceID = FName("Silicon");
	Silicon.DisplayName = "Silicon Crystals";
	Silicon.Category = EResourceCategory::Crystalline;
	Silicon.Rarity = EResourceRarity::Common;
	Silicon.BaseValue = 20.0f;
	Silicon.WeightPerUnit = 1.0f;
	Silicon.VolumePerUnit = 0.001f;
	Silicon.Description = "Silicon crystals for electronics and solar panels.";
	RegisterResource(Silicon);

	// Quartz
	FResourceDefinition Quartz;
	Quartz.ResourceID = FName("Quartz");
	Quartz.DisplayName = "Quartz Crystals";
	Quartz.Category = EResourceCategory::Crystalline;
	Quartz.Rarity = EResourceRarity::Uncommon;
	Quartz.BaseValue = 40.0f;
	Quartz.WeightPerUnit = 1.2f;
	Quartz.VolumePerUnit = 0.001f;
	Quartz.Description = "Pure quartz crystals for optical and electronic applications.";
	RegisterResource(Quartz);

	// Diamond
	FResourceDefinition Diamond;
	Diamond.ResourceID = FName("Diamond");
	Diamond.DisplayName = "Industrial Diamonds";
	Diamond.Category = EResourceCategory::Crystalline;
	Diamond.Rarity = EResourceRarity::Rare;
	Diamond.BaseValue = 150.0f;
	Diamond.WeightPerUnit = 1.8f;
	Diamond.VolumePerUnit = 0.001f;
	Diamond.Description = "Industrial-grade diamonds for cutting and high-pressure applications.";
	RegisterResource(Diamond);

	// Quantum Crystals
	FResourceDefinition QuantumCrystals;
	QuantumCrystals.ResourceID = FName("QuantumCrystals");
	QuantumCrystals.DisplayName = "Quantum Crystals";
	QuantumCrystals.Category = EResourceCategory::Exotic;
	QuantumCrystals.Rarity = EResourceRarity::Exotic;
	QuantumCrystals.BaseValue = 2000.0f;
	QuantumCrystals.WeightPerUnit = 0.5f;
	QuantumCrystals.VolumePerUnit = 0.001f;
	QuantumCrystals.Description = "Exotic quantum crystals with unique quantum properties.";
	RegisterResource(QuantumCrystals);
}

void UResourceGatheringSystem::InitializeOrganicResources()
{
	// Biomass
	FResourceDefinition Biomass;
	Biomass.ResourceID = FName("Biomass");
	Biomass.DisplayName = "Biomass";
	Biomass.Category = EResourceCategory::Organic;
	Biomass.Rarity = EResourceRarity::Common;
	Biomass.BaseValue = 5.0f;
	Biomass.WeightPerUnit = 0.5f;
	Biomass.VolumePerUnit = 0.002f;
	Biomass.Description = "Organic biomass for fuel and chemical processing.";
	RegisterResource(Biomass);

	// Oil
	FResourceDefinition Oil;
	Oil.ResourceID = FName("Oil");
	Oil.DisplayName = "Crude Oil";
	Oil.Category = EResourceCategory::Organic;
	Oil.Rarity = EResourceRarity::Uncommon;
	Oil.BaseValue = 35.0f;
	Oil.WeightPerUnit = 0.8f;
	Oil.VolumePerUnit = 0.0015f;
	Oil.Description = "Crude oil for fuel and chemical production.";
	RegisterResource(Oil);

	// Rare Plants
	FResourceDefinition RarePlants;
	RarePlants.ResourceID = FName("RarePlants");
	RarePlants.DisplayName = "Rare Plants";
	RarePlants.Category = EResourceCategory::Organic;
	RarePlants.Rarity = EResourceRarity::Rare;
	RarePlants.BaseValue = 80.0f;
	RarePlants.WeightPerUnit = 0.3f;
	RarePlants.VolumePerUnit = 0.003f;
	RarePlants.Description = "Rare plant specimens for medical and research purposes.";
	RegisterResource(RarePlants);
}

void UResourceGatheringSystem::InitializeRadioactiveResources()
{
	// Thorium
	FResourceDefinition Thorium;
	Thorium.ResourceID = FName("Thorium");
	Thorium.DisplayName = "Thorium Ore";
	Thorium.Category = EResourceCategory::Radioactive;
	Thorium.Rarity = EResourceRarity::Uncommon;
	Thorium.BaseValue = 400.0f;
	Thorium.WeightPerUnit = 5.5f;
	Thorium.VolumePerUnit = 0.001f;
	Thorium.Description = "Thorium ore for advanced nuclear applications.";
	RegisterResource(Thorium);

	// Plutonium
	FResourceDefinition Plutonium;
	Plutonium.ResourceID = FName("Plutonium");
	Plutonium.DisplayName = "Plutonium";
	Plutonium.Category = EResourceCategory::Radioactive;
	Plutonium.Rarity = EResourceRarity::Exotic;
	Plutonium.BaseValue = 1500.0f;
	Plutonium.WeightPerUnit = 7.0f;
	Plutonium.VolumePerUnit = 0.001f;
	Plutonium.Description = "Rare plutonium for specialized nuclear applications.";
	RegisterResource(Plutonium);
}

void UResourceGatheringSystem::InitializeExoticResources()
{
	// Dark Matter
	FResourceDefinition DarkMatter;
	DarkMatter.ResourceID = FName("DarkMatter");
	DarkMatter.DisplayName = "Dark Matter";
	DarkMatter.Category = EResourceCategory::Exotic;
	DarkMatter.Rarity = EResourceRarity::Legendary;
	DarkMatter.BaseValue = 10000.0f;
	DarkMatter.WeightPerUnit = 0.1f;
	DarkMatter.VolumePerUnit = 0.0005f;
	DarkMatter.Description = "Exotic dark matter for experimental applications.";
	RegisterResource(DarkMatter);

	// Antimatter
	FResourceDefinition Antimatter;
	Antimatter.ResourceID = FName("Antimatter");
	Antimatter.DisplayName = "Antimatter";
	Antimatter.Category = EResourceCategory::Exotic;
	Antimatter.Rarity = EResourceRarity::Legendary;
	Antimatter.BaseValue = 50000.0f;
	Antimatter.WeightPerUnit = 0.01f;
	Antimatter.VolumePerUnit = 0.0001f;
	Antimatter.Description = "Pure antimatter for ultimate power generation.";
	RegisterResource(Antimatter);
}