// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "ShipCustomizationComponent.generated.h"

/**
 * Ship Part Categories
 */
UENUM(BlueprintType)
enum class EShipPartCategory : uint8
{
	Engine UMETA(DisplayName = "Engine"),
	Thrusters UMETA(DisplayName = "Thrusters"),
	Hull UMETA(DisplayName = "Hull"),
	Wings UMETA(DisplayName = "Wings"),
	Cockpit UMETA(DisplayName = "Cockpit"),
	Weapon UMETA(DisplayName = "Weapon"),
	Shield UMETA(DisplayName = "Shield"),
	Utility UMETA(DisplayName = "Utility")
};

/**
 * Ship Part Rarity
 */
UENUM(BlueprintType)
enum class EShipPartRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Ship Stats - affects flight performance
 */
USTRUCT(BlueprintType)
struct FShipStats
{
	GENERATED_BODY()

	// Thrust power multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ThrustPower = 1.0f;

	// Max velocity multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxVelocity = 1.0f;

	// Rotation speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RotationSpeed = 1.0f;

	// Acceleration multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Acceleration = 1.0f;

	// Hull integrity (health)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HullIntegrity = 100.0f;

	// Shield strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ShieldStrength = 0.0f;

	// Weapon damage multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WeaponDamage = 1.0f;

	// Energy capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float EnergyCapacity = 100.0f;

	// Energy regen rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float EnergyRegenRate = 10.0f;

	// Mass (affects handling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Mass = 1000.0f;

	// Combine stats (adds values)
	FShipStats operator+(const FShipStats& Other) const
	{
		FShipStats Result;
		Result.ThrustPower = ThrustPower + Other.ThrustPower;
		Result.MaxVelocity = MaxVelocity + Other.MaxVelocity;
		Result.RotationSpeed = RotationSpeed + Other.RotationSpeed;
		Result.Acceleration = Acceleration + Other.Acceleration;
		Result.HullIntegrity = HullIntegrity + Other.HullIntegrity;
		Result.ShieldStrength = ShieldStrength + Other.ShieldStrength;
		Result.WeaponDamage = WeaponDamage + Other.WeaponDamage;
		Result.EnergyCapacity = EnergyCapacity + Other.EnergyCapacity;
		Result.EnergyRegenRate = EnergyRegenRate + Other.EnergyRegenRate;
		Result.Mass = Mass + Other.Mass;
		return Result;
	}
};

/**
 * Ship Part Definition - for Data Tables
 */
USTRUCT(BlueprintType)
struct FShipPartData : public FTableRowBase
{
	GENERATED_BODY()

	// Part ID (unique identifier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText Description;

	// Category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EShipPartCategory Category;

	// Rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EShipPartRarity Rarity;

	// Level requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int32 LevelRequirement = 1;

	// Unlock cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int32 UnlockCost = 0;

	// Is unlocked by default?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	bool bUnlockedByDefault = false;

	// Stat modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FShipStats StatModifiers;

	// Visual mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> PartMesh;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	// Parent part (for upgrade tree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	FName ParentPartID;

	// Can this part be upgraded?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	bool bCanBeUpgraded = true;

	// Upgrade level (0 = base, 1-5 = upgrades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 UpgradeLevel = 0;
};

/**
 * Ship Skin Definition
 */
USTRUCT(BlueprintType)
struct FShipSkinData : public FTableRowBase
{
	GENERATED_BODY()

	// Skin ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
	FName SkinID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
	FText Description;

	// Rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
	EShipPartRarity Rarity;

	// Material instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UMaterialInstance> SkinMaterial;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	// Unlock cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 UnlockCost = 0;

	// Is unlocked by default?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	bool bUnlockedByDefault = true;
};

/**
 * Ship Loadout - complete ship configuration
 */
USTRUCT(BlueprintType)
struct FShipLoadout
{
	GENERATED_BODY()

	// Loadout name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FText LoadoutName;

	// Equipped parts (category -> part ID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	TMap<EShipPartCategory, FName> EquippedParts;

	// Equipped skin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FName EquippedSkin;

	// Total stats (cached)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loadout")
	FShipStats TotalStats;
};

/**
 * Player Progression Data
 */
USTRUCT(BlueprintType)
struct FPlayerProgressionData
{
	GENERATED_BODY()

	// Player level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 PlayerLevel = 1;

	// Total XP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 TotalXP = 0;

	// Currency (credits)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 Credits = 0;

	// Unlocked parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TArray<FName> UnlockedParts;

	// Unlocked skins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TArray<FName> UnlockedSkins;

	// Saved loadouts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TArray<FShipLoadout> SavedLoadouts;

	// Active loadout index
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 ActiveLoadoutIndex = 0;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

// Called when loadout changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadoutChanged);

// Called when part equipped
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartEquipped, EShipPartCategory, Category, FName, PartID);

// Called when skin equipped
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkinEquipped, FName, SkinID);

// Called when part unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartUnlocked, FName, PartID);

// Called when player levels up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp, int32, OldLevel, int32, NewLevel);

/**
 * Ship Customization Component
 * Manages ship parts, loadouts, stats, and visual customization
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UShipCustomizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipCustomizationComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// ============================================================================
	// DATA TABLES
	// ============================================================================

	// Ship parts data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ShipPartsDataTable;

	// Ship skins data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ShipSkinsDataTable;

	// ============================================================================
	// PHYSICS CONFIGURATION
	// ============================================================================

	// Preset physics configuration to apply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	class UShipPhysicsConfig* PhysicsConfig;

	// Apply physics config to ship
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	void ApplyPhysicsConfig(UShipPhysicsConfig* Config);

	// ============================================================================
	// CURRENT STATE
	// ============================================================================

	// Current loadout (not replicated - physics stats replicate via OrbitalBody)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	FShipLoadout CurrentLoadout;

	// Progression data (client-side only, saved in SaveGame)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	FPlayerProgressionData ProgressionData;

	// ============================================================================
	// LOADOUT MANAGEMENT
	// ============================================================================

	// Equip a part
	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool EquipPart(EShipPartCategory Category, FName PartID);

	// Unequip a part
	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool UnequipPart(EShipPartCategory Category);

	// Equip a skin
	UFUNCTION(BlueprintCallable, Category = "Customization")
	bool EquipSkin(FName SkinID);

	// Get equipped part for category
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FShipPartData GetEquippedPart(EShipPartCategory Category, bool& bFound);

	// Get equipped skin
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FShipSkinData GetEquippedSkin(bool& bFound);

	// ============================================================================
	// STATS CALCULATION
	// ============================================================================

	// Calculate total ship stats from equipped parts
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FShipStats CalculateTotalStats();

	// Apply stats to FlightController
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyStatsToFlightController();

	// Get total stats
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FShipStats GetTotalStats() const { return CurrentLoadout.TotalStats; }

	// ============================================================================
	// PROGRESSION
	// ============================================================================

	// Unlock a part
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool UnlockPart(FName PartID);

	// Unlock a skin
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool UnlockSkin(FName SkinID);

	// Is part unlocked?
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool IsPartUnlocked(FName PartID) const;

	// Is skin unlocked?
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool IsSkinUnlocked(FName SkinID) const;

	// Add XP
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddXP(int32 Amount);

	// Add credits
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCredits(int32 Amount);

	// Get player level
	UFUNCTION(BlueprintCallable, Category = "Progression")
	int32 GetPlayerLevel() const { return ProgressionData.PlayerLevel; }

	// Get credits
	UFUNCTION(BlueprintCallable, Category = "Progression")
	int32 GetCredits() const { return ProgressionData.Credits; }

	// ============================================================================
	// LOADOUT PRESETS
	// ============================================================================

	// Save current loadout
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void SaveLoadout(const FText& LoadoutName);

	// Load a loadout
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	bool LoadLoadout(int32 LoadoutIndex);

	// Delete a loadout
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	bool DeleteLoadout(int32 LoadoutIndex);

	// Get all saved loadouts
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	TArray<FShipLoadout> GetSavedLoadouts() const { return ProgressionData.SavedLoadouts; }

	// ============================================================================
	// VISUAL UPDATES
	// ============================================================================

	// Update ship visual appearance
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void UpdateShipVisuals();

	// Apply skin material to ship mesh
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ApplySkinMaterial();

	// ============================================================================
	// DATA QUERIES
	// ============================================================================

	// Get all parts in category
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FShipPartData> GetPartsInCategory(EShipPartCategory Category);

	// Get all unlocked parts in category
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FShipPartData> GetUnlockedPartsInCategory(EShipPartCategory Category);

	// Get all available skins
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FShipSkinData> GetAllSkins();

	// Get all unlocked skins
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FShipSkinData> GetUnlockedSkins();

	// Get part data by ID
	UFUNCTION(BlueprintCallable, Category = "Data")
	FShipPartData GetPartData(FName PartID, bool& bFound);

	// Get skin data by ID
	UFUNCTION(BlueprintCallable, Category = "Data")
	FShipSkinData GetSkinData(FName SkinID, bool& bFound);

	// ============================================================================
	// SAVE/LOAD
	// ============================================================================

	// Save customization data to SaveGame
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	bool SaveCustomizationData();

	// Load customization data from SaveGame
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	bool LoadCustomizationData();

	// ============================================================================
	// EVENTS
	// ============================================================================

	// Called when loadout changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadoutChanged OnLoadoutChanged;

	// Called when part equipped
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPartEquipped OnPartEquipped;

	// Called when skin equipped
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkinEquipped OnSkinEquipped;

	// Called when part unlocked
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPartUnlocked OnPartUnlocked;

	// Called when player levels up
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLevelUp OnPlayerLevelUp;

protected:
	// Initialize default loadout
	void InitializeDefaultLoadout();

	// Calculate XP required for level
	int32 GetXPForLevel(int32 Level) const;

	// Check for level up
	void CheckLevelUp();

	// Get FlightController reference
	class UFlightController* GetFlightController() const;

	// Get ship mesh reference
	UStaticMeshComponent* GetShipMesh() const;

	// Get OrbitalBody reference (for n-body physics integration)
	class AOrbitalBody* GetOrbitalBody() const;

	// Cached FlightController reference
	UPROPERTY()
	mutable class UFlightController* CachedFlightController;

	// Cached ship mesh reference
	UPROPERTY()
	mutable UStaticMeshComponent* CachedShipMesh;

	// Cached OrbitalBody reference (for gravity/mass integration)
	UPROPERTY()
	mutable class AOrbitalBody* CachedOrbitalBody;
};
