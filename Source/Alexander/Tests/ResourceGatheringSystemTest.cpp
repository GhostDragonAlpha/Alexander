// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Engine.h"
#include "ResourceGatheringSystem.h"
#include "AsteroidMiningSystem.h"
#include "PlanetaryMiningSystem.h"
#include "RefiningSystem.h"
#include "CraftingSystem.h"
#include "Asteroid.h"
#include "Planet.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

/**
 * Test suite for Resource Gathering & Crafting System
 * This test suite verifies all components work correctly and integrate properly
 */
class FResourceGatheringSystemTest
{
public:
	// Test resource definitions
	static void TestResourceDefinitions()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Resource Definitions ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		UResourceGatheringSystem* ResourceSystem = World->GetSubsystem<UResourceGatheringSystem>();
		if (!ResourceSystem)
		{
			UE_LOG(LogTemp, Error, TEXT("ResourceGatheringSystem not available"));
			return;
		}

		// Test 1: Check default resources are registered
		TArray<FResourceDefinition> AllResources = ResourceSystem->GetAllResourceDefinitions();
		UE_LOG(LogTemp, Log, TEXT("Total resources registered: %d"), AllResources.Num());

		if (AllResources.Num() < 20) // Should have many default resources
		{
			UE_LOG(LogTemp, Error, TEXT("Insufficient resources registered: %d"), AllResources.Num());
		}

		// Test 2: Get specific resource
		FResourceDefinition IronDef = ResourceSystem->GetResourceDefinition(FName("Iron"));
		if (IronDef.ResourceID.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("Iron resource not found"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Iron resource found: %s (Value: %.1f, Weight: %.1f)"), 
				   *IronDef.DisplayName, IronDef.BaseValue, IronDef.WeightPerUnit);
		}

		// Test 3: Get resources by category
		TArray<FResourceDefinition> MetallicResources = ResourceSystem->GetResourcesByCategory(EResourceCategory::Metallic);
		UE_LOG(LogTemp, Log, TEXT("Metallic resources: %d"), MetallicResources.Num());

		TArray<FResourceDefinition> ExoticResources = ResourceSystem->GetResourcesByCategory(EResourceCategory::Exotic);
		UE_LOG(LogTemp, Log, TEXT("Exotic resources: %d"), ExoticResources.Num());

		UE_LOG(LogTemp, Log, TEXT("=== Resource Definitions Test Complete ==="));
	}

	// Test asteroid mining
	static void TestAsteroidMining()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Asteroid Mining ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		UResourceGatheringSystem* ResourceSystem = World->GetSubsystem<UResourceGatheringSystem>();
		UAsteroidMiningSystem* MiningSystem = World->GetSubsystem<UAsteroidMiningSystem>();

		if (!ResourceSystem || !MiningSystem)
		{
			UE_LOG(LogTemp, Error, TEXT("Required systems not available"));
			return;
		}

		// Create test asteroid
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AAsteroid* TestAsteroid = World->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		if (!TestAsteroid)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn test asteroid"));
			return;
		}

		TestAsteroid->AsteroidID = 999;
		TestAsteroid->AsteroidRadius = 50.0f;

		// Generate resources for asteroid
		ResourceSystem->GenerateAsteroidResources(TestAsteroid);

		// Test scanning
		FAsteroidScanResult ScanResult = MiningSystem->ScanAsteroid(TestAsteroid, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Asteroid scan: %d resources detected (Success: %s)"), 
			   ScanResult.DetectedResources.Num(), ScanResult.bSuccess ? TEXT("true") : TEXT("false"));

		if (ScanResult.DetectedResources.Num() > 0)
		{
			// Test mining
			bool bMiningStarted = MiningSystem->StartMining(TestAsteroid, EMiningLaserType::Basic);
			UE_LOG(LogTemp, Log, TEXT("Mining operation started: %s"), bMiningStarted ? TEXT("true") : TEXT("false"));

			// Wait for mining to complete (simulate)
			float MiningDuration = MiningSystem->GetMiningProgress();
			UE_LOG(LogTemp, Log, TEXT("Mining progress: %.2f"), MiningDuration);
		}

		// Cleanup
		TestAsteroid->Destroy();

		UE_LOG(LogTemp, Log, TEXT("=== Asteroid Mining Test Complete ==="));
	}

	// Test planetary mining
	static void TestPlanetaryMining()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Planetary Mining ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		UResourceGatheringSystem* ResourceSystem = World->GetSubsystem<UResourceGatheringSystem>();
		UPlanetaryMiningSystem* PlanetarySystem = World->GetSubsystem<UPlanetaryMiningSystem>();

		if (!ResourceSystem || !PlanetarySystem)
		{
			UE_LOG(LogTemp, Error, TEXT("Required systems not available"));
			return;
		}

		// Create test planet
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		APlanet* TestPlanet = World->SpawnActor<APlanet>(APlanet::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		if (!TestPlanet)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn test planet"));
			return;
		}

		TestPlanet->PlanetRadius = 100.0f; // 100km radius

		// Generate planetary resources
		ResourceSystem->GeneratePlanetaryResources(TestPlanet);

		// Test scanning
		FVector TestLocation = FVector(100000.0f, 0.0f, 0.0f); // 100km from center
		FPlanetaryScanResult ScanResult = PlanetarySystem->ScanPlanetaryLocation(TestPlanet, TestLocation, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("Planetary scan: %d resources detected (Success: %s, Impact: %s)"),
			   ScanResult.DetectedResources.Num(), ScanResult.bSuccess ? TEXT("true") : TEXT("false"),
			   *UEnum::GetValueAsString(ScanResult.EnvironmentalImpact));

		// Test mining permit
		FMiningPermit Permit = PlanetarySystem->ApplyForMiningPermit(TestPlanet, 1000, EEnvironmentalImpact::Moderate);
		UE_LOG(LogTemp, Log, TEXT("Mining permit: %s (Status: %s)"),
			   Permit.Status == EPermitStatus::Granted ? TEXT("Granted") : TEXT("Denied"),
			   *UEnum::GetValueAsString(Permit.Status));

		if (ScanResult.DetectedResources.Num() > 0 && Permit.Status == EPermitStatus::Granted)
		{
			// Test mining
			FPlanetaryMiningParams Params;
			Params.EquipmentType = EPlanetaryMiningEquipment::SurfaceMiner;
			Params.MiningPower = 1.0f;
			Params.MiningEfficiency = 1.0f;
			Params.EnvironmentalImpactMultiplier = 1.0f;

			FPlanetaryMiningResult MiningResult = PlanetarySystem->MinePlanetaryLocation(TestPlanet, TestLocation, Params);
			UE_LOG(LogTemp, Log, TEXT("Planetary mining: %s (Resources: %d, Value: %.1f, Impact: %s)"),
				   MiningResult.bSuccess ? TEXT("true") : TEXT("false"),
				   MiningResult.ResourcesExtracted.Num(),
				   MiningResult.TotalValue,
				   *UEnum::GetValueAsString(MiningResult.EnvironmentalImpact));
		}

		// Cleanup
		TestPlanet->Destroy();

		UE_LOG(LogTemp, Log, TEXT("=== Planetary Mining Test Complete ==="));
	}

	// Test refining system
	static void TestRefiningSystem()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Refining System ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		URefiningSystem* RefiningSystem = World->GetSubsystem<URefiningSystem>();
		if (!RefiningSystem)
		{
			UE_LOG(LogTemp, Error, TEXT("RefiningSystem not available"));
			return;
		}

		// Test 1: Check default recipes
		TArray<FRefiningRecipe> AllRecipes = RefiningSystem->GetAllRecipes();
		UE_LOG(LogTemp, Log, TEXT("Total refining recipes: %d"), AllRecipes.Num());

		if (AllRecipes.Num() < 10) // Should have many default recipes
		{
			UE_LOG(LogTemp, Error, TEXT("Insufficient recipes: %d"), AllRecipes.Num());
		}

		// Test 2: Get specific recipe
		FRefiningRecipe IronIngotsRecipe = RefiningSystem->GetRecipe(FName("IronIngots"));
		if (!IronIngotsRecipe.bIsUnlocked)
		{
			UE_LOG(LogTemp, Error, TEXT("Iron ingots recipe not found or not unlocked"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Iron ingots recipe: %s (Inputs: %d, Outputs: %d)"), 
				   *IronIngotsRecipe.DisplayName, IronIngotsRecipe.InputResources.Num(), IronIngotsRecipe.OutputResources.Num());
		}

		// Test 3: Get recipes by module type
		TArray<FRefiningRecipe> SmelterRecipes = RefiningSystem->GetRecipesByModuleType(ERefineryModuleType::Smelter);
		UE_LOG(LogTemp, Log, TEXT("Smelter recipes: %d"), SmelterRecipes.Num());

		// Test 4: Unlock a recipe
		bool bUnlockResult = RefiningSystem->UnlockRecipe(FName("SteelIngots"));
		UE_LOG(LogTemp, Log, TEXT("Unlock steel recipe: %s"), bUnlockResult ? TEXT("true") : TEXT("false"));

		UE_LOG(LogTemp, Log, TEXT("=== Refining System Test Complete ==="));
	}

	// Test crafting system
	static void TestCraftingSystem()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Crafting System ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		UCraftingSystem* CraftingSystem = World->GetSubsystem<UCraftingSystem>();
		if (!CraftingSystem)
		{
			UE_LOG(LogTemp, Error, TEXT("CraftingSystem not available"));
			return;
		}

		// Test 1: Check default blueprints
		TArray<FCraftingBlueprint> AllBlueprints = CraftingSystem->GetAllBlueprints();
		UE_LOG(LogTemp, Log, TEXT("Total crafting blueprints: %d"), AllBlueprints.Num());

		if (AllBlueprints.Num() < 15) // Should have many default blueprints
		{
			UE_LOG(LogTemp, Error, TEXT("Insufficient blueprints: %d"), AllBlueprints.Num());
		}

		// Test 2: Get blueprints by tier
		TArray<FCraftingBlueprint> Tier1Blueprints = CraftingSystem->GetBlueprintsByTier(ECraftingTier::Tier1_Basic);
		UE_LOG(LogTemp, Log, TEXT("Tier 1 blueprints: %d"), Tier1Blueprints.Num());

		TArray<FCraftingBlueprint> Tier4Blueprints = CraftingSystem->GetBlueprintsByTier(ECraftingTier::Tier4_Masterwork);
		UE_LOG(LogTemp, Log, TEXT("Tier 4 blueprints: %d"), Tier4Blueprints.Num());

		// Test 3: Get blueprints by category
		TArray<FCraftingBlueprint> ShipComponents = CraftingSystem->GetBlueprintsByCategory(ECraftedItemCategory::ShipComponent);
		UE_LOG(LogTemp, Log, TEXT("Ship component blueprints: %d"), ShipComponents.Num());

		TArray<FCraftingBlueprint> Consumables = CraftingSystem->GetBlueprintsByCategory(ECraftedItemCategory::Consumable);
		UE_LOG(LogTemp, Log, TEXT("Consumable blueprints: %d"), Consumables.Num());

		// Test 4: Unlock a blueprint
		bool bUnlockResult = CraftingSystem->UnlockBlueprint(FName("HabitatQuartersModule"));
		UE_LOG(LogTemp, Log, TEXT("Unlock habitat module blueprint: %s"), bUnlockResult ? TEXT("true") : TEXT("false"));

		// Test 5: Test crafting skill
		float InitialSkill = CraftingSystem->GetCraftingSkillLevel();
		UE_LOG(LogTemp, Log, TEXT("Initial crafting skill: %.1f"), InitialSkill);

		CraftingSystem->IncreaseCraftingSkill(10.0f);
		float NewSkill = CraftingSystem->GetCraftingSkillLevel();
		UE_LOG(LogTemp, Log, TEXT("New crafting skill: %.1f"), NewSkill);

		UE_LOG(LogTemp, Log, TEXT("=== Crafting System Test Complete ==="));
	}

	// Test inventory manager
	static void TestInventoryManager()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing Inventory Manager ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		UInventoryManager* InventoryManager = World->GetSubsystem<UInventoryManager>();
		if (!InventoryManager)
		{
			UE_LOG(LogTemp, Error, TEXT("InventoryManager not available"));
			return;
		}

		// Test 1: Create test inventory
		FGuid TestPlayerID = FGuid::NewGuid();
		InventoryManager->InitializeDefaultInventories(TestPlayerID);

		// Test 2: Add resources
		bool bAddResult1 = InventoryManager->AddResource(FName("Iron"), 50, EResourceQuality::Normal, EInventoryType::PersonalInventory, TestPlayerID);
		bool bAddResult2 = InventoryManager->AddResource(FName("Copper"), 30, EResourceQuality::Normal, EInventoryType::PersonalInventory, TestPlayerID);
		bool bAddResult3 = InventoryManager->AddResource(FName("Silicon"), 20, EResourceQuality::Pristine, EInventoryType::PersonalInventory, TestPlayerID);

		UE_LOG(LogTemp, Log, TEXT("Add resources: Iron=%s, Copper=%s, Silicon=%s"), 
			   bAddResult1 ? TEXT("true") : TEXT("false"),
			   bAddResult2 ? TEXT("true") : TEXT("false"),
			   bAddResult3 ? TEXT("true") : TEXT("false"));

		// Test 3: Check resource quantities
		int32 IronQuantity = InventoryManager->GetResourceQuantity(FName("Iron"), EInventoryType::PersonalInventory, TestPlayerID);
		int32 CopperQuantity = InventoryManager->GetResourceQuantity(FName("Copper"), EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Resource quantities: Iron=%d, Copper=%d"), IronQuantity, CopperQuantity);

		// Test 4: Get all resources
		TArray<FResourceQuantity> AllResources = InventoryManager->GetAllResources(EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Total resources in inventory: %d"), AllResources.Num());

		// Test 5: Get inventory value and weight
		float TotalValue = InventoryManager->GetTotalInventoryValue(EInventoryType::PersonalInventory, TestPlayerID);
		float TotalWeight = InventoryManager->GetTotalInventoryWeight(EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Inventory stats: Value=%.1f, Weight=%.1f kg"), TotalValue, TotalWeight);

		// Test 6: Remove resources
		bool bRemoveResult = InventoryManager->RemoveResource(FName("Iron"), 20, EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Remove 20 iron: %s"), bRemoveResult ? TEXT("true") : TEXT("false"));

		int32 RemainingIron = InventoryManager->GetResourceQuantity(FName("Iron"), EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Remaining iron: %d"), RemainingIron);

		// Test 7: Create container
		FGuid ContainerID = InventoryManager->CreateContainer(EContainerType::Crate, TEXT("Test Crate"), 100.0f, 5.0f, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Created container: %s"), *ContainerID.ToString());

		// Test 8: Add resources to container
		bool bAddToContainer = InventoryManager->AddResourceToContainer(FName("Iron"), 25, EResourceQuality::Normal, ContainerID);
		UE_LOG(LogTemp, Log, TEXT("Add iron to container: %s"), bAddToContainer ? TEXT("true") : TEXT("false"));

		// Test 9: Transfer between containers
		FGuid ContainerID2 = InventoryManager->CreateContainer(EContainerType::Crate, TEXT("Test Crate 2"), 100.0f, 5.0f, TestPlayerID);
		FInventoryTransferResult TransferResult = InventoryManager->TransferBetweenContainers(FName("Iron"), 10, ContainerID, ContainerID2);
		UE_LOG(LogTemp, Log, TEXT("Transfer between containers: %s (Amount: %d)"), 
			   TransferResult.bSuccess ? TEXT("true") : TEXT("false"), TransferResult.AmountTransferred);

		// Test 10: Search and filter
		TArray<FResourceQuantity> SearchResults = InventoryManager->SearchResources(TEXT("iron"), EInventoryType::PersonalInventory, TestPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Search results for 'iron': %d"), SearchResults.Num());

		// Test 11: Statistics
		TMap<FName, int32> TotalResources = InventoryManager->GetTotalResourcesAcrossAllInventories();
		float AllValue = InventoryManager->GetTotalValueAcrossAllInventories();
		UE_LOG(LogTemp, Log, TEXT("All resources: %d types, Total value: %.1f"), TotalResources.Num(), AllValue);

		UE_LOG(LogTemp, Log, TEXT("=== Inventory Manager Test Complete ==="));
	}

	// Test system integration
	static void TestSystemIntegration()
	{
		UE_LOG(LogTemp, Log, TEXT("=== Testing System Integration ==="));

		UWorld* World = GEngine->GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No world available for testing"));
			return;
		}

		// Get all systems
		UResourceGatheringSystem* ResourceSystem = World->GetSubsystem<UResourceGatheringSystem>();
		UAsteroidMiningSystem* MiningSystem = World->GetSubsystem<UAsteroidMiningSystem>();
		UPlanetaryMiningSystem* PlanetarySystem = World->GetSubsystem<UPlanetaryMiningSystem>();
		URefiningSystem* RefiningSystem = World->GetSubsystem<URefiningSystem>();
		UCraftingSystem* CraftingSystem = World->GetSubsystem<UCraftingSystem>();
		UInventoryManager* InventoryManager = World->GetSubsystem<UInventoryManager>();

		if (!ResourceSystem || !MiningSystem || !PlanetarySystem || !RefiningSystem || !CraftingSystem || !InventoryManager)
		{
			UE_LOG(LogTemp, Error, TEXT("Not all systems are available"));
			return;
		}

		// Test 1: Full resource gathering and processing chain
		UE_LOG(LogTemp, Log, TEXT("--- Testing Full Resource Chain ---"));

		// Create test asteroid
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AAsteroid* TestAsteroid = World->SpawnActor<AAsteroid>(AAsteroid::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestAsteroid->AsteroidID = 1000;
		TestAsteroid->AsteroidRadius = 75.0f;

		// Generate and mine resources
		ResourceSystem->GenerateAsteroidResources(TestAsteroid);
		
		// Scan and mine
		FAsteroidScanResult ScanResult = MiningSystem->ScanAsteroid(TestAsteroid, 1.5f);
		if (ScanResult.DetectedResources.Num() > 0)
		{
			// Mine resources
			FMiningOperationResult MiningResult;
			if (MiningSystem->StartMining(TestAsteroid, EMiningLaserType::Advanced))
			{
				// Simulate mining completion
				UE_LOG(LogTemp, Log, TEXT("Mining operation in progress..."));
			}
		}

		// Test 2: Refining mined resources
		UE_LOG(LogTemp, Log, TEXT("--- Testing Refining Integration ---"));

		// Unlock and process iron ingots
		RefiningSystem->UnlockRecipe(FName("IronIngots"));
		FRefiningRecipe IronRecipe = RefiningSystem->GetRecipe(FName("IronIngots"));
		UE_LOG(LogTemp, Log, TEXT("Iron ingots recipe: %s (Unlocked: %s)"), 
			   *IronRecipe.DisplayName, IronRecipe.bIsUnlocked ? TEXT("true") : TEXT("false"));

		// Test 3: Crafting refined resources
		UE_LOG(LogTemp, Log, TEXT("--- Testing Crafting Integration ---"));

		// Unlock basic engine blueprint
		CraftingSystem->UnlockBlueprint(FName("BasicEngine"));
		FCraftingBlueprint EngineBlueprint = CraftingSystem->GetBlueprint(FName("BasicEngine"));
		UE_LOG(LogTemp, Log, TEXT("Basic engine blueprint: %s (Unlocked: %s, Tier: %d)"), 
			   *EngineBlueprint.DisplayName, EngineBlueprint.bIsUnlocked ? TEXT("true") : TEXT("false"),
			   (int32)EngineBlueprint.CraftingTier);

		// Test 4: Inventory integration
		UE_LOG(LogTemp, Log, TEXT("--- Testing Inventory Integration ---"));

		FGuid TestPlayerID = FGuid::NewGuid();
		InventoryManager->InitializeDefaultInventories(TestPlayerID);

		// Add mined resources to inventory
		InventoryManager->AddResource(FName("Iron"), 100, EResourceQuality::Normal, EInventoryType::PersonalInventory, TestPlayerID);
		InventoryManager->AddResource(FName("Copper"), 50, EResourceQuality::Normal, EInventoryType::PersonalInventory, TestPlayerID);

		// Check if we have resources for crafting
		bool bHasResources = true;
		for (const FResourceQuantity& RequiredResource : EngineBlueprint.RequiredResources)
		{
			int32 Available = InventoryManager->GetResourceQuantity(RequiredResource.ResourceID, EInventoryType::PersonalInventory, TestPlayerID);
			UE_LOG(LogTemp, Log, TEXT("Resource %s: Required=%d, Available=%d"), 
				   *RequiredResource.ResourceID.ToString(), RequiredResource.Quantity, Available);
			
			if (Available < RequiredResource.Quantity)
			{
				bHasResources = false;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Has resources for engine crafting: %s"), bHasResources ? TEXT("true") : TEXT("false"));

		// Cleanup
		TestAsteroid->Destroy();

		UE_LOG(LogTemp, Log, TEXT("=== System Integration Test Complete ==="));
	}

	// Run all tests
	static void RunAllTests()
	{
		UE_LOG(LogTemp, Log, TEXT("========== RESOURCE GATHERING & CRAFTING SYSTEM TEST SUITE =========="));

		TestResourceDefinitions();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestAsteroidMining();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestPlanetaryMining();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestRefiningSystem();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestCraftingSystem();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestInventoryManager();
		UE_LOG(LogTemp, Log, TEXT(""));

		TestSystemIntegration();
		UE_LOG(LogTemp, Log, TEXT(""));

		UE_LOG(LogTemp, Log, TEXT("========== ALL TESTS COMPLETE =========="));
	}
};

// Export test function for external calling
void RunResourceGatheringSystemTests()
{
	FResourceGatheringSystemTest::RunAllTests();
}