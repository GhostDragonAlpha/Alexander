// Fill out your copyright notice in the Description page of Project Settings.

#include "Planetary/PlanetaryResourcesComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UPlanetaryResourcesComponent::UPlanetaryResourcesComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;
    
    // Set replication
    SetIsReplicatedByDefault(true);
    
    // Initialize configuration
    ResourceScanRadius = DEFAULT_SCAN_RADIUS;
    ResourceScanInterval = DEFAULT_SCAN_INTERVAL;
    MiningUpdateInterval = DEFAULT_MINING_INTERVAL;
    ProcessingUpdateInterval = DEFAULT_PROCESSING_INTERVAL;
    StorageDecayRate = DEFAULT_DECAY_RATE;
    TradeUpdateInterval = DEFAULT_TRADE_INTERVAL;
    
    // Initialize internal state
    LastResourceScanTime = 0.0f;
    LastMiningUpdateTime = 0.0f;
    LastProcessingUpdateTime = 0.0f;
    LastTradeUpdateTime = 0.0f;
    NextMiningOperationID = 1;
    NextProcessingID = 1;
    NextStorageID = 1;
    NextTradeID = 1;
    
    // Initialize resource data
    AvailableResources = FResourceAmount();
    ResourceDeposits.Empty();
    MiningOperations.Empty();
    ProcessingOperations.Empty();
    StorageFacilities.Empty();
    ActiveTrades.Empty();
}

// Called when the game starts
void UPlanetaryResourcesComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize resource system
    if (GetOwner()->HasAuthority())
    {
        InitializeResourceSystem();
    }
    
    // Set up timers
    SetupResourceTimers();
}

// Called every frame
void UPlanetaryResourcesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update resource systems
    if (GetOwner()->HasAuthority())
    {
        UpdateMiningOperations(DeltaTime);
        UpdateProcessingOperations(DeltaTime);
        UpdateStorageFacilities(DeltaTime);
        UpdateTrades(DeltaTime);
        UpdateResourceDecay(DeltaTime);
    }
}

// Replicated properties
void UPlanetaryResourcesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UPlanetaryResourcesComponent, AvailableResources);
    DOREPLIFETIME(UPlanetaryResourcesComponent, ResourceDeposits);
    DOREPLIFETIME(UPlanetaryResourcesComponent, MiningOperations);
    DOREPLIFETIME(UPlanetaryResourcesComponent, ProcessingOperations);
    DOREPLIFETIME(UPlanetaryResourcesComponent, StorageFacilities);
    DOREPLIFETIME(UPlanetaryResourcesComponent, ActiveTrades);
}

// Resource management
bool UPlanetaryResourcesComponent::AddResources(const FResourceAmount& Resources)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    if (Resources.IsEmpty()) return true;
    
    AvailableResources = AvailableResources + Resources;
    
    // Store in available storage
    StoreResourcesInAvailableStorage(Resources);
    
    // Trigger event
    OnResourceStored.Broadcast(Resources);
    
    return true;
}

bool UPlanetaryResourcesComponent::ConsumeResources(const FResourceAmount& Resources)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    if (!HasEnoughResources(Resources)) return false;
    
    AvailableResources = AvailableResources - Resources;
    
    // Remove from storage
    RetrieveResourcesFromStorage(Resources);
    
    return true;
}

bool UPlanetaryResourcesComponent::HasEnoughResources(const FResourceAmount& Resources) const
{
    return AvailableResources >= Resources;
}

// Resource deposits
FResourceDeposit* UPlanetaryResourcesComponent::GetResourceDeposit(int32 DepositID)
{
    return FindDepositByID(DepositID);
}

TArray<FResourceDeposit> UPlanetaryResourcesComponent::GetDepositsByType(EResourceType ResourceType) const
{
    TArray<FResourceDeposit> Deposits;
    
    for (const FResourceDeposit& Deposit : ResourceDeposits)
    {
        if (Deposit.ResourceType == ResourceType && !Deposit.bIsDepleted)
        {
            Deposits.Add(Deposit);
        }
    }
    
    return Deposits;
}

bool UPlanetaryResourcesComponent::DiscoverResourceDeposit(const FVector& Location, EResourceType ResourceType, float Amount)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    if (!IsValidResourceType(ResourceType) || Amount <= 0.0f) return false;
    
    // Check if deposit already exists at this location
    for (const FResourceDeposit& ExistingDeposit : ResourceDeposits)
    {
        if (FVector::Dist(Location, ExistingDeposit.Location) < 100.0f)
        {
            return false; // Too close to existing deposit
        }
    }
    
    // Create new deposit
    FResourceDeposit NewDeposit;
    NewDeposit.ResourceType = ResourceType;
    NewDeposit.Location = Location;
    NewDeposit.TotalAmount = Amount;
    NewDeposit.CurrentAmount = Amount;
    NewDeposit.Richness = FMath::RandRange(0.5f, 2.0f);
    NewDeposit.Difficulty = FMath::RandRange(0.5f, 1.5f);
    NewDeposit.DiscoveryTime = FDateTime::Now();
    NewDeposit.bIsDepleted = false;
    NewDeposit.bIsBeingMined = false;
    
    // Add byproducts based on resource type
    GenerateDepositByproducts(NewDeposit);
    
    ResourceDeposits.Add(NewDeposit);
    
    // Trigger event
    OnResourceDepositDiscovered.Broadcast(NewDeposit);
    
    UE_LOG(LogTemp, Log, TEXT("Discovered %s deposit with %f units at %s"), 
           *GetResourceName(ResourceType), Amount, *Location.ToString());
    
    return true;
}

bool UPlanetaryResourcesComponent::ScanForResources(const FVector& Center, float Radius)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    // Perform resource scan
    int32 DiscoveriesMade = 0;
    const int32 MaxDiscoveries = 5; // Limit discoveries per scan
    
    for (int32 i = 0; i < MaxDiscoveries; i++)
    {
        // Random location within scan radius
        FVector RandomLocation = Center + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, Radius);
        
        // Random resource type
        TArray<EResourceType> ValidTypes = {EResourceType::Metals, EResourceType::Crystals, EResourceType::Gas, EResourceType::Biomass};
        EResourceType RandomType = ValidTypes[FMath::RandRange(0, ValidTypes.Num() - 1)];
        
        // Random amount based on resource type
        float Amount = FMath::RandRange(100.0f, 1000.0f);
        
        // Try to discover deposit
        if (DiscoverResourceDeposit(RandomLocation, RandomType, Amount))
        {
            DiscoveriesMade++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Resource scan completed. Discovered %d new deposits."), DiscoveriesMade);
    return DiscoveriesMade > 0;
}

// Mining operations
FMiningOperation* UPlanetaryResourcesComponent::GetMiningOperation(int32 OperationID)
{
    return FindMiningOperation(OperationID);
}

int32 UPlanetaryResourcesComponent::StartMiningOperation(int32 DepositID, int32 FacilityID, float ExtractionRate)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    // Find deposit
    FResourceDeposit* Deposit = FindDepositByID(DepositID);
    if (!Deposit || Deposit->bIsDepleted || Deposit->bIsBeingMined)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mining operation: Invalid or unavailable deposit"));
        return -1;
    }
    
    // Create mining operation
    FMiningOperation NewOperation;
    NewOperation.OperationID = NextMiningOperationID++;
    NewOperation.MiningFacilityID = FacilityID;
    NewOperation.TargetDepositID = DepositID;
    NewOperation.ResourceType = Deposit->ResourceType;
    NewOperation.ExtractionRate = ExtractionRate;
    NewOperation.Efficiency = CalculateMiningEfficiency(NewOperation);
    NewOperation.bIsActive = true;
    NewOperation.bIsAutomated = true;
    NewOperation.StartTime = FDateTime::Now();
    NewOperation.LastExtractionTime = FDateTime::Now();
    NewOperation.TotalExtracted = 0.0f;
    NewOperation.PowerConsumption = 10.0f * ExtractionRate;
    NewOperation.MaintenanceCost = 5.0f * ExtractionRate;
    
    // Add byproducts
    NewOperation.ExtractedByproducts = Deposit->Byproducts;
    
    MiningOperations.Add(NewOperation);
    
    // Mark deposit as being mined
    Deposit->bIsBeingMined = true;
    Deposit->MiningFacilityID = FacilityID;
    
    // Trigger event
    OnMiningOperationStarted.Broadcast(NewOperation);
    
    UE_LOG(LogTemp, Log, TEXT("Started mining operation %d for %s"), 
           NewOperation.OperationID, *GetResourceName(Deposit->ResourceType));
    
    return NewOperation.OperationID;
}

bool UPlanetaryResourcesComponent::StopMiningOperation(int32 OperationID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FMiningOperation* Operation = FindMiningOperation(OperationID);
    if (!Operation) return false;
    
    // Find associated deposit
    FResourceDeposit* Deposit = FindDepositByID(Operation->TargetDepositID);
    if (Deposit)
    {
        Deposit->bIsBeingMined = false;
        Deposit->MiningFacilityID = -1;
    }
    
    // Remove operation
    MiningOperations.RemoveAll([&](const FMiningOperation& Op)
    {
        return Op.OperationID == OperationID;
    });
    
    // Trigger event
    OnMiningOperationCompleted.Broadcast(*Operation);
    
    UE_LOG(LogTemp, Log, TEXT("Stopped mining operation %d"), OperationID);
    return true;
}

bool UPlanetaryResourcesComponent::UpgradeMiningOperation(int32 OperationID, float NewExtractionRate)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FMiningOperation* Operation = FindMiningOperation(OperationID);
    if (!Operation) return false;
    
    Operation->ExtractionRate = NewExtractionRate;
    Operation->Efficiency = CalculateMiningEfficiency(*Operation);
    Operation->PowerConsumption = 10.0f * NewExtractionRate;
    Operation->MaintenanceCost = 5.0f * NewExtractionRate;
    
    UE_LOG(LogTemp, Log, TEXT("Upgraded mining operation %d to extraction rate %f"), 
           OperationID, NewExtractionRate);
    
    return true;
}

// Resource processing
FResourceProcessing* UPlanetaryResourcesComponent::GetProcessingOperation(int32 ProcessingID)
{
    return FindProcessingOperation(ProcessingID);
}

int32 UPlanetaryResourcesComponent::StartProcessingOperation(EResourceType InputResource, EResourceType OutputResource, int32 FacilityID)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    if (!CanProcessResource(InputResource)) return -1;
    
    // Create processing operation
    FResourceProcessing NewProcessing;
    NewProcessing.ProcessingFacilityID = FacilityID;
    NewProcessing.InputResource = InputResource;
    NewProcessing.OutputResource = OutputResource;
    NewProcessing.ProcessingRate = 1.0f;
    NewProcessing.Efficiency = CalculateProcessingEfficiency(NewProcessing);
    NewProcessing.bIsActive = true;
    NewProcessing.bIsAutomated = true;
    NewProcessing.StartTime = FDateTime::Now();
    NewProcessing.TotalProcessed = 0.0f;
    NewProcessing.PowerConsumption = 15.0f;
    
    // Set required catalysts based on recipe
    NewProcessing.RequiredCatalysts = GetRequiredCatalysts(InputResource, OutputResource);
    
    ProcessingOperations.Add(NewProcessing);
    
    UE_LOG(LogTemp, Log, TEXT("Started processing operation for %s -> %s"), 
           *GetResourceName(InputResource), *GetResourceName(OutputResource));
    
    return NewProcessing.ProcessingFacilityID;
}

bool UPlanetaryResourcesComponent::StopProcessingOperation(int32 ProcessingID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FResourceProcessing* Processing = FindProcessingOperation(ProcessingID);
    if (!Processing) return false;
    
    Processing->bIsActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Stopped processing operation %d"), ProcessingID);
    return true;
}

// Resource storage
FResourceStorage* UPlanetaryResourcesComponent::GetStorageFacility(int32 StorageID)
{
    return FindStorageFacility(StorageID);
}

int32 UPlanetaryResourcesComponent::AddStorageFacility(const FResourceAmount& MaxCapacity)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    FResourceStorage NewStorage;
    NewStorage.StorageFacilityID = NextStorageID++;
    NewStorage.MaxStorage = MaxCapacity;
    NewStorage.StoredResources = FResourceAmount();
    NewStorage.StorageEfficiency = 1.0f;
    NewStorage.DecayRate = StorageDecayRate;
    NewStorage.bIsClimateControlled = false;
    NewStorage.bIsSecure = false;
    
    StorageFacilities.Add(NewStorage);
    
    UE_LOG(LogTemp, Log, TEXT("Added storage facility %d"), NewStorage.StorageFacilityID);
    return NewStorage.StorageFacilityID;
}

bool UPlanetaryResourcesComponent::StoreResources(int32 StorageID, const FResourceAmount& Resources)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FResourceStorage* Storage = FindStorageFacility(StorageID);
    if (!Storage) return false;
    
    // Check storage capacity
    FResourceAmount NewTotal = Storage->StoredResources + Resources;
    if (!(NewTotal <= Storage->MaxStorage))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient storage capacity in facility %d"), StorageID);
        return false;
    }
    
    Storage->StoredResources = NewTotal;
    
    UE_LOG(LogTemp, Log, TEXT("Stored resources in facility %d"), StorageID);
    return true;
}

FResourceAmount UPlanetaryResourcesComponent::RetrieveResources(int32 StorageID, const FResourceAmount& RequestedAmount)
{
    if (!GetOwner()->HasAuthority()) return FResourceAmount();
    
    FResourceStorage* Storage = FindStorageFacility(StorageID);
    if (!Storage) return FResourceAmount();
    
    // Check if enough resources are available
    if (!(Storage->StoredResources >= RequestedAmount))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient resources in storage facility %d"), StorageID);
        return FResourceAmount();
    }
    
    // Retrieve resources
    Storage->StoredResources = Storage->StoredResources - RequestedAmount;
    
    UE_LOG(LogTemp, Log, TEXT("Retrieved resources from storage facility %d"), StorageID);
    return RequestedAmount;
}

// Resource trading
FResourceTrade* UPlanetaryResourcesComponent::GetTrade(int32 TradeID)
{
    return FindTrade(TradeID);
}

int32 UPlanetaryResourcesComponent::CreateTrade(const FString& Partner, const FResourceAmount& Offered, const FResourceAmount& Requested)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    if (!HasEnoughResources(Offered))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient resources to create trade"));
        return -1;
    }
    
    FResourceTrade NewTrade;
    NewTrade.TradeID = NextTradeID++;
    NewTrade.TradingPartner = Partner;
    NewTrade.OfferedResources = Offered;
    NewTrade.RequestedResources = Requested;
    NewTrade.TradeRatio = CalculateTradeValue(NewTrade);
    NewTrade.bIsExport = !Offered.IsEmpty();
    NewTrade.bIsActive = true;
    NewTrade.bIsRecurring = false;
    NewTrade.StartTime = FDateTime::Now();
    NewTrade.NextTradeTime = FDateTime::Now();
    NewTrade.TradeFrequency = 24; // 24 hours
    NewTrade.TotalTraded = 0.0f;
    
    ActiveTrades.Add(NewTrade);
    
    UE_LOG(LogTemp, Log, TEXT("Created trade %d with partner %s"), NewTrade.TradeID, *Partner);
    return NewTrade.TradeID;
}

bool UPlanetaryResourcesComponent::ExecuteTrade(int32 TradeID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FResourceTrade* Trade = FindTrade(TradeID);
    if (!Trade || !Trade->bIsActive) return false;
    
    if (!ValidateTrade(*Trade))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trade %d validation failed"), TradeID);
        return false;
    }
    
    ExecuteTradeInternal(*Trade);
    
    // Update next trade time for recurring trades
    if (Trade->bIsRecurring)
    {
        Trade->NextTradeTime = FDateTime::Now() + FTimespan::FromHours(Trade->TradeFrequency);
    }
    else
    {
        Trade->bIsActive = false;
    }
    
    // Trigger event
    OnTradeExecuted.Broadcast(*Trade);
    
    UE_LOG(LogTemp, Log, TEXT("Executed trade %d"), TradeID);
    return true;
}

bool UPlanetaryResourcesComponent::CancelTrade(int32 TradeID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FResourceTrade* Trade = FindTrade(TradeID);
    if (!Trade) return false;
    
    Trade->bIsActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Cancelled trade %d"), TradeID);
    return true;
}

// Resource analysis
FResourceAnalysis UPlanetaryResourcesComponent::AnalyzeResource(EResourceType ResourceType) const
{
    FResourceAnalysis Analysis;
    Analysis.ResourceType = ResourceType;
    
    // Calculate total deposits
    for (const FResourceDeposit& Deposit : ResourceDeposits)
    {
        if (Deposit.ResourceType == ResourceType)
        {
            Analysis.TotalDeposits += Deposit.TotalAmount;
            Analysis.TotalExtracted += (Deposit.TotalAmount - Deposit.CurrentAmount);
            Analysis.ExtractionLocations.Add(FString::SanitizeFloat(Deposit.Location.X));
        }
    }
    
    // Calculate current extraction rate
    for (const FMiningOperation& Operation : MiningOperations)
    {
        if (Operation.ResourceType == ResourceType && Operation.bIsActive)
        {
            Analysis.CurrentExtractionRate += Operation.ExtractionRate * Operation.Efficiency;
        }
    }
    
    // Calculate projected depletion time
    if (Analysis.CurrentExtractionRate > 0.0f)
    {
        float RemainingResources = Analysis.TotalDeposits - Analysis.TotalExtracted;
        Analysis.ProjectedDepletionTime = RemainingResources / Analysis.CurrentExtractionRate;
    }
    
    // Get market value
    Analysis.MarketValue = GetResourceValue(ResourceType);
    
    // Calculate demand and supply (simplified)
    Analysis.Demand = CalculateResourceDemand(ResourceType);
    Analysis.Supply = Analysis.CurrentExtractionRate;
    
    return Analysis;
}

TArray<FResourceAnalysis> UPlanetaryResourcesComponent::GetAllResourceAnalysis() const
{
    TArray<FResourceAnalysis> AllAnalysis;
    
    TArray<EResourceType> ResourceTypes = {
        EResourceType::Metals, EResourceType::Crystals, EResourceType::Gas, 
        EResourceType::Biomass, EResourceType::Water, EResourceType::RareElements
    };
    
    for (EResourceType ResourceType : ResourceTypes)
    {
        AllAnalysis.Add(AnalyzeResource(ResourceType));
    }
    
    return AllAnalysis;
}

FResourceStatistics UPlanetaryResourcesComponent::GetResourceStatistics() const
{
    FResourceStatistics Stats;
    Stats.TotalResources = AvailableResources;
    Stats.StorageUsage = AvailableResources;
    
    // Calculate production and consumption rates
    for (const FMiningOperation& Operation : MiningOperations)
    {
        if (Operation.bIsActive)
        {
            float ProductionRate = Operation.ExtractionRate * Operation.Efficiency;
            AddResourceToProductionRate(Stats.ProductionRate, Operation.ResourceType, ProductionRate);
        }
    }
    
    for (const FResourceProcessing& Processing : ProcessingOperations)
    {
        if (Processing.bIsActive)
        {
            float ConsumptionRate = Processing.ProcessingRate * Processing.Efficiency;
            AddResourceToProductionRate(Stats.ConsumptionRate, Processing.InputResource, ConsumptionRate);
        }
    }
    
    // Calculate storage capacity
    for (const FResourceStorage& Storage : StorageFacilities)
    {
        Stats.StorageCapacity = Stats.StorageCapacity + Storage.MaxStorage;
    }
    
    // Count active facilities
    Stats.ActiveMiningOperations = MiningOperations.FilterByPredicate([](const FMiningOperation& Op)
    {
        return Op.bIsActive;
    }).Num();
    
    Stats.ActiveProcessingFacilities = ProcessingOperations.FilterByPredicate([](const FResourceProcessing& Proc)
    {
        return Proc.bIsActive;
    }).Num();
    
    Stats.ActiveStorageFacilities = StorageFacilities.Num();
    Stats.ActiveTrades = ActiveTrades.FilterByPredicate([](const FResourceTrade& Trade)
    {
        return Trade.bIsActive;
    }).Num();
    
    // Calculate total value and efficiencies
    Stats.TotalResourceValue = AvailableResources.GetTotalValue();
    Stats.MiningEfficiency = CalculateAverageMiningEfficiency();
    Stats.ProcessingEfficiency = CalculateAverageProcessingEfficiency();
    Stats.StorageEfficiency = CalculateAverageStorageEfficiency();
    
    return Stats;
}

// Resource utilities
float UPlanetaryResourcesComponent::GetResourceValue(EResourceType ResourceType) const
{
    if (ResourceBaseValues.Contains(ResourceType))
    {
        return ResourceBaseValues[ResourceType];
    }
    
    // Default values
    switch (ResourceType)
    {
        case EResourceType::Metals: return 1.0f;
        case EResourceType::Crystals: return 2.0f;
        case EResourceType::Gas: return 1.5f;
        case EResourceType::Biomass: return 1.2f;
        case EResourceType::Water: return 1.0f;
        case EResourceType::RareElements: return 5.0f;
        case EResourceType::Energy: return 0.8f;
        case EResourceType::Food: return 1.3f;
        case EResourceType::Medicine: return 3.0f;
        case EResourceType::Electronics: return 2.5f;
        case EResourceType::Fuel: return 1.8f;
        default: return 1.0f;
    }
}

FString UPlanetaryResourcesComponent::GetResourceName(EResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EResourceType::Metals: return TEXT("Metals");
        case EResourceType::Crystals: return TEXT("Crystals");
        case EResourceType::Gas: return TEXT("Gas");
        case EResourceType::Biomass: return TEXT("Biomass");
        case EResourceType::Water: return TEXT("Water");
        case EResourceType::RareElements: return TEXT("Rare Elements");
        case EResourceType::Energy: return TEXT("Energy");
        case EResourceType::Food: return TEXT("Food");
        case EResourceType::Medicine: return TEXT("Medicine");
        case EResourceType::Electronics: return TEXT("Electronics");
        case EResourceType::Fuel: return TEXT("Fuel");
        default: return TEXT("Unknown");
    }
}

bool UPlanetaryResourcesComponent::CanProcessResource(EResourceType ResourceType) const
{
    return ProcessingRecipes.Contains(ResourceType);
}

TArray<EResourceType> UPlanetaryResourcesComponent::GetProcessingChain(EResourceType ResourceType) const
{
    TArray<EResourceType> Chain;
    
    if (ProcessingRecipes.Contains(ResourceType))
    {
        Chain = ProcessingRecipes[ResourceType];
    }
    
    return Chain;
}

// Internal functions
void UPlanetaryResourcesComponent::InitializeResourceSystem()
{
    // Initialize resource values
    InitializeResourceValues();
    
    // Initialize processing recipes
    InitializeProcessingRecipes();
    
    // Generate initial resource deposits
    GenerateResourceDeposits();
    
    // Add default storage facility
    AddStorageFacility(FResourceAmount(10000.0f, 5000.0f, 7500.0f, 2500.0f));
    
    UE_LOG(LogTemp, Log, TEXT("Resource system initialized"));
}

void UPlanetaryResourcesComponent::UpdateMiningOperations(float DeltaTime)
{
    for (FMiningOperation& Operation : MiningOperations)
    {
        if (Operation.bIsActive)
        {
            ProcessMiningOperation(Operation, DeltaTime);
        }
    }
}

void UPlanetaryResourcesComponent::UpdateProcessingOperations(float DeltaTime)
{
    for (FResourceProcessing& Processing : ProcessingOperations)
    {
        if (Processing.bIsActive)
        {
            ProcessProcessingOperation(Processing, DeltaTime);
        }
    }
}

void UPlanetaryResourcesComponent::UpdateStorageFacilities(float DeltaTime)
{
    for (FResourceStorage& Storage : StorageFacilities)
    {
        ProcessStorageDecay(Storage, DeltaTime);
    }
}

void UPlanetaryResourcesComponent::UpdateTrades(float DeltaTime)
{
    for (FResourceTrade& Trade : ActiveTrades)
    {
        if (Trade.bIsActive && Trade.NextTradeTime <= FDateTime::Now())
        {
            ProcessTrade(Trade);
        }
    }
}

void UPlanetaryResourcesComponent::UpdateResourceDecay(float DeltaTime)
{
    // Apply decay to available resources
    float DecayAmount = StorageDecayRate * DeltaTime;
    
    // Apply decay to each resource type
    AvailableResources.Metals = FMath::Max(0.0f, AvailableResources.Metals - DecayAmount * 0.1f);
    AvailableResources.Biomass = FMath::Max(0.0f, AvailableResources.Biomass - DecayAmount * 0.5f);
    AvailableResources.Food = FMath::Max(0.0f, AvailableResources.Food - DecayAmount * 0.8f);
    AvailableResources.Medicine = FMath::Max(0.0f, AvailableResources.Medicine - DecayAmount * 0.2f);
}

void UPlanetaryResourcesComponent::ProcessMiningOperation(FMiningOperation& Operation, float DeltaTime)
{
    FResourceDeposit* Deposit = FindDepositByID(Operation.TargetDepositID);
    if (!Deposit || Deposit->bIsDepleted)
    {
        Operation.bIsActive = false;
        return;
    }
    
    // Calculate extraction amount
    float ExtractionAmount = Operation.ExtractionRate * Operation.Efficiency * DeltaTime;
    
    // Extract resources
    FResourceAmount ExtractedResources = ExtractResourcesFromDeposit(*Deposit, ExtractionAmount);
    
    if (!ExtractedResources.IsEmpty())
    {
        // Add to available resources
        AddResources(ExtractedResources);
        
        // Update operation stats
        Operation.TotalExtracted += ExtractionAmount;
        Operation.LastExtractionTime = FDateTime::Now();
        
        // Trigger event
        OnResourceExtracted.Broadcast(Operation.ResourceType, ExtractionAmount);
        
        // Check if deposit is depleted
        if (Deposit->CurrentAmount <= 0.0f)
        {
            Deposit->bIsDepleted = true;
            Operation.bIsActive = false;
            OnResourceDepleted.Broadcast(Operation.ResourceType);
        }
    }
}

void UPlanetaryResourcesComponent::ProcessProcessingOperation(FResourceProcessing& Processing, float DeltaTime)
{
    // Calculate processing amount
    float ProcessingAmount = Processing.ProcessingRate * Processing.Efficiency * DeltaTime;
    
    // Check if we have enough input resources
    FResourceAmount InputResources;
    SetResourceAmount(InputResources, Processing.InputResource, ProcessingAmount);
    
    if (!HasEnoughResources(InputResources))
    {
        return; // Not enough resources
    }
    
    // Consume input resources
    ConsumeResources(InputResources);
    
    // Process resources
    FResourceAmount OutputResources = ProcessResources(Processing, InputResources);
    
    if (!OutputResources.IsEmpty())
    {
        // Add output resources
        AddResources(OutputResources);
        
        // Update processing stats
        Processing.TotalProcessed += ProcessingAmount;
        
        // Trigger event
        OnResourceProcessed.Broadcast(Processing.InputResource, Processing.OutputResource);
    }
}

void UPlanetaryResourcesComponent::ProcessStorageDecay(FResourceStorage& Storage, float DeltaTime)
{
    if (Storage.bIsClimateControlled) return;
    
    // Apply decay to stored resources
    float DecayAmount = Storage.DecayRate * DeltaTime;
    
    Storage.StoredResources.Biomass = FMath::Max(0.0f, Storage.StoredResources.Biomass - DecayAmount * 0.5f);
    Storage.StoredResources.Food = FMath::Max(0.0f, Storage.StoredResources.Food - DecayAmount * 0.8f);
    Storage.StoredResources.Medicine = FMath::Max(0.0f, Storage.StoredResources.Medicine - DecayAmount * 0.2f);
}

void UPlanetaryResourcesComponent::ProcessTrade(FResourceTrade& Trade)
{
    if (!ValidateTrade(Trade)) return;
    
    ExecuteTradeInternal(Trade);
    
    // Update next trade time
    if (Trade.bIsRecurring)
    {
        Trade.NextTradeTime = FDateTime::Now() + FTimespan::FromHours(Trade.TradeFrequency);
    }
    else
    {
        Trade.bIsActive = false;
    }
    
    // Trigger event
    OnTradeExecuted.Broadcast(Trade);
}

FResourceAmount UPlanetaryResourcesComponent::ExtractResourcesFromDeposit(FResourceDeposit& Deposit, float Amount)
{
    if (Deposit.bIsDepleted || Deposit.CurrentAmount <= 0.0f)
    {
        return FResourceAmount();
    }
    
    // Calculate actual extraction amount
    float ActualAmount = FMath::Min(Amount, Deposit.CurrentAmount);
    
    // Apply richness modifier
    ActualAmount *= Deposit.Richness;
    
    // Update deposit
    Deposit.CurrentAmount -= ActualAmount;
    Deposit.LastExtractionTime = FDateTime::Now();
    
    if (Deposit.CurrentAmount <= 0.0f)
    {
        Deposit.bIsDepleted = true;
    }
    
    // Create resource amount
    FResourceAmount ExtractedResources;
    SetResourceAmount(ExtractedResources, Deposit.ResourceType, ActualAmount);
    
    // Add byproducts
    for (EResourceType Byproduct : Deposit.Byproducts)
    {
        float ByproductAmount = ActualAmount * 0.1f; // 10% byproduct rate
        SetResourceAmount(ExtractedResources, Byproduct, ByproductAmount);
    }
    
    return ExtractedResources;
}

FResourceAmount UPlanetaryResourcesComponent::ProcessResources(const FResourceProcessing& Processing, const FResourceAmount& Input)
{
    // Calculate output amount based on processing efficiency
    float InputAmount = GetResourceAmount(Input, Processing.InputResource);
    float OutputAmount = InputAmount * Processing.Efficiency * 0.8f; // 80% conversion efficiency
    
    FResourceAmount OutputResources;
    SetResourceAmount(OutputResources, Processing.OutputResource, OutputAmount);
    
    return OutputResources;
}

bool UPlanetaryResourcesComponent::ValidateTrade(const FResourceTrade& Trade) const
{
    if (Trade.bIsExport)
    {
        return HasEnoughResources(Trade.OfferedResources);
    }
    else
    {
        return true; // Import trades are always valid (assuming partner has resources)
    }
}

void UPlanetaryResourcesComponent::ExecuteTradeInternal(FResourceTrade& Trade)
{
    if (Trade.bIsExport)
    {
        // Export: give resources, get nothing in return (simplified)
        ConsumeResources(Trade.OfferedResources);
    }
    else
    {
        // Import: get resources, give nothing in return (simplified)
        AddResources(Trade.RequestedResources);
    }
    
    Trade.TotalTraded += FMath::Max(Trade.OfferedResources.GetTotalValue(), Trade.RequestedResources.GetTotalValue());
}

void UPlanetaryResourcesComponent::GenerateResourceDeposits()
{
    // Generate initial resource deposits around the planet
    FVector Center = GetOwner()->GetActorLocation();
    const int32 NumDeposits = 20;
    
    for (int32 i = 0; i < NumDeposits; i++)
    {
        FVector RandomLocation = Center + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(1000.0f, 5000.0f);
        
        TArray<EResourceType> ResourceTypes = {
            EResourceType::Metals, EResourceType::Crystals, EResourceType::Gas, EResourceType::Biomass
        };
        
        EResourceType RandomType = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
        float Amount = FMath::RandRange(500.0f, 5000.0f);
        
        DiscoverResourceDeposit(RandomLocation, RandomType, Amount);
    }
}

void UPlanetaryResourcesComponent::InitializeProcessingRecipes()
{
    // Initialize processing recipes
    ProcessingRecipes.Add(EResourceType::Biomass, {EResourceType::Food, EResourceType::Medicine});
    ProcessingRecipes.Add(EResourceType::Metals, {EResourceType::Electronics, EResourceType::Fuel});
    ProcessingRecipes.Add(EResourceType::Crystals, {EResourceType::Electronics, EResourceType::Energy});
    ProcessingRecipes.Add(EResourceType::Gas, {EResourceType::Fuel, EResourceType::Energy});
    ProcessingRecipes.Add(EResourceType::Water, {EResourceType::Biomass, EResourceType::Food});
}

void UPlanetaryResourcesComponent::InitializeResourceValues()
{
    // Initialize base resource values
    ResourceBaseValues.Add(EResourceType::Metals, 1.0f);
    ResourceBaseValues.Add(EResourceType::Crystals, 2.0f);
    ResourceBaseValues.Add(EResourceType::Gas, 1.5f);
    ResourceBaseValues.Add(EResourceType::Biomass, 1.2f);
    ResourceBaseValues.Add(EResourceType::Water, 1.0f);
    ResourceBaseValues.Add(EResourceType::RareElements, 5.0f);
    ResourceBaseValues.Add(EResourceType::Energy, 0.8f);
    ResourceBaseValues.Add(EResourceType::Food, 1.3f);
    ResourceBaseValues.Add(EResourceType::Medicine, 3.0f);
    ResourceBaseValues.Add(EResourceType::Electronics, 2.5f);
    ResourceBaseValues.Add(EResourceType::Fuel, 1.8f);
}

// Utility functions
bool UPlanetaryResourcesComponent::IsValidResourceType(EResourceType ResourceType) const
{
    return ResourceType != EResourceType::None;
}

float UPlanetaryResourcesComponent::CalculateMiningEfficiency(const FMiningOperation& Operation) const
{
    FResourceDeposit* Deposit = FindDepositByID(Operation.TargetDepositID);
    if (!Deposit) return 0.5f;
    
    float Efficiency = 1.0f;
    
    // Apply difficulty modifier
    Efficiency /= Deposit->Difficulty;
    
    // Apply richness modifier
    Efficiency *= FMath::Clamp(Deposit->Richness, 0.5f, 2.0f);
    
    return FMath::Clamp(Efficiency, 0.1f, 2.0f);
}

float UPlanetaryResourcesComponent::CalculateProcessingEfficiency(const FResourceProcessing& Processing) const
{
    float Efficiency = Processing.Efficiency;
    
    // Apply technology bonuses (simplified)
    Efficiency *= 1.2f; // Assume 20% technology bonus
    
    return FMath::Clamp(Efficiency, 0.1f, 2.0f);
}

float UPlanetaryResourcesComponent::CalculateStorageEfficiency(const FResourceStorage& Storage) const
{
    float Efficiency = Storage.StorageEfficiency;
    
    // Apply climate control bonus
    if (Storage.bIsClimateControlled)
    {
        Efficiency *= 1.5f;
    }
    
    // Apply security bonus
    if (Storage.bIsSecure)
    {
        Efficiency *= 1.1f;
    }
    
    return FMath::Clamp(Efficiency, 0.5f, 2.0f);
}

float UPlanetaryResourcesComponent::CalculateTradeValue(const FResourceTrade& Trade) const
{
    float OfferedValue = Trade.OfferedResources.GetTotalValue();
    float RequestedValue = Trade.RequestedResources.GetTotalValue();
    
    if (RequestedValue > 0.0f)
    {
        return OfferedValue / RequestedValue;
    }
    
    return 0.0f;
}

FResourceDeposit* UPlanetaryResourcesComponent::FindDepositByID(int32 DepositID)
{
    for (FResourceDeposit& Deposit : ResourceDeposits)
    {
        if (Deposit.ResourceType != EResourceType::None) // Use ResourceType as ID placeholder
        {
            return &Deposit;
        }
    }
    return nullptr;
}

FMiningOperation* UPlanetaryResourcesComponent::FindMiningOperation(int32 OperationID)
{
    for (FMiningOperation& Operation : MiningOperations)
    {
        if (Operation.OperationID == OperationID)
        {
            return &Operation;
        }
    }
    return nullptr;
}

FResourceProcessing* UPlanetaryResourcesComponent::FindProcessingOperation(int32 ProcessingID)
{
    for (FResourceProcessing& Processing : ProcessingOperations)
    {
        if (Processing.ProcessingFacilityID == ProcessingID)
        {
            return &Processing;
        }
    }
    return nullptr;
}

FResourceStorage* UPlanetaryResourcesComponent::FindStorageFacility(int32 StorageID)
{
    for (FResourceStorage& Storage : StorageFacilities)
    {
        if (Storage.StorageFacilityID == StorageID)
        {
            return &Storage;
        }
    }
    return nullptr;
}

FResourceTrade* UPlanetaryResourcesComponent::FindTrade(int32 TradeID)
{
    for (FResourceTrade& Trade : ActiveTrades)
    {
        if (Trade.TradeID == TradeID)
        {
            return &Trade;
        }
    }
    return nullptr;
}

// Helper functions
void UPlanetaryResourcesComponent::SetupResourceTimers()
{
    if (!GetOwner()->HasAuthority()) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Set up resource scan timer
    World->GetTimerManager().SetTimer(
        ResourceScanTimer,
        this,
        &UPlanetaryResourcesComponent::PerformResourceScan,
        ResourceScanInterval,
        true
    );
    
    // Set up mining update timer
    World->GetTimerManager().SetTimer(
        MiningUpdateTimer,
        this,
        &UPlanetaryResourcesComponent::UpdateMiningOperations,
        MiningUpdateInterval,
        true
    );
    
    // Set up processing update timer
    World->GetTimerManager().SetTimer(
        ProcessingUpdateTimer,
        this,
        &UPlanetaryResourcesComponent::UpdateProcessingOperations,
        ProcessingUpdateInterval,
        true
    );
    
    // Set up trade update timer
    World->GetTimerManager().SetTimer(
        TradeUpdateTimer,
        this,
        &UPlanetaryResourcesComponent::UpdateTrades,
        TradeUpdateInterval,
        true
    );
}

void UPlanetaryResourcesComponent::PerformResourceScan()
{
    FVector Center = GetOwner()->GetActorLocation();
    ScanForResources(Center, ResourceScanRadius);
}

void UPlanetaryResourcesComponent::StoreResourcesInAvailableStorage(const FResourceAmount& Resources)
{
    // Find storage facility with available capacity
    for (FResourceStorage& Storage : StorageFacilities)
    {
        if (Storage.StoredResources + Resources <= Storage.MaxStorage)
        {
            Storage.StoredResources = Storage.StoredResources + Resources;
            return;
        }
    }
    
    // If no storage has capacity, resources remain in AvailableResources
}

void UPlanetaryResourcesComponent::RetrieveResourcesFromStorage(const FResourceAmount& Resources)
{
    // Retrieve from storage facilities (simplified - just remove from first available)
    for (FResourceStorage& Storage : StorageFacilities)
    {
        if (Storage.StoredResources >= Resources)
        {
            Storage.StoredResources = Storage.StoredResources - Resources;
            return;
        }
    }
}

void UPlanetaryResourcesComponent::GenerateDepositByproducts(FResourceDeposit& Deposit)
{
    // Generate byproducts based on resource type
    switch (Deposit.ResourceType)
    {
        case EResourceType::Metals:
            Deposit.Byproducts.Add(EResourceType::RareElements);
            break;
        case EResourceType::Crystals:
            Deposit.Byproducts.Add(EResourceType::Energy);
            break;
        case EResourceType::Gas:
            Deposit.Byproducts.Add(EResourceType::Fuel);
            break;
        case EResourceType::Biomass:
            Deposit.Byproducts.Add(EResourceType::Food);
            break;
        default:
            break;
    }
}

TArray<EResourceType> UPlanetaryResourcesComponent::GetRequiredCatalysts(EResourceType InputResource, EResourceType OutputResource) const
{
    TArray<EResourceType> Catalysts;
    
    // Define catalyst requirements based on processing recipes
    if (InputResource == EResourceType::Biomass && OutputResource == EResourceType::Medicine)
    {
        Catalysts.Add(EResourceType::Water);
    }
    else if (InputResource == EResourceType::Metals && OutputResource == EResourceType::Electronics)
    {
        Catalysts.Add(EResourceType::Crystals);
    }
    
    return Catalysts;
}

void UPlanetaryResourcesComponent::AddResourceToProductionRate(FResourceAmount& ProductionRate, EResourceType ResourceType, float Rate) const
{
    switch (ResourceType)
    {
        case EResourceType::Metals: ProductionRate.Metals += Rate; break;
        case EResourceType::Crystals: ProductionRate.Crystals += Rate; break;
        case EResourceType::Gas: ProductionRate.Gas += Rate; break;
        case EResourceType::Biomass: ProductionRate.Biomass += Rate; break;
        case EResourceType::Water: ProductionRate.Water += Rate; break;
        case EResourceType::RareElements: ProductionRate.RareElements += Rate; break;
        case EResourceType::Energy: ProductionRate.Energy += Rate; break;
        case EResourceType::Food: ProductionRate.Food += Rate; break;
        case EResourceType::Medicine: ProductionRate.Medicine += Rate; break;
        case EResourceType::Electronics: ProductionRate.Electronics += Rate; break;
        case EResourceType::Fuel: ProductionRate.Fuel += Rate; break;
        default: break;
    }
}

void UPlanetaryResourcesComponent::SetResourceAmount(FResourceAmount& ResourceAmount, EResourceType ResourceType, float Amount) const
{
    switch (ResourceType)
    {
        case EResourceType::Metals: ResourceAmount.Metals = Amount; break;
        case EResourceType::Crystals: ResourceAmount.Crystals = Amount; break;
        case EResourceType::Gas: ResourceAmount.Gas = Amount; break;
        case EResourceType::Biomass: ResourceAmount.Biomass = Amount; break;
        case EResourceType::Water: ResourceAmount.Water = Amount; break;
        case EResourceType::RareElements: ResourceAmount.RareElements = Amount; break;
        case EResourceType::Energy: ResourceAmount.Energy = Amount; break;
        case EResourceType::Food: ResourceAmount.Food = Amount; break;
        case EResourceType::Medicine: ResourceAmount.Medicine = Amount; break;
        case EResourceType::Electronics: ResourceAmount.Electronics = Amount; break;
        case EResourceType::Fuel: ResourceAmount.Fuel = Amount; break;
        default: break;
    }
}

float UPlanetaryResourcesComponent::GetResourceAmount(const FResourceAmount& ResourceAmount, EResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EResourceType::Metals: return ResourceAmount.Metals;
        case EResourceType::Crystals: return ResourceAmount.Crystals;
        case EResourceType::Gas: return ResourceAmount.Gas;
        case EResourceType::Biomass: return ResourceAmount.Biomass;
        case EResourceType::Water: return ResourceAmount.Water;
        case EResourceType::RareElements: return ResourceAmount.RareElements;
        case EResourceType::Energy: return ResourceAmount.Energy;
        case EResourceType::Food: return ResourceAmount.Food;
        case EResourceType::Medicine: return ResourceAmount.Medicine;
        case EResourceType::Electronics: return ResourceAmount.Electronics;
        case EResourceType::Fuel: return ResourceAmount.Fuel;
        default: return 0.0f;
    }
}

float UPlanetaryResourcesComponent::CalculateResourceDemand(EResourceType ResourceType) const
{
    // Simplified demand calculation based on processing operations
    float Demand = 0.0f;
    
    for (const FResourceProcessing& Processing : ProcessingOperations)
    {
        if (Processing.InputResource == ResourceType && Processing.bIsActive)
        {
            Demand += Processing.ProcessingRate * Processing.Efficiency;
        }
    }
    
    return Demand;
}

float UPlanetaryResourcesComponent::CalculateAverageMiningEfficiency() const
{
    if (MiningOperations.Num() == 0) return 1.0f;
    
    float TotalEfficiency = 0.0f;
    for (const FMiningOperation& Operation : MiningOperations)
    {
        TotalEfficiency += Operation.Efficiency;
    }
    
    return TotalEfficiency / MiningOperations.Num();
}

float UPlanetaryResourcesComponent::CalculateAverageProcessingEfficiency() const
{
    if (ProcessingOperations.Num() == 0) return 1.0f;
    
    float TotalEfficiency = 0.0f;
    for (const FResourceProcessing& Processing : ProcessingOperations)
    {
        TotalEfficiency += Processing.Efficiency;
    }
    
    return TotalEfficiency / ProcessingOperations.Num();
}

float UPlanetaryResourcesComponent::CalculateAverageStorageEfficiency() const
{
    if (StorageFacilities.Num() == 0) return 1.0f;
    
    float TotalEfficiency = 0.0f;
    for (const FResourceStorage& Storage : StorageFacilities)
    {
        TotalEfficiency += Storage.StorageEfficiency;
    }
    
    return TotalEfficiency / StorageFacilities.Num();
}