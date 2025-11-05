// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "PlanetaryResourcesComponent.generated.h"

// Resource types
UENUM(BlueprintType)
enum class EResourceType : uint8
{
    None            UMETA(DisplayName = "None"),
    Metals          UMETA(DisplayName = "Metals"),
    Crystals        UMETA(DisplayName = "Crystals"),
    Gas             UMETA(DisplayName = "Gas"),
    Biomass         UMETA(DisplayName = "Biomass"),
    Water           UMETA(DisplayName = "Water"),
    RareElements    UMETA(DisplayName = "Rare Elements"),
    Energy          UMETA(DisplayName = "Energy"),
    Food            UMETA(DisplayName = "Food"),
    Medicine        UMETA(DisplayName = "Medicine"),
    Electronics     UMETA(DisplayName = "Electronics"),
    Fuel            UMETA(DisplayName = "Fuel")
};

// Resource amount structure
USTRUCT(BlueprintType)
struct FResourceAmount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Metals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Crystals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Gas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Biomass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Water;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RareElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Food;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Medicine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Electronics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fuel;

    FResourceAmount()
    {
        Metals = 0.0f;
        Crystals = 0.0f;
        Gas = 0.0f;
        Biomass = 0.0f;
        Water = 0.0f;
        RareElements = 0.0f;
        Energy = 0.0f;
        Food = 0.0f;
        Medicine = 0.0f;
        Electronics = 0.0f;
        Fuel = 0.0f;
    }

    FResourceAmount(float InMetals, float InCrystals, float InGas, float InBiomass)
    {
        Metals = InMetals;
        Crystals = InCrystals;
        Gas = InGas;
        Biomass = InBiomass;
        Water = 0.0f;
        RareElements = 0.0f;
        Energy = 0.0f;
        Food = 0.0f;
        Medicine = 0.0f;
        Electronics = 0.0f;
        Fuel = 0.0f;
    }

    bool IsEmpty() const
    {
        return Metals <= 0.0f && Crystals <= 0.0f && Gas <= 0.0f && Biomass <= 0.0f &&
               Water <= 0.0f && RareElements <= 0.0f && Energy <= 0.0f && Food <= 0.0f &&
               Medicine <= 0.0f && Electronics <= 0.0f && Fuel <= 0.0f;
    }

    float GetTotalValue() const
    {
        return Metals + Crystals * 2.0f + Gas * 1.5f + Biomass * 1.2f + 
               Water * 1.0f + RareElements * 5.0f + Energy * 0.8f + 
               Food * 1.3f + Medicine * 3.0f + Electronics * 2.5f + Fuel * 1.8f;
    }

    FResourceAmount operator+(const FResourceAmount& Other) const
    {
        FResourceAmount Result;
        Result.Metals = Metals + Other.Metals;
        Result.Crystals = Crystals + Other.Crystals;
        Result.Gas = Gas + Other.Gas;
        Result.Biomass = Biomass + Other.Biomass;
        Result.Water = Water + Other.Water;
        Result.RareElements = RareElements + Other.RareElements;
        Result.Energy = Energy + Other.Energy;
        Result.Food = Food + Other.Food;
        Result.Medicine = Medicine + Other.Medicine;
        Result.Electronics = Electronics + Other.Electronics;
        Result.Fuel = Fuel + Other.Fuel;
        return Result;
    }

    FResourceAmount operator-(const FResourceAmount& Other) const
    {
        FResourceAmount Result;
        Result.Metals = FMath::Max(0.0f, Metals - Other.Metals);
        Result.Crystals = FMath::Max(0.0f, Crystals - Other.Crystals);
        Result.Gas = FMath::Max(0.0f, Gas - Other.Gas);
        Result.Biomass = FMath::Max(0.0f, Biomass - Other.Biomass);
        Result.Water = FMath::Max(0.0f, Water - Other.Water);
        Result.RareElements = FMath::Max(0.0f, RareElements - Other.RareElements);
        Result.Energy = FMath::Max(0.0f, Energy - Other.Energy);
        Result.Food = FMath::Max(0.0f, Food - Other.Food);
        Result.Medicine = FMath::Max(0.0f, Medicine - Other.Medicine);
        Result.Electronics = FMath::Max(0.0f, Electronics - Other.Electronics);
        Result.Fuel = FMath::Max(0.0f, Fuel - Other.Fuel);
        return Result;
    }

    bool operator>=(const FResourceAmount& Other) const
    {
        return Metals >= Other.Metals && Crystals >= Other.Crystals && Gas >= Other.Gas &&
               Biomass >= Other.Biomass && Water >= Other.Water && RareElements >= Other.RareElements &&
               Energy >= Other.Energy && Food >= Other.Food && Medicine >= Other.Medicine &&
               Electronics >= Other.Electronics && Fuel >= Other.Fuel;
    }

    bool operator<=(const FResourceAmount& Other) const
    {
        return Metals <= Other.Metals && Crystals <= Other.Crystals && Gas <= Other.Gas &&
               Biomass <= Other.Biomass && Water <= Other.Water && RareElements <= Other.RareElements &&
               Energy <= Other.Energy && Food <= Other.Food && Medicine <= Other.Medicine &&
               Electronics <= Other.Electronics && Fuel <= Other.Fuel;
    }
};

// Resource deposit data
USTRUCT(BlueprintType)
struct FResourceDeposit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExtractionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Richness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDepleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBeingMined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredTechnologies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EResourceType> Byproducts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime DiscoveryTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastExtractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MiningFacilityID;

    FResourceDeposit()
    {
        ResourceType = EResourceType::None;
        Location = FVector::ZeroVector;
        TotalAmount = 0.0f;
        CurrentAmount = 0.0f;
        ExtractionRate = 1.0f;
        Richness = 1.0f;
        Difficulty = 1.0f;
        bIsDepleted = false;
        bIsBeingMined = false;
        RequiredTechnologies.Empty();
        Byproducts.Empty();
        DiscoveryTime = FDateTime::Now();
        LastExtractionTime = FDateTime::Now();
        MiningFacilityID = -1;
    }
};

// Mining operation data
USTRUCT(BlueprintType)
struct FMiningOperation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OperationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MiningFacilityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetDepositID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExtractionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Efficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAutomated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastExtractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalExtracted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EResourceType> ExtractedByproducts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaintenanceCost;

    FMiningOperation()
    {
        OperationID = -1;
        MiningFacilityID = -1;
        TargetDepositID = -1;
        ResourceType = EResourceType::None;
        ExtractionRate = 1.0f;
        Efficiency = 1.0f;
        bIsActive = false;
        bIsAutomated = false;
        StartTime = FDateTime::Now();
        LastExtractionTime = FDateTime::Now();
        TotalExtracted = 0.0f;
        ExtractedByproducts.Empty();
        PowerConsumption = 10.0f;
        MaintenanceCost = 5.0f;
    }
};

// Resource processing data
USTRUCT(BlueprintType)
struct FResourceProcessing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProcessingFacilityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceType InputResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceType OutputResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InputAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OutputAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProcessingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Efficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAutomated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalProcessed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EResourceType> RequiredCatalysts;

    FResourceProcessing()
    {
        ProcessingFacilityID = -1;
        InputResource = EResourceType::None;
        OutputResource = EResourceType::None;
        InputAmount = 0.0f;
        OutputAmount = 0.0f;
        ProcessingRate = 1.0f;
        Efficiency = 1.0f;
        bIsActive = false;
        bIsAutomated = false;
        StartTime = FDateTime::Now();
        TotalProcessed = 0.0f;
        PowerConsumption = 15.0f;
        RequiredCatalysts.Empty();
    }
};

// Resource storage data
USTRUCT(BlueprintType)
struct FResourceStorage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StorageFacilityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount StoredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount MaxStorage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StorageEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClimateControlled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSecure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EResourceType> AllowedResources;

    FResourceStorage()
    {
        StorageFacilityID = -1;
        StoredResources = FResourceAmount();
        MaxStorage = FResourceAmount(1000.0f, 500.0f, 750.0f, 250.0f);
        StorageEfficiency = 1.0f;
        DecayRate = 0.01f;
        bIsClimateControlled = false;
        bIsSecure = false;
        AllowedResources.Empty();
    }
};

// Resource trade data
USTRUCT(BlueprintType)
struct FResourceTrade
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TradeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TradingPartner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount OfferedResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount RequestedResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TradeRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsExport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRecurring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime NextTradeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TradeFrequency; // In hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalTraded;

    FResourceTrade()
    {
        TradeID = -1;
        TradingPartner = TEXT("");
        OfferedResources = FResourceAmount();
        RequestedResources = FResourceAmount();
        TradeRatio = 1.0f;
        bIsExport = false;
        bIsActive = false;
        bIsRecurring = false;
        StartTime = FDateTime::Now();
        NextTradeTime = FDateTime::Now();
        TradeFrequency = 24;
        TotalTraded = 0.0f;
    }
};

// Resource analysis data
USTRUCT(BlueprintType)
struct FResourceAnalysis
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDeposits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalExtracted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentExtractionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProjectedDepletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MarketValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Demand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Supply;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ExtractionLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UsageLocations;

    FResourceAnalysis()
    {
        ResourceType = EResourceType::None;
        TotalDeposits = 0.0f;
        TotalExtracted = 0.0f;
        CurrentExtractionRate = 0.0f;
        ProjectedDepletionTime = 0.0f;
        MarketValue = 1.0f;
        Demand = 0.0f;
        Supply = 0.0f;
        ExtractionLocations.Empty();
        UsageLocations.Empty();
    }
};

// Resource statistics
USTRUCT(BlueprintType)
struct FResourceStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount TotalResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount ProductionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount ConsumptionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount StorageCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceAmount StorageUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveMiningOperations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveProcessingFacilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveStorageFacilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveTrades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalResourceValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MiningEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProcessingEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StorageEfficiency;

    FResourceStatistics()
    {
        TotalResources = FResourceAmount();
        ProductionRate = FResourceAmount();
        ConsumptionRate = FResourceAmount();
        StorageCapacity = FResourceAmount();
        StorageUsage = FResourceAmount();
        ActiveMiningOperations = 0;
        ActiveProcessingFacilities = 0;
        ActiveStorageFacilities = 0;
        ActiveTrades = 0;
        TotalResourceValue = 0.0f;
        MiningEfficiency = 1.0f;
        ProcessingEfficiency = 1.0f;
        StorageEfficiency = 1.0f;
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceDepositDiscovered, FResourceDeposit, Deposit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceExtracted, EResourceType, ResourceType, float, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceProcessed, EResourceType, InputType, EResourceType, OutputType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceStored, FResourceAmount, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceDepleted, EResourceType, ResourceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiningOperationStarted, FMiningOperation, Operation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiningOperationCompleted, FMiningOperation, Operation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeExecuted, FResourceTrade, Trade);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetaryResourcesComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UPlanetaryResourcesComponent();

    // Component overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Resource management
    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool AddResources(const FResourceAmount& Resources);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ConsumeResources(const FResourceAmount& Resources);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool HasEnoughResources(const FResourceAmount& Resources) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    FResourceAmount GetAvailableResources() const { return AvailableResources; }

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void SetAvailableResources(const FResourceAmount& Resources) { AvailableResources = Resources; }

    // Resource deposits
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceDeposit> GetResourceDeposits() const { return ResourceDeposits; }

    FResourceDeposit* GetResourceDeposit(int32 DepositID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceDeposit> GetDepositsByType(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool DiscoverResourceDeposit(const FVector& Location, EResourceType ResourceType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ScanForResources(const FVector& Center, float Radius);

    // Mining operations
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FMiningOperation> GetMiningOperations() const { return MiningOperations; }

    FMiningOperation* GetMiningOperation(int32 OperationID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 StartMiningOperation(int32 DepositID, int32 FacilityID, float ExtractionRate);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool StopMiningOperation(int32 OperationID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool UpgradeMiningOperation(int32 OperationID, float NewExtractionRate);

    // Resource processing
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceProcessing> GetProcessingOperations() const { return ProcessingOperations; }

    FResourceProcessing* GetProcessingOperation(int32 ProcessingID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 StartProcessingOperation(EResourceType InputResource, EResourceType OutputResource, int32 FacilityID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool StopProcessingOperation(int32 ProcessingID);

    // Resource storage
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceStorage> GetStorageFacilities() const { return StorageFacilities; }

    FResourceStorage* GetStorageFacility(int32 StorageID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 AddStorageFacility(const FResourceAmount& MaxCapacity);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool StoreResources(int32 StorageID, const FResourceAmount& Resources);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    FResourceAmount RetrieveResources(int32 StorageID, const FResourceAmount& RequestedAmount);

    // Resource trading
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceTrade> GetActiveTrades() const { return ActiveTrades; }

    FResourceTrade* GetTrade(int32 TradeID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 CreateTrade(const FString& Partner, const FResourceAmount& Offered, const FResourceAmount& Requested);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ExecuteTrade(int32 TradeID);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool CancelTrade(int32 TradeID);

    // Resource analysis
    UFUNCTION(BlueprintCallable, Category = "Resources")
    FResourceAnalysis AnalyzeResource(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceAnalysis> GetAllResourceAnalysis() const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    FResourceStatistics GetResourceStatistics() const;

    // Resource utilities
    UFUNCTION(BlueprintCallable, Category = "Resources")
    float GetResourceValue(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    FString GetResourceName(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool CanProcessResource(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<EResourceType> GetProcessingChain(EResourceType ResourceType) const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float ResourceScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float ResourceScanInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float MiningUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float ProcessingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float StorageDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float TradeUpdateInterval;

    // Resource settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EResourceType, float> ResourceBaseValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EResourceType, float> ResourceDecayRates;

    // Processing recipes (C++ only due to TArray in TMap limitation)
    TMap<EResourceType, TArray<EResourceType>> ProcessingRecipes;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnResourceDepositDiscovered OnResourceDepositDiscovered;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnResourceExtracted OnResourceExtracted;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnResourceProcessed OnResourceProcessed;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnResourceStored OnResourceStored;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnResourceDepleted OnResourceDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnMiningOperationStarted OnMiningOperationStarted;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnMiningOperationCompleted OnMiningOperationCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Resources")
    FOnTradeExecuted OnTradeExecuted;

protected:
    // Resource data
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    FResourceAmount AvailableResources;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FResourceDeposit> ResourceDeposits;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FMiningOperation> MiningOperations;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FResourceProcessing> ProcessingOperations;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FResourceStorage> StorageFacilities;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    TArray<FResourceTrade> ActiveTrades;

    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    float LastResourceScanTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    float LastMiningUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    float LastProcessingUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    float LastTradeUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 NextMiningOperationID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 NextProcessingID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 NextStorageID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 NextTradeID;

    // Internal functions
    void InitializeResourceSystem();
    void UpdateMiningOperations(float DeltaTime);
    void UpdateProcessingOperations(float DeltaTime);
    void UpdateStorageFacilities(float DeltaTime);
    void UpdateTrades(float DeltaTime);
    void UpdateResourceDecay(float DeltaTime);

    void ProcessMiningOperation(FMiningOperation& Operation, float DeltaTime);
    void ProcessProcessingOperation(FResourceProcessing& Operation, float DeltaTime);
    void ProcessStorageDecay(FResourceStorage& Storage, float DeltaTime);
    void ProcessTrade(FResourceTrade& Trade);

    FResourceAmount ExtractResourcesFromDeposit(FResourceDeposit& Deposit, float Amount);
    FResourceAmount ProcessResources(const FResourceProcessing& Processing, const FResourceAmount& Input);
    bool ValidateTrade(const FResourceTrade& Trade) const;
    void ExecuteTradeInternal(FResourceTrade& Trade);

    void GenerateResourceDeposits();
    void InitializeProcessingRecipes();
    void InitializeResourceValues();

    // Utility functions
    bool IsValidResourceType(EResourceType ResourceType) const;
    float CalculateMiningEfficiency(const FMiningOperation& Operation) const;
    float CalculateProcessingEfficiency(const FResourceProcessing& Processing) const;
    float CalculateStorageEfficiency(const FResourceStorage& Storage) const;
    float CalculateTradeValue(const FResourceTrade& Trade) const;

    FResourceDeposit* FindDepositByID(int32 DepositID);
    const FResourceDeposit* FindDepositByID(int32 DepositID) const;
    FMiningOperation* FindMiningOperation(int32 OperationID);
    FResourceProcessing* FindProcessingOperation(int32 ProcessingID);
    FResourceStorage* FindStorageFacility(int32 StorageID);
    FResourceTrade* FindTrade(int32 TradeID);

    // Helper functions
    void SetupResourceTimers();
    void PerformResourceScan();
    void StoreResourcesInAvailableStorage(const FResourceAmount& Resources);
    void RetrieveResourcesFromStorage(const FResourceAmount& Resources);
    void GenerateDepositByproducts(FResourceDeposit& Deposit);
    TArray<EResourceType> GetRequiredCatalysts(EResourceType InputResource, EResourceType OutputResource) const;
    float CalculateResourceDemand(EResourceType ResourceType) const;
    void AddResourceToProductionRate(FResourceAmount& ProductionRate, EResourceType ResourceType, float Rate) const;
    void SetResourceAmount(FResourceAmount& ResourceAmount, EResourceType ResourceType, float Amount) const;
    float GetResourceAmount(const FResourceAmount& ResourceAmount, EResourceType ResourceType) const;
    float CalculateAverageMiningEfficiency() const;
    float CalculateAverageProcessingEfficiency() const;
    float CalculateAverageStorageEfficiency() const;

private:
    // Timers
    UPROPERTY()
    FTimerHandle ResourceScanTimer;

    UPROPERTY()
    FTimerHandle MiningUpdateTimer;

    UPROPERTY()
    FTimerHandle ProcessingUpdateTimer;

    UPROPERTY()
    FTimerHandle TradeUpdateTimer;

    // Constants
    static constexpr float DEFAULT_SCAN_RADIUS = 5000.0f;
    static constexpr float DEFAULT_SCAN_INTERVAL = 30.0f;
    static constexpr float DEFAULT_MINING_INTERVAL = 1.0f;
    static constexpr float DEFAULT_PROCESSING_INTERVAL = 2.0f;
    static constexpr float DEFAULT_TRADE_INTERVAL = 60.0f;
    static constexpr float DEFAULT_DECAY_RATE = 0.001f;
};