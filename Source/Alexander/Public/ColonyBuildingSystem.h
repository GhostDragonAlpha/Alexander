#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "ColonyBuildingSystem.generated.h"

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
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UColonyBuildingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UColonyBuildingSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Building Management
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void InitializeBuildingSystem();

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    FName StartConstruction(const FName& TemplateID, const FVector& Location, APlayerState* Manager = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool CancelConstruction(const FName& ProjectID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void CompleteConstruction(const FName& ProjectID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool DemolishBuilding(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    FColonyBuilding GetBuilding(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    TArray<FColonyBuilding> GetAllBuildings();

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    TArray<FColonyBuilding> GetBuildingsByCategory(EBuildingCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    TArray<FColonyBuilding> GetBuildingsInArea(const FVector& Center, float Radius);

    // Construction Management
    UFUNCTION(BlueprintCallable, Category = "Construction")
    FConstructionProject GetConstructionProject(const FName& ProjectID);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    TArray<FConstructionProject> GetActiveProjects();

    UFUNCTION(BlueprintCallable, Category = "Construction")
    TArray<FConstructionProject> GetPlayerProjects(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    bool AssignWorker(const FName& ProjectID, APlayerState* Worker);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    bool RemoveWorker(const FName& ProjectID, APlayerState* Worker);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    bool AllocateResources(const FName& ProjectID, const TMap<FName, int32>& Resources);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void PauseConstruction(const FName& ProjectID);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void ResumeConstruction(const FName& ProjectID);

    // Building Templates
    UFUNCTION(BlueprintCallable, Category = "Templates")
    void RegisterBuildingTemplate(const FBuildingTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    FBuildingTemplate GetBuildingTemplate(const FName& TemplateID);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    TArray<FBuildingTemplate> GetAvailableTemplates();

    UFUNCTION(BlueprintCallable, Category = "Templates")
    TArray<FBuildingTemplate> GetTemplatesByCategory(EBuildingCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    bool IsTemplateUnlocked(const FName& TemplateID);

    UFUNCTION(BlueprintCallable, Category = "Templates")
    void UnlockTemplate(const FName& TemplateID);

    // Building Upgrades
    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    bool CanUpgradeBuilding(const FName& BuildingID, const FName& UpgradeID);

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    FName StartUpgrade(const FName& BuildingID, const FName& UpgradeID);

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    bool CompleteUpgrade(const FName& ProjectID);

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    TArray<FBuildingUpgrade> GetAvailableUpgrades(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Upgrades")
    FBuildingUpgrade GetUpgradeInfo(const FName& BuildingID, const FName& UpgradeID);

    // Building Operations
    UFUNCTION(BlueprintCallable, Category = "Operations")
    bool ActivateBuilding(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Operations")
    bool DeactivateBuilding(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Operations")
    bool IsBuildingOperational(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Operations")
    void UpdateBuildingProduction(const FName& BuildingID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Operations")
    TMap<FName, float> GetBuildingProduction(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Operations")
    TMap<FName, float> GetBuildingConsumption(const FName& BuildingID);

    // Building Maintenance
    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    void UpdateBuildingMaintenance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    bool RepairBuilding(const FName& BuildingID, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    float GetBuildingHealth(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    float GetBuildingCondition(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    bool NeedsMaintenance(const FName& BuildingID);

    // Power Management
    UFUNCTION(BlueprintCallable, Category = "Power")
    void UpdatePowerGrid();

    UFUNCTION(BlueprintCallable, Category = "Power")
    float GetTotalPowerGeneration();

    UFUNCTION(BlueprintCallable, Category = "Power")
    float GetTotalPowerConsumption();

    UFUNCTION(BlueprintCallable, Category = "Power")
    bool IsBuildingPowered(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Power")
    void ConnectBuildingToPower(const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Power")
    void DisconnectBuildingFromPower(const FName& BuildingID);

    // Resource Management
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void UpdateResourceProduction(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TMap<FName, float> GetTotalProduction();

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TMap<FName, float> GetTotalConsumption();

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool CanSupportBuilding(const FName& TemplateID, const FVector& Location);

    // Building Placement
    UFUNCTION(BlueprintCallable, Category = "Placement")
    bool CanPlaceBuilding(const FName& TemplateID, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Placement")
    bool IsValidLocation(const FVector& Location, EBuildingSize Size);

    UFUNCTION(BlueprintCallable, Category = "Placement")
    TArray<FVector> GetValidPlacementLocations(const FVector& Center, float Radius, EBuildingSize Size);

    UFUNCTION(BlueprintCallable, Category = "Placement")
    FVector FindOptimalLocation(const FName& TemplateID, const FVector& PreferredLocation);

    // Building Statistics
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    TMap<EBuildingCategory, int32> GetBuildingCountByCategory();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetAverageBuildingCondition();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetOperationalBuildingCount();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetUnderConstructionBuildingCount();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetTotalBuildingValue();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    void GenerateBuildingReport();

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float CalculateConstructionTime(const FName& TemplateID, EConstructionMethod Method, int32 Workers);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TMap<FName, int32> CalculateConstructionCost(const FName& TemplateID, EBuildingSize Size);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool HasRequiredResources(const FBuildingRequirement& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool HasRequiredWorkers(const FBuildingRequirement& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetBuildingEfficiency(const FName& BuildingID);

protected:
    // Building Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buildings")
    TMap<FName, FColonyBuilding> Buildings;

    // Construction Projects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
    TMap<FName, FConstructionProject> ConstructionProjects;

    // Building Templates
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Templates")
    TMap<FName, FBuildingTemplate> BuildingTemplates;

    // Player Data
    TMap<APlayerState*, FPlayerProjectList> PlayerProjects;

    // Power Grid
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
    TMap<FName, bool> BuildingPowerConnections;

    // Resource Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TMap<FName, float> TotalProductionRates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TMap<FName, float> TotalConsumptionRates;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ConstructionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaintenanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ProductionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutoMaintenance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePowerGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableResourceTracking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BaseConstructionSpeed;

private:
    // Internal Functions
    void UpdateConstructionProjects(float DeltaTime);
    void UpdateBuildingOperations(float DeltaTime);
    void ProcessBuildingMaintenance(float DeltaTime);
    void UpdatePowerConnections();

    // Construction Internal
    FName GenerateProjectID();
    FName GenerateBuildingID();
    void ProcessConstruction(const FName& ProjectID, float DeltaTime);
    void StartBuildingConstruction(const FName& ProjectID);
    float CalculateConstructionProgress(const FConstructionProject& Project, float DeltaTime);
    bool ValidateConstructionRequirements(const FConstructionProject& Project);

    // Building Internal
    void InitializeBuilding(const FName& BuildingID, const FConstructionProject& Project);
    void UpdateBuildingStatus(const FName& BuildingID, EBuildingStatus NewStatus);
    void ProcessBuildingDamage(const FName& BuildingID, float DamageAmount);
    void ProcessBuildingRepair(const FName& BuildingID, float RepairAmount);

    // Production Internal
    void CalculateTotalProduction();
    void CalculateTotalConsumption();
    void ProcessBuildingProduction(const FName& BuildingID, float DeltaTime);
    void UpdateBuildingEfficiency(const FName& BuildingID);

    // Power Internal
    void CalculatePowerBalance();
    void DistributePower();
    void UpdateBuildingPowerStatus(const FName& BuildingID);

    // Maintenance Internal
    void ProcessMaintenanceRequirements(float DeltaTime);
    void ApplyMaintenanceDamage(float DeltaTime);
    void ScheduleMaintenance(const FName& BuildingID);

    // Placement Internal
    bool CheckLocationConstraints(const FVector& Location, EBuildingSize Size);
    bool CheckResourceAvailability(const FBuildingRequirement& Requirements);
    bool CheckWorkerAvailability(const FBuildingRequirement& Requirements);
    bool CheckBuildingConnections(const FVector& Location);

    // Template Internal
    void InitializeDefaultTemplates();
    void LoadBuildingTemplates();
    void SaveBuildingTemplates();

    // Validation
    bool ValidateBuilding(const FColonyBuilding& Building);
    bool ValidateTemplate(const FBuildingTemplate& Template);
    bool ValidateProject(const FConstructionProject& Project);

    // Utilities Internal
    float GetConstructionSpeedModifier(EConstructionMethod Method);
    float GetWorkerEfficiency(const TArray<APlayerState*>& Workers);
    float GetQualityModifier(const TMap<FName, float>& QualityFactors);
    void UpdateBuildingVisuals(const FName& BuildingID);

    // Timers
    UPROPERTY()
    FTimerHandle ConstructionUpdateTimer;

    UPROPERTY()
    FTimerHandle MaintenanceUpdateTimer;

    UPROPERTY()
    FTimerHandle ProductionUpdateTimer;

    UPROPERTY()
    FTimerHandle PowerUpdateTimer;

    // Constants
    static constexpr float DEFAULT_CONSTRUCTION_UPDATE_INTERVAL = 0.1f;
    static constexpr float DEFAULT_MAINTENANCE_UPDATE_INTERVAL = 5.0f;
    static constexpr float DEFAULT_PRODUCTION_UPDATE_INTERVAL = 1.0f;
    static constexpr float DEFAULT_BASE_CONSTRUCTION_SPEED = 10.0f;
    static constexpr float MAINTENANCE_DAMAGE_RATE = 0.001f; // 0.1% per update
    static constexpr float POWER_UPDATE_INTERVAL = 0.5f;
    static constexpr int32 MAX_CONSTRUCTION_PROJECTS = 50;
    static constexpr float BUILDING_PLACEMENT_SPACING = 100.0f;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConstructionStarted, const FName&, ProjectID, const FName&, TemplateID, APlayerState*, Manager);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConstructionCompleted, const FName&, ProjectID, const FName&, BuildingID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConstructionCancelled, const FName&, ProjectID, const FString&, Reason);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingOperational, const FName&, BuildingID, bool, bIsOperational);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingDamaged, const FName&, BuildingID, float, DamageAmount);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingRepaired, const FName&, BuildingID, float, RepairAmount);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingUpgraded, const FName&, BuildingID, const FName&, UpgradeID);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnConstructionStarted OnConstructionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnConstructionCompleted OnConstructionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnConstructionCancelled OnConstructionCancelled;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingOperational OnBuildingOperational;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingDamaged OnBuildingDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingRepaired OnBuildingRepaired;

    UPROPERTY(BlueprintAssignable, Category = "Building Events")
    FOnBuildingUpgraded OnBuildingUpgraded;
};