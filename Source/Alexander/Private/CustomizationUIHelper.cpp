// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationUIHelper.h"
#include "ShipCustomizationBlueprintLibrary.h"
#include "Engine/Texture2D.h"

// ============================================================================
// STAT FORMATTING
// ============================================================================

FText UCustomizationUIHelper::FormatStatMultiplier(float StatValue, int32 DecimalPlaces)
{
	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = DecimalPlaces;
	FormatOptions.MaximumFractionalDigits = DecimalPlaces;
	FString ValueString = FText::AsNumber(StatValue, &FormatOptions).ToString();
	return FText::FromString(ValueString + TEXT("x"));
}

FText UCustomizationUIHelper::FormatStatValue(float StatValue, int32 DecimalPlaces)
{
	// Format with thousand separators and decimal places
	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = DecimalPlaces;
	FormatOptions.MaximumFractionalDigits = DecimalPlaces;
	FormatOptions.UseGrouping = true;
	return FText::AsNumber(StatValue, &FormatOptions);
}

FText UCustomizationUIHelper::FormatStatDifference(float StatDifference, bool bIsMultiplier, int32 DecimalPlaces)
{
	if (FMath::IsNearlyZero(StatDifference, 0.01f))
	{
		return FText::FromString(TEXT("--"));
	}

	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = DecimalPlaces;
	FormatOptions.MaximumFractionalDigits = DecimalPlaces;
	FString ValueString = FText::AsNumber(FMath::Abs(StatDifference), &FormatOptions).ToString();

	FString Prefix = StatDifference > 0.0f ? TEXT("+") : TEXT("-");
	FString Suffix = bIsMultiplier ? TEXT("x") : TEXT("");

	return FText::FromString(Prefix + ValueString + Suffix);
}

FLinearColor UCustomizationUIHelper::GetStatDifferenceColor(float StatDifference, bool bLowerIsBetter)
{
	if (FMath::IsNearlyZero(StatDifference, 0.01f))
	{
		return GetNeutralColor();
	}

	bool bIsPositive = bLowerIsBetter ? (StatDifference < 0.0f) : (StatDifference > 0.0f);

	return bIsPositive ? GetPositiveColor() : GetNegativeColor();
}

FText UCustomizationUIHelper::FormatShipStats(const FShipStats& Stats, bool bShowZeroValues)
{
	TArray<FString> StatLines;

	// Helper lambda to add stat line
	auto AddStatLine = [&](const FString& Name, float Value, bool bIsMultiplier)
	{
		if (!bShowZeroValues && FMath::IsNearlyZero(Value, 0.01f))
		{
			return;
		}

		FText FormattedValue = bIsMultiplier ? FormatStatMultiplier(Value, 2) : FormatStatValue(Value, 0);
		StatLines.Add(FString::Printf(TEXT("%s: %s"), *Name, *FormattedValue.ToString()));
	};

	// Add all stats
	AddStatLine(TEXT("Thrust Power"), Stats.ThrustPower, true);
	AddStatLine(TEXT("Max Velocity"), Stats.MaxVelocity, true);
	AddStatLine(TEXT("Rotation Speed"), Stats.RotationSpeed, true);
	AddStatLine(TEXT("Acceleration"), Stats.Acceleration, true);
	AddStatLine(TEXT("Hull Integrity"), Stats.HullIntegrity, false);
	AddStatLine(TEXT("Shield Strength"), Stats.ShieldStrength, false);
	AddStatLine(TEXT("Weapon Damage"), Stats.WeaponDamage, true);
	AddStatLine(TEXT("Energy Capacity"), Stats.EnergyCapacity, false);
	AddStatLine(TEXT("Energy Regen"), Stats.EnergyRegenRate, false);
	AddStatLine(TEXT("Mass"), Stats.Mass, false);

	return FText::FromString(FString::Join(StatLines, TEXT("\n")));
}

FText UCustomizationUIHelper::FormatStatsComparison(const FShipStats& CurrentStats, const FShipStats& NewStats)
{
	TArray<FString> ComparisonLines;

	// Helper lambda to add comparison line
	auto AddComparisonLine = [&](const FString& Name, float Current, float New, bool bIsMultiplier, bool bLowerIsBetter = false)
	{
		float Diff = New - Current;

		if (FMath::IsNearlyZero(Diff, 0.01f))
		{
			return; // Skip unchanged stats
		}

		FText DiffText = FormatStatDifference(Diff, bIsMultiplier, 2);
		FText CurrentText = bIsMultiplier ? FormatStatMultiplier(Current, 2) : FormatStatValue(Current, 0);
		FText NewText = bIsMultiplier ? FormatStatMultiplier(New, 2) : FormatStatValue(New, 0);

		FString Symbol = (bLowerIsBetter ? (Diff < 0.0f) : (Diff > 0.0f)) ? TEXT("▲") : TEXT("▼");

		ComparisonLines.Add(FString::Printf(TEXT("%s %s: %s → %s (%s)"),
			*Symbol, *Name, *CurrentText.ToString(), *NewText.ToString(), *DiffText.ToString()));
	};

	// Add all stat comparisons
	AddComparisonLine(TEXT("Thrust"), CurrentStats.ThrustPower, NewStats.ThrustPower, true);
	AddComparisonLine(TEXT("Velocity"), CurrentStats.MaxVelocity, NewStats.MaxVelocity, true);
	AddComparisonLine(TEXT("Rotation"), CurrentStats.RotationSpeed, NewStats.RotationSpeed, true);
	AddComparisonLine(TEXT("Accel"), CurrentStats.Acceleration, NewStats.Acceleration, true);
	AddComparisonLine(TEXT("Hull"), CurrentStats.HullIntegrity, NewStats.HullIntegrity, false);
	AddComparisonLine(TEXT("Shield"), CurrentStats.ShieldStrength, NewStats.ShieldStrength, false);
	AddComparisonLine(TEXT("Weapon"), CurrentStats.WeaponDamage, NewStats.WeaponDamage, true);
	AddComparisonLine(TEXT("Energy"), CurrentStats.EnergyCapacity, NewStats.EnergyCapacity, false);
	AddComparisonLine(TEXT("Regen"), CurrentStats.EnergyRegenRate, NewStats.EnergyRegenRate, false);
	AddComparisonLine(TEXT("Mass"), CurrentStats.Mass, NewStats.Mass, false, true); // Lower is better

	if (ComparisonLines.Num() == 0)
	{
		return FText::FromString(TEXT("No stat changes"));
	}

	return FText::FromString(FString::Join(ComparisonLines, TEXT("\n")));
}

// ============================================================================
// CURRENCY & PROGRESSION FORMATTING
// ============================================================================

FText UCustomizationUIHelper::FormatCredits(int32 Credits)
{
	FText NumberText = FormatNumber(Credits);
	return FText::FromString(NumberText.ToString() + TEXT(" CR"));
}

FText UCustomizationUIHelper::FormatXP(int32 XP)
{
	FText NumberText = FormatNumber(XP);
	return FText::FromString(NumberText.ToString() + TEXT(" XP"));
}

FText UCustomizationUIHelper::FormatLevel(int32 Level)
{
	return FText::FromString(FString::Printf(TEXT("Level %d"), Level));
}

FText UCustomizationUIHelper::FormatXPProgress(int32 CurrentXP, int32 RequiredXP)
{
	float Percentage = GetXPProgressPercentage(CurrentXP, RequiredXP) * 100.0f;
	FText CurrentText = FormatNumber(CurrentXP);
	FText RequiredText = FormatNumber(RequiredXP);

	return FText::FromString(FString::Printf(TEXT("%s / %s XP (%.0f%%)"),
		*CurrentText.ToString(), *RequiredText.ToString(), Percentage));
}

float UCustomizationUIHelper::GetXPProgressPercentage(int32 CurrentXP, int32 RequiredXP)
{
	if (RequiredXP <= 0)
	{
		return 1.0f;
	}

	return ClampPercentage((float)CurrentXP / (float)RequiredXP);
}

// ============================================================================
// RARITY & COLOR HELPERS
// ============================================================================

FLinearColor UCustomizationUIHelper::GetRarityColor(EShipPartRarity Rarity)
{
	// Delegate to the existing ShipCustomizationBlueprintLibrary
	return UShipCustomizationBlueprintLibrary::GetRarityColor(Rarity);
}

FText UCustomizationUIHelper::GetRarityName(EShipPartRarity Rarity)
{
	switch (Rarity)
	{
	case EShipPartRarity::Common:
		return FText::FromString(TEXT("Common"));
	case EShipPartRarity::Uncommon:
		return FText::FromString(TEXT("Uncommon"));
	case EShipPartRarity::Rare:
		return FText::FromString(TEXT("Rare"));
	case EShipPartRarity::Epic:
		return FText::FromString(TEXT("Epic"));
	case EShipPartRarity::Legendary:
		return FText::FromString(TEXT("Legendary"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FText UCustomizationUIHelper::GetCategoryName(EShipPartCategory Category)
{
	switch (Category)
	{
	case EShipPartCategory::Engine:
		return FText::FromString(TEXT("Engine"));
	case EShipPartCategory::Thrusters:
		return FText::FromString(TEXT("Thrusters"));
	case EShipPartCategory::Hull:
		return FText::FromString(TEXT("Hull"));
	case EShipPartCategory::Wings:
		return FText::FromString(TEXT("Wings"));
	case EShipPartCategory::Cockpit:
		return FText::FromString(TEXT("Cockpit"));
	case EShipPartCategory::Weapon:
		return FText::FromString(TEXT("Weapon"));
	case EShipPartCategory::Shield:
		return FText::FromString(TEXT("Shield"));
	case EShipPartCategory::Utility:
		return FText::FromString(TEXT("Utility"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FString UCustomizationUIHelper::GetCategoryIconName(EShipPartCategory Category)
{
	return GetCategoryName(Category).ToString();
}

// ============================================================================
// PURCHASE VALIDATION
// ============================================================================

bool UCustomizationUIHelper::CanAffordPart(const FShipPartData& PartData, int32 CurrentCredits)
{
	return CurrentCredits >= PartData.UnlockCost;
}

bool UCustomizationUIHelper::CanAffordSkin(const FShipSkinData& SkinData, int32 CurrentCredits)
{
	return CurrentCredits >= SkinData.UnlockCost;
}

bool UCustomizationUIHelper::MeetsLevelRequirement(const FShipPartData& PartData, int32 PlayerLevel)
{
	return PlayerLevel >= PartData.LevelRequirement;
}

FText UCustomizationUIHelper::GetPurchaseStatus(const FShipPartData& PartData, int32 CurrentCredits, int32 PlayerLevel, bool bIsUnlocked)
{
	if (bIsUnlocked)
	{
		return FText::FromString(TEXT("Owned"));
	}

	if (!MeetsLevelRequirement(PartData, PlayerLevel))
	{
		return FText::FromString(FString::Printf(TEXT("Requires Level %d"), PartData.LevelRequirement));
	}

	if (!CanAffordPart(PartData, CurrentCredits))
	{
		int32 Needed = PartData.UnlockCost - CurrentCredits;
		FText NeededText = FormatCredits(Needed);
		return FText::FromString(FString::Printf(TEXT("Need %s more"), *NeededText.ToString()));
	}

	return FText::FromString(TEXT("Can Purchase"));
}

FLinearColor UCustomizationUIHelper::GetPurchaseStatusColor(const FShipPartData& PartData, int32 CurrentCredits, int32 PlayerLevel, bool bIsUnlocked)
{
	if (bIsUnlocked)
	{
		return GetOwnedColor();
	}

	if (!MeetsLevelRequirement(PartData, PlayerLevel))
	{
		return GetLockedColor();
	}

	if (!CanAffordPart(PartData, CurrentCredits))
	{
		return GetNegativeColor();
	}

	return GetPositiveColor();
}

// ============================================================================
// ICON & ASSET PATH HELPERS
// ============================================================================

FString UCustomizationUIHelper::GetIconPath(const TSoftObjectPtr<UTexture2D>& IconSoftPtr)
{
	if (IconSoftPtr.IsNull())
	{
		return FString();
	}

	return IconSoftPtr.ToString();
}

UTexture2D* UCustomizationUIHelper::LoadIconTexture(const TSoftObjectPtr<UTexture2D>& IconSoftPtr)
{
	if (IconSoftPtr.IsNull())
	{
		return nullptr;
	}

	return IconSoftPtr.LoadSynchronous();
}

bool UCustomizationUIHelper::IsIconValid(const TSoftObjectPtr<UTexture2D>& IconSoftPtr)
{
	return !IconSoftPtr.IsNull();
}

// ============================================================================
// COMPARISON HELPERS
// ============================================================================

FShipStats UCustomizationUIHelper::CalculatePartStatDifference(const FShipPartData& CurrentPart, const FShipPartData& NewPart)
{
	FShipStats Difference;

	Difference.ThrustPower = NewPart.StatModifiers.ThrustPower - CurrentPart.StatModifiers.ThrustPower;
	Difference.MaxVelocity = NewPart.StatModifiers.MaxVelocity - CurrentPart.StatModifiers.MaxVelocity;
	Difference.RotationSpeed = NewPart.StatModifiers.RotationSpeed - CurrentPart.StatModifiers.RotationSpeed;
	Difference.Acceleration = NewPart.StatModifiers.Acceleration - CurrentPart.StatModifiers.Acceleration;
	Difference.HullIntegrity = NewPart.StatModifiers.HullIntegrity - CurrentPart.StatModifiers.HullIntegrity;
	Difference.ShieldStrength = NewPart.StatModifiers.ShieldStrength - CurrentPart.StatModifiers.ShieldStrength;
	Difference.WeaponDamage = NewPart.StatModifiers.WeaponDamage - CurrentPart.StatModifiers.WeaponDamage;
	Difference.EnergyCapacity = NewPart.StatModifiers.EnergyCapacity - CurrentPart.StatModifiers.EnergyCapacity;
	Difference.EnergyRegenRate = NewPart.StatModifiers.EnergyRegenRate - CurrentPart.StatModifiers.EnergyRegenRate;
	Difference.Mass = NewPart.StatModifiers.Mass - CurrentPart.StatModifiers.Mass;

	return Difference;
}

void UCustomizationUIHelper::CountStatChanges(const FShipStats& StatDifference, int32& OutImproved, int32& OutWorsened)
{
	OutImproved = 0;
	OutWorsened = 0;

	// Helper lambda to count change
	auto CountChange = [&](float Diff, bool bLowerIsBetter = false)
	{
		if (FMath::IsNearlyZero(Diff, 0.01f))
		{
			return;
		}

		bool bIsPositive = bLowerIsBetter ? (Diff < 0.0f) : (Diff > 0.0f);

		if (bIsPositive)
		{
			OutImproved++;
		}
		else
		{
			OutWorsened++;
		}
	};

	// Count all stats
	CountChange(StatDifference.ThrustPower);
	CountChange(StatDifference.MaxVelocity);
	CountChange(StatDifference.RotationSpeed);
	CountChange(StatDifference.Acceleration);
	CountChange(StatDifference.HullIntegrity);
	CountChange(StatDifference.ShieldStrength);
	CountChange(StatDifference.WeaponDamage);
	CountChange(StatDifference.EnergyCapacity);
	CountChange(StatDifference.EnergyRegenRate);
	CountChange(StatDifference.Mass, true); // Lower is better
}

float UCustomizationUIHelper::GetUpgradeRating(const FShipStats& StatDifference)
{
	int32 Improved = 0;
	int32 Worsened = 0;

	CountStatChanges(StatDifference, Improved, Worsened);

	int32 TotalChanges = Improved + Worsened;

	if (TotalChanges == 0)
	{
		return 0.5f; // No change
	}

	return (float)Improved / (float)TotalChanges;
}

FText UCustomizationUIHelper::GetUpgradeSummary(int32 ImprovedStats, int32 WorsenedStats)
{
	if (ImprovedStats == 0 && WorsenedStats == 0)
	{
		return FText::FromString(TEXT("No Change"));
	}

	if (ImprovedStats > WorsenedStats)
	{
		return FText::FromString(TEXT("Upgrade"));
	}

	if (WorsenedStats > ImprovedStats)
	{
		return FText::FromString(TEXT("Downgrade"));
	}

	return FText::FromString(TEXT("Mixed"));
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

FText UCustomizationUIHelper::FormatNumber(int32 Number)
{
	return FText::AsNumber(Number);
}

FText UCustomizationUIHelper::FormatPercentage(float Percentage, int32 DecimalPlaces)
{
	float PercentValue = ClampPercentage(Percentage) * 100.0f;

	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = DecimalPlaces;
	FormatOptions.MaximumFractionalDigits = DecimalPlaces;
	FString ValueString = FText::AsNumber(PercentValue, &FormatOptions).ToString();
	return FText::FromString(ValueString + TEXT("%"));
}

float UCustomizationUIHelper::ClampPercentage(float Percentage)
{
	return FMath::Clamp(Percentage, 0.0f, 1.0f);
}

FText UCustomizationUIHelper::GetAbbreviatedStatName(const FString& StatName)
{
	// Abbreviation mapping
	static TMap<FString, FString> Abbreviations = {
		{TEXT("ThrustPower"), TEXT("Thrust")},
		{TEXT("MaxVelocity"), TEXT("Speed")},
		{TEXT("RotationSpeed"), TEXT("Rotation")},
		{TEXT("Acceleration"), TEXT("Accel")},
		{TEXT("HullIntegrity"), TEXT("Hull")},
		{TEXT("ShieldStrength"), TEXT("Shield")},
		{TEXT("WeaponDamage"), TEXT("Weapon")},
		{TEXT("EnergyCapacity"), TEXT("Energy")},
		{TEXT("EnergyRegenRate"), TEXT("Regen")},
		{TEXT("Mass"), TEXT("Mass")}
	};

	FString* Abbrev = Abbreviations.Find(StatName);
	return FText::FromString(Abbrev ? *Abbrev : StatName);
}

// ============================================================================
// COLOR CONSTANTS
// ============================================================================

FLinearColor UCustomizationUIHelper::GetPositiveColor()
{
	return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
}

FLinearColor UCustomizationUIHelper::GetNegativeColor()
{
	return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
}

FLinearColor UCustomizationUIHelper::GetNeutralColor()
{
	return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
}

FLinearColor UCustomizationUIHelper::GetOwnedColor()
{
	return FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold
}

FLinearColor UCustomizationUIHelper::GetLockedColor()
{
	return FLinearColor(0.3f, 0.3f, 0.3f, 1.0f); // Dark gray
}
