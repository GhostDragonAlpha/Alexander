// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipCustomizationComponent.h"
#include "FlightController.h"
#include "OrbitalBody.h"
#include "Spaceship.h"
#include "ShipPhysicsConfig.h"
#include "ShipCustomizationProfiling.h"
#include "ShipCustomizationProfiler.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "HAL/PlatformTime.h"

// SaveGame class for persistence (simple internal class, no UCLASS needed)
class UShipCustomizationSaveGame : public USaveGame
{
public:
	FPlayerProgressionData ProgressionData;
	FShipLoadout CurrentLoadout;
};

UShipCustomizationComponent::UShipCustomizationComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Only tick if needed
	SetIsReplicated(true);

	// Initialize empty loadout
	CurrentLoadout.LoadoutName = FText::FromString("Default Loadout");
	CurrentLoadout.EquippedSkin = NAME_None;

	// Initialize progression
	ProgressionData.PlayerLevel = 1;
	ProgressionData.TotalXP = 0;
	ProgressionData.Credits = 1000; // Starting credits
	ProgressionData.ActiveLoadoutIndex = 0;

	// Cached references
	CachedFlightController = nullptr;
	CachedShipMesh = nullptr;
	CachedOrbitalBody = nullptr;
}

void UShipCustomizationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShipCustomizationComponent, CurrentLoadout);
}

void UShipCustomizationComponent::BeginPlay()
{
	PROFILE_SHIP_CUSTOMIZATION_SCOPE(BeginPlay);

	Super::BeginPlay();

	// Load saved customization data
	LoadCustomizationData();

	// Initialize default loadout if needed
	if (CurrentLoadout.EquippedParts.Num() == 0)
	{
		InitializeDefaultLoadout();
	}

	// Calculate and apply stats
	CurrentLoadout.TotalStats = CalculateTotalStats();
	ApplyStatsToFlightController();

	// Update visuals
	UpdateShipVisuals();

	UE_LOG(LogTemp, Log, TEXT("ShipCustomizationComponent initialized - Level %d, %d parts equipped"),
		ProgressionData.PlayerLevel, CurrentLoadout.EquippedParts.Num());
}

void UShipCustomizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ============================================================================
// LOADOUT MANAGEMENT
// ============================================================================

bool UShipCustomizationComponent::EquipPart(EShipPartCategory Category, FName PartID)
{
	SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_EquipPart);
	double StartTime = FPlatformTime::Seconds();

	// Check if part exists
	bool bFound = false;
	FShipPartData PartData = GetPartData(PartID, bFound);

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Part not found: %s"), *PartID.ToString());
		return false;
	}

	// Check if part is unlocked
	if (!IsPartUnlocked(PartID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Part not unlocked: %s"), *PartID.ToString());
		return false;
	}

	// Check level requirement
	if (PartData.LevelRequirement > ProgressionData.PlayerLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Part requires level %d (current: %d)"),
			PartData.LevelRequirement, ProgressionData.PlayerLevel);
		return false;
	}

	// Check category matches
	if (PartData.Category != Category)
	{
		UE_LOG(LogTemp, Warning, TEXT("Part category mismatch"));
		return false;
	}

	// Equip the part
	CurrentLoadout.EquippedParts.Add(Category, PartID);

	// Recalculate stats
	CurrentLoadout.TotalStats = CalculateTotalStats();
	ApplyStatsToFlightController();

	// Update visuals
	UpdateShipVisuals();

	// Fire events
	OnPartEquipped.Broadcast(Category, PartID);
	OnLoadoutChanged.Broadcast();

	// Profile timing
	double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Find profiler component and record timing
	UShipCustomizationProfiler* Profiler = GetOwner() ? GetOwner()->FindComponentByClass<UShipCustomizationProfiler>() : nullptr;
	if (Profiler)
	{
		Profiler->RecordEquipPartTime(ElapsedMS);
	}

	UE_LOG(LogTemp, Log, TEXT("Equipped part: %s (%s)"),
		*PartData.DisplayName.ToString(), *PartID.ToString());

	return true;
}

bool UShipCustomizationComponent::UnequipPart(EShipPartCategory Category)
{
	if (!CurrentLoadout.EquippedParts.Contains(Category))
	{
		return false;
	}

	CurrentLoadout.EquippedParts.Remove(Category);

	// Recalculate stats
	CurrentLoadout.TotalStats = CalculateTotalStats();
	ApplyStatsToFlightController();

	// Update visuals
	UpdateShipVisuals();

	// Fire event
	OnLoadoutChanged.Broadcast();

	return true;
}

bool UShipCustomizationComponent::EquipSkin(FName SkinID)
{
	// Check if skin exists
	bool bFound = false;
	FShipSkinData SkinData = GetSkinData(SkinID, bFound);

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skin not found: %s"), *SkinID.ToString());
		return false;
	}

	// Check if skin is unlocked
	if (!IsSkinUnlocked(SkinID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Skin not unlocked: %s"), *SkinID.ToString());
		return false;
	}

	// Equip the skin
	CurrentLoadout.EquippedSkin = SkinID;

	// Update visuals
	ApplySkinMaterial();

	// Fire events
	OnSkinEquipped.Broadcast(SkinID);
	OnLoadoutChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Equipped skin: %s"), *SkinData.DisplayName.ToString());

	return true;
}

FShipPartData UShipCustomizationComponent::GetEquippedPart(EShipPartCategory Category, bool& bFound)
{
	bFound = false;

	if (!CurrentLoadout.EquippedParts.Contains(Category))
	{
		return FShipPartData();
	}

	FName PartID = CurrentLoadout.EquippedParts[Category];
	return GetPartData(PartID, bFound);
}

FShipSkinData UShipCustomizationComponent::GetEquippedSkin(bool& bFound)
{
	if (CurrentLoadout.EquippedSkin.IsNone())
	{
		bFound = false;
		return FShipSkinData();
	}

	return GetSkinData(CurrentLoadout.EquippedSkin, bFound);
}

// ============================================================================
// STATS CALCULATION
// ============================================================================

FShipStats UShipCustomizationComponent::CalculateTotalStats()
{
	SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_CalculateStats);
	double StartTime = FPlatformTime::Seconds();

	FShipStats TotalStats;

	// Start with base stats
	TotalStats.ThrustPower = 1.0f;
	TotalStats.MaxVelocity = 1.0f;
	TotalStats.RotationSpeed = 1.0f;
	TotalStats.Acceleration = 1.0f;
	TotalStats.HullIntegrity = 100.0f;
	TotalStats.ShieldStrength = 0.0f;
	TotalStats.WeaponDamage = 1.0f;
	TotalStats.EnergyCapacity = 100.0f;
	TotalStats.EnergyRegenRate = 10.0f;
	TotalStats.Mass = 1000.0f;

	// Add stats from all equipped parts
	for (const TPair<EShipPartCategory, FName>& EquippedPart : CurrentLoadout.EquippedParts)
	{
		bool bFound = false;
		FShipPartData PartData = GetPartData(EquippedPart.Value, bFound);

		if (bFound)
		{
			TotalStats = TotalStats + PartData.StatModifiers;
		}
	}

	// Profile timing
	double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Find profiler component and record timing
	UShipCustomizationProfiler* Profiler = GetOwner() ? GetOwner()->FindComponentByClass<UShipCustomizationProfiler>() : nullptr;
	if (Profiler)
	{
		Profiler->RecordCalculateStatsTime(ElapsedMS);
	}

	return TotalStats;
}

void UShipCustomizationComponent::ApplyStatsToFlightController()
{
	SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_ApplyStats);
	double StartTime = FPlatformTime::Seconds();

	AOrbitalBody* OrbitalBody = GetOrbitalBody();
	UFlightController* FlightController = GetFlightController();

	if (!OrbitalBody || !FlightController)
	{
		UE_LOG(LogTemp, Warning, TEXT("OrbitalBody (%s) or FlightController (%s) not found, cannot apply stats"),
			OrbitalBody ? TEXT("OK") : TEXT("MISSING"),
			FlightController ? TEXT("OK") : TEXT("MISSING"));
		return;
	}

	FShipStats Stats = CurrentLoadout.TotalStats;

	// ============================================================================
	// PHASE 3: NETWORK REPLICATION INTEGRATION
	// ============================================================================

	// Check if we're in multiplayer
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// If server (or standalone), apply directly
	if (Owner->HasAuthority())
	{
		// ============================================================================
		// INTEGRATION WITH ORBITAL BODY (N-Body Physics)
		// ============================================================================

		// Apply Mass to OrbitalBody - THIS AFFECTS GRAVITY CALCULATIONS!
		// F = G * m1 * m2 / r²
		// Heavier ships are pulled MORE by planets, but also have more inertia
		OrbitalBody->Mass = Stats.Mass;

		// Apply ThrustPower (Phase 3: replicate to clients)
		// NOTE: OrbitalBody has ThrustPower property, Spaceship uses MaxThrust
		OrbitalBody->ThrustPower = Stats.ThrustPower * 100000.0f; // Convert multiplier to Newtons

		// If this is a Spaceship, also set MaxThrust for compatibility
		if (ASpaceship* Spaceship = Cast<ASpaceship>(OrbitalBody))
		{
			Spaceship->MaxThrust = Stats.ThrustPower * 100000.0f;
		}

		// Recalculate standard gravitational parameter (μ = G * Mass)
		// This is used by orbital mechanics calculations
		const float GravitationalConstant = 6.67430e-11f; // m³/(kg·s²)
		OrbitalBody->StandardGravitationalParameter = GravitationalConstant * Stats.Mass;

		// ============================================================================
		// INTEGRATION WITH FLIGHT CONTROLLER
		// ============================================================================

		// Apply MaxVelocity to FlightController's safety limit
		FlightController->MaxSafeVelocity = Stats.MaxVelocity * 1000.0f; // Convert to m/s (base is in km/s multiplier)

		UE_LOG(LogTemp, Log, TEXT("Applied ship stats to physics system (SERVER):"));
		UE_LOG(LogTemp, Log, TEXT("  - Mass: %.2f kg"), Stats.Mass);
		UE_LOG(LogTemp, Log, TEXT("  - ThrustPower: %.2f N"), OrbitalBody->ThrustPower);
		UE_LOG(LogTemp, Log, TEXT("  - MaxVelocity: %.2f m/s"), FlightController->MaxSafeVelocity);
	}
	else
	{
		// ============================================================================
		// CLIENT: Request server to apply stats via RPC
		// ============================================================================

		if (ASpaceship* Spaceship = Cast<ASpaceship>(OrbitalBody))
		{
			// Call server RPC to apply customization with authority
			Spaceship->ServerApplyCustomization(
				Stats.Mass,
				Stats.ThrustPower * 100000.0f, // Scale to Newtons
				Stats.MaxVelocity * 1000.0f    // Scale to m/s
			);

			UE_LOG(LogTemp, Log, TEXT("Requested server to apply ship stats (CLIENT):"));
			UE_LOG(LogTemp, Log, TEXT("  - Mass: %.2f kg"), Stats.Mass);
			UE_LOG(LogTemp, Log, TEXT("  - ThrustPower: %.2f N"), Stats.ThrustPower * 100000.0f);
			UE_LOG(LogTemp, Log, TEXT("  - MaxVelocity: %.2f m/s"), Stats.MaxVelocity * 1000.0f);
		}

		// Still apply FlightController settings locally for immediate feedback
		// (server will replicate authoritative values)
		FlightController->MaxSafeVelocity = Stats.MaxVelocity * 1000.0f;
	}

	// Profile timing
	double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Find profiler component and record timing
	UShipCustomizationProfiler* Profiler = GetOwner() ? GetOwner()->FindComponentByClass<UShipCustomizationProfiler>() : nullptr;
	if (Profiler)
	{
		Profiler->RecordApplyStatsTime(ElapsedMS);
	}
}

// ============================================================================
// PHYSICS CONFIGURATION
// ============================================================================

void UShipCustomizationComponent::ApplyPhysicsConfig(UShipPhysicsConfig* Config)
{
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot apply null physics config"));
		return;
	}

	PhysicsConfig = Config;

	// Override base stats with config values
	FShipStats BaseStats;
	BaseStats.Mass = Config->Mass;
	BaseStats.ThrustPower = Config->ThrustPower / 100000.0f; // Convert to multiplier
	BaseStats.MaxVelocity = Config->MaxVelocity / 1000.0f;   // Convert to multiplier
	BaseStats.RotationSpeed = Config->RotationSpeedMultiplier;
	BaseStats.Acceleration = Config->AccelerationMultiplier;

	// Recalculate with current parts
	CurrentLoadout.TotalStats = BaseStats + CalculateTotalStats();
	ApplyStatsToFlightController();

	UE_LOG(LogTemp, Log, TEXT("Applied physics config: %s"), *Config->ConfigName.ToString());
	UE_LOG(LogTemp, Log, TEXT("  - Mass: %.2f kg"), Config->Mass);
	UE_LOG(LogTemp, Log, TEXT("  - ThrustPower: %.2f N"), Config->ThrustPower);
	UE_LOG(LogTemp, Log, TEXT("  - MaxVelocity: %.2f m/s"), Config->MaxVelocity);
	UE_LOG(LogTemp, Log, TEXT("  - AngularThrustPower: %.2f N"), Config->AngularThrustPower);
	UE_LOG(LogTemp, Log, TEXT("  - AccelerationMultiplier: %.2f"), Config->AccelerationMultiplier);
	UE_LOG(LogTemp, Log, TEXT("  - RotationSpeedMultiplier: %.2f"), Config->RotationSpeedMultiplier);
	UE_LOG(LogTemp, Log, TEXT("  - StabilityAssistStrength: %.2f"), Config->StabilityAssistStrength);
}

// ============================================================================
// PROGRESSION
// ============================================================================

bool UShipCustomizationComponent::UnlockPart(FName PartID)
{
	// Check if part exists
	bool bFound = false;
	FShipPartData PartData = GetPartData(PartID, bFound);

	if (!bFound)
	{
		return false;
	}

	// Check if already unlocked
	if (IsPartUnlocked(PartID))
	{
		return true; // Already unlocked
	}

	// Check if player has enough credits
	if (ProgressionData.Credits < PartData.UnlockCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough credits to unlock %s (need %d, have %d)"),
			*PartData.DisplayName.ToString(), PartData.UnlockCost, ProgressionData.Credits);
		return false;
	}

	// Deduct credits
	ProgressionData.Credits -= PartData.UnlockCost;

	// Add to unlocked parts
	ProgressionData.UnlockedParts.AddUnique(PartID);

	// Fire event
	OnPartUnlocked.Broadcast(PartID);

	// Save progress
	SaveCustomizationData();

	UE_LOG(LogTemp, Log, TEXT("Unlocked part: %s"), *PartData.DisplayName.ToString());

	return true;
}

bool UShipCustomizationComponent::UnlockSkin(FName SkinID)
{
	// Check if skin exists
	bool bFound = false;
	FShipSkinData SkinData = GetSkinData(SkinID, bFound);

	if (!bFound)
	{
		return false;
	}

	// Check if already unlocked
	if (IsSkinUnlocked(SkinID))
	{
		return true; // Already unlocked
	}

	// Check if player has enough credits
	if (ProgressionData.Credits < SkinData.UnlockCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough credits to unlock skin %s"),
			*SkinData.DisplayName.ToString());
		return false;
	}

	// Deduct credits
	ProgressionData.Credits -= SkinData.UnlockCost;

	// Add to unlocked skins
	ProgressionData.UnlockedSkins.AddUnique(SkinID);

	// Save progress
	SaveCustomizationData();

	UE_LOG(LogTemp, Log, TEXT("Unlocked skin: %s"), *SkinData.DisplayName.ToString());

	return true;
}

bool UShipCustomizationComponent::IsPartUnlocked(FName PartID) const
{
	// Check if part is unlocked by default
	bool bFound = false;
	FShipPartData PartData = const_cast<UShipCustomizationComponent*>(this)->GetPartData(PartID, bFound);

	if (bFound && PartData.bUnlockedByDefault)
	{
		return true;
	}

	return ProgressionData.UnlockedParts.Contains(PartID);
}

bool UShipCustomizationComponent::IsSkinUnlocked(FName SkinID) const
{
	// Check if skin is unlocked by default
	bool bFound = false;
	FShipSkinData SkinData = const_cast<UShipCustomizationComponent*>(this)->GetSkinData(SkinID, bFound);

	if (bFound && SkinData.bUnlockedByDefault)
	{
		return true;
	}

	return ProgressionData.UnlockedSkins.Contains(SkinID);
}

void UShipCustomizationComponent::AddXP(int32 Amount)
{
	int32 OldLevel = ProgressionData.PlayerLevel;

	ProgressionData.TotalXP += Amount;

	// Check for level up
	CheckLevelUp();

	if (ProgressionData.PlayerLevel > OldLevel)
	{
		OnPlayerLevelUp.Broadcast(OldLevel, ProgressionData.PlayerLevel);

		UE_LOG(LogTemp, Log, TEXT("Player leveled up! %d -> %d"), OldLevel, ProgressionData.PlayerLevel);
	}

	// Save progress
	SaveCustomizationData();
}

void UShipCustomizationComponent::AddCredits(int32 Amount)
{
	ProgressionData.Credits += Amount;

	// Save progress
	SaveCustomizationData();

	UE_LOG(LogTemp, Log, TEXT("Added %d credits (total: %d)"), Amount, ProgressionData.Credits);
}

// ============================================================================
// LOADOUT PRESETS
// ============================================================================

void UShipCustomizationComponent::SaveLoadout(const FText& LoadoutName)
{
	FShipLoadout NewLoadout = CurrentLoadout;
	NewLoadout.LoadoutName = LoadoutName;

	ProgressionData.SavedLoadouts.Add(NewLoadout);

	// Save to disk
	SaveCustomizationData();

	UE_LOG(LogTemp, Log, TEXT("Saved loadout: %s"), *LoadoutName.ToString());
}

bool UShipCustomizationComponent::LoadLoadout(int32 LoadoutIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_LoadLoadout);
	double StartTime = FPlatformTime::Seconds();

	if (!ProgressionData.SavedLoadouts.IsValidIndex(LoadoutIndex))
	{
		return false;
	}

	CurrentLoadout = ProgressionData.SavedLoadouts[LoadoutIndex];
	ProgressionData.ActiveLoadoutIndex = LoadoutIndex;

	// Recalculate stats
	CurrentLoadout.TotalStats = CalculateTotalStats();
	ApplyStatsToFlightController();

	// Update visuals
	UpdateShipVisuals();

	// Fire event
	OnLoadoutChanged.Broadcast();

	// Profile timing
	double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Find profiler component and record timing
	UShipCustomizationProfiler* Profiler = GetOwner() ? GetOwner()->FindComponentByClass<UShipCustomizationProfiler>() : nullptr;
	if (Profiler)
	{
		Profiler->RecordLoadLoadoutTime(ElapsedMS);
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded loadout: %s"), *CurrentLoadout.LoadoutName.ToString());

	return true;
}

bool UShipCustomizationComponent::DeleteLoadout(int32 LoadoutIndex)
{
	if (!ProgressionData.SavedLoadouts.IsValidIndex(LoadoutIndex))
	{
		return false;
	}

	ProgressionData.SavedLoadouts.RemoveAt(LoadoutIndex);

	// Update active loadout index if needed
	if (ProgressionData.ActiveLoadoutIndex >= ProgressionData.SavedLoadouts.Num())
	{
		ProgressionData.ActiveLoadoutIndex = FMath::Max(0, ProgressionData.SavedLoadouts.Num() - 1);
	}

	// Save to disk
	SaveCustomizationData();

	return true;
}

// ============================================================================
// VISUAL UPDATES
// ============================================================================

void UShipCustomizationComponent::UpdateShipVisuals()
{
	SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_UpdateVisuals);
	double StartTime = FPlatformTime::Seconds();

	// Apply skin material
	ApplySkinMaterial();

	// Update part meshes
	// TODO: Implement part mesh swapping based on equipped parts
	// This would involve having attachment points on the ship mesh
	// and swapping out component meshes based on equipped parts

	// Profile timing
	double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Find profiler component and record timing
	UShipCustomizationProfiler* Profiler = GetOwner() ? GetOwner()->FindComponentByClass<UShipCustomizationProfiler>() : nullptr;
	if (Profiler)
	{
		Profiler->RecordUpdateVisualsTime(ElapsedMS);
	}

	UE_LOG(LogTemp, Log, TEXT("Updated ship visuals"));
}

void UShipCustomizationComponent::ApplySkinMaterial()
{
	UStaticMeshComponent* ShipMesh = GetShipMesh();

	if (!ShipMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ship mesh not found"));
		return;
	}

	if (CurrentLoadout.EquippedSkin.IsNone())
	{
		// No skin equipped, use default
		return;
	}

	// Get skin data
	bool bFound = false;
	FShipSkinData SkinData = GetSkinData(CurrentLoadout.EquippedSkin, bFound);

	if (!bFound || SkinData.SkinMaterial.IsNull())
	{
		return;
	}

	// Load material if needed
	UMaterialInstance* Material = SkinData.SkinMaterial.LoadSynchronous();

	if (Material)
	{
		// Apply material to all mesh elements
		for (int32 i = 0; i < ShipMesh->GetNumMaterials(); i++)
		{
			ShipMesh->SetMaterial(i, Material);
		}

		UE_LOG(LogTemp, Log, TEXT("Applied skin material: %s"), *SkinData.DisplayName.ToString());
	}
}

// ============================================================================
// DATA QUERIES
// ============================================================================

TArray<FShipPartData> UShipCustomizationComponent::GetPartsInCategory(EShipPartCategory Category)
{
	TArray<FShipPartData> Parts;

	if (!ShipPartsDataTable)
	{
		return Parts;
	}

	TArray<FName> RowNames = ShipPartsDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(RowName, TEXT(""));

		if (PartData && PartData->Category == Category)
		{
			Parts.Add(*PartData);
		}
	}

	return Parts;
}

TArray<FShipPartData> UShipCustomizationComponent::GetUnlockedPartsInCategory(EShipPartCategory Category)
{
	TArray<FShipPartData> AllParts = GetPartsInCategory(Category);
	TArray<FShipPartData> UnlockedParts;

	for (const FShipPartData& Part : AllParts)
	{
		if (IsPartUnlocked(Part.PartID))
		{
			UnlockedParts.Add(Part);
		}
	}

	return UnlockedParts;
}

TArray<FShipSkinData> UShipCustomizationComponent::GetAllSkins()
{
	TArray<FShipSkinData> Skins;

	if (!ShipSkinsDataTable)
	{
		return Skins;
	}

	TArray<FName> RowNames = ShipSkinsDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FShipSkinData* SkinData = ShipSkinsDataTable->FindRow<FShipSkinData>(RowName, TEXT(""));

		if (SkinData)
		{
			Skins.Add(*SkinData);
		}
	}

	return Skins;
}

TArray<FShipSkinData> UShipCustomizationComponent::GetUnlockedSkins()
{
	TArray<FShipSkinData> AllSkins = GetAllSkins();
	TArray<FShipSkinData> UnlockedSkins;

	for (const FShipSkinData& Skin : AllSkins)
	{
		if (IsSkinUnlocked(Skin.SkinID))
		{
			UnlockedSkins.Add(Skin);
		}
	}

	return UnlockedSkins;
}

FShipPartData UShipCustomizationComponent::GetPartData(FName PartID, bool& bFound)
{
	bFound = false;

	if (!ShipPartsDataTable)
	{
		return FShipPartData();
	}

	FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(PartID, TEXT(""));

	if (PartData)
	{
		bFound = true;
		return *PartData;
	}

	return FShipPartData();
}

FShipSkinData UShipCustomizationComponent::GetSkinData(FName SkinID, bool& bFound)
{
	bFound = false;

	if (!ShipSkinsDataTable)
	{
		return FShipSkinData();
	}

	FShipSkinData* SkinData = ShipSkinsDataTable->FindRow<FShipSkinData>(SkinID, TEXT(""));

	if (SkinData)
	{
		bFound = true;
		return *SkinData;
	}

	return FShipSkinData();
}

// ============================================================================
// SAVE/LOAD
// ============================================================================

bool UShipCustomizationComponent::SaveCustomizationData()
{
	UShipCustomizationSaveGame* SaveGameInstance = Cast<UShipCustomizationSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UShipCustomizationSaveGame::StaticClass()));

	if (!SaveGameInstance)
	{
		return false;
	}

	// Copy data to save game
	SaveGameInstance->ProgressionData = ProgressionData;
	SaveGameInstance->CurrentLoadout = CurrentLoadout;

	// Save to slot
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("ShipCustomization"), 0);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Saved customization data"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save customization data"));
	}

	return bSuccess;
}

bool UShipCustomizationComponent::LoadCustomizationData()
{
	UShipCustomizationSaveGame* LoadedGame = Cast<UShipCustomizationSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("ShipCustomization"), 0));

	if (!LoadedGame)
	{
		UE_LOG(LogTemp, Log, TEXT("No saved customization data found, using defaults"));
		return false;
	}

	// Load data from save game
	ProgressionData = LoadedGame->ProgressionData;
	CurrentLoadout = LoadedGame->CurrentLoadout;

	UE_LOG(LogTemp, Log, TEXT("Loaded customization data - Level %d, %d unlocked parts"),
		ProgressionData.PlayerLevel, ProgressionData.UnlockedParts.Num());

	return true;
}

// ============================================================================
// PROTECTED HELPERS
// ============================================================================

void UShipCustomizationComponent::InitializeDefaultLoadout()
{
	// Find all default-unlocked parts and equip one from each category
	if (!ShipPartsDataTable)
	{
		return;
	}

	TArray<FName> RowNames = ShipPartsDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(RowName, TEXT(""));

		if (PartData && PartData->bUnlockedByDefault)
		{
			// Equip first default part in each category
			if (!CurrentLoadout.EquippedParts.Contains(PartData->Category))
			{
				CurrentLoadout.EquippedParts.Add(PartData->Category, PartData->PartID);

				UE_LOG(LogTemp, Log, TEXT("Default equipped: %s"), *PartData->DisplayName.ToString());
			}
		}
	}

	// Equip default skin if available
	if (ShipSkinsDataTable)
	{
		TArray<FName> SkinRows = ShipSkinsDataTable->GetRowNames();

		for (const FName& SkinRow : SkinRows)
		{
			FShipSkinData* SkinData = ShipSkinsDataTable->FindRow<FShipSkinData>(SkinRow, TEXT(""));

			if (SkinData && SkinData->bUnlockedByDefault)
			{
				CurrentLoadout.EquippedSkin = SkinData->SkinID;
				break;
			}
		}
	}
}

int32 UShipCustomizationComponent::GetXPForLevel(int32 Level) const
{
	// Exponential XP curve: Level^2 * 100
	return Level * Level * 100;
}

void UShipCustomizationComponent::CheckLevelUp()
{
	while (ProgressionData.TotalXP >= GetXPForLevel(ProgressionData.PlayerLevel + 1))
	{
		ProgressionData.PlayerLevel++;

		// Award credits on level up
		int32 CreditReward = ProgressionData.PlayerLevel * 100;
		ProgressionData.Credits += CreditReward;

		UE_LOG(LogTemp, Log, TEXT("Level up! New level: %d, awarded %d credits"),
			ProgressionData.PlayerLevel, CreditReward);
	}
}

UFlightController* UShipCustomizationComponent::GetFlightController() const
{
	if (CachedFlightController)
	{
		return CachedFlightController;
	}

	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return nullptr;
	}

	CachedFlightController = Owner->FindComponentByClass<UFlightController>();

	return CachedFlightController;
}

UStaticMeshComponent* UShipCustomizationComponent::GetShipMesh() const
{
	if (CachedShipMesh)
	{
		return CachedShipMesh;
	}

	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return nullptr;
	}

	// Find first static mesh component
	TArray<UStaticMeshComponent*> MeshComponents;
	Owner->GetComponents<UStaticMeshComponent>(MeshComponents);

	if (MeshComponents.Num() > 0)
	{
		CachedShipMesh = MeshComponents[0];
	}

	return CachedShipMesh;
}

AOrbitalBody* UShipCustomizationComponent::GetOrbitalBody() const
{
	if (CachedOrbitalBody)
	{
		return CachedOrbitalBody;
	}

	AActor* Owner = GetOwner();

	if (!Owner)
	{
		return nullptr;
	}

	// Check if owner IS an OrbitalBody (e.g., ASpaceship : public AOrbitalBody)
	CachedOrbitalBody = Cast<AOrbitalBody>(Owner);

	// If cast failed, owner is not derived from OrbitalBody - this is expected for non-physics ships
	if (!CachedOrbitalBody)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ShipCustomizationComponent: Owner %s is not derived from OrbitalBody"),
			Owner ? *Owner->GetName() : TEXT("NULL"));
	}

	return CachedOrbitalBody;
}
