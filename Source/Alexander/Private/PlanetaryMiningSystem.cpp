// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetaryMiningSystem.h"
#include "ResourceGatheringSystem.h"
#include "Planet.h"
#include "Spaceship.h"
#include "InventoryManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "SystemSelfTestInterface.h"

UPlanetaryMiningSystem::UPlanetaryMiningSystem()
{
	CurrentEquipment = EPlanetaryMiningEquipment::HandDrill;
	TotalMiningOperations = 0;
	TotalEnvironmentalImpact = 0.0f;
	MiningSkillLevel = 0.0f;

	InitializeEquipmentStatistics();
}

void UPlanetaryMiningSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("PlanetaryMiningSystem initialized"));

	// Get references to other systems
	ResourceGatheringSystem = GetWorld()->GetSubsystem<UResourceGatheringSystem>();
	// InventoryManager will be set later when available

	// Reset statistics
	TotalMiningOperations = 0;
	TotalResourcesExtracted.Empty();
	TotalEnvironmentalImpact = 0.0f;
}

void UPlanetaryMiningSystem::Deinitialize()
{
	// Clear scan history and permits
	ScanHistory.Empty();
	MiningPermits.Empty();
	PlanetaryEnvironmentalHealth.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("PlanetaryMiningSystem deinitialized"));
}

//~ Begin ISystemSelfTestInterface interface
bool UPlanetaryMiningSystem::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
	OutResult.SystemName = GetSystemName_Implementation();
	OutResult.WarningMessages.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("=== Testing Planetary Mining System ==="));
	
	// Test 1: Check if system is properly initialized
	if (!ResourceGatheringSystem.IsValid())
	{
		OutResult.bPassed = false;
		OutResult.ErrorMessage = TEXT("Planetary Mining System not properly initialized - ResourceGatheringSystem not available");
		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: System not properly initialized"));
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("✓ System initialization test PASSED"));
	
	// Test 2: Test equipment statistics
	if (EquipmentPowerMap.Num() != 5 || EquipmentEfficiencyMap.Num() != 5 || EquipmentImpactMap.Num() != 5)
	{
 70 		OutResult.bPassed = false;
 71 		OutResult.ErrorMessage = TEXT("Equipment statistics not properly initialized");
 72 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment statistics initialization"));
 73 		return false;
 74 	}
 75 	UE_LOG(LogTemp, Log, TEXT("✓ Equipment statistics test PASSED - %d equipment types registered"), EquipmentPowerMap.Num());
 76
 77 	// Test 3: Test equipment queries
 78 	float HandDrillPower = GetEquipmentMiningPower(EPlanetaryMiningEquipment::HandDrill);
 79 	float HandDrillEfficiency = GetEquipmentEfficiency(EPlanetaryMiningEquipment::HandDrill);
 80 	float HandDrillImpact = GetEquipmentEnvironmentalImpact(EPlanetaryMiningEquipment::HandDrill);
 81
 82 	if (HandDrillPower <= 0.0f || HandDrillEfficiency <= 0.0f || HandDrillImpact <= 0.0f)
 83 	{
 84 		OutResult.bPassed = false;
 85 		OutResult.ErrorMessage = TEXT("Equipment statistics queries returned invalid values");
 86 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment statistics queries"));
 87 		return false;
 88 	}
 89 	UE_LOG(LogTemp, Log, TEXT("✓ Equipment queries test PASSED - HandDrill: Power=%.2f, Efficiency=%.2f, Impact=%.2f"),
 90 		HandDrillPower, HandDrillEfficiency, HandDrillImpact);
 91
 92 	// Test 4: Test equipment upgrade
 93 	EPlanetaryMiningEquipment OriginalEquipment = CurrentEquipment;
 94 	if (!UpgradeEquipment(EPlanetaryMiningEquipment::SurfaceMiner))
 95 	{
 96 		OutResult.bPassed = false;
 97 		OutResult.ErrorMessage = TEXT("Equipment upgrade failed");
 98 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment upgrade"));
 99 		return false;
 100 	}
 101 	if (CurrentEquipment != EPlanetaryMiningEquipment::SurfaceMiner)
 102 	{
 103 		OutResult.bPassed = false;
 104 		OutResult.ErrorMessage = TEXT("Equipment upgrade did not change current equipment");
 105 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment upgrade verification"));
 106 		return false;
 107 	}
 108 	UE_LOG(LogTemp, Log, TEXT("✓ Equipment upgrade test PASSED - Upgraded to SurfaceMiner"));
 109
 110 	// Test 5: Test environmental impact calculation
 111 	FPlanetaryMiningParams TestParams;
 112 	TestParams.EquipmentType = EPlanetaryMiningEquipment::HandDrill;
 113 	TestParams.MiningPower = 1.0f;
 114 	TestParams.MiningEfficiency = 1.0f;
 115 	TestParams.EnvironmentalImpactMultiplier = 1.0f;
 116
 117 	FResourceDeposit TestDeposit;
 118 	TestDeposit.DepositType = EDepositType::Surface;
 119 	TestDeposit.Depth = 10.0f;
 120 	TestDeposit.MaxQuantity = 100;
 121
 122 	float ImpactValue = CalculateEnvironmentalImpact(TestParams, TestDeposit);
 123 	EEnvironmentalImpact Impact = (EEnvironmentalImpact)ImpactValue;
 124
 125 	if (Impact != EEnvironmentalImpact::Minimal && Impact != EEnvironmentalImpact::Low)
 126 	{
 127 		OutResult.WarningMessages.Add(TEXT("Environmental impact calculation may be too high for basic equipment"));
 128 		UE_LOG(LogTemp, Warning, TEXT("WARNING: Environmental impact test - Impact=%s for HandDrill on surface deposit"),
 129 			*UEnum::GetDisplayValueAsText(Impact).ToString());
 130 	}
 131 	else
 132 	{
 133 		UE_LOG(LogTemp, Log, TEXT("✓ Environmental impact calculation test PASSED - Impact=%s"),
 134 			*UEnum::GetDisplayValueAsText(Impact).ToString());
 135 	}
 136
 137 	// Test 6: Test mining duration calculation
 138 	float Duration = CalculateMiningDuration(TestDeposit, EPlanetaryMiningEquipment::HandDrill);
 139 	if (Duration < 2.0f || Duration > 60.0f) // Should be between 2 and 60 seconds
 140 	{
 141 		OutResult.WarningMessages.Add(TEXT("Mining duration calculation may be out of expected range"));
 142 		UE_LOG(LogTemp, Warning, TEXT("WARNING: Mining duration test - Duration=%.2f seconds (expected 2-60)"), Duration);
 143 	}
 144 	else
 145 	{
 146 		UE_LOG(LogTemp, Log, TEXT("✓ Mining duration calculation test PASSED - Duration=%.2f seconds"), Duration);
 147 	}
 148
 149 	// Test 7: Test energy consumption calculation
 150 	float EnergyConsumption = CalculateEnergyConsumption(Duration, EPlanetaryMiningEquipment::HandDrill);
 151 	if (EnergyConsumption <= 0.0f)
 152 	{
 153 		OutResult.bPassed = false;
 154 		OutResult.ErrorMessage = TEXT("Energy consumption calculation returned invalid value");
 155 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Energy consumption calculation"));
 156 		return false;
 157 	}
 158 	UE_LOG(LogTemp, Log, TEXT("✓ Energy consumption calculation test PASSED - Energy=%.2f units"), EnergyConsumption);
 159
 160 	// Test 8: Test equipment suitability checks
 161 	if (!IsEquipmentSuitableForDeposit(EPlanetaryMiningEquipment::HandDrill, EDepositType::Surface))
 162 	{
 163 		OutResult.bPassed = false;
 164 		OutResult.ErrorMessage = TEXT("Equipment suitability check failed for valid combination");
 165 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment suitability check"));
 166 		return false;
 167 	}
 168 	if (IsEquipmentSuitableForDeposit(EPlanetaryMiningEquipment::HandDrill, EDepositType::Deep))
 169 	{
 170 		OutResult.bPassed = false;
 171 		OutResult.ErrorMessage = TEXT("Equipment suitability check passed for invalid combination");
 172 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Equipment suitability check (invalid combination)"));
 173 		return false;
 174 	}
 175 	UE_LOG(LogTemp, Log, TEXT("✓ Equipment suitability test PASSED"));
 176
 177 	// Test 9: Test permit system
 178 	// Create a mock planet for testing
 179 	APlanet* TestPlanet = nullptr; // Would need to spawn a test planet in a real scenario
 180 	if (TestPlanet)
 181 	{
 182 		FMiningPermit Permit = ApplyForMiningPermit(TestPlanet, 1000, EEnvironmentalImpact::Moderate);
 183 		if (Permit.Status == EPermitStatus::NotRequired)
 184 		{
 185 			OutResult.WarningMessages.Add(TEXT("Permit system test skipped - no test planet available"));
 186 			UE_LOG(LogTemp, Warning, TEXT("WARNING: Permit system test skipped"));
 187 		}
 188 		else if (Permit.Status != EPermitStatus::Granted && Permit.Status != EPermitStatus::Denied)
 189 		{
 190 			OutResult.bPassed = false;
 191 			OutResult.ErrorMessage = TEXT("Permit application returned unexpected status");
 192 			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Permit system"));
 193 			return false;
 194 		}
 195 		else
 196 		{
 197 			UE_LOG(LogTemp, Log, TEXT("✓ Permit system test PASSED - Status=%s"),
 198 				*UEnum::GetDisplayValueAsText(Permit.Status).ToString());
 199 		}
 200 	}
 201 	else
 202 	{
 203 		OutResult.WarningMessages.Add(TEXT("Permit system test skipped - no test planet available"));
 204 		UE_LOG(LogTemp, Warning, TEXT("WARNING: Permit system test skipped - could not create test planet"));
 205 	}
 206
 207 	// Test 10: Test statistics tracking
 208 	ResetStatistics();
 209 	if (TotalMiningOperations != 0 || TotalEnvironmentalImpact != 0.0f)
 210 	{
 211 		OutResult.bPassed = false;
 212 		OutResult.ErrorMessage = TEXT("Statistics reset failed");
 213 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics reset"));
 214 		return false;
 215 	}
 216
 217 	// Simulate some mining operations
 218 	TotalMiningOperations = 5;
 219 	TotalEnvironmentalImpact = 2.5f;
 220 	TotalResourcesExtracted.Add(FName(TEXT("Iron")), 100);
 221 	TotalResourcesExtracted.Add(FName(TEXT("Copper")), 50);
 222
 223 	if (GetTotalMiningOperations() != 5)
 224 	{
 225 		OutResult.bPassed = false;
 226 		OutResult.ErrorMessage = TEXT("Statistics tracking not working properly");
 227 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics tracking"));
 228 		return false;
 229 	}
 230
 231 	TMap<FName, int32> Resources = GetTotalResourcesExtracted();
 232 	if (Resources.Num() != 2 || Resources.FindRef(FName(TEXT("Iron"))) != 100)
 233 	{
 234 		OutResult.bPassed = false;
 235 		OutResult.ErrorMessage = TEXT("Resource extraction tracking not working properly");
 236 		UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Resource extraction tracking"));
 237 		return false;
 238 	}
 239 	UE_LOG(LogTemp, Log, TEXT("✓ Statistics tracking test PASSED"));
 240
 241 	// Record performance metrics
 242 	OutResult.bPassed = true;
 243 	OutResult.PerformanceMetrics.Add(TEXT("EquipmentTypesTested"), EquipmentPowerMap.Num());
 244 	OutResult.PerformanceMetrics.Add(TEXT("MiningDuration"), Duration);
 245 	OutResult.PerformanceMetrics.Add(TEXT("EnergyConsumption"), EnergyConsumption);
 246
 247 	UE_LOG(LogTemp, Log, TEXT("=== Planetary Mining System test PASSED ==="));
 248 	return true;
 249 }
 250
 251 FString UPlanetaryMiningSystem::GetSystemName_Implementation() const
 252 {
 253 	return TEXT("PlanetaryMiningSystem");
 254 }
 255
 256 FString UPlanetaryMiningSystem::GetTestDescription_Implementation() const
 257 {
 258 	return TEXT("Tests planetary mining operations, equipment system, environmental impact, and permit management");
 259 }
 260
 261 bool UPlanetaryMiningSystem::IsReadyForTesting_Implementation() const
 262 {
 263 	return ResourceGatheringSystem.IsValid() && EquipmentPowerMap.Num() > 0;
 264 }
 265 //~ End ISystemSelfTestInterface interface
266
 267 void UPlanetaryMiningSystem::Tick(float DeltaTime)
 268 {
 269 	// Update any ongoing mining operations or environmental recovery
 270 	// For now, this is a placeholder for future timed operations
 271 }

FPlanetaryScanResult UPlanetaryMiningSystem::ScanPlanetaryLocation(APlanet* Planet, FVector Location, float ScanStrength)
{
	FPlanetaryScanResult Result;
	Result.bSuccess = false;

	if (!Planet)
	{
		Result.FailureReason = "Invalid planet";
		return Result;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		Result.FailureReason = "Resource gathering system not available";
		return Result;
	}

	// Perform the scan
	TArray<FResourceDeposit> DetectedDeposits;
	bool bScanSuccess = ResourceGatheringSystem->ScanPlanetaryLocation(Planet, Location, ScanStrength, DetectedDeposits);

	Result.bSuccess = bScanSuccess;
	Result.ScanStrength = ScanStrength;
	Result.DetectedResources = DetectedDeposits;
	Result.ScanTime = GetWorld()->GetTimeSeconds();

	// Calculate scan quality based on number of resources detected vs total
	if (bScanSuccess && DetectedDeposits.Num() > 0)
	{
		// Get all deposits to calculate quality
		TArray<FResourceDeposit> AllDeposits = ResourceGatheringSystem->GetPlanetaryResources(Planet, Location);
		Result.ScanQuality = (float)DetectedDeposits.Num() / (float)AllDeposits.Num();
	}
	else
	{
		Result.ScanQuality = 0.0f;
	}

	// Determine environmental impact based on scan strength
	if (ScanStrength < 0.8f)
	{
		Result.EnvironmentalImpact = EEnvironmentalImpact::Minimal;
	}
	else if (ScanStrength < 1.5f)
	{
		Result.EnvironmentalImpact = EEnvironmentalImpact::Low;
	}
	else if (ScanStrength < 2.5f)
	{
		Result.EnvironmentalImpact = EEnvironmentalImpact::Moderate;
	}
	else
	{
		Result.EnvironmentalImpact = EEnvironmentalImpact::High;
	}

	// Store in scan history
	TPair<TWeakObjectPtr<APlanet>, FVector> Key(Planet, Location);
	ScanHistory.Add(Key, Result);

	// Fire scan event
	OnPlanetaryScanned.Broadcast(Planet, Result);

	UE_LOG(LogTemp, Log, TEXT("Scanned planetary location: %d resources detected (Quality: %.2f, Impact: %s)"), 
		   DetectedDeposits.Num(), Result.ScanQuality, *UEnum::GetDisplayValueAsText(Result.EnvironmentalImpact).ToString());

	return Result;
}

FPlanetaryScanResult UPlanetaryMiningSystem::QuickSurfaceScan(APlanet* Planet, FVector Location)
{
	return ScanPlanetaryLocation(Planet, Location, 0.6f); // 60% scan strength
}

FPlanetaryScanResult UPlanetaryMiningSystem::DeepGeologicalScan(APlanet* Planet, FVector Location)
{
	return ScanPlanetaryLocation(Planet, Location, 2.0f); // 200% scan strength
}

FPlanetaryScanResult UPlanetaryMiningSystem::GetLastScanResult(APlanet* Planet, FVector Location) const
{
	TPair<TWeakObjectPtr<APlanet>, FVector> Key(Planet, Location);
	const FPlanetaryScanResult* Result = ScanHistory.Find(Key);
	return Result ? *Result : FPlanetaryScanResult();
}

FPlanetaryMiningResult UPlanetaryMiningSystem::MinePlanetaryLocation(APlanet* Planet, FVector Location, const FPlanetaryMiningParams& Params)
{
	FPlanetaryMiningResult Result;
	Result.bSuccess = false;

	if (!Planet)
	{
		Result.FailureReason = "Invalid planet";
		return Result;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		Result.FailureReason = "Resource gathering system not available";
		return Result;
	}

	// Check mining permit
	FMiningPermit Permit = GetMiningPermit(Planet);
	if (!IsMiningPermitted(Planet, Location, Params.EnvironmentalImpactMultiplier))
	{
		Result.FailureReason = "Mining not permitted at this location";
		return Result;
	}

	// Get resources at location
	TArray<FResourceDeposit> Deposits = ResourceGatheringSystem->GetPlanetaryResources(Planet, Location);
	if (Deposits.Num() == 0)
	{
		Result.FailureReason = "No resources found at location";
		return Result;
	}

	// Find first available deposit
	FResourceDeposit* TargetDeposit = nullptr;
	for (FResourceDeposit& Deposit : Deposits)
	{
		if (!Deposit.bIsDepleted)
		{
			TargetDeposit = &Deposit;
			break;
		}
	}

	if (!TargetDeposit)
	{
		Result.FailureReason = "All deposits depleted";
		return Result;
	}

	// Check if deposit type is compatible with equipment
	if (!IsEquipmentSuitableForDeposit(Params.EquipmentType, TargetDeposit->DepositType))
	{
		Result.FailureReason = "Current equipment not suitable for deposit type";
		return Result;
	}

	// Perform mining operation
	float EquipmentPower = GetEquipmentMiningPower(Params.EquipmentType);
	float EquipmentEfficiency = GetEquipmentEfficiency(Params.EquipmentType);
	float SkillMultiplier = 1.0f + (MiningSkillLevel / 100.0f) * 0.5f;

	FMiningResult MiningResult = ResourceGatheringSystem->MineDeposit(*TargetDeposit, 
																	  EquipmentPower * Params.MiningPower, 
																	  EquipmentEfficiency * Params.MiningEfficiency * SkillMultiplier);

	if (MiningResult.bSuccess)
	{
		Result.bSuccess = true;
		Result.ResourcesExtracted = MiningResult.ResourcesExtracted;
		Result.ExperienceGained = MiningResult.ExperienceGained;

		// Calculate operation time based on equipment and deposit
		Result.OperationTime = CalculateMiningDuration(*TargetDeposit, Params.EquipmentType);

		// Calculate energy consumption
		Result.EnergyConsumed = CalculateEnergyConsumption(Result.OperationTime, Params.EquipmentType);

		// Calculate environmental impact
		Result.EnvironmentalImpact = CalculateEnvironmentalImpact(Params, *TargetDeposit);

		// Calculate total value
		for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
		{
			FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Resource.ResourceID);
			Result.TotalValue += ResourceDef.BaseValue * Resource.Quantity;
		}

		// Update statistics
		TotalMiningOperations++;
		TotalEnvironmentalImpact += (float)Result.EnvironmentalImpact;

		for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
		{
			int32& ExtractedAmount = TotalResourcesExtracted.FindOrAdd(Resource.ResourceID);
			ExtractedAmount += Resource.Quantity;
		}

		// Update permit extraction count
		if (Permit.Status == EPermitStatus::Granted)
		{
			FMiningPermit& MutablePermit = MiningPermits.FindOrAdd(Planet);
			MutablePermit.CurrentExtraction += Result.ResourcesExtracted[0].Quantity;
		}

		// Update environmental health
		UpdateEnvironmentalHealth(Planet, Result.EnvironmentalImpact);

		// Add to inventory if manager is available
		if (InventoryManager.IsValid())
		{
			for (const FResourceQuantity& Resource : Result.ResourcesExtracted)
			{
				// InventoryManager->AddResource(Resource); // Uncomment when InventoryManager is implemented
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Completed planetary mining: Extracted %d resources (Value: %.1f credits, Impact: %s)"), 
			   Result.ResourcesExtracted.Num(), Result.TotalValue, *UEnum::GetDisplayValueAsText(Result.EnvironmentalImpact).ToString());
	}
	else
	{
		Result.FailureReason = MiningResult.FailureReason;
		Result.ExperienceGained = MiningResult.ExperienceGained;
	}

	// Fire completion event
	OnPlanetaryMiningCompleted.Broadcast(Planet, Result);

	return Result;
}

FPlanetaryMiningResult UPlanetaryMiningSystem::MineWithCurrentEquipment(APlanet* Planet, FVector Location)
{
	FPlanetaryMiningParams Params;
	Params.EquipmentType = CurrentEquipment;
	Params.MiningPower = 1.0f;
	Params.MiningEfficiency = 1.0f;
	Params.EnvironmentalImpactMultiplier = 1.0f;

	return MinePlanetaryLocation(Planet, Location, Params);
}

void UPlanetaryMiningSystem::SetCurrentEquipment(EPlanetaryMiningEquipment Equipment)
{
	CurrentEquipment = Equipment;
	UE_LOG(LogTemp, Log, TEXT("Current planetary mining equipment set to: %s"), *UEnum::GetDisplayValueAsText(Equipment).ToString());
}

EEnvironmentalImpact UPlanetaryMiningSystem::GetEnvironmentalImpact(APlanet* Planet, FVector Location) const
{
	if (!Planet)
	{
		return EEnvironmentalImpact::Minimal;
	}

	// Get the current environmental health
	const float* Health = PlanetaryEnvironmentalHealth.Find(Planet);
	float CurrentHealth = Health ? *Health : 1.0f;

	// Convert health to impact level
	if (CurrentHealth > 0.8f)
	{
		return EEnvironmentalImpact::Minimal;
	}
	else if (CurrentHealth > 0.6f)
	{
		return EEnvironmentalImpact::Low;
	}
	else if (CurrentHealth > 0.4f)
	{
		return EEnvironmentalImpact::Moderate;
	}
	else if (CurrentHealth > 0.2f)
	{
		return EEnvironmentalImpact::High;
	}
	else
	{
		return EEnvironmentalImpact::Severe;
	}
}

float UPlanetaryMiningSystem::GetPlanetaryHealth(APlanet* Planet) const
{
	if (!Planet)
	{
		return 1.0f;
	}

	const float* Health = PlanetaryEnvironmentalHealth.Find(Planet);
	return Health ? *Health : 1.0f;
}

void UPlanetaryMiningSystem::RestoreEnvironmentalHealth(APlanet* Planet, float Amount)
{
	if (!Planet)
	{
		return;
	}

	float& Health = PlanetaryEnvironmentalHealth.FindOrAdd(Planet);
	Health = FMath::Clamp(Health + Amount, 0.0f, 1.0f);

	UE_LOG(LogTemp, Log, TEXT("Restored planetary health: +%.2f (Current: %.2f)"), Amount, Health);
}

FMiningPermit UPlanetaryMiningSystem::ApplyForMiningPermit(APlanet* Planet, int32 RequestedExtraction, EEnvironmentalImpact MaxImpact)
{
	FMiningPermit Permit;
	Permit.bSuccess = false;

	if (!Planet)
	{
		Permit.FailureReason = "Invalid planet";
		return Permit;
	}

	// Generate permit
	Permit.PemitID = FGuid::NewGuid();
	Permit.TargetPlanet = Planet;
	Permit.Status = EPermitStatus::Applied;
	Permit.IssueDate = FDateTime::Now();
	Permit.ExpirationDate = Permit.IssueDate + FTimespan(30, 0, 0, 0); // 30 days
	Permit.MaxExtraction = RequestedExtraction;
	Permit.CurrentExtraction = 0;
	Permit.MaxAllowedImpact = MaxImpact;

	// Store permit
	MiningPermits.Add(Planet, Permit);

	// Simulate permit approval (in a real implementation, this would involve faction reputation)
	float ApprovalChance = 0.7f; // 70% base approval chance
	if (FMath::FRand() < ApprovalChance)
	{
		Permit.Status = EPermitStatus::Granted;
		UE_LOG(LogTemp, Log, TEXT("Mining permit granted for planet"));
	}
	else
	{
		Permit.Status = EPermitStatus::Denied;
		UE_LOG(LogTemp, Warning, TEXT("Mining permit denied for planet"));
	}

	// Update stored permit
	MiningPermits[Planet] = Permit;

	// Fire permit changed event
	OnMiningPermitChanged.Broadcast(Permit, Permit.Status);

	return Permit;
}

FMiningPermit UPlanetaryMiningSystem::GetMiningPermit(APlanet* Planet) const
{
	if (!Planet)
	{
		return FMiningPermit();
	}

	const FMiningPermit* Permit = MiningPermits.Find(Planet);
	return Permit ? *Permit : FMiningPermit();
}

bool UPlanetaryMiningSystem::IsMiningPermitted(APlanet* Planet, FVector Location, EEnvironmentalImpact Impact) const
{
	if (!Planet)
	{
		return false;
	}

	// Check if planet requires permits
	if (!DoesPlanetRequirePermits(Planet))
	{
		return true; // No permits required
	}

	// Get permit for planet
	FMiningPermit Permit = GetMiningPermit(Planet);
	if (Permit.Status != EPermitStatus::Granted)
	{
		return false; // No valid permit
	}

	// Check if permit has expired
	if (FDateTime::Now() > Permit.ExpirationDate)
	{
		return false; // Permit expired
	}

	// Check environmental impact restriction
	if (Impact > Permit.MaxAllowedImpact)
	{
		return false; // Impact exceeds permit limits
	}

	// Check extraction limit
	if (Permit.CurrentExtraction >= Permit.MaxExtraction)
	{
		return false; // Extraction limit reached
	}

	return true;
}

void UPlanetaryMiningSystem::UpdatePermitStatus(APlanet* Planet, EPermitStatus NewStatus)
{
	if (!Planet)
	{
		return;
	}

	FMiningPermit& Permit = MiningPermits.FindOrAdd(Planet);
	EPermitStatus OldStatus = Permit.Status;
	Permit.Status = NewStatus;

	// Fire permit changed event
	OnMiningPermitChanged.Broadcast(Permit, NewStatus);

	UE_LOG(LogTemp, Log, TEXT("Mining permit status updated from %s to %s"), 
		   *UEnum::GetDisplayValueAsText(OldStatus).ToString(), 
		   *UEnum::GetDisplayValueAsText(NewStatus).ToString());
}

float UPlanetaryMiningSystem::GetEquipmentMiningPower(EPlanetaryMiningEquipment Equipment) const
{
	const float* Power = EquipmentPowerMap.Find(Equipment);
	return Power ? *Power : 1.0f;
}

float UPlanetaryMiningSystem::GetEquipmentEfficiency(EPlanetaryMiningEquipment Equipment) const
{
	const float* Efficiency = EquipmentEfficiencyMap.Find(Equipment);
	return Efficiency ? *Efficiency : 1.0f;
}

float UPlanetaryMiningSystem::GetEquipmentEnvironmentalImpact(EPlanetaryMiningEquipment Equipment) const
{
	const float* Impact = EquipmentImpactMap.Find(Equipment);
	return Impact ? *Impact : 1.0f;
}

bool UPlanetaryMiningSystem::UpgradeEquipment(EPlanetaryMiningEquipment NewEquipment)
{
	if (NewEquipment <= CurrentEquipment)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot upgrade: New equipment is not better than current"));
		return false;
	}

	CurrentEquipment = NewEquipment;

	UE_LOG(LogTemp, Log, TEXT("Planetary mining equipment upgraded to: %s"), *UEnum::GetDisplayValueAsText(NewEquipment).ToString());

	return true;
}

void UPlanetaryMiningSystem::ResetStatistics()
{
	TotalMiningOperations = 0;
	TotalResourcesExtracted.Empty();
	TotalEnvironmentalImpact = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Planetary mining statistics reset"));
}

void UPlanetaryMiningSystem::InitializeEquipmentStatistics()
{
	// Hand Drill
	EquipmentPowerMap.Add(EPlanetaryMiningEquipment::HandDrill, 0.5f);
	EquipmentEfficiencyMap.Add(EPlanetaryMiningEquipment::HandDrill, 0.6f);
	EquipmentImpactMap.Add(EPlanetaryMiningEquipment::HandDrill, 0.3f);

	// Surface Miner
	EquipmentPowerMap.Add(EPlanetaryMiningEquipment::SurfaceMiner, 1.0f);
	EquipmentEfficiencyMap.Add(EPlanetaryMiningEquipment::SurfaceMiner, 1.0f);
	EquipmentImpactMap.Add(EPlanetaryMiningEquipment::SurfaceMiner, 0.8f);

	// Deep Drill
	EquipmentPowerMap.Add(EPlanetaryMiningEquipment::DeepDrill, 2.0f);
	EquipmentEfficiencyMap.Add(EPlanetaryMiningEquipment::DeepDrill, 1.2f);
	EquipmentImpactMap.Add(EPlanetaryMiningEquipment::DeepDrill, 1.5f);

	// Fracking Rig
	EquipmentPowerMap.Add(EPlanetaryMiningEquipment::FrackingRig, 3.0f);
	EquipmentEfficiencyMap.Add(EPlanetaryMiningEquipment::FrackingRig, 1.5f);
	EquipmentImpactMap.Add(EPlanetaryMiningEquipment::FrackingRig, 2.5f);

	// Excavator
	EquipmentPowerMap.Add(EPlanetaryMiningEquipment::Excavator, 2.5f);
	EquipmentEfficiencyMap.Add(EPlanetaryMiningEquipment::Excavator, 1.3f);
	EquipmentImpactMap.Add(EPlanetaryMiningEquipment::Excavator, 1.8f);
}

float UPlanetaryMiningSystem::CalculateEnvironmentalImpact(const FPlanetaryMiningParams& Params, const FResourceDeposit& Deposit) const
{
	// Base impact from equipment
	float BaseImpact = GetEquipmentEnvironmentalImpact(Params.EquipmentType);

	// Adjust based on deposit type
	float DepositMultiplier = 1.0f;
	switch (Deposit.DepositType)
	{
		case EDepositType::Surface: DepositMultiplier = 1.0f; break;
		case EDepositType::Underground: DepositMultiplier = 1.3f; break;
		case EDepositType::Deep: DepositMultiplier = 1.8f; break;
		case EDepositType::Fracking: DepositMultiplier = 2.5f; break;
	}

	// Adjust based on mining parameters
	float ParamMultiplier = Params.EnvironmentalImpactMultiplier;

	// Calculate final impact
	float ImpactValue = BaseImpact * DepositMultiplier * ParamMultiplier;

	// Convert to enum
	if (ImpactValue < 0.5f)
	{
		return (float)EEnvironmentalImpact::Minimal;
	}
	else if (ImpactValue < 1.0f)
	{
		return (float)EEnvironmentalImpact::Low;
	}
	else if (ImpactValue < 1.8f)
	{
		return (float)EEnvironmentalImpact::Moderate;
	}
	else if (ImpactValue < 2.5f)
	{
		return (float)EEnvironmentalImpact::High;
	}
	else
	{
		return (float)EEnvironmentalImpact::Severe;
	}
}

bool UPlanetaryMiningSystem::CheckPermitRestrictions(const FMiningPermit& Permit, const FPlanetaryMiningParams& Params, int32 AmountExtracted) const
{
	// Check extraction limit
	if (Permit.CurrentExtraction + AmountExtracted > Permit.MaxExtraction)
	{
		return false;
	}

	// Check environmental impact
	EEnvironmentalImpact OperationImpact = (EEnvironmentalImpact)CalculateEnvironmentalImpact(Params, FResourceDeposit());
	if (OperationImpact > Permit.MaxAllowedImpact)
	{
		return false;
	}

	return true;
}

void UPlanetaryMiningSystem::UpdateEnvironmentalHealth(APlanet* Planet, EEnvironmentalImpact Impact)
{
	if (!Planet)
	{
		return;
	}

	float& Health = PlanetaryEnvironmentalHealth.FindOrAdd(Planet);
	float ImpactDamage = 0.0f;

	// Convert impact to health damage
	switch (Impact)
	{
		case EEnvironmentalImpact::Minimal: ImpactDamage = 0.01f; break;
		case EEnvironmentalImpact::Low: ImpactDamage = 0.03f; break;
		case EEnvironmentalImpact::Moderate: ImpactDamage = 0.08f; break;
		case EEnvironmentalImpact::High: ImpactDamage = 0.15f; break;
		case EEnvironmentalImpact::Severe: ImpactDamage = 0.25f; break;
	}

	// Apply damage
	Health = FMath::Max(0.0f, Health - ImpactDamage);

	// Fire environmental impact event
	OnEnvironmentalImpactChanged.Broadcast(Planet, Impact);

	UE_LOG(LogTemp, Log, TEXT("Environmental health updated: -%.2f (Current: %.2f)"), ImpactDamage, Health);
}

float UPlanetaryMiningSystem::CalculateMiningDuration(const FResourceDeposit& Deposit, EPlanetaryMiningEquipment Equipment) const
{
	// Base duration based on deposit depth and quantity
	float BaseDuration = (Deposit.Depth * 0.1f) + (Deposit.MaxQuantity * 0.01f);

	// Adjust based on equipment power
	float EquipmentPower = GetEquipmentMiningPower(Equipment);
	float Duration = BaseDuration / EquipmentPower;

	// Apply skill bonus
	float SkillMultiplier = 1.0f + (MiningSkillLevel / 100.0f) * 0.5f;
	Duration /= SkillMultiplier;

	return FMath::Max(Duration, 2.0f); // Minimum 2 seconds
}

float UPlanetaryMiningSystem::CalculateEnergyConsumption(float Duration, EPlanetaryMiningEquipment Equipment) const
{
	float BaseConsumption = GetEquipmentMiningPower(Equipment) * 10.0f; // 10 energy per power unit per second
	return BaseConsumption * Duration;
}

bool UPlanetaryMiningSystem::IsEquipmentSuitableForDeposit(EPlanetaryMiningEquipment Equipment, EDepositType DepositType) const
{
	// Check equipment suitability for deposit type
	switch (DepositType)
	{
		case EDepositType::Surface:
			return Equipment == EPlanetaryMiningEquipment::HandDrill || 
				   Equipment == EPlanetaryMiningEquipment::SurfaceMiner || 
				   Equipment == EPlanetaryMiningEquipment::Excavator;

		case EDepositType::Underground:
			return Equipment == EPlanetaryMiningEquipment::DeepDrill || 
				   Equipment == EPlanetaryMiningEquipment::Excavator;

		case EDepositType::Deep:
			return Equipment == EPlanetaryMiningEquipment::DeepDrill;

		case EDepositType::Fracking:
			return Equipment == EPlanetaryMiningEquipment::FrackingRig;

		default:
			return false;
	}
}

bool UPlanetaryMiningSystem::DoesPlanetRequirePermits(APlanet* Planet) const
{
	if (!Planet)
	{
		return false;
	}

	// For now, assume all inhabited planets require permits
	// In a full implementation, this would check faction ownership and planet type
	return true;
}