#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "PlanetaryLandingZone.h"
#include "OutpostManager.generated.h"

UENUM(BlueprintType)
enum class EOutpostType : uint8
{
    None,
    Mining,
    Research,
    Military,
    Agricultural,
    Trading,
    Medical,
    Communication,
    Refinery,
    Shipyard,
    Habitat,
    Industrial
};

UENUM(BlueprintType)
enum class EOutpostSize : uint8
{
    Outpost,
    Settlement,
    Colony,
    City,
    Metropolis
};

UENUM(BlueprintType)
enum class EOutpostStatus : uint8
{
    UnderConstruction,
    Active,
    Damaged,
    Abandoned,
    Quarantined,
    UnderAttack,
    Offline
};

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    None,
    Habitat,
    PowerPlant,
    Refinery,
    Factory,
    Laboratory,
    MedicalCenter,
    TradingPost,
    CommunicationArray,
    DefenseTurret,
    Storage,
    Farm,
    Mine,
    LaunchPad,
    Spaceport,
    CommandCenter,
    MiningFacility,
    ResearchLab
};

USTRUCT(BlueprintType)
struct FOutpostBuilding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BuildingID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType BuildingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOperational;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPowered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProductionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> Production;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> UpgradeCosts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* BuildingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* BuildingMaterial;
};

USTRUCT(BlueprintType)
struct FOutpostResources
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> StoredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ProductionRates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ConsumptionRates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> MaxStorage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxPowerStorage;
};

USTRUCT(BlueprintType)
struct FOutpostPopulation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> PopulationByProfession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, int32> PopulationByFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Happiness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Education;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Productivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GrowthRate;
};

USTRUCT(BlueprintType)
struct FOutpostDefense
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> DefenseBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDefenseValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShieldStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxShieldStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShieldsOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> EnemyThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> WeaponSystems;
};

USTRUCT(BlueprintType)
struct FOutpost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OutpostID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOutpostType OutpostType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOutpostSize Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOutpostStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class APlanet* ParentPlanet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FOutpostBuilding> Buildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FOutpostResources Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FOutpostPopulation Population;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FOutpostDefense Defense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> FactionInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Wealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Reputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> OutpostProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ConnectedOutposts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasSpaceport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasTradingPost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasMissionBoard;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UOutpostManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UOutpostManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Outpost Management
    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    void InitializeOutpostManager();

    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    void CreateOutpost(const FOutpost& OutpostData);

    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    void DestroyOutpost(const FName& OutpostID);

    FOutpost* GetOutpost(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    TArray<FOutpost> GetAllOutposts();

    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    TArray<FOutpost> GetOutpostsByType(EOutpostType Type);

    UFUNCTION(BlueprintCallable, Category = "Outpost Management")
    TArray<FOutpost> GetOutpostsByPlanet(class APlanet* Planet);

    // Building Management
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool CanBuildStructure(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool BuildStructure(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool UpgradeBuilding(const FName& OutpostID, const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool DestroyBuilding(const FName& OutpostID, const FName& BuildingID);

    FOutpostBuilding* GetBuilding(const FName& OutpostID, const FName& BuildingID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    TArray<FOutpostBuilding> GetOutpostBuildings(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool IsBuildingOperational(const FName& OutpostID, const FName& BuildingID);

    // Resource Management
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void UpdateOutpostResources(const FName& OutpostID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    float GetResourceAmount(const FName& OutpostID, const FName& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool HasResource(const FName& OutpostID, const FName& ResourceType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool ConsumeResource(const FName& OutpostID, const FName& ResourceType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddResource(const FName& OutpostID, const FName& ResourceType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    TMap<FName, float> GetAllResources(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    float GetPowerLevel(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool HasSufficientPower(const FName& OutpostID);

    // Population Management
    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void UpdateOutpostPopulation(const FName& OutpostID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    bool CanAcceptPopulation(const FName& OutpostID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void AddPopulation(const FName& OutpostID, int32 Amount, const FName& Profession = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void RemovePopulation(const FName& OutpostID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    float GetPopulationHappiness(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void SetPopulationHappiness(const FName& OutpostID, float Happiness);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    TMap<FName, int32> GetPopulationByProfession(const FName& OutpostID);

    // Defense Management
    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    void UpdateOutpostDefense(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    float GetDefenseValue(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    bool IsUnderAttack(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    void HandleAttack(const FName& OutpostID, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    void RepairDefenses(const FName& OutpostID, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    void ActivateShields(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Defense Management")
    void DeactivateShields(const FName& OutpostID);

    // Outpost Development
    UFUNCTION(BlueprintCallable, Category = "Development")
    bool CanUpgradeOutpost(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Development")
    bool UpgradeOutpost(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Development")
    EOutpostSize GetOutpostSize(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Development")
    void SetOutpostStatus(const FName& OutpostID, EOutpostStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Development")
    EOutpostStatus GetOutpostStatus(const FName& OutpostID);

    // Trading and Economy
    UFUNCTION(BlueprintCallable, Category = "Economy")
    void ProcessOutpostTrade(const FName& OutpostID, const FName& Resource, float Amount, bool bExport);

    UFUNCTION(BlueprintCallable, Category = "Economy")
    TMap<FName, float> GetOutpostPrices(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Economy")
    float GetOutpostWealth(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Economy")
    void UpdateOutpostWealth(const FName& OutpostID, float Change);

    // Faction Management
    UFUNCTION(BlueprintCallable, Category = "Factions")
    float GetFactionInfluence(const FName& OutpostID, const FName& Faction);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    void UpdateFactionInfluence(const FName& OutpostID, const FName& Faction, float Change);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    TArray<FName> GetControllingFactions(const FName& OutpostID);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    bool IsFactionWelcome(const FName& OutpostID, const FName& Faction);

    // Outpost Connections
    UFUNCTION(BlueprintCallable, Category = "Connections")
    bool ConnectOutposts(const FName& OutpostID1, const FName& OutpostID2);

    UFUNCTION(BlueprintCallable, Category = "Connections")
    bool DisconnectOutposts(const FName& OutpostID1, const FName& OutpostID2);

    UFUNCTION(BlueprintCallable, Category = "Connections")
    bool AreOutpostsConnected(const FName& OutpostID1, const FName& OutpostID2);

    UFUNCTION(BlueprintCallable, Category = "Connections")
    TArray<FName> GetConnectedOutposts(const FName& OutpostID);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDistanceToOutpost(const FName& OutpostID, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FName GetNearestOutpost(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<FName> GetOutpostsInRange(const FVector& Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsLocationInOutpost(const FName& OutpostID, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void GenerateOutpostReport(const FName& OutpostID);

protected:
    // Outpost Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Outposts")
    TMap<FName, FOutpost> Outposts;

    // Building Templates
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buildings")
    TMap<EBuildingType, FOutpostBuilding> BuildingTemplates;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ResourceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PopulationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefenseUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutoExpansion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableResourceTrading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableFactionDynamics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxOutpostsPerPlanet;

private:
    // Internal Functions
    void UpdateAllOutposts(float DeltaTime);
    void ProcessOutpostExpansion();
    void HandleOutpostEvents();
    void UpdateOutpostConnections();

    // Building Internal
    void InitializeBuildingTemplates();
    bool ValidateBuildingPlacement(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location);
    void ProcessBuildingProduction(const FName& OutpostID, FOutpostBuilding& Building, float DeltaTime);
    void UpdateBuildingPower(const FName& OutpostID, FOutpostBuilding& Building);

    // Resource Internal
    void CalculateResourceProduction(const FName& OutpostID);
    void CalculateResourceConsumption(const FName& OutpostID);
    void ProcessResourceStorage(const FName& OutpostID);
    void HandleResourceShortages(const FName& OutpostID);

    // Population Internal
    void CalculatePopulationGrowth(const FName& OutpostID);
    void UpdatePopulationNeeds(const FName& OutpostID);
    void ProcessPopulationMigration(const FName& OutpostID);
    void CalculatePopulationHappiness(const FName& OutpostID);

    // Defense Internal
    void UpdateDefenseSystems(const FName& OutpostID);
    void ProcessThreatDetection(const FName& OutpostID);
    void HandleDefenseRepairs(const FName& OutpostID);

    // Economy Internal
    void CalculateOutpostIncome(const FName& OutpostID);
    void ProcessOutpostExpenses(const FName& OutpostID);
    void UpdateLocalPrices(const FName& OutpostID);

    // Faction Internal
    void ProcessFactionInfluence(const FName& OutpostID);
    void HandleFactionConflicts(const FName& OutpostID);
    void UpdateFactionRelations(const FName& OutpostID);

    // Validation
    bool ValidateOutpost(const FOutpost& Outpost);
    bool ValidateBuilding(const FOutpostBuilding& Building);
    bool CanSupportBuilding(const FName& OutpostID, const FOutpostBuilding& Building);

    // Utilities Internal
    FName GenerateOutpostID();
    FName GenerateBuildingID();
    float CalculateOutpostEfficiency(const FName& OutpostID);
    void UpdateOutpostSize(const FName& OutpostID);

    // Timers
    UPROPERTY()
    FTimerHandle ResourceUpdateTimer;

    UPROPERTY()
    FTimerHandle PopulationUpdateTimer;

    UPROPERTY()
    FTimerHandle DefenseUpdateTimer;

    UPROPERTY()
    FTimerHandle ExpansionTimer;

    // Constants
    static constexpr float DEFAULT_RESOURCE_UPDATE_INTERVAL = 1.0f;
    static constexpr float DEFAULT_POPULATION_UPDATE_INTERVAL = 5.0f;
    static constexpr float DEFAULT_DEFENSE_UPDATE_INTERVAL = 0.5f;
    static constexpr float EXPANSION_CHECK_INTERVAL = 60.0f; // 1 minute
    static constexpr int32 DEFAULT_MAX_OUTPOSTS_PER_PLANET = 50;
    static constexpr float DEFAULT_POPULATION_GROWTH_RATE = 0.001f; // 0.1% per update
    static constexpr float DEFAULT_HAPPINESS_DECAY_RATE = 0.0001f; // 0.01% per update

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOutpostCreated, const FName&, OutpostID, const FOutpost&, OutpostData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOutpostDestroyed, const FName&, OutpostID, const FOutpost&, OutpostData);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBuildingConstructed, const FName&, OutpostID, const FName&, BuildingID, EBuildingType, BuildingType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOutpostUpgraded, const FName&, OutpostID, EOutpostSize, NewSize);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOutpostAttacked, const FName&, OutpostID, float, DamageAmount);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPopulationChanged, const FName&, OutpostID, int32, NewPopulation);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnOutpostCreated OnOutpostCreated;

    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnOutpostDestroyed OnOutpostDestroyed;

    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnBuildingConstructed OnBuildingConstructed;

    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnOutpostUpgraded OnOutpostUpgraded;

    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnOutpostAttacked OnOutpostAttacked;

    UPROPERTY(BlueprintAssignable, Category = "Outpost Events")
    FOnPopulationChanged OnPopulationChanged;
};