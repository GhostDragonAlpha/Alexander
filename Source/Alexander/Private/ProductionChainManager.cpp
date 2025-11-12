// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProductionChainManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Math/UnrealMathUtility.h"

AProductionChainManager::AProductionChainManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default configuration
	bAutoRoutingEnabled = true;
	MaxProductionModules = 50;

	// Initialize resource values (credits per unit)
	ResourceValues.Add(FName("Wheat"), 8.0f);
	ResourceValues.Add(FName("Corn"), 12.0f);
	ResourceValues.Add(FName("Tomatoes"), 18.0f);
	ResourceValues.Add(FName("MedicalHerbs"), 45.0f);
	ResourceValues.Add(FName("Pharmaceuticals"), 120.0f);
	ResourceValues.Add(FName("Cotton"), 25.0f);
	ResourceValues.Add(FName("Biofuel"), 15.0f);
	ResourceValues.Add(FName("Steel"), 20.0f);
	ResourceValues.Add(FName("Electronics"), 100.0f);
	ResourceValues.Add(FName("Components"), 150.0f);
	ResourceValues.Add(FName("AdvancedComponents"), 300.0f);
}

void AProductionChainManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeRecipeDatabase();
	UE_LOG(LogTemp, Log, TEXT("ProductionChainManager initialized with %d recipes"), RecipeDatabase.Num());
}

void AProductionChainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update production
	UpdateProduction(DeltaTime);

	// Update resource routing
	UpdateResourceRouting(DeltaTime);

	// Update statistics periodically
	static float StatsAccumulator = 0.0f;
	StatsAccumulator += DeltaTime;

	if (StatsAccumulator >= 10.0f) // Update every 10 seconds
	{
		StatsAccumulator = 0.0f;
		ProductionStats = GetProductionStatistics();
		OnProductionStatisticsUpdated.Broadcast(ProductionStats);
	}
}

void AProductionChainManager::Initialize(ABaseBuildingManager* BuildingManager, APlanetaryFarmingSystem* FarmingSystem)
{
	if (!BuildingManager || !FarmingSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid building manager or farming system"));
		return;
	}

	BaseBuildingManager = BuildingManager;
	FarmingSystem = FarmingSystem;

	UE_LOG(LogTemp, Log, TEXT("ProductionChainManager initialized with BuildingManager and FarmingSystem"));
}

void AProductionChainManager::InitializeRecipeDatabase()
{
	RecipeDatabase.Empty();

	// === TIER 1: RAW PROCESSING ===

	// Wheat -> Flour
	FProductionRecipe WheatToFlour;
	WheatToFlour.RecipeName = TEXT("Wheat to Flour");
	WheatToFlour.InputResources.Add(FName("Wheat"), 10);
	WheatToFlour.OutputResources.Add(FName("Flour"), 8);
	WheatToFlour.ProductionTime = 30.0f;
	WheatToFlour.RequiredTier = EProductionTier::Tier1_RawProcessing;
	WheatToFlour.RequiredModuleType = EBaseModuleType::Refinery;
	WheatToFlour.PowerConsumption = 5.0f;
	WheatToFlour.WorkerRequirement = 1;
	RecipeDatabase.Add(WheatToFlour);

	// Corn -> Cornmeal
	FProductionRecipe CornToCornmeal;
	CornToCornmeal.RecipeName = TEXT("Corn to Cornmeal");
	CornToCornmeal.InputResources.Add(FName("Corn"), 10);
	CornToCornmeal.OutputResources.Add(FName("Cornmeal"), 8);
	CornToCornmeal.ProductionTime = 30.0f;
	CornToCornmeal.RequiredTier = EProductionTier::Tier1_RawProcessing;
	CornToCornmeal.RequiredModuleType = EBaseModuleType::Refinery;
	CornToCornmeal.PowerConsumption = 5.0f;
	CornToCornmeal.WorkerRequirement = 1;
	RecipeDatabase.Add(CornToCornmeal);

	// Tomatoes -> Tomato Paste
	FProductionRecipe TomatoesToPaste;
	TomatoesToPaste.RecipeName = TEXT("Tomatoes to Paste");
	TomatoesToPaste.InputResources.Add(FName("Tomatoes"), 15);
	TomatoesToPaste.OutputResources.Add(FName("TomatoPaste"), 10);
	TomatoesToPaste.ProductionTime = 40.0f;
	TomatoesToPaste.RequiredTier = EProductionTier::Tier1_RawProcessing;
	TomatoesToPaste.RequiredModuleType = EBaseModuleType::Refinery;
	TomatoesToPaste.PowerConsumption = 8.0f;
	TomatoesToPaste.WorkerRequirement = 1;
	RecipeDatabase.Add(TomatoesToPaste);

	// Medical Herbs -> Medical Extract
	FProductionRecipe HerbsToExtract;
	HerbsToExtract.RecipeName = TEXT("Herbs to Medical Extract");
	HerbsToExtract.InputResources.Add(FName("MedicalHerbs"), 8);
	HerbsToExtract.OutputResources.Add(FName("MedicalExtract"), 5);
	HerbsToExtract.ProductionTime = 60.0f;
	HerbsToExtract.RequiredTier = EProductionTier::Tier1_RawProcessing;
	HerbsToExtract.RequiredModuleType = EBaseModuleType::Laboratory;
	HerbsToExtract.PowerConsumption = 15.0f;
	HerbsToExtract.WorkerRequirement = 2;
	RecipeDatabase.Add(HerbsToExtract);

	// Cotton -> Textiles
	FProductionRecipe CottonToTextiles;
	CottonToTextiles.RecipeName = TEXT("Cotton to Textiles");
	CottonToTextiles.InputResources.Add(FName("Cotton"), 12);
	CottonToTextiles.OutputResources.Add(FName("Textiles"), 8);
	CottonToTextiles.ProductionTime = 45.0f;
	CottonToTextiles.RequiredTier = EProductionTier::Tier1_RawProcessing;
	CottonToTextiles.RequiredModuleType = EBaseModuleType::Factory;
	CottonToTextiles.PowerConsumption = 10.0f;
	CottonToTextiles.WorkerRequirement = 2;
	RecipeDatabase.Add(CottonToTextiles);

	// Biofuel Crops -> Biofuel
	FProductionRecipe CropsToBiofuel;
	CropsToBiofuel.RecipeName = TEXT("Crops to Biofuel");
	CropsToBiofuel.InputResources.Add(FName("BiofuelCrops"), 20);
	CropsToBiofuel.OutputResources.Add(FName("Biofuel"), 15);
	CropsToBiofuel.ProductionTime = 50.0f;
	CropsToBiofuel.RequiredTier = EProductionTier::Tier1_RawProcessing;
	CropsToBiofuel.RequiredModuleType = EBaseModuleType::Refinery;
	CropsToBiofuel.PowerConsumption = 12.0f;
	CropsToBiofuel.WorkerRequirement = 1;
	RecipeDatabase.Add(CropsToBiofuel);

	// === TIER 2: COMPONENT MANUFACTURING ===

	// Flour + Cornmeal -> Rations
	FProductionRecipe MakeRations;
	MakeRations.RecipeName = TEXT("Make Rations");
	MakeRations.InputResources.Add(FName("Flour"), 5);
	MakeRations.InputResources.Add(FName("Cornmeal"), 5);
	MakeRations.OutputResources.Add(FName("Rations"), 12);
	MakeRations.ProductionTime = 40.0f;
	MakeRations.RequiredTier = EProductionTier::Tier2_ComponentManufacturing;
	MakeRations.RequiredModuleType = EBaseModuleType::Factory;
	MakeRations.PowerConsumption = 10.0f;
	MakeRations.WorkerRequirement = 2;
	RecipeDatabase.Add(MakeRations);

	// Medical Extract -> Pharmaceuticals
	FProductionRecipe ExtractToPharma;
	ExtractToPharma.RecipeName = TEXT("Extract to Pharmaceuticals");
	ExtractToPharma.InputResources.Add(FName("MedicalExtract"), 5);
	ExtractToPharma.OutputResources.Add(FName("Pharmaceuticals"), 3);
	ExtractToPharma.ProductionTime = 90.0f;
	ExtractToPharma.RequiredTier = EProductionTier::Tier2_ComponentManufacturing;
	ExtractToPharma.RequiredModuleType = EBaseModuleType::Laboratory;
	ExtractToPharma.PowerConsumption = 25.0f;
	ExtractToPharma.WorkerRequirement = 3;
	RecipeDatabase.Add(ExtractToPharma);

	// Textiles + Biofuel -> Advanced Components
	FProductionRecipe MakeComponents;
	MakeComponents.RecipeName = TEXT("Make Advanced Components");
	MakeComponents.InputResources.Add(FName("Textiles"), 8);
	MakeComponents.InputResources.Add(FName("Biofuel"), 10);
	MakeComponents.OutputResources.Add(FName("AdvancedComponents"), 5);
	MakeComponents.ProductionTime = 120.0f;
	MakeComponents.RequiredTier = EProductionTier::Tier2_ComponentManufacturing;
	MakeComponents.RequiredModuleType = EBaseModuleType::Factory;
	MakeComponents.PowerConsumption = 30.0f;
	MakeComponents.WorkerRequirement = 3;
	RecipeDatabase.Add(MakeComponents);

	// Steel + Electronics -> Components
	FProductionRecipe MakeBasicComponents;
	MakeBasicComponents.RecipeName = TEXT("Make Basic Components");
	MakeBasicComponents.InputResources.Add(FName("Steel"), 15);
	MakeBasicComponents.InputResources.Add(FName("Electronics"), 8);
	MakeBasicComponents.OutputResources.Add(FName("Components"), 10);
	MakeBasicComponents.ProductionTime = 80.0f;
	MakeBasicComponents.RequiredTier = EProductionTier::Tier2_ComponentManufacturing;
	MakeBasicComponents.RequiredModuleType = EBaseModuleType::Workshop;
	MakeBasicComponents.PowerConsumption = 20.0f;
	MakeBasicComponents.WorkerRequirement = 2;
	RecipeDatabase.Add(MakeBasicComponents);

	// === TIER 3: ADVANCED PRODUCTS ===

	// Components + Advanced Components -> Ship Parts
	FProductionRecipe MakeShipParts;
	MakeShipParts.RecipeName = TEXT("Make Ship Parts");
	MakeShipParts.InputResources.Add(FName("Components"), 10);
	MakeShipParts.InputResources.Add(FName("AdvancedComponents"), 5);
	MakeShipParts.OutputResources.Add(FName("ShipParts"), 3);
	MakeShipParts.ProductionTime = 180.0f;
	MakeShipParts.RequiredTier = EProductionTier::Tier3_AdvancedProducts;
	MakeShipParts.RequiredModuleType = EBaseModuleType::Factory;
	MakeShipParts.PowerConsumption = 50.0f;
	MakeShipParts.WorkerRequirement = 4;
	RecipeDatabase.Add(MakeShipParts);

	// Components + Steel -> Weapons
	FProductionRecipe MakeWeapons;
	MakeWeapons.RecipeName = TEXT("Make Weapons");
	MakeWeapons.InputResources.Add(FName("Components"), 8);
	MakeWeapons.InputResources.Add(FName("Steel"), 20);
	MakeWeapons.OutputResources.Add(FName("Weapons"), 2);
	MakeWeapons.ProductionTime = 150.0f;
	MakeWeapons.RequiredTier = EProductionTier::Tier3_AdvancedProducts;
	MakeWeapons.RequiredModuleType = EBaseModuleType::Factory;
	MakeWeapons.PowerConsumption = 40.0f;
	MakeWeapons.WorkerRequirement = 3;
	RecipeDatabase.Add(MakeWeapons);

	// Electronics + Advanced Components -> Electronics
	FProductionRecipe MakeElectronics;
	MakeElectronics.RecipeName = TEXT("Make Electronics");
	MakeElectronics.InputResources.Add(FName("Electronics"), 5);
	MakeElectronics.InputResources.Add(FName("AdvancedComponents"), 3);
	MakeElectronics.OutputResources.Add(FName("HighTechElectronics"), 4);
	MakeElectronics.ProductionTime = 100.0f;
	MakeElectronics.RequiredTier = EProductionTier::Tier3_AdvancedProducts;
	MakeElectronics.RequiredModuleType = EBaseModuleType::Workshop;
	MakeElectronics.PowerConsumption = 35.0f;
	MakeElectronics.WorkerRequirement = 3;
	RecipeDatabase.Add(MakeElectronics);

	// === TIER 4: SPECIALIZED ===

	// Pharmaceuticals + HighTechElectronics -> Medical Equipment
	FProductionRecipe MakeMedicalEquipment;
	MakeMedicalEquipment.RecipeName = TEXT("Make Medical Equipment");
	MakeMedicalEquipment.InputResources.Add(FName("Pharmaceuticals"), 3);
	MakeMedicalEquipment.InputResources.Add(FName("HighTechElectronics"), 2);
	MakeMedicalEquipment.OutputResources.Add(FName("MedicalEquipment"), 1);
	MakeMedicalEquipment.ProductionTime = 200.0f;
	MakeMedicalEquipment.RequiredTier = EProductionTier::Tier4_Specialized;
	MakeMedicalEquipment.RequiredModuleType = EBaseModuleType::Laboratory;
	MakeMedicalEquipment.PowerConsumption = 60.0f;
	MakeMedicalEquipment.WorkerRequirement = 5;
	RecipeDatabase.Add(MakeMedicalEquipment);

	// Ship Parts + HighTechElectronics -> Advanced Ship Systems
	FProductionRecipe MakeAdvancedShipSystems;
	MakeAdvancedShipSystems.RecipeName = TEXT("Make Advanced Ship Systems");
	MakeAdvancedShipSystems.InputResources.Add(FName("ShipParts"), 2);
	MakeAdvancedShipSystems.InputResources.Add(FName("HighTechElectronics"), 3);
	MakeAdvancedShipSystems.OutputResources.Add(FName("AdvancedShipSystems"), 1);
	MakeAdvancedShipSystems.ProductionTime = 250.0f;
	MakeAdvancedShipSystems.RequiredTier = EProductionTier::Tier4_Specialized;
	MakeAdvancedShipSystems.RequiredModuleType = EBaseModuleType::Factory;
	MakeAdvancedShipSystems.PowerConsumption = 80.0f;
	MakeAdvancedShipSystems.WorkerRequirement = 6;
	RecipeDatabase.Add(MakeAdvancedShipSystems);

	UE_LOG(LogTemp, Log, TEXT("ProductionChainManager: Initialized %d recipes across %d tiers"), 
		RecipeDatabase.Num(), 4);
}

bool AProductionChainManager::CreateProductionChain(EProductionChainType ChainType, const FGuid& StartModuleID)
{
	// This would create a complete production chain based on the chain type
	// For now, just log the attempt
	UE_LOG(LogTemp, Log, TEXT("Creating production chain: %s starting at module %s"), 
		*UEnum::GetDisplayValueAsText(ChainType).ToString(), *StartModuleID.ToString());

	OnProductionChainCreated.Broadcast(ChainType);
	return true;
}

FGuid AProductionChainManager::AddProductionModule(EBaseModuleType ModuleType, FVector Location, const FGuid& ClaimID)
{
	if (!BaseBuildingManager.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("No base building manager available"));
		return FGuid();
	}

	// Create module through base building manager
	FGuid GridPos = FGuid::NewGuid(); // Simplified - would convert location to grid position
	// FIntPoint GridPosition = LocationToGridPosition(Location); // Would implement this
	FIntPoint GridPosition(0, 0); // Placeholder

	FGuid ModuleID = BaseBuildingManager->CreateModule(ModuleType, GridPosition, ClaimID);
	
	if (ModuleID.IsValid())
	{
		// Add to production modules
		FProductionModuleData ProductionModule;
		ProductionModule.ModuleID = ModuleID;
		ProductionModule.ModuleType = ModuleType;
		ProductionModule.MaxStorageCapacity = 1000;
		ProductionModule.Efficiency = 1.0f;

		ProductionModules.Add(ProductionModule);
		ProductionModuleIndexMap.Add(ModuleID, ProductionModules.Num() - 1);

		UE_LOG(LogTemp, Log, TEXT("Production module added: %s"), *UEnum::GetDisplayValueAsText(ModuleType).ToString());
	}

	return ModuleID;
}

bool AProductionChainManager::SetModuleRecipe(const FGuid& ModuleID, const FProductionRecipe& Recipe)
{
	int32 ModuleIndex = FindProductionModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FProductionModuleData& Module = ProductionModules[ModuleIndex];

	// Check if module can produce this recipe
	if (!CanModuleProduceRecipe(Module, Recipe))
	{
		UE_LOG(LogTemp, Warning, TEXT("Module cannot produce this recipe"));
		return false;
	}

	Module.CurrentRecipe = Recipe;
	UE_LOG(LogTemp, Log, TEXT("Recipe set for module: %s"), *Recipe.RecipeName);
	return true;
}

bool AProductionChainManager::StartProduction(const FGuid& ModuleID)
{
	int32 ModuleIndex = FindProductionModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FProductionModuleData& Module = ProductionModules[ModuleIndex];

	// Check if recipe is set
	if (Module.CurrentRecipe.RecipeName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No recipe set for module"));
		return false;
	}

	// Check if already producing
	if (Module.bIsProducing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Module is already producing"));
		return false;
	}

	// Check resource availability
	if (!CheckResourceAvailability(Module.CurrentRecipe))
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient resources for production"));
		return false;
	}

	// Check power availability
	if (BaseBuildingManager.IsValid())
	{
		float CurrentPower = BaseBuildingManager->GetCurrentPower();
		if (CurrentPower < Module.CurrentRecipe.PowerConsumption)
		{
			UE_LOG(LogTemp, Warning, TEXT("Insufficient power for production"));
			return false;
		}
	}

	// Start production
	Module.bIsProducing = true;
	Module.ProductionProgress = 0.0f;
	Module.bIsOperational = true;

	// Consume input resources
	for (const auto& Input : Module.CurrentRecipe.InputResources)
	{
		Module.InputStorage[Input.Key] = Module.InputStorage.FindRef(Input.Key) - Input.Value;
	}

	OnProductionStarted.Broadcast(Module);
	UE_LOG(LogTemp, Log, TEXT("Production started: %s"), *Module.CurrentRecipe.RecipeName);
	return true;
}

bool AProductionChainManager::StopProduction(const FGuid& ModuleID)
{
	int32 ModuleIndex = FindProductionModuleIndex(ModuleID);
	if (ModuleIndex == INDEX_NONE)
	{
		return false;
	}

	FProductionModuleData& Module = ProductionModules[ModuleIndex];

	if (!Module.bIsProducing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Module is not producing"));
		return false;
	}

	Module.bIsProducing = false;
	Module.ProductionProgress = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Production stopped: %s"), *Module.CurrentRecipe.RecipeName);
	return true;
}

bool AProductionChainManager::RouteResources(const FGuid& SourceModuleID, const FGuid& DestinationModuleID, FName ResourceType, float TransportRate)
{
	// Create routing path
	FResourceRoutingPath NewPath;
	NewPath.SourceModuleID = SourceModuleID;
	NewPath.DestinationModuleID = DestinationModuleID;
	NewPath.ResourceType = ResourceType;
	NewPath.TransportRate = TransportRate;
	NewPath.bIsActive = true;

	RoutingPaths.Add(NewPath);

	OnResourceRouted.Broadcast(NewPath);
	UE_LOG(LogTemp, Log, TEXT("Resource routing established: %s from %s to %s at rate %.1f"), 
		*ResourceType.ToString(), *SourceModuleID.ToString(), *DestinationModuleID.ToString(), TransportRate);
	return true;
}

bool AProductionChainManager::AutoRouteResources(const FGuid& StartModuleID)
{
	if (!bAutoRoutingEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Auto-routing is disabled"));
		return false;
	}

	// This would automatically find and create routes between modules
	// For now, just log the attempt
	UE_LOG(LogTemp, Log, TEXT("Auto-routing resources for production chain starting at %s"), *StartModuleID.ToString());
	return true;
}

FProductionModuleData AProductionChainManager::GetProductionModule(const FGuid& ModuleID) const
{
	const int32* IndexPtr = ProductionModuleIndexMap.Find(ModuleID);
	if (IndexPtr)
	{
		return ProductionModules[*IndexPtr];
	}
	return FProductionModuleData();
}

TArray<FProductionModuleData> AProductionChainManager::GetAllProductionModules() const
{
	return ProductionModules;
}

FProductionStatistics AProductionChainManager::GetProductionStatistics() const
{
	FProductionStatistics Stats;
	Stats.ActiveModules = ProductionModules.Num();

	float TotalProductionRate = 0.0f;
	float TotalEfficiency = 0.0f;

	for (const FProductionModuleData& Module : ProductionModules)
	{
		if (Module.bIsProducing)
		{
			Stats.ProducingModules++;
			TotalProductionRate += 1.0f / Module.CurrentRecipe.ProductionTime;
		}

		if (Module.CurrentRecipe.RecipeName.IsEmpty())
		{
			continue;
		}

		// Track inputs and outputs
		for (const auto& Input : Module.CurrentRecipe.InputResources)
		{
			int32 CurrentCount = Stats.TotalInputsConsumed.FindRef(Input.Key);
			Stats.TotalInputsConsumed.Add(Input.Key, CurrentCount + Input.Value);
		}

		for (const auto& Output : Module.CurrentRecipe.OutputResources)
		{
			int32 CurrentCount = Stats.TotalOutputsProduced.FindRef(Output.Key);
			Stats.TotalOutputsProduced.Add(Output.Key, CurrentCount + Output.Value);
		}

		TotalEfficiency += Module.Efficiency;
	}

	Stats.CurrentProductionRate = TotalProductionRate;
	Stats.EfficiencyRating = Stats.ActiveModules > 0 ? TotalEfficiency / Stats.ActiveModules : 0.0f;

	// Calculate production value per hour
	Stats.ProductionValuePerHour = 0.0f;
	for (const auto& Output : Stats.TotalOutputsProduced)
	{
		float ResourceValue = ResourceValues.FindRef(Output.Key);
		Stats.ProductionValuePerHour += Output.Value * ResourceValue * 6.0f; // Convert to hourly
	}

	return Stats;
}

FProductionRecipe AProductionChainManager::FindOptimalRecipe(FName DesiredOutput) const
{
	// Find recipes that produce the desired output
	TArray<FProductionRecipe> MatchingRecipes;
	for (const FProductionRecipe& Recipe : RecipeDatabase)
	{
		if (Recipe.OutputResources.Contains(DesiredOutput))
		{
			MatchingRecipes.Add(Recipe);
		}
	}

	// Return the most efficient recipe
	FProductionRecipe BestRecipe;
	float BestEfficiency = 0.0f;

	for (const FProductionRecipe& Recipe : MatchingRecipes)
	{
		if (Recipe.Efficiency > BestEfficiency)
		{
			BestEfficiency = Recipe.Efficiency;
			BestRecipe = Recipe;
		}
	}

	return BestRecipe;
}

bool AProductionChainManager::CheckResourceAvailability(const FProductionRecipe& Recipe) const
{
	// This would check if required resources are available in storage
	// For now, assume resources are available
	return true;
}

TMap<FName, int32> AProductionChainManager::GetBottleneckAnalysis() const
{
	TMap<FName, int32> Bottlenecks;

	// Analyze which resources are limiting production
	for (const FProductionModuleData& Module : ProductionModules)
	{
		if (!Module.bIsProducing && !Module.CurrentRecipe.RecipeName.IsEmpty())
		{
			// Check which inputs are missing
			for (const auto& Input : Module.CurrentRecipe.InputResources)
			{
				int32 StoredAmount = Module.InputStorage.FindRef(Input.Key);
				if (StoredAmount < Input.Value)
				{
					int32 MissingAmount = Input.Value - StoredAmount;
					int32 CurrentBottleneck = Bottlenecks.FindRef(Input.Key);
					Bottlenecks.Add(Input.Key, CurrentBottleneck + MissingAmount);
				}
			}
		}
	}

	return Bottlenecks;
}

bool AProductionChainManager::OptimizeProductionChain(const FGuid& StartModuleID)
{
	// This would analyze the production chain and suggest optimizations
	// For now, just log the attempt
	UE_LOG(LogTemp, Log, TEXT("Optimizing production chain starting at %s"), *StartModuleID.ToString());
	return true;
}

void AProductionChainManager::UpdateProduction(float DeltaTime)
{
	for (FProductionModuleData& Module : ProductionModules)
	{
		if (!Module.bIsProducing || Module.CurrentRecipe.RecipeName.IsEmpty())
		{
			continue;
		}

		// Update production progress
		float ProgressDelta = (DeltaTime / Module.CurrentRecipe.ProductionTime) * Module.ProductionRate * Module.Efficiency;
		Module.ProductionProgress = FMath::Clamp(Module.ProductionProgress + ProgressDelta, 0.0f, 1.0f);

		// Check if production is complete
		if (Module.ProductionProgress >= 1.0f)
		{
			// Production complete - add outputs to storage
			for (const auto& Output : Module.CurrentRecipe.OutputResources)
			{
				int32 CurrentAmount = Module.OutputStorage.FindRef(Output.Key);
				Module.OutputStorage.Add(Output.Key, CurrentAmount + Output.Value);
			}

			// Add byproducts if any
			for (const auto& Byproduct : Module.CurrentRecipe.Byproducts)
			{
				int32 CurrentAmount = Module.OutputStorage.FindRef(Byproduct.Key);
				Module.OutputStorage.Add(Byproduct.Key, CurrentAmount + Byproduct.Value);
			}

			// Reset progress
			Module.ProductionProgress = 0.0f;

			// Check if we should continue production
			if (CheckResourceAvailability(Module.CurrentRecipe))
			{
				// Consume inputs for next cycle
				for (const auto& Input : Module.CurrentRecipe.InputResources)
				{
					Module.InputStorage[Input.Key] = Module.InputStorage.FindRef(Input.Key) - Input.Value;
				}
			}
			else
			{
				// Stop production due to lack of resources
				Module.bIsProducing = false;
				UE_LOG(LogTemp, Warning, TEXT("Production stopped due to insufficient resources: %s"), *Module.CurrentRecipe.RecipeName);
			}

			// Update statistics
			ProductionStats.TotalCyclesCompleted++;

			// Fire event
			OnProductionCompleted.Broadcast(Module);
		}
	}
}

void AProductionChainManager::UpdateResourceRouting(float DeltaTime)
{
	for (FResourceRoutingPath& Route : RoutingPaths)
	{
		if (!Route.bIsActive)
		{
			continue;
		}

		// Find source and destination modules
		int32 SourceIndex = FindProductionModuleIndex(Route.SourceModuleID);
		int32 DestIndex = FindProductionModuleIndex(Route.DestinationModuleID);

		if (SourceIndex == INDEX_NONE || DestIndex == INDEX_NONE)
		{
			continue;
		}

		FProductionModuleData& SourceModule = ProductionModules[SourceIndex];
		FProductionModuleData& DestModule = ProductionModules[DestIndex];

		// Transfer resources
		int32 TransferAmount = FMath::FloorToInt(Route.TransportRate * DeltaTime);
		int32 SourceAmount = SourceModule.OutputStorage.FindRef(Route.ResourceType);

		if (SourceAmount >= TransferAmount)
		{
			// Check destination capacity
			int32 DestCurrentAmount = DestModule.InputStorage.FindRef(Route.ResourceType);
			if (DestCurrentAmount + TransferAmount <= DestModule.MaxStorageCapacity)
			{
				// Perform transfer
				SourceModule.OutputStorage[Route.ResourceType] = SourceAmount - TransferAmount;
				DestModule.InputStorage[Route.ResourceType] = DestCurrentAmount + TransferAmount;
			}
		}
	}
}

int32 AProductionChainManager::FindProductionModuleIndex(const FGuid& ModuleID) const
{
	const int32* IndexPtr = ProductionModuleIndexMap.Find(ModuleID);
	return IndexPtr ? *IndexPtr : INDEX_NONE;
}

bool AProductionChainManager::CanModuleProduceRecipe(const FProductionModuleData& Module, const FProductionRecipe& Recipe) const
{
	// Check if module type matches recipe requirement
	if (Module.ModuleType != Recipe.RequiredModuleType)
	{
		return false;
	}

	// Check if module is operational
	if (!Module.bIsOperational)
	{
		return false;
	}

	return true;
}

bool AProductionChainManager::TransferResources(const FGuid& SourceID, const FGuid& DestID, FName ResourceType, int32 Amount)
{
	int32 SourceIndex = FindProductionModuleIndex(SourceID);
	int32 DestIndex = FindProductionModuleIndex(DestID);

	if (SourceIndex == INDEX_NONE || DestIndex == INDEX_NONE)
	{
		return false;
	}

	FProductionModuleData& SourceModule = ProductionModules[SourceIndex];
	FProductionModuleData& DestModule = ProductionModules[DestIndex];

	int32 SourceAmount = SourceModule.OutputStorage.FindRef(ResourceType);
	if (SourceAmount < Amount)
	{
		return false;
	}

	int32 DestCurrentAmount = DestModule.InputStorage.FindRef(ResourceType);
	if (DestCurrentAmount + Amount > DestModule.MaxStorageCapacity)
	{
		return false;
	}

	// Perform transfer
	SourceModule.OutputStorage[ResourceType] = SourceAmount - Amount;
	DestModule.InputStorage[ResourceType] = DestCurrentAmount + Amount;

	return true;
}

FGuid AProductionChainManager::FindStorageForResource(FName ResourceType) const
{
	// Find a storage module that can accept this resource
	for (const FProductionModuleData& Module : ProductionModules)
	{
		if (Module.ModuleType == EBaseModuleType::RawMaterialsStorage ||
			Module.ModuleType == EBaseModuleType::ProcessedGoodsStorage)
		{
			// Check if module has capacity
			int32 CurrentStorage = 0;
			for (const auto& Storage : Module.InputStorage)
			{
				CurrentStorage += Storage.Value;
			}
			for (const auto& Storage : Module.OutputStorage)
			{
				CurrentStorage += Storage.Value;
			}

			if (CurrentStorage < Module.MaxStorageCapacity)
			{
				return Module.ModuleID;
			}
		}
	}

	return FGuid();
}

float AProductionChainManager::CalculateRecipeValue(const FProductionRecipe& Recipe) const
{
	float InputValue = 0.0f;
	float OutputValue = 0.0f;

	// Calculate input value
	for (const auto& Input : Recipe.InputResources)
	{
		float ResourceValue = ResourceValues.FindRef(Input.Key);
		InputValue += ResourceValue * Input.Value;
	}

	// Calculate output value
	for (const auto& Output : Recipe.OutputResources)
	{
		float ResourceValue = ResourceValues.FindRef(Output.Key);
		OutputValue += ResourceValue * Output.Value;
	}

	// Recipe value is profit margin
	return OutputValue - InputValue;
}

float AProductionChainManager::GetModuleEfficiency(const FProductionModuleData& Module) const
{
	float Efficiency = Module.Efficiency;

	// Check power availability
	if (BaseBuildingManager.IsValid())
	{
		float CurrentPower = BaseBuildingManager->GetCurrentPower();
		if (CurrentPower < 0.0f)
		{
			// Power deficit reduces efficiency
			Efficiency *= 0.5f;
		}
	}

	// Check for bottlenecks
	TMap<FName, int32> Bottlenecks = GetBottleneckAnalysis();
	if (Bottlenecks.Num() > 0)
	{
		Efficiency *= 0.8f; // Bottlenecks reduce efficiency
	}

	return FMath::Clamp(Efficiency, 0.1f, 2.0f);
}