#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInterface.h"
#include "OutpostManager.h"
#include "PlanetaryLandingZone.h"
// #include "../Planetary/PlanetaryResourcesComponent.h"  // TODO: Fix include path - file exists but compiler can't find it

// Forward declarations
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UParticleSystem;
class UUserWidget;

// Forward declarations and includes for types from other headers
struct FResourceAmount;  // Forward declare
struct FGuid;  // Forward declare (it's a struct, not class)

// Need full definition of FResourceAmount for UPROPERTY use
#include "../Planetary/PlanetaryResourcesComponent.h"

#include "ColonyBuildingSystem.generated.h"  // MUST BE LAST include before UENUM/USTRUCT/UCLASS

// Enums
UENUM(BlueprintType)
enum class EBuildingMode : uint8
{
	None,
	Placement,
	Demolition,
	Upgrade
};

// Structs

USTRUCT(BlueprintType)
struct FBuildingEffect
{
	GENERATED_BODY()

	UPROPERTY() FString EffectName;
	UPROPERTY() float Value;

	UPROPERTY() FString EffectType;
	UPROPERTY() float Duration;
	UPROPERTY() FGuid SourceBuildingID;
};

USTRUCT(BlueprintType)
struct FBuildingGridCell
{
	GENERATED_BODY()

	UPROPERTY() int32 X;
	UPROPERTY() int32 Y;
	UPROPERTY() bool bOccupied;

	UPROPERTY() FIntPoint GridPosition;
	UPROPERTY() FVector WorldPosition;
	UPROPERTY() bool bIsOccupied;
	UPROPERTY() FGuid OccupyingBuildingID;  // ID of building occupying this cell (not pointer - UHT doesn't allow exposed USTRUCT*)
	UPROPERTY() ETerrainType TerrainType;
	UPROPERTY() bool bIsValidForBuilding;
};

USTRUCT(BlueprintType)
struct FBuildingData
{
	GENERATED_BODY()

	UPROPERTY() EBuildingType Type;
	UPROPERTY() FVector Location;

	UPROPERTY() EBuildingType BuildingType;
	UPROPERTY() int32 BuildingLevel;
	UPROPERTY() FRotator Rotation;
	UPROPERTY() float Health;
	UPROPERTY() float MaxHealth;
	UPROPERTY() float PowerConsumption;
	UPROPERTY() float PowerGeneration;
	UPROPERTY() int32 PopulationCapacity;
	UPROPERTY() int32 StorageCapacity;
	UPROPERTY() float DefenseRating;
	UPROPERTY() float ConstructionProgress;
	UPROPERTY() bool bIsCompleted;
	UPROPERTY() bool bIsOperational;
	UPROPERTY() float ConstructionStartTime;
	UPROPERTY() float LastUpgradeTime;
	UPROPERTY() TArray<FBuildingEffect> Effects;
	UPROPERTY() int32 AssignedWorkers;
	UPROPERTY() int32 MaxWorkers;
	UPROPERTY() float ProductionRate;
	UPROPERTY() float MaintenanceCost;
	UPROPERTY() FGuid BuildingID;
	UPROPERTY() bool bIsUnderConstruction;
};

USTRUCT(BlueprintType)
struct FBuildingStatistics
{
	GENERATED_BODY()

	UPROPERTY() int32 TotalBuildings;
	UPROPERTY() int32 ActiveBuildings;

	UPROPERTY() int32 OperationalBuildings;
	UPROPERTY() int32 UnderConstruction;
	UPROPERTY() float TotalPowerConsumption;
	UPROPERTY() float TotalPowerGeneration;
	UPROPERTY() int32 TotalPopulationCapacity;
	UPROPERTY() int32 TotalStorageCapacity;
	UPROPERTY() float TotalDefenseRating;
	UPROPERTY() float AverageBuildingLevel;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingConstructionStartedSignature, const FBuildingData&, BuildingData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingConstructionCompletedSignature, const FBuildingData&, BuildingData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingUpgradedSignature, const FBuildingData&, BuildingData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingDestroyedSignature, const FBuildingData&, BuildingData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingDamagedSignature, const FBuildingData&, BuildingData, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingRepairedSignature, const FBuildingData&, BuildingData);

UENUM(BlueprintType)
enum class EBuildingCategory : uint8
{
    Residential,
    Commercial,
    Industrial,
    Agricultural,
    Scientific,
    Military,
    Medical,
    Educational,
    Entertainment,
    Infrastructure,
    Power,
    Water,
    Communication,
    Storage,
    Defense
};

UENUM(BlueprintType)
enum class EBuildingSize : uint8
{
    Small,
    Medium,
    Large,
    Massive
};

UENUM(BlueprintType)
enum class EBuildingStatus : uint8
{
    Planning,
    UnderConstruction,
    Operational,
    Damaged,
    Abandoned,
    Demolished,
    Upgrading
};

UENUM(BlueprintType)
enum class EConstructionMethod : uint8
{
    Manual,
    Automated,
    Replicator,
    Assembler,
    Print3D,
    Nanite
};

USTRUCT(BlueprintType)
struct FBuildingRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> RequiredTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> RequiredSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RequiredPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingSize MinimumSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PrerequisiteBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> RequiredTechnologies;
};

USTRUCT(BlueprintType)
struct FBuildingProduction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ProductionRates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ConsumptionRates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProductionEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OperatingCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> Byproducts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWorkers;
};

USTRUCT(BlueprintType)
struct FBuildingUpgrade
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UpgradeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingRequirement Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConstructionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> StatModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UnlockFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* NewMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* NewMaterial;
};

/**
 * Wrapper for player project names
 */
USTRUCT()
struct FPlayerProjectList
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FName> ProjectNames;
};

USTRUCT(BlueprintType)
struct FColonyBuilding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BuildingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingSize Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* BuildingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* BuildingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingRequirement Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingProduction Production;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBuildingUpgrade> AvailableUpgrades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Capacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentOccupancy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaintenanceCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> BuildingStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ConnectedBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPowered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConnected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConstructionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConstructionTime;
};

USTRUCT(BlueprintType)
struct FConstructionProject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ProjectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APlayerState* ProjectManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConstructionMethod ConstructionMethod;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ConstructionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedCompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPaused;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<APlayerState*> AssignedWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> AllocatedResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> QualityFactors;
};

USTRUCT(BlueprintType)
struct FBuildingTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TemplateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TemplateName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingSize DefaultSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingRequirement BaseRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingProduction BaseProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseConstructionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* DefaultMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBuildingUpgrade> UpgradePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EBuildingSize, FBuildingRequirement> SizeRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EBuildingSize, float> SizeConstructionTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredTechnologies;

    // Additional properties used in ColonyBuildingSystem.cpp
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType BuildingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BuildingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount BaseCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BasePopulationCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EBuildingType> PrerequisiteBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* BuildingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* ConstructionMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePowerGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseStorageCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDefenseRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseMaxWorkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseProductionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMaintenanceCost;
};

UCLASS()
class ALEXANDER_API AColonyBuildingSystem : public AActor
{
    GENERATED_BODY()

public:
    AColonyBuildingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* BuildingPlacementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BuildingMeshComponent;

    // Building System State
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building System")
    EBuildingMode CurrentBuildingMode;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building System")
    bool bIsBuildingValid;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building System")
    bool bIsInBuildMode;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building System")
    EBuildingType CurrentBuildingType;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building System")
    int32 CurrentBuildingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building System")
    float PlacementGridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building System")
    float MaxBuildDistance;

    // Construction State
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Construction")
    float ConstructionProgress;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Construction")
    bool bIsUnderConstruction;

    UPROPERTY()
    float ConstructionTime;

    UPROPERTY()
    TArray<FResourceAmount> RequiredResources;

    UPROPERTY()
    FTimerHandle ConstructionTimer;

    // Building Stats
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    float BuildingHealth;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    float MaxBuildingHealth;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    float PowerConsumption;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    float PowerGeneration;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    int32 PopulationCapacity;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    int32 StorageCapacity;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building Stats")
    float DefenseRating;

    // Upgrade System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
    int32 MaxBuildingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
    float UpgradeCostMultiplier;

    // Building Lists
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<FBuildingTemplate> AvailableBuildings;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Buildings")
    TArray<FBuildingData> ConstructedBuildings;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Buildings")
    TArray<FBuildingData> BuildingQueue;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Buildings")
    TArray<FBuildingGridCell> BuildingGrid;

    // Building Effects
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Effects")
    TArray<FBuildingEffect> BuildingEffects;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Effects")
    TArray<FBuildingEffect> ActiveEffects;

    // Building Statistics
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Statistics")
    int32 TotalBuildingsConstructed;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Statistics")
    float TotalResourcesSpent;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Statistics")
    float TotalConstructionTime;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ValidPlacementMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* InvalidPlacementMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ConstructionMaterial;

    // Sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ConstructionStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ConstructionCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* BuildingDestroyedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* UpgradeSound;

    // Particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* ConstructionParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* DestructionParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* UpgradeParticles;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UUserWidget* BuildingUIWidget;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    bool bShowBuildingUI;

    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bAutoRepair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bAutoUpgrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float RepairThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<EBuildingType> UpgradePriority;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingConstructionStartedSignature OnBuildingConstructionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingConstructionCompletedSignature OnBuildingConstructionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingUpgradedSignature OnBuildingUpgraded;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingDestroyedSignature OnBuildingDestroyed;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingDamagedSignature OnBuildingDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingRepairedSignature OnBuildingRepaired;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Building System")
    void InitializeBuildingSystem();

    UFUNCTION(BlueprintCallable, Category = "Building System")
    void EnterBuildMode(EBuildingType BuildingType);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    void ExitBuildMode();

    UFUNCTION(BlueprintCallable, Category = "Building System")
    bool PlaceBuilding(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    bool UpgradeBuilding(const FGuid& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    bool DemolishBuilding(const FGuid& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    bool RepairBuilding(const FGuid& BuildingID, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    void DamageBuilding(const FGuid& BuildingID, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Building System")
    FBuildingStatistics GetBuildingStatistics() const;

    UFUNCTION(BlueprintCallable, Category = "Building System")
    TArray<FBuildingData> GetBuildingsByType(EBuildingType BuildingType) const;

    UFUNCTION(BlueprintCallable, Category = "Building System")
    TArray<FBuildingTemplate> GetAvailableBuildingTemplates() const;

    UFUNCTION(BlueprintCallable, Category = "Building System")
    bool CanPlaceBuildingAtLocation(EBuildingType BuildingType, const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Building System")
    FResourceAmount GetBuildingPlacementCost(EBuildingType BuildingType) const;

protected:
    // Internal Functions
    void SetupBuildingPlacement();
    void InitializeBuildingGrid();
    void LoadAvailableBuildings();
    void UpdateBuildingPlacement();
    void StartBuildingConstruction(const FBuildingData& BuildingData);
    void CompleteBuildingConstruction();
    void UpdateConstructionProgress(float DeltaTime);
    void StartBuildingUpgrade(FBuildingData& BuildingData);
    void CompleteBuildingUpgrade(FBuildingData& BuildingData);
    void UpdateBuildingStats(FBuildingData& BuildingData);
    void UpdateBuildingGrid(const FBuildingData& BuildingData, bool bOccupying);
    void ApplyBuildingEffects(const FBuildingData& BuildingData);
    void RemoveBuildingEffects(const FBuildingData& BuildingData);
    void UpdateBuildingEffects(float DeltaTime);
    void UpdateNavigationMesh();
    void HandleAutoRepair(float DeltaTime);
    void HandleAutoUpgrade(float DeltaTime);
    void UpdateBuildingStatistics(float DeltaTime);
    void CalculateColonyStats();
    void ShowBuildingPlacementPreview();
    void HideBuildingPlacementPreview();
    void UpdatePlacementMaterial();
    void PlayConstructionEffects();
    void PlayConstructionCompleteEffects();
    void PlayUpgradeEffects();
    void PlayUpgradeCompleteEffects();
    void PlayDemolitionEffects();
    void PlayRepairEffects();
    void PlayDamageEffects();
    void UpdateConstructionEffects();
    void InitializeBuildingTemplates();
    void InitializeBuildingCosts();
    void InitializeBuildingRequirements();
    void InitializeBuildingEffects();
    void SetupBuildingEvents();

    // Helper Functions
    const FBuildingTemplate* GetBuildingTemplate(EBuildingType BuildingType) const;
    FBuildingData* FindBuildingByID(const FGuid& BuildingID);
    FVector SnapToGrid(const FVector& Location) const;
    bool IsPlacementValid(const FVector& Location) const;
    ETerrainType GetTerrainType(const FVector& Location) const;
    float GetTerrainSlope(const FVector& Location) const;
    bool HasEnoughResources(const FResourceAmount& Cost) const;
    void DeductResources(const FResourceAmount& Cost);
    float CalculateTotalResourceValue(const FResourceAmount& Resources) const;
    bool MeetsPrerequisites(const FBuildingTemplate& BuildingTemplate) const;
    bool HasTechnology(const FName& Technology) const;
    bool HasBuildingType(EBuildingType BuildingType) const;
    bool CanUpgradeBuilding(const FBuildingData& BuildingData) const;
    bool CanDemolishBuilding(const FBuildingData& BuildingData) const;
    FResourceAmount CalculateUpgradeCost(const FBuildingData& BuildingData) const;
    FResourceAmount CalculateRepairCost(const FBuildingData& BuildingData, float RepairAmount) const;
    FIntPoint WorldToGrid(const FVector& WorldLocation) const;

    // Network Functions
    UFUNCTION(NetMulticast, Reliable)
    void OnBuildModeEntered(EBuildingType BuildingType);

    UFUNCTION(NetMulticast, Reliable)
    void OnBuildModeExited();

    UFUNCTION(NetMulticast, Reliable)
    void OnBuildingPlaced(const FBuildingData& BuildingData);
};