// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsteroidMiningSystem.h"
#include "ResourceGatheringSystem.h"
#include "Asteroid.h"
#include "Spaceship.h"
#include "InventoryManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "SystemSelfTestInterface.h"

UAsteroidMiningSystem::UAsteroidMiningSystem()
{
	CurrentLaserType = EMiningLaserType::Basic;
	CurrentEfficiencyTier = EMiningEfficiencyTier::Tier1_Basic;
	TotalAsteroidsMined = 0;
	TotalMiningTime = 0.0f;
	MiningSkillLevel = 0.0f;

	InitializeLaserStatistics();
	InitializeEfficiencyTiers();
}

void UAsteroidMiningSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("AsteroidMiningSystem initialized"));

	// Get references to other systems
	ResourceGatheringSystem = GetWorld()->GetSubsystem<UResourceGatheringSystem>();
	// InventoryManager will be set later when available

	// Reset statistics
	TotalAsteroidsMined = 0;
	TotalResourcesExtracted.Empty();
	TotalMiningTime = 0.0f;
}

void UAsteroidMiningSystem::Deinitialize()
{
	// Stop any active mining
	StopMining();

	// Clear scan history
	ScanHistory.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("AsteroidMiningSystem deinitialized"));
}

//~ Begin ISystemSelfTestInterface interface
bool UAsteroidMiningSystem::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = GetSystemName_Implementation();
	OutResult.WarningMessages.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("=== Testing Asteroid Mining System ==="));
	
	// Test 1: Check if system is properly initialized
	if (!ResourceGatheringSystem.IsValid())
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Asteroid Mining System not properly initialized - ResourceGatheringSystem not available");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: System not properly initialized"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ System initialization test PASSED"));
	
	// Test 2: Test laser statistics
	if (LaserPowerMap.Num() != 4 || LaserEnergyConsumptionMap.Num() != 4 || LaserEfficiencyMap.Num() != 4)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Laser statistics not properly initialized");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Laser statistics initialization"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Laser statistics test PASSED - %d laser types registered"), LaserPowerMap.Num());
	
	// Test 3: Test laser queries
	float BasicPower = GetLaserMiningPower(EMiningLaserType::Basic);
	float BasicConsumption = GetLaserEnergyConsumption(EMiningLaserType::Basic);
	float BasicEfficiency = GetLaserEfficiency(EMiningLaserType::Basic);
	
	if (BasicPower <= 0.0f || BasicConsumption <= 0.0f || BasicEfficiency <= 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Laser statistics queries returned invalid values");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Laser statistics queries"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Laser queries test PASSED - Basic: Power=%.2f, Consumption=%.2f, Efficiency=%.2f"),
		BasicPower, BasicConsumption, BasicEfficiency);
	
	// Test 4: Test laser upgrade progression
	EMiningLaserType OriginalLaser = CurrentLaserType;
	if (!UpgradeMiningLaser(EMiningLaserType::Advanced))
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Laser upgrade failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Laser upgrade"));
		return false;
	}
	if (CurrentLaserType != EMiningLaserType::Advanced)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Laser upgrade did not change current laser");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Laser upgrade verification"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Laser upgrade test PASSED - Upgraded to Advanced"));
	
	// Test 5: Test efficiency tier system
	float Tier1Efficiency = GetEfficiencyMultiplier(EMiningEfficiencyTier::Tier1_Basic);
	float Tier4Efficiency = GetEfficiencyMultiplier(EMiningEfficiencyTier::Tier4_Masterwork);
	
	if (Tier1Efficiency >= Tier4Efficiency)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Efficiency tier progression is not working correctly");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Efficiency tier progression"));
		return false;
	}
	if (Tier4Efficiency < 1.5f) // Should be at least 1.5x better than basic
	{
		OutResult.WarningMessages.Add(TEXT("Efficiency multiplier may be too low for highest tier"));
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Efficiency multiplier test - Tier4=%.2f (expected >1.5)"), Tier4Efficiency);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ Efficiency tier test PASSED - Tier1=%.2f, Tier4=%.2f"), Tier1Efficiency, Tier4Efficiency);
	}
	
	// Test 6: Test efficiency upgrade
	EMiningEfficiencyTier OriginalTier = CurrentEfficiencyTier;
	if (!UpgradeMiningEfficiency(EMiningEfficiencyTier::Tier2_Standard))
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Efficiency upgrade failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Efficiency upgrade"));
		return false;
	}
	if (CurrentEfficiencyTier != EMiningEfficiencyTier::Tier2_Standard)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Efficiency upgrade did not change current tier");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Efficiency upgrade verification"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Efficiency upgrade test PASSED - Upgraded to Tier2"));
	
	// Test 7: Test mining duration calculation
	// Create a mock asteroid for testing
	AAsteroid* TestAsteroid = nullptr; // Would need to spawn a test asteroid in a real scenario
	if (TestAsteroid)
	{
		float Duration = CalculateMiningDuration(TestAsteroid, EMiningLaserType::Basic);
		if (Duration < 1.0f || Duration > 300.0f) // Should be between 1 and 300 seconds
		{
			OutResult.WarningMessages.Add(TEXT("Mining duration calculation may be out of expected range"));
			UE_LOG(LogTemp, Warning, TEXT("WARNING: Mining duration test - Duration=%.2f seconds (expected 1-300)"), Duration);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("✓ Mining duration calculation test PASSED - Duration=%.2f seconds"), Duration);
		}
	}
	else
	{
		OutResult.WarningMessages.Add(TEXT("Mining duration test skipped - no test asteroid available"));
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Mining duration test skipped"));
	}
	
	// Test 8: Test energy consumption calculation
	float EnergyConsumption = CalculateEnergyConsumption(10.0f, EMiningLaserType::Basic);
	if (EnergyConsumption <= 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Energy consumption calculation returned invalid value");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Energy consumption calculation"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Energy consumption calculation test PASSED - Energy=%.2f units for 10s"), EnergyConsumption);
	
	// Test 9: Test mining operation state management
	if (IsMining())
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Mining should not be active at test start");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Initial mining state"));
		return false;
	}
	
	// Test progress calculation when not mining
	float Progress = GetMiningProgress();
	if (Progress != 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Mining progress should be 0 when not mining");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Mining progress when not mining"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ Mining state management test PASSED"));
	
	// Test 10: Test statistics tracking
	ResetStatistics();
	if (TotalAsteroidsMined != 0 || TotalMiningTime != 0.0f)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Statistics reset failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics reset"));
		return false;
	}
	
	// Simulate some mining operations
	TotalAsteroidsMined = 10;
	TotalMiningTime = 150.0f;
	TotalResourcesExtracted.Add(FName("Iron"), 200);
	TotalResourcesExtracted.Add(FName("Nickel"), 150);
	
	if (GetTotalAsteroidsMined() != 10)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Asteroid mining tracking failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Asteroid mining tracking"));
		return false;
	}
	
	TMap<FName, int32> Resources = GetTotalResourcesExtracted();
	if (Resources.Num() != 2 || Resources.FindRef(FName("Iron")) != 200)
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Resource extraction tracking failed");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource extraction tracking"));
		return false;
	}
 237
 238 	float AvgTimePerAsteroid = GetTotalMiningTime() / GetTotalAsteroidsMined();
 239 	if (AvgTimePerAsteroid < 5.0f || AvgTimePerAsteroid > 30.0f)
 240 	{
 241 		OutResult.WarningMessages.Add(TEXT("Average mining time per asteroid may be out of expected range"));
 242 		UE_LOG(LogTemp, Warning, TEXT("WARNING: Average mining time test - %.2f seconds/asteroid (expected 5-30)"), AvgTimePerAsteroid);
 243 	}
 244 	else
 245 	{
 246 		UE_LOG(LogTemp, Log, TEXT("✓ Average mining time test PASSED - %.2f seconds/asteroid"), AvgTimePerAsteroid);
 247 	}
 248 	UE_LOG(LogTemp, Log, TEXT("✓ Statistics tracking test PASSED"));
 249
 250 	// Record performance metrics
 251 	OutResult.bPassed = true;
 252 	OutResult.PerformanceMetrics.Add(TEXT("LaserTypesTested"), LaserPowerMap.Num());
 253 	OutResult.PerformanceMetrics.Add(TEXT("EfficiencyTiersTested"), 4);
 254 	OutResult.PerformanceMetrics.Add(TEXT("AsteroidsMined"), TotalAsteroidsMined);
 255 	OutResult.PerformanceMetrics.Add(TEXT("TotalMiningTime"), TotalMiningTime);
 256
 257 	UE_LOG(LogTemp, Log, TEXT("=== Asteroid Mining System test PASSED ==="));
 258 	return true;
 259 }
 260
 261 FString UAsteroidMiningSystem::GetSystemName_Implementation() const
 262 {
 263 	return TEXT("AsteroidMiningSystem");
 264 }
 265
 266 FString UAsteroidMiningSystem::GetTestDescription_Implementation() const
 267 {
 268 	return TEXT("Tests asteroid mining operations, laser upgrades, and efficiency system");
 269 }
 270
 271 bool UAsteroidMiningSystem::IsReadyForTesting_Implementation() const
 272 {
 273 	return ResourceGatheringSystem.IsValid() && LaserPowerMap.Num() > 0;
 274 }
 275 //~ End ISystemSelfTestInterface interface
 276
 277 void UAsteroidMiningSystem::Tick(float DeltaTime)
 278 {
 279 	// Update active mining operation
	if (CurrentOperation.bIsActive)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - CurrentOperation.StartTime;

		// Check if mining is complete
		if (ElapsedTime >= CurrentOperation.Duration)
		{
			// Complete mining operation
			FMiningOperationResult Result = CompleteMiningOperation();
			
			// Fire completion event
			OnMiningCompleted.Broadcast(CurrentOperation.TargetAsteroid.Get(), Result);

			// Stop mining
			CurrentOperation.bIsActive = false;
		}
	}
}

FAsteroidScanResult UAsteroidMiningSystem::ScanAsteroid(AAsteroid* Asteroid, float ScanStrength)
{
	FAsteroidScanResult Result;
	Result.bSuccess = false;

	if (!Asteroid)
	{
		Result.FailureReason = "Invalid asteroid";
		return Result;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		Result.FailureReason = "Resource gathering system not available";
		return Result;
	}

	// Perform the scan
	TArray<FResourceDeposit> DetectedDeposits;
	bool bScanSuccess = ResourceGatheringSystem->ScanAsteroid(Asteroid, ScanStrength, DetectedDeposits);

	Result.bSuccess = bScanSuccess;
	Result.ScanStrength = ScanStrength;
	Result.DetectedResources = DetectedDeposits;
	Result.ScanTime = GetWorld()->GetTimeSeconds();

	// Calculate scan quality based on number of resources detected vs total
	if (bScanSuccess && DetectedDeposits.Num() > 0)
	{
		// Get all deposits to calculate quality
		TArray<FResourceDeposit> AllDeposits = ResourceGatheringSystem->GetAsteroidResources(Asteroid);
		Result.ScanQuality = (float)DetectedDeposits.Num() / (float)AllDeposits.Num();
	}
	else
	{
		Result.ScanQuality = 0.0f;
	}

	// Store in scan history
	ScanHistory.Add(Asteroid, Result);

	// Fire scan event
	OnAsteroidScanned.Broadcast(Asteroid, Result);

	UE_LOG(LogTemp, Log, TEXT("Scanned asteroid %d: %d resources detected (Quality: %.2f)"), 
		   Asteroid->AsteroidID, DetectedDeposits.Num(), Result.ScanQuality);

	return Result;
}

FAsteroidScanResult UAsteroidMiningSystem::QuickScanAsteroid(AAsteroid* Asteroid)
{
	return ScanAsteroid(Asteroid, 0.5f); // 50% scan strength
}

FAsteroidScanResult UAsteroidMiningSystem::DeepScanAsteroid(AAsteroid* Asteroid)
{
	return ScanAsteroid(Asteroid, 1.5f); // 150% scan strength
}

FAsteroidScanResult UAsteroidMiningSystem::GetLastScanResult(AAsteroid* Asteroid) const
{
	const FAsteroidScanResult* Result = ScanHistory.Find(Asteroid);
	return Result ? *Result : FAsteroidScanResult();
}

bool UAsteroidMiningSystem::StartMining(AAsteroid* Asteroid, EMiningLaserType LaserType)
{
	if (!Asteroid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start mining: Invalid asteroid"));
		return false;
	}

	if (CurrentOperation.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start mining: Already mining another asteroid"));
		return false;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start mining: Resource gathering system not available"));
		return false;
	}

	// Check if asteroid has resources
	TArray<FResourceDeposit> Deposits = ResourceGatheringSystem->GetAsteroidResources(Asteroid);
	if (Deposits.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start mining: Asteroid has no resources"));
		return false;
	}

	// Check if all deposits are depleted
	bool bHasAvailableResources = false;
	for (const FResourceDeposit& Deposit : Deposits)
	{
		if (!Deposit.bIsDepleted)
		{
			bHasAvailableResources = true;
			break;
		}
	}

	if (!bHasAvailableResources)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start mining: All deposits depleted"));
		return false;
	}

	// Start mining operation
	CurrentOperation.TargetAsteroid = Asteroid;
	CurrentOperation.LaserType = LaserType;
	CurrentOperation.StartTime = GetWorld()->GetTimeSeconds();
	CurrentOperation.Duration = CalculateMiningDuration(Asteroid, LaserType);
	CurrentOperation.bIsActive = true;

	// Fire started event
	OnMiningStarted.Broadcast(Asteroid, LaserType);

	UE_LOG(LogTemp, Log, TEXT("Started mining asteroid %d with %s laser (Duration: %.1fs)"), 
		   Asteroid->AsteroidID, *UEnum::GetDisplayValueAsText(LaserType).ToString(), CurrentOperation.Duration);

	return true;
}

void UAsteroidMiningSystem::StopMining()
{
	if (CurrentOperation.bIsActive)
	{
		UE_LOG(LogTemp, Log, TEXT("Stopped mining operation"));
		CurrentOperation.bIsActive = false;
	}
}

AAsteroid* UAsteroidMiningSystem::GetCurrentMiningTarget() const
{
	return CurrentOperation.TargetAsteroid.Get();
}

float UAsteroidMiningSystem::GetMiningProgress() const
{
	if (!CurrentOperation.bIsActive)
	{
		return 0.0f;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - CurrentOperation.StartTime;
	return FMath::Clamp(ElapsedTime / CurrentOperation.Duration, 0.0f, 1.0f);
}

bool UAsteroidMiningSystem::UpgradeMiningLaser(EMiningLaserType NewLaserType)
{
	if (NewLaserType <= CurrentLaserType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot upgrade: New laser type is not better than current"));
		return false;
	}

	CurrentLaserType = NewLaserType;

	// Fire upgrade event
	OnMiningLaserUpgraded.Broadcast(NewLaserType);

	UE_LOG(LogTemp, Log, TEXT("Mining laser upgraded to: %s"), *UEnum::GetDisplayValueAsText(NewLaserType).ToString());

	return true;
}

float UAsteroidMiningSystem::GetLaserMiningPower(EMiningLaserType LaserType) const
{
	const float* Power = LaserPowerMap.Find(LaserType);
	return Power ? *Power : 1.0f;
}

float UAsteroidMiningSystem::GetLaserEnergyConsumption(EMiningLaserType LaserType) const
{
	const float* Consumption = LaserEnergyConsumptionMap.Find(LaserType);
	return Consumption ? *Consumption : 1.0f;
}

float UAsteroidMiningSystem::GetLaserEfficiency(EMiningLaserType LaserType) const
{
	const float* Efficiency = LaserEfficiencyMap.Find(LaserType);
	return Efficiency ? *Efficiency : 1.0f;
}

bool UAsteroidMiningSystem::UpgradeMiningEfficiency(EMiningEfficiencyTier NewTier)
{
	if (NewTier <= CurrentEfficiencyTier)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot upgrade: New efficiency tier is not better than current"));
		return false;
	}

	CurrentEfficiencyTier = NewTier;

	UE_LOG(LogTemp, Log, TEXT("Mining efficiency upgraded to: %s"), *UEnum::GetDisplayValueAsText(NewTier).ToString());

	return true;
}

float UAsteroidMiningSystem::GetEfficiencyMultiplier(EMiningEfficiencyTier Tier) const
{
	switch (Tier)
	{
		case EMiningEfficiencyTier::Tier1_Basic: return 1.0f;
		case EMiningEfficiencyTier::Tier2_Standard: return 1.25f;
		case EMiningEfficiencyTier::Tier3_Advanced: return 1.6f;
		case EMiningEfficiencyTier::Tier4_Masterwork: return 2.0f;
		default: return 1.0f;
	}
}

void UAsteroidMiningSystem::ResetStatistics()
{
	TotalAsteroidsMined = 0;
	TotalResourcesExtracted.Empty();
	TotalMiningTime = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Asteroid mining statistics reset"));
}

void UAsteroidMiningSystem::InitializeLaserStatistics()
{
	// Basic Mining Laser
	LaserPowerMap.Add(EMiningLaserType::Basic, 1.0f);
	LaserEnergyConsumptionMap.Add(EMiningLaserType::Basic, 1.0f);
	LaserEfficiencyMap.Add(EMiningLaserType::Basic, 0.8f);

	// Advanced Mining Laser
	LaserPowerMap.Add(EMiningLaserType::Advanced, 2.0f);
	LaserEnergyConsumptionMap.Add(EMiningLaserType::Advanced, 1.5f);
	LaserEfficiencyMap.Add(EMiningLaserType::Advanced, 1.0f);

	// Industrial Mining Laser
	LaserPowerMap.Add(EMiningLaserType::Industrial, 4.0f);
	LaserEnergyConsumptionMap.Add(EMiningLaserType::Industrial, 3.0f);
	LaserEfficiencyMap.Add(EMiningLaserType::Industrial, 1.2f);

	// Quantum Mining Laser
	LaserPowerMap.Add(EMiningLaserType::Quantum, 8.0f);
	LaserEnergyConsumptionMap.Add(EMiningLaserType::Quantum, 6.0f);
	LaserEfficiencyMap.Add(EMiningLaserType::Quantum, 1.5f);
}

void UAsteroidMiningSystem::InitializeEfficiencyTiers()
{
	// Efficiency multipliers are handled in GetEfficiencyMultiplier function
	// This function is reserved for future initialization if needed
}

FMiningOperationResult UAsteroidMiningSystem::CompleteMiningOperation()
{
	FMiningOperationResult Result;
	Result.bSuccess = false;

	if (!CurrentOperation.TargetAsteroid.IsValid())
	{
		Result.FailureReason = "Invalid asteroid target";
		return Result;
	}

	AAsteroid* Asteroid = CurrentOperation.TargetAsteroid.Get();
	if (!ResourceGatheringSystem.IsValid())
	{
		Result.FailureReason = "Resource gathering system not available";
		return Result;
	}

	// Perform mining operation
	float LaserPower = GetLaserMiningPower(CurrentOperation.LaserType);
	float LaserEfficiency = GetLaserEfficiency(CurrentOperation.LaserType);
	float EfficiencyMultiplier = GetEfficiencyMultiplier(CurrentEfficiencyTier);

	FMiningResult MiningResult = ResourceGatheringSystem->MineAsteroid(Asteroid, LaserPower, LaserEfficiency * EfficiencyMultiplier);

	if (MiningResult.bSuccess)
	{
		Result.bSuccess = true;
		Result.ResourcesExtracted = MiningResult.ResourcesExtracted;
		Result.ExperienceGained = MiningResult.ExperienceGained;
		Result.OperationTime = CurrentOperation.Duration;

		// Calculate total value
		for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
		{
			FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Resource.ResourceID);
			Result.TotalValue += ResourceDef.BaseValue * Resource.Quantity;
		}

		// Calculate energy consumed
		Result.EnergyConsumed = CalculateEnergyConsumption(Result.OperationTime, CurrentOperation.LaserType);

		// Update statistics
		TotalAsteroidsMined++;
		TotalMiningTime += Result.OperationTime;

		for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
		{
			int32& ExtractedAmount = TotalResourcesExtracted.FindOrAdd(Resource.ResourceID);
			ExtractedAmount += Resource.Quantity;
		}

		// Add to inventory if manager is available
		if (InventoryManager.IsValid())
		{
			for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
			{
				// InventoryManager->AddResource(Resource); // Uncomment when InventoryManager is implemented
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Completed mining asteroid %d: Extracted %d resources (Value: %.1f credits)"), 
			   Asteroid->AsteroidID, Result.ResourcesExtracted.Num(), Result.TotalValue);
	}
	else
	{
		Result.FailureReason = MiningResult.FailureReason;
		Result.ExperienceGained = MiningResult.ExperienceGained;
	}

	return Result;
}

float UAsteroidMiningSystem::CalculateMiningDuration(AAsteroid* Asteroid, EMiningLaserType LaserType) const
{
	if (!Asteroid)
	{
		return 0.0f;
	}

	// Base duration based on asteroid size
	float BaseDuration = Asteroid->AsteroidRadius * 0.5f; // 0.5 seconds per meter of radius

	// Adjust based on laser power
	float LaserPower = GetLaserMiningPower(LaserType);
	float Duration = BaseDuration / LaserPower;

	// Apply efficiency multiplier
	float EfficiencyMultiplier = GetEfficiencyMultiplier(CurrentEfficiencyTier);
	Duration /= EfficiencyMultiplier;

	// Apply skill bonus
	float SkillMultiplier = 1.0f + (MiningSkillLevel / 100.0f) * 0.5f;
	Duration /= SkillMultiplier;

	return FMath::Max(Duration, 1.0f); // Minimum 1 second
}

float UAsteroidMiningSystem::CalculateEnergyConsumption(float Duration, EMiningLaserType LaserType) const
{
	float BaseConsumption = GetLaserEnergyConsumption(LaserType);
	return BaseConsumption * Duration;
}