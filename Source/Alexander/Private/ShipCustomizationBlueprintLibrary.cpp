// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipCustomizationBlueprintLibrary.h"
#include "ShipCustomizationComponent.h"
#include "ShipPhysicsConfig.h"
#include "GameFramework/Actor.h"

// ============================================================================
// COMPONENT ACCESS
// ============================================================================

UShipCustomizationComponent* UShipCustomizationBlueprintLibrary::GetShipCustomizationComponent(AActor* Ship)
{
	if (!Ship)
	{
		return nullptr;
	}

	return Ship->FindComponentByClass<UShipCustomizationComponent>();
}

// ============================================================================
// PHYSICS CONFIGURATION
// ============================================================================

bool UShipCustomizationBlueprintLibrary::ApplyPhysicsConfig(UShipCustomizationComponent* Component, UShipPhysicsConfig* Config)
{
	if (!Component || !Config)
	{
		return false;
	}

	Component->ApplyPhysicsConfig(Config);
	return true;
}

// ============================================================================
// PART MANAGEMENT
// ============================================================================

bool UShipCustomizationBlueprintLibrary::EquipPartByID(UShipCustomizationComponent* Component, EShipPartCategory Category, FName PartID)
{
	if (!Component)
	{
		return false;
	}

	return Component->EquipPart(Category, PartID);
}

bool UShipCustomizationBlueprintLibrary::UnequipPart(UShipCustomizationComponent* Component, EShipPartCategory Category)
{
	if (!Component)
	{
		return false;
	}

	return Component->UnequipPart(Category);
}

FShipPartData UShipCustomizationBlueprintLibrary::GetEquippedPart(UShipCustomizationComponent* Component, EShipPartCategory Category, bool& bFound)
{
	if (!Component)
	{
		bFound = false;
		return FShipPartData();
	}

	return Component->GetEquippedPart(Category, bFound);
}

// ============================================================================
// SKIN MANAGEMENT
// ============================================================================

bool UShipCustomizationBlueprintLibrary::EquipSkinByID(UShipCustomizationComponent* Component, FName SkinID)
{
	if (!Component)
	{
		return false;
	}

	return Component->EquipSkin(SkinID);
}

FShipSkinData UShipCustomizationBlueprintLibrary::GetEquippedSkin(UShipCustomizationComponent* Component, bool& bFound)
{
	if (!Component)
	{
		bFound = false;
		return FShipSkinData();
	}

	return Component->GetEquippedSkin(bFound);
}

// ============================================================================
// STATS
// ============================================================================

FShipStats UShipCustomizationBlueprintLibrary::GetCurrentStats(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return FShipStats();
	}

	return Component->GetTotalStats();
}

bool UShipCustomizationBlueprintLibrary::ApplyStatsToFlightController(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	Component->ApplyStatsToFlightController();
	return true;
}

// ============================================================================
// LOADOUT MANAGEMENT
// ============================================================================

bool UShipCustomizationBlueprintLibrary::SaveLoadout(UShipCustomizationComponent* Component, const FText& LoadoutName)
{
	if (!Component)
	{
		return false;
	}

	Component->SaveLoadout(LoadoutName);
	return true;
}

bool UShipCustomizationBlueprintLibrary::LoadLoadout(UShipCustomizationComponent* Component, int32 LoadoutIndex)
{
	if (!Component)
	{
		return false;
	}

	return Component->LoadLoadout(LoadoutIndex);
}

bool UShipCustomizationBlueprintLibrary::DeleteLoadout(UShipCustomizationComponent* Component, int32 LoadoutIndex)
{
	if (!Component)
	{
		return false;
	}

	return Component->DeleteLoadout(LoadoutIndex);
}

TArray<FShipLoadout> UShipCustomizationBlueprintLibrary::GetSavedLoadouts(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return TArray<FShipLoadout>();
	}

	return Component->GetSavedLoadouts();
}

// ============================================================================
// PROGRESSION
// ============================================================================

bool UShipCustomizationBlueprintLibrary::UnlockPart(UShipCustomizationComponent* Component, FName PartID)
{
	if (!Component)
	{
		return false;
	}

	return Component->UnlockPart(PartID);
}

bool UShipCustomizationBlueprintLibrary::UnlockSkin(UShipCustomizationComponent* Component, FName SkinID)
{
	if (!Component)
	{
		return false;
	}

	return Component->UnlockSkin(SkinID);
}

bool UShipCustomizationBlueprintLibrary::IsPartUnlocked(UShipCustomizationComponent* Component, FName PartID)
{
	if (!Component)
	{
		return false;
	}

	return Component->IsPartUnlocked(PartID);
}

bool UShipCustomizationBlueprintLibrary::IsSkinUnlocked(UShipCustomizationComponent* Component, FName SkinID)
{
	if (!Component)
	{
		return false;
	}

	return Component->IsSkinUnlocked(SkinID);
}

void UShipCustomizationBlueprintLibrary::AddXP(UShipCustomizationComponent* Component, int32 Amount)
{
	if (!Component)
	{
		return;
	}

	Component->AddXP(Amount);
}

void UShipCustomizationBlueprintLibrary::AddCredits(UShipCustomizationComponent* Component, int32 Amount)
{
	if (!Component)
	{
		return;
	}

	Component->AddCredits(Amount);
}

int32 UShipCustomizationBlueprintLibrary::GetPlayerLevel(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return 0;
	}

	return Component->GetPlayerLevel();
}

int32 UShipCustomizationBlueprintLibrary::GetCredits(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return 0;
	}

	return Component->GetCredits();
}

// ============================================================================
// DATA QUERIES
// ============================================================================

TArray<FShipPartData> UShipCustomizationBlueprintLibrary::GetPartsInCategory(UShipCustomizationComponent* Component, EShipPartCategory Category)
{
	if (!Component)
	{
		return TArray<FShipPartData>();
	}

	return Component->GetPartsInCategory(Category);
}

TArray<FShipPartData> UShipCustomizationBlueprintLibrary::GetUnlockedPartsInCategory(UShipCustomizationComponent* Component, EShipPartCategory Category)
{
	if (!Component)
	{
		return TArray<FShipPartData>();
	}

	return Component->GetUnlockedPartsInCategory(Category);
}

TArray<FShipSkinData> UShipCustomizationBlueprintLibrary::GetAllSkins(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return TArray<FShipSkinData>();
	}

	return Component->GetAllSkins();
}

TArray<FShipSkinData> UShipCustomizationBlueprintLibrary::GetUnlockedSkins(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return TArray<FShipSkinData>();
	}

	return Component->GetUnlockedSkins();
}

FShipPartData UShipCustomizationBlueprintLibrary::GetPartData(UShipCustomizationComponent* Component, FName PartID, bool& bFound)
{
	if (!Component)
	{
		bFound = false;
		return FShipPartData();
	}

	return Component->GetPartData(PartID, bFound);
}

FShipSkinData UShipCustomizationBlueprintLibrary::GetSkinData(UShipCustomizationComponent* Component, FName SkinID, bool& bFound)
{
	if (!Component)
	{
		bFound = false;
		return FShipSkinData();
	}

	return Component->GetSkinData(SkinID, bFound);
}

// ============================================================================
// SAVE/LOAD
// ============================================================================

bool UShipCustomizationBlueprintLibrary::SaveCustomizationData(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	return Component->SaveCustomizationData();
}

bool UShipCustomizationBlueprintLibrary::LoadCustomizationData(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	return Component->LoadCustomizationData();
}

// ============================================================================
// VISUAL UPDATES
// ============================================================================

bool UShipCustomizationBlueprintLibrary::UpdateShipVisuals(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	Component->UpdateShipVisuals();
	return true;
}

bool UShipCustomizationBlueprintLibrary::ApplySkinMaterial(UShipCustomizationComponent* Component)
{
	if (!Component)
	{
		return false;
	}

	Component->ApplySkinMaterial();
	return true;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

FText UShipCustomizationBlueprintLibrary::FormatStatsForDisplay(const FShipStats& Stats)
{
	FString FormattedString = FString::Printf(
		TEXT("Thrust Power: %.1f\n")
		TEXT("Max Velocity: %.1f\n")
		TEXT("Rotation Speed: %.1f\n")
		TEXT("Acceleration: %.1f\n")
		TEXT("Hull Integrity: %.0f\n")
		TEXT("Shield Strength: %.0f\n")
		TEXT("Weapon Damage: %.1f\n")
		TEXT("Energy Capacity: %.0f\n")
		TEXT("Energy Regen: %.1f\n")
		TEXT("Mass: %.0f kg"),
		Stats.ThrustPower,
		Stats.MaxVelocity,
		Stats.RotationSpeed,
		Stats.Acceleration,
		Stats.HullIntegrity,
		Stats.ShieldStrength,
		Stats.WeaponDamage,
		Stats.EnergyCapacity,
		Stats.EnergyRegenRate,
		Stats.Mass
	);

	return FText::FromString(FormattedString);
}

FLinearColor UShipCustomizationBlueprintLibrary::GetRarityColor(EShipPartRarity Rarity)
{
	switch (Rarity)
	{
	case EShipPartRarity::Common:
		return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
	case EShipPartRarity::Uncommon:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // Green
	case EShipPartRarity::Rare:
		return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f); // Blue
	case EShipPartRarity::Epic:
		return FLinearColor(0.7f, 0.2f, 1.0f, 1.0f); // Purple
	case EShipPartRarity::Legendary:
		return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange/Gold
	default:
		return FLinearColor::White;
	}
}

FShipStats UShipCustomizationBlueprintLibrary::CompareShipStats(const FShipStats& StatsA, const FShipStats& StatsB)
{
	FShipStats Difference;
	Difference.ThrustPower = StatsB.ThrustPower - StatsA.ThrustPower;
	Difference.MaxVelocity = StatsB.MaxVelocity - StatsA.MaxVelocity;
	Difference.RotationSpeed = StatsB.RotationSpeed - StatsA.RotationSpeed;
	Difference.Acceleration = StatsB.Acceleration - StatsA.Acceleration;
	Difference.HullIntegrity = StatsB.HullIntegrity - StatsA.HullIntegrity;
	Difference.ShieldStrength = StatsB.ShieldStrength - StatsA.ShieldStrength;
	Difference.WeaponDamage = StatsB.WeaponDamage - StatsA.WeaponDamage;
	Difference.EnergyCapacity = StatsB.EnergyCapacity - StatsA.EnergyCapacity;
	Difference.EnergyRegenRate = StatsB.EnergyRegenRate - StatsA.EnergyRegenRate;
	Difference.Mass = StatsB.Mass - StatsA.Mass;

	return Difference;
}
