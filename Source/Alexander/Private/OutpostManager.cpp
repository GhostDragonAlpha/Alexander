#include "OutpostManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

UOutpostManager::UOutpostManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    ResourceUpdateInterval = DEFAULT_RESOURCE_UPDATE_INTERVAL;
    PopulationUpdateInterval = DEFAULT_POPULATION_UPDATE_INTERVAL;
    DefenseUpdateInterval = DEFAULT_DEFENSE_UPDATE_INTERVAL;
    bEnableAutoExpansion = true;
    bEnableResourceTrading = true;
    bEnableFactionDynamics = true;
    MaxOutpostsPerPlanet = DEFAULT_MAX_OUTPOSTS_PER_PLANET;
}

void UOutpostManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeOutpostManager();

    // TODO: SetTimer API changed in UE5.6 - these functions require parameters and cannot be called directly
    // These periodic updates should be handled in TickComponent() instead using accumulated time deltas
    // if (GetWorld())
    // {
    //     GetWorld()->GetTimerManager().SetTimer(ResourceUpdateTimer, this, &UOutpostManager::UpdateAllOutposts, ResourceUpdateInterval, true);
    //     GetWorld()->GetTimerManager().SetTimer(PopulationUpdateTimer, this, &UOutpostManager::UpdateAllOutposts, PopulationUpdateInterval, true);
    //     GetWorld()->GetTimerManager().SetTimer(DefenseUpdateTimer, this, &UOutpostManager::UpdateAllOutposts, DefenseUpdateInterval, true);
    //     GetWorld()->GetTimerManager().SetTimer(ExpansionTimer, this, &UOutpostManager::ProcessOutpostExpansion, EXPANSION_CHECK_INTERVAL, true);
    // }
}

void UOutpostManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    HandleOutpostEvents();
    UpdateOutpostConnections();
}

void UOutpostManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // TODO: Timer cleanup commented out since timers are not being set in BeginPlay
    // Once timers are reimplemented (if needed), re-enable these ClearTimer calls
    // if (GetWorld())
    // {
    //     GetWorld()->GetTimerManager().ClearTimer(ResourceUpdateTimer);
    //     GetWorld()->GetTimerManager().ClearTimer(PopulationUpdateTimer);
    //     GetWorld()->GetTimerManager().ClearTimer(DefenseUpdateTimer);
    //     GetWorld()->GetTimerManager().ClearTimer(ExpansionTimer);
    // }

    Super::EndPlay(EndPlayReason);
}

void UOutpostManager::InitializeOutpostManager()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Outpost Manager"));

    // Initialize building templates
    InitializeBuildingTemplates();

    UE_LOG(LogTemp, Log, TEXT("Outpost Manager initialized with %d building templates"), BuildingTemplates.Num());
}

void UOutpostManager::CreateOutpost(const FOutpost& OutpostData)
{
    if (!ValidateOutpost(OutpostData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid outpost data for %s"), *OutpostData.Name);
        return;
    }

    FName OutpostID = GenerateOutpostID();
    FOutpost NewOutpost = OutpostData;
    NewOutpost.OutpostID = OutpostID;

    // Initialize outpost systems
    if (NewOutpost.Buildings.Num() == 0)
    {
        // Create basic command center
        FOutpostBuilding CommandCenter;
        CommandCenter.BuildingID = GenerateBuildingID();
        CommandCenter.BuildingType = EBuildingType::CommandCenter;
        CommandCenter.Location = NewOutpost.Location;
        CommandCenter.Rotation = FRotator::ZeroRotator;
        CommandCenter.Level = 1;
        CommandCenter.Health = 1000.0f;
        CommandCenter.MaxHealth = 1000.0f;
        CommandCenter.bIsOperational = true;
        CommandCenter.bIsPowered = true;
        CommandCenter.PowerConsumption = 50.0f;
        CommandCenter.ProductionRate = 1.0f;
        
        NewOutpost.Buildings.Add(CommandCenter);
    }

    // Initialize resources
    if (NewOutpost.Resources.StoredResources.Num() == 0)
    {
        NewOutpost.Resources.StoredResources.Add(TEXT("Power"), 100.0f);
        NewOutpost.Resources.StoredResources.Add(TEXT("Materials"), 500.0f);
        NewOutpost.Resources.StoredResources.Add(TEXT("Food"), 100.0f);
        NewOutpost.Resources.MaxStorage.Add(TEXT("Power"), 1000.0f);
        NewOutpost.Resources.MaxStorage.Add(TEXT("Materials"), 5000.0f);
        NewOutpost.Resources.MaxStorage.Add(TEXT("Food"), 1000.0f);
    }

    // Initialize population
    if (NewOutpost.Population.TotalPopulation == 0)
    {
        NewOutpost.Population.TotalPopulation = 10;
        NewOutpost.Population.MaxPopulation = 100;
        NewOutpost.Population.PopulationByProfession.Add(TEXT("Worker"), 8);
        NewOutpost.Population.PopulationByProfession.Add(TEXT("Engineer"), 2);
        NewOutpost.Population.Happiness = 0.7f;
        NewOutpost.Population.Health = 0.8f;
        NewOutpost.Population.Education = 0.5f;
        NewOutpost.Population.Productivity = 0.6f;
        NewOutpost.Population.GrowthRate = DEFAULT_POPULATION_GROWTH_RATE;
    }

    Outposts.Add(OutpostID, NewOutpost);

    OnOutpostCreated.Broadcast(OutpostID, NewOutpost);

    UE_LOG(LogTemp, Log, TEXT("Created outpost: %s (ID: %s) with %d buildings"), 
        *NewOutpost.Name, *OutpostID.ToString(), NewOutpost.Buildings.Num());
}

void UOutpostManager::DestroyOutpost(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost Outpost = Outposts[OutpostID];
    Outposts.Remove(OutpostID);

    OnOutpostDestroyed.Broadcast(OutpostID, Outpost);

    UE_LOG(LogTemp, Log, TEXT("Destroyed outpost: %s (ID: %s)"), *Outpost.Name, *OutpostID.ToString());
}

FOutpost* UOutpostManager::GetOutpost(const FName& OutpostID)
{
    return Outposts.Find(OutpostID);
}

TArray<FOutpost> UOutpostManager::GetAllOutposts()
{
    TArray<FOutpost> Result;
    for (const auto& OutpostPair : Outposts)
    {
        Result.Add(OutpostPair.Value);
    }
    return Result;
}

TArray<FOutpost> UOutpostManager::GetOutpostsByType(EOutpostType Type)
{
    TArray<FOutpost> Result;
    for (const auto& OutpostPair : Outposts)
    {
        if (OutpostPair.Value.OutpostType == Type)
        {
            Result.Add(OutpostPair.Value);
        }
    }
    return Result;
}

TArray<FOutpost> UOutpostManager::GetOutpostsByPlanet(class APlanet* Planet)
{
    TArray<FOutpost> Result;
    for (const auto& OutpostPair : Outposts)
    {
        if (OutpostPair.Value.ParentPlanet == Planet)
        {
            Result.Add(OutpostPair.Value);
        }
    }
    return Result;
}

bool UOutpostManager::CanBuildStructure(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Check if outpost can support this building
    if (!BuildingTemplates.Contains(BuildingType))
    {
        return false;
    }

    const FOutpostBuilding& Template = BuildingTemplates[BuildingType];
    
    // Check resources
    for (const auto& ResourceCost : Template.UpgradeCosts)
    {
        if (!HasResource(OutpostID, ResourceCost.Key, ResourceCost.Value))
        {
            return false;
        }
    }

    // Check power
    if (Outpost.Resources.PowerGeneration < Outpost.Resources.PowerConsumption + Template.PowerConsumption)
    {
        return false;
    }

    // Check placement
    return ValidateBuildingPlacement(OutpostID, BuildingType, Location);
}

bool UOutpostManager::BuildStructure(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location)
{
    if (!CanBuildStructure(OutpostID, BuildingType, Location))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    const FOutpostBuilding& Template = BuildingTemplates[BuildingType];

    // Consume resources
    for (const auto& ResourceCost : Template.UpgradeCosts)
    {
        ConsumeResource(OutpostID, ResourceCost.Key, ResourceCost.Value);
    }

    // Create building
    FOutpostBuilding NewBuilding = Template;
    NewBuilding.BuildingID = GenerateBuildingID();
    NewBuilding.Location = Location;
    NewBuilding.Level = 1;
    NewBuilding.bIsOperational = true;
    NewBuilding.bIsPowered = true;

    Outpost.Buildings.Add(NewBuilding);
    Outpost.Resources.PowerConsumption += NewBuilding.PowerConsumption;

    OnBuildingConstructed.Broadcast(OutpostID, NewBuilding.BuildingID, BuildingType);

    UE_LOG(LogTemp, Log, TEXT("Built structure %s at outpost %s"), 
        *NewBuilding.BuildingID.ToString(), *OutpostID.ToString());

    return true;
}

bool UOutpostManager::UpgradeBuilding(const FName& OutpostID, const FName& BuildingID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    
    // Find building
    FOutpostBuilding* Building = nullptr;
    for (FOutpostBuilding& B : Outpost.Buildings)
    {
        if (B.BuildingID == BuildingID)
        {
            Building = &B;
            break;
        }
    }

    if (!Building || Building->Level >= 5) // Max level 5
    {
        return false;
    }

    // Check upgrade costs
    int32 UpgradeCost = Building->Level * 100;
    if (!HasResource(OutpostID, TEXT("Materials"), UpgradeCost))
    {
        return false;
    }

    // Perform upgrade
    ConsumeResource(OutpostID, TEXT("Materials"), UpgradeCost);
    Building->Level++;
    Building->MaxHealth *= 1.2f;
    Building->Health = Building->MaxHealth;
    Building->ProductionRate *= 1.3f;
    Building->PowerConsumption *= 1.1f;

    UE_LOG(LogTemp, Log, TEXT("Upgraded building %s to level %d at outpost %s"), 
        *BuildingID.ToString(), Building->Level, *OutpostID.ToString());

    return true;
}

bool UOutpostManager::DestroyBuilding(const FName& OutpostID, const FName& BuildingID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    
    // Find and remove building
    for (int32 i = 0; i < Outpost.Buildings.Num(); i++)
    {
        if (Outpost.Buildings[i].BuildingID == BuildingID)
        {
            Outpost.Resources.PowerConsumption -= Outpost.Buildings[i].PowerConsumption;
            Outpost.Buildings.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Destroyed building %s at outpost %s"), 
                *BuildingID.ToString(), *OutpostID.ToString());
            return true;
        }
    }

    return false;
}

FOutpostBuilding* UOutpostManager::GetBuilding(const FName& OutpostID, const FName& BuildingID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return nullptr;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    for (FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.BuildingID == BuildingID)
        {
            return &Building;
        }
    }

    return nullptr;
}

TArray<FOutpostBuilding> UOutpostManager::GetOutpostBuildings(const FName& OutpostID)
{
    if (Outposts.Contains(OutpostID))
    {
        return Outposts[OutpostID].Buildings;
    }
    return TArray<FOutpostBuilding>();
}

bool UOutpostManager::IsBuildingOperational(const FName& OutpostID, const FName& BuildingID)
{
    FOutpostBuilding* Building = GetBuilding(OutpostID, BuildingID);
    return Building && Building->bIsOperational && Building->bIsPowered;
}

void UOutpostManager::UpdateOutpostResources(const FName& OutpostID, float DeltaTime)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Calculate production and consumption
    CalculateResourceProduction(OutpostID);
    CalculateResourceConsumption(OutpostID);

    // Process resource changes
    for (const auto& ProductionPair : Outpost.Resources.ProductionRates)
    {
        AddResource(OutpostID, ProductionPair.Key, ProductionPair.Value * DeltaTime);
    }

    for (const auto& ConsumptionPair : Outpost.Resources.ConsumptionRates)
    {
        ConsumeResource(OutpostID, ConsumptionPair.Key, ConsumptionPair.Value * DeltaTime);
    }

    // Process storage
    ProcessResourceStorage(OutpostID);

    // Handle shortages
    HandleResourceShortages(OutpostID);
}

float UOutpostManager::GetResourceAmount(const FName& OutpostID, const FName& ResourceType)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    if (Outpost.Resources.StoredResources.Contains(ResourceType))
    {
        return Outpost.Resources.StoredResources[ResourceType];
    }
    return 0.0f;
}

bool UOutpostManager::HasResource(const FName& OutpostID, const FName& ResourceType, float Amount)
{
    return GetResourceAmount(OutpostID, ResourceType) >= Amount;
}

bool UOutpostManager::ConsumeResource(const FName& OutpostID, const FName& ResourceType, float Amount)
{
    if (!HasResource(OutpostID, ResourceType, Amount))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    float CurrentAmount = Outpost.Resources.StoredResources[ResourceType];
    Outpost.Resources.StoredResources.Add(ResourceType, CurrentAmount - Amount);
    return true;
}

void UOutpostManager::AddResource(const FName& OutpostID, const FName& ResourceType, float Amount)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    float CurrentAmount = Outpost.Resources.StoredResources.Contains(ResourceType) ? 
        Outpost.Resources.StoredResources[ResourceType] : 0.0f;
    float MaxAmount = Outpost.Resources.MaxStorage.Contains(ResourceType) ? 
        Outpost.Resources.MaxStorage[ResourceType] : 1000.0f;
    
    float NewAmount = FMath::Min(CurrentAmount + Amount, MaxAmount);
    Outpost.Resources.StoredResources.Add(ResourceType, NewAmount);
}

TMap<FName, float> UOutpostManager::GetAllResources(const FName& OutpostID)
{
    if (Outposts.Contains(OutpostID))
    {
        return Outposts[OutpostID].Resources.StoredResources;
    }
    return TMap<FName, float>();
}

float UOutpostManager::GetPowerLevel(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    if (Outpost.Resources.PowerConsumption > 0.0f)
    {
        return Outpost.Resources.PowerGeneration / Outpost.Resources.PowerConsumption;
    }
    return 1.0f; // No consumption means full power
}

bool UOutpostManager::HasSufficientPower(const FName& OutpostID)
{
    return GetPowerLevel(OutpostID) >= 1.0f;
}

void UOutpostManager::UpdateOutpostPopulation(const FName& OutpostID, float DeltaTime)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Calculate population growth
    CalculatePopulationGrowth(OutpostID);

    // Update population needs
    UpdatePopulationNeeds(OutpostID);

    // Process migration
    ProcessPopulationMigration(OutpostID);

    // Calculate happiness
    CalculatePopulationHappiness(OutpostID);

    // Apply growth
    if (Outpost.Population.GrowthRate != 0.0f)
    {
        int32 PopulationChange = FMath::FloorToInt(Outpost.Population.TotalPopulation * Outpost.Population.GrowthRate * DeltaTime);
        if (PopulationChange != 0)
        {
            Outpost.Population.TotalPopulation += PopulationChange;
            Outpost.Population.TotalPopulation = FMath::Clamp(Outpost.Population.TotalPopulation, 0, Outpost.Population.MaxPopulation);
            OnPopulationChanged.Broadcast(OutpostID, Outpost.Population.TotalPopulation);
        }
    }
}

bool UOutpostManager::CanAcceptPopulation(const FName& OutpostID, int32 Amount)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    return Outpost.Population.TotalPopulation + Amount <= Outpost.Population.MaxPopulation;
}

void UOutpostManager::AddPopulation(const FName& OutpostID, int32 Amount, const FName& Profession)
{
    if (!CanAcceptPopulation(OutpostID, Amount) || Amount <= 0)
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Population.TotalPopulation += Amount;

    if (!Profession.IsNone())
    {
        int32 CurrentProfession = Outpost.Population.PopulationByProfession.Contains(Profession) ? 
            Outpost.Population.PopulationByProfession[Profession] : 0;
        Outpost.Population.PopulationByProfession.Add(Profession, CurrentProfession + Amount);
    }

    OnPopulationChanged.Broadcast(OutpostID, Outpost.Population.TotalPopulation);
}

void UOutpostManager::RemovePopulation(const FName& OutpostID, int32 Amount)
{
    if (!Outposts.Contains(OutpostID) || Amount <= 0)
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    int32 ActualAmount = FMath::Min(Amount, Outpost.Population.TotalPopulation);
    Outpost.Population.TotalPopulation -= ActualAmount;

    // Remove from professions proportionally
    for (auto& ProfessionPair : Outpost.Population.PopulationByProfession)
    {
        float Proportion = (float)ProfessionPair.Value / (Outpost.Population.TotalPopulation + ActualAmount);
        int32 RemoveFromProfession = FMath::FloorToInt(ActualAmount * Proportion);
        ProfessionPair.Value = FMath::Max(0, ProfessionPair.Value - RemoveFromProfession);
    }

    OnPopulationChanged.Broadcast(OutpostID, Outpost.Population.TotalPopulation);
}

float UOutpostManager::GetPopulationHappiness(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }
    return Outposts[OutpostID].Population.Happiness;
}

void UOutpostManager::SetPopulationHappiness(const FName& OutpostID, float Happiness)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Population.Happiness = FMath::Clamp(Happiness, 0.0f, 1.0f);
}

TMap<FName, int32> UOutpostManager::GetPopulationByProfession(const FName& OutpostID)
{
    if (Outposts.Contains(OutpostID))
    {
        return Outposts[OutpostID].Population.PopulationByProfession;
    }
    return TMap<FName, int32>();
}

void UOutpostManager::UpdateOutpostDefense(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Update defense systems
    UpdateDefenseSystems(OutpostID);

    // Process threat detection
    ProcessThreatDetection(OutpostID);

    // Handle repairs
    HandleDefenseRepairs(OutpostID);
}

float UOutpostManager::GetDefenseValue(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }
    return Outposts[OutpostID].Defense.TotalDefenseValue;
}

bool UOutpostManager::IsUnderAttack(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }
    return Outposts[OutpostID].Defense.AlertLevel > 0.5f;
}

void UOutpostManager::HandleAttack(const FName& OutpostID, float DamageAmount)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Apply damage to shields first
    if (Outpost.Defense.bShieldsOnline && Outpost.Defense.ShieldStrength > 0.0f)
    {
        float ShieldDamage = FMath::Min(DamageAmount, Outpost.Defense.ShieldStrength);
        Outpost.Defense.ShieldStrength -= ShieldDamage;
        DamageAmount -= ShieldDamage;

        if (Outpost.Defense.ShieldStrength <= 0.0f)
        {
            Outpost.Defense.bShieldsOnline = false;
        }
    }

    // Apply remaining damage to buildings
    if (DamageAmount > 0.0f)
    {
        for (FOutpostBuilding& Building : Outpost.Buildings)
        {
            if (Building.Health > 0.0f)
            {
                float BuildingDamage = FMath::Min(DamageAmount, Building.Health);
                Building.Health -= BuildingDamage;
                DamageAmount -= BuildingDamage;

                if (Building.Health <= 0.0f)
                {
                    Building.bIsOperational = false;
                }

                if (DamageAmount <= 0.0f)
                {
                    break;
                }
            }
        }
    }

    // Set alert level
    Outpost.Defense.AlertLevel = FMath::Min(1.0f, Outpost.Defense.AlertLevel + 0.2f);

    OnOutpostAttacked.Broadcast(OutpostID, DamageAmount);
}

void UOutpostManager::RepairDefenses(const FName& OutpostID, float RepairAmount)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Repair shields
    if (Outpost.Defense.ShieldStrength < Outpost.Defense.MaxShieldStrength)
    {
        float ShieldRepair = FMath::Min(RepairAmount, Outpost.Defense.MaxShieldStrength - Outpost.Defense.ShieldStrength);
        Outpost.Defense.ShieldStrength += ShieldRepair;
        RepairAmount -= ShieldRepair;
        Outpost.Defense.bShieldsOnline = true;
    }

    // Repair buildings
    if (RepairAmount > 0.0f)
    {
        for (FOutpostBuilding& Building : Outpost.Buildings)
        {
            if (Building.Health < Building.MaxHealth)
            {
                float BuildingRepair = FMath::Min(RepairAmount, Building.MaxHealth - Building.Health);
                Building.Health += BuildingRepair;
                RepairAmount -= BuildingRepair;

                if (Building.Health >= Building.MaxHealth)
                {
                    Building.bIsOperational = true;
                }

                if (RepairAmount <= 0.0f)
                {
                    break;
                }
            }
        }
    }
}

void UOutpostManager::ActivateShields(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    if (Outpost.Defense.ShieldStrength > 0.0f)
    {
        Outpost.Defense.bShieldsOnline = true;
    }
}

void UOutpostManager::DeactivateShields(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Defense.bShieldsOnline = false;
}

bool UOutpostManager::CanUpgradeOutpost(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    
    // Check population requirement
    if (Outpost.Population.TotalPopulation < Outpost.Population.MaxPopulation * 0.8f)
    {
        return false;
    }

    // Check building requirement
    if (Outpost.Buildings.Num() < 5)
    {
        return false;
    }

    // Check resource requirement
    if (!HasResource(OutpostID, TEXT("Materials"), 1000))
    {
        return false;
    }

    return true;
}

bool UOutpostManager::UpgradeOutpost(const FName& OutpostID)
{
    if (!CanUpgradeOutpost(OutpostID))
    {
        return false;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Consume resources
    ConsumeResource(OutpostID, TEXT("Materials"), 1000);

    // Upgrade outpost
    switch (Outpost.Size)
    {
        case EOutpostSize::Outpost:
            Outpost.Size = EOutpostSize::Settlement;
            Outpost.Population.MaxPopulation = 500;
            break;
        case EOutpostSize::Settlement:
            Outpost.Size = EOutpostSize::Colony;
            Outpost.Population.MaxPopulation = 2000;
            break;
        case EOutpostSize::Colony:
            Outpost.Size = EOutpostSize::City;
            Outpost.Population.MaxPopulation = 10000;
            break;
        case EOutpostSize::City:
            Outpost.Size = EOutpostSize::Metropolis;
            Outpost.Population.MaxPopulation = 50000;
            break;
        case EOutpostSize::Metropolis:
            return false; // Already max size
    }

    OnOutpostUpgraded.Broadcast(OutpostID, Outpost.Size);

    UE_LOG(LogTemp, Log, TEXT("Upgraded outpost %s to size %d"), *OutpostID.ToString(), (int32)Outpost.Size);
    return true;
}

EOutpostSize UOutpostManager::GetOutpostSize(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return EOutpostSize::Outpost;
    }
    return Outposts[OutpostID].Size;
}

void UOutpostManager::SetOutpostStatus(const FName& OutpostID, EOutpostStatus NewStatus)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Status = NewStatus;

    UE_LOG(LogTemp, Log, TEXT("Set outpost %s status to %d"), *OutpostID.ToString(), (int32)NewStatus);
}

EOutpostStatus UOutpostManager::GetOutpostStatus(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return EOutpostStatus::Offline;
    }
    return Outposts[OutpostID].Status;
}

void UOutpostManager::ProcessOutpostTrade(const FName& OutpostID, const FName& Resource, float Amount, bool bExport)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    if (bExport)
    {
        if (ConsumeResource(OutpostID, Resource, Amount))
        {
            Outpost.Wealth += Amount * 10; // Simple trade value
        }
    }
    else
    {
        AddResource(OutpostID, Resource, Amount);
        Outpost.Wealth -= Amount * 10;
    }
}

TMap<FName, float> UOutpostManager::GetOutpostPrices(const FName& OutpostID)
{
    TMap<FName, float> Prices;
    
    if (!Outposts.Contains(OutpostID))
    {
        return Prices;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    
    // Calculate prices based on supply/demand
    for (const auto& ResourcePair : Outpost.Resources.StoredResources)
    {
        float BasePrice = 100.0f;
        float Supply = ResourcePair.Value;
        float Demand = Outpost.Resources.ConsumptionRates.Contains(ResourcePair.Key) ? 
            Outpost.Resources.ConsumptionRates[ResourcePair.Key] : 0.0f;
        
        float PriceModifier = 1.0f;
        if (Supply > 0.0f)
        {
            PriceModifier = Demand / Supply;
        }
        
        Prices.Add(ResourcePair.Key, BasePrice * PriceModifier);
    }

    return Prices;
}

float UOutpostManager::GetOutpostWealth(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }
    return Outposts[OutpostID].Wealth;
}

void UOutpostManager::UpdateOutpostWealth(const FName& OutpostID, float Change)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Wealth = FMath::Max(0.0f, Outpost.Wealth + Change);
}

float UOutpostManager::GetFactionInfluence(const FName& OutpostID, const FName& Faction)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    return Outpost.FactionInfluence.Contains(Faction) ? Outpost.FactionInfluence[Faction] : 0.0f;
}

void UOutpostManager::UpdateFactionInfluence(const FName& OutpostID, const FName& Faction, float Change)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    float CurrentInfluence = Outpost.FactionInfluence.Contains(Faction) ? Outpost.FactionInfluence[Faction] : 0.0f;
    Outpost.FactionInfluence.Add(Faction, FMath::Max(0.0f, CurrentInfluence + Change));
}

TArray<FName> UOutpostManager::GetControllingFactions(const FName& OutpostID)
{
    TArray<FName> ControllingFactions;
    
    if (!Outposts.Contains(OutpostID))
    {
        return ControllingFactions;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    float TotalInfluence = 0.0f;

    // Calculate total influence
    for (const auto& FactionPair : Outpost.FactionInfluence)
    {
        TotalInfluence += FactionPair.Value;
    }

    if (TotalInfluence <= 0.0f)
    {
        return ControllingFactions;
    }

    // Add factions with significant influence (>25%)
    for (const auto& FactionPair : Outpost.FactionInfluence)
    {
        float InfluencePercentage = (FactionPair.Value / TotalInfluence) * 100.0f;
        if (InfluencePercentage > 25.0f)
        {
            ControllingFactions.Add(FactionPair.Key);
        }
    }

    return ControllingFactions;
}

bool UOutpostManager::IsFactionWelcome(const FName& OutpostID, const FName& Faction)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    float Influence = GetFactionInfluence(OutpostID, Faction);
    return Influence >= 10.0f; // Welcome if influence >= 10
}

bool UOutpostManager::ConnectOutposts(const FName& OutpostID1, const FName& OutpostID2)
{
    if (!Outposts.Contains(OutpostID1) || !Outposts.Contains(OutpostID2))
    {
        return false;
    }

    if (AreOutpostsConnected(OutpostID1, OutpostID2))
    {
        return false;
    }

    FOutpost& Outpost1 = Outposts[OutpostID1];
    FOutpost& Outpost2 = Outposts[OutpostID2];

    Outpost1.ConnectedOutposts.AddUnique(OutpostID2);
    Outpost2.ConnectedOutposts.AddUnique(OutpostID1);

    UE_LOG(LogTemp, Log, TEXT("Connected outposts %s and %s"), *OutpostID1.ToString(), *OutpostID2.ToString());
    return true;
}

bool UOutpostManager::DisconnectOutposts(const FName& OutpostID1, const FName& OutpostID2)
{
    if (!Outposts.Contains(OutpostID1) || !Outposts.Contains(OutpostID2))
    {
        return false;
    }

    FOutpost& Outpost1 = Outposts[OutpostID1];
    FOutpost& Outpost2 = Outposts[OutpostID2];

    bool bDisconnected1 = Outpost1.ConnectedOutposts.Remove(OutpostID2) > 0;
    bool bDisconnected2 = Outpost2.ConnectedOutposts.Remove(OutpostID1) > 0;

    if (bDisconnected1 || bDisconnected2)
    {
        UE_LOG(LogTemp, Log, TEXT("Disconnected outposts %s and %s"), *OutpostID1.ToString(), *OutpostID2.ToString());
        return true;
    }

    return false;
}

bool UOutpostManager::AreOutpostsConnected(const FName& OutpostID1, const FName& OutpostID2)
{
    if (!Outposts.Contains(OutpostID1))
    {
        return false;
    }

    const FOutpost& Outpost1 = Outposts[OutpostID1];
    return Outpost1.ConnectedOutposts.Contains(OutpostID2);
}

TArray<FName> UOutpostManager::GetConnectedOutposts(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return TArray<FName>();
    }

    return Outposts[OutpostID].ConnectedOutposts;
}

float UOutpostManager::GetDistanceToOutpost(const FName& OutpostID, const FVector& Location)
{
    if (!Outposts.Contains(OutpostID))
    {
        return -1.0f;
    }

    return FVector::Dist(Location, Outposts[OutpostID].Location);
}

FName UOutpostManager::GetNearestOutpost(const FVector& Location)
{
    FName NearestOutpost;
    float NearestDistance = FLT_MAX;

    for (const auto& OutpostPair : Outposts)
    {
        float Distance = FVector::Dist(Location, OutpostPair.Value.Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestOutpost = OutpostPair.Key;
        }
    }

    return NearestOutpost;
}

TArray<FName> UOutpostManager::GetOutpostsInRange(const FVector& Location, float Range)
{
    TArray<FName> OutpostsInRange;

    for (const auto& OutpostPair : Outposts)
    {
        float Distance = FVector::Dist(Location, OutpostPair.Value.Location);
        if (Distance <= Range)
        {
            OutpostsInRange.Add(OutpostPair.Key);
        }
    }

    return OutpostsInRange;
}

bool UOutpostManager::IsLocationInOutpost(const FName& OutpostID, const FVector& Location)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    float OutpostRadius = 1000.0f; // Default radius, could be stored in outpost data
    
    return FVector::Dist(Location, Outpost.Location) <= OutpostRadius;
}

void UOutpostManager::GenerateOutpostReport(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    const FOutpost& Outpost = Outposts[OutpostID];

    UE_LOG(LogTemp, Log, TEXT("=== OUTPOST REPORT: %s ==="), *Outpost.Name);
    UE_LOG(LogTemp, Log, TEXT("Type: %d, Size: %d, Status: %d"), (int32)Outpost.OutpostType, (int32)Outpost.Size, (int32)Outpost.Status);
    UE_LOG(LogTemp, Log, TEXT("Population: %d/%d (Happiness: %.1f%%)"), 
        Outpost.Population.TotalPopulation, Outpost.Population.MaxPopulation, Outpost.Population.Happiness * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Buildings: %d, Defense: %.1f, Wealth: %.0f"), 
        Outpost.Buildings.Num(), Outpost.Defense.TotalDefenseValue, Outpost.Wealth);
    UE_LOG(LogTemp, Log, TEXT("Power: %.0f/%.0f (%.1f%%)"), 
        Outpost.Resources.PowerGeneration, Outpost.Resources.PowerConsumption, GetPowerLevel(OutpostID) * 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Resources:"));
    for (const auto& ResourcePair : Outpost.Resources.StoredResources)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %.1f"), *ResourcePair.Key.ToString(), ResourcePair.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("==============================="));
}

void UOutpostManager::UpdateAllOutposts(float DeltaTime)
{
    for (auto& OutpostPair : Outposts)
    {
        const FName& OutpostID = OutpostPair.Key;
        FOutpost& Outpost = OutpostPair.Value;

        if (Outpost.Status == EOutpostStatus::Active)
        {
            UpdateOutpostResources(OutpostID, DeltaTime);
            UpdateOutpostPopulation(OutpostID, DeltaTime);
            UpdateOutpostDefense(OutpostID);

            // Update building production
            for (FOutpostBuilding& Building : Outpost.Buildings)
            {
                if (Building.bIsOperational && Building.bIsPowered)
                {
                    ProcessBuildingProduction(OutpostID, Building, DeltaTime);
                }
            }
        }
    }
}

void UOutpostManager::ProcessOutpostExpansion()
{
    if (!bEnableAutoExpansion)
    {
        return;
    }

    for (auto& OutpostPair : Outposts)
    {
        const FName& OutpostID = OutpostPair.Key;
        FOutpost& Outpost = OutpostPair.Value;

        // Check if outpost can expand
        if (CanUpgradeOutpost(OutpostID) && FMath::FRand() < 0.1f) // 10% chance per check
        {
            UpgradeOutpost(OutpostID);
        }

        // Check if new buildings can be constructed
        if (Outpost.Buildings.Num() < 20 && HasResource(OutpostID, TEXT("Materials"), 500))
        {
            EBuildingType BuildingTypes[] = {
                EBuildingType::Habitat, EBuildingType::PowerPlant, EBuildingType::Storage,
                EBuildingType::Factory, EBuildingType::Laboratory
            };
            
            EBuildingType RandomType = BuildingTypes[FMath::RandRange(0, 4)];
            FVector RandomLocation = Outpost.Location + FVector(FMath::RandRange(-500.0f, 500.0f), 
                FMath::RandRange(-500.0f, 500.0f), 0.0f);
            
            if (CanBuildStructure(OutpostID, RandomType, RandomLocation))
            {
                BuildStructure(OutpostID, RandomType, RandomLocation);
            }
        }
    }
}

void UOutpostManager::HandleOutpostEvents()
{
    // Process random events for outposts
    for (auto& OutpostPair : Outposts)
    {
        const FName& OutpostID = OutpostPair.Key;
        FOutpost& Outpost = OutpostPair.Value;

        if (Outpost.Status != EOutpostStatus::Active)
        {
            continue;
        }

        // Random events
        if (FMath::FRand() < 0.001f) // 0.1% chance per tick
        {
            int32 EventType = FMath::RandRange(0, 4);
            switch (EventType)
            {
                case 0: // Trade opportunity
                    UpdateOutpostWealth(OutpostID, FMath::RandRange(100, 500));
                    break;
                case 1: // Population boom
                    AddPopulation(OutpostID, FMath::RandRange(5, 20));
                    break;
                case 2: // Resource discovery
                    AddResource(OutpostID, TEXT("Materials"), FMath::RandRange(200, 1000));
                    break;
                case 3: // Attack
                    HandleAttack(OutpostID, FMath::RandRange(50, 200));
                    break;
                case 4: // Technology boost
                    for (FOutpostBuilding& Building : Outpost.Buildings)
                    {
                        Building.ProductionRate *= 1.1f;
                    }
                    break;
            }
        }
    }
}

void UOutpostManager::UpdateOutpostConnections()
{
    // Process trade between connected outposts
    if (!bEnableResourceTrading)
    {
        return;
    }

    for (auto& OutpostPair : Outposts)
    {
        const FName& OutpostID = OutpostPair.Key;
        FOutpost& Outpost = OutpostPair.Value;

        for (const FName& ConnectedID : Outpost.ConnectedOutposts)
        {
            if (!Outposts.Contains(ConnectedID))
            {
                continue;
            }

            FOutpost& ConnectedOutpost = Outposts[ConnectedID];

            // Simple resource exchange
            if (FMath::FRand() < 0.01f) // 1% chance per tick
            {
                FName TradeResource = TEXT("Materials");
                float TradeAmount = 10.0f;

                if (HasResource(OutpostID, TradeResource, TradeAmount))
                {
                    ConsumeResource(OutpostID, TradeResource, TradeAmount);
                    AddResource(ConnectedID, TradeResource, TradeAmount);
                }
            }
        }
    }
}

void UOutpostManager::InitializeBuildingTemplates()
{
    // Habitat
    FOutpostBuilding Habitat;
    Habitat.BuildingType = EBuildingType::Habitat;
    Habitat.Level = 1;
    Habitat.Health = 500.0f;
    Habitat.MaxHealth = 500.0f;
    Habitat.PowerConsumption = 20.0f;
    Habitat.ProductionRate = 0.0f;
    Habitat.UpgradeCosts.Add(TEXT("Materials"), 200);
    BuildingTemplates.Add(EBuildingType::Habitat, Habitat);

    // Power Plant
    FOutpostBuilding PowerPlant;
    PowerPlant.BuildingType = EBuildingType::PowerPlant;
    PowerPlant.Level = 1;
    PowerPlant.Health = 750.0f;
    PowerPlant.MaxHealth = 750.0f;
    PowerPlant.PowerConsumption = -100.0f; // Generates power
    PowerPlant.ProductionRate = 0.0f;
    PowerPlant.UpgradeCosts.Add(TEXT("Materials"), 300);
    BuildingTemplates.Add(EBuildingType::PowerPlant, PowerPlant);

    // Mine
    FOutpostBuilding Mine;
    Mine.BuildingType = EBuildingType::Mine;
    Mine.Level = 1;
    Mine.Health = 600.0f;
    Mine.MaxHealth = 600.0f;
    Mine.PowerConsumption = 50.0f;
    Mine.ProductionRate = 10.0f;
    Mine.Production.Add(TEXT("Materials"), 10.0f);
    Mine.UpgradeCosts.Add(TEXT("Materials"), 400);
    BuildingTemplates.Add(EBuildingType::Mine, Mine);

    // Factory
    FOutpostBuilding Factory;
    Factory.BuildingType = EBuildingType::Factory;
    Factory.Level = 1;
    Factory.Health = 800.0f;
    Factory.MaxHealth = 800.0f;
    Factory.PowerConsumption = 80.0f;
    Factory.ProductionRate = 5.0f;
    Factory.Production.Add(TEXT("Components"), 5.0f);
    // TODO: FOutpostBuilding does not have ConsumptionRates property - needs to be added to struct or handled differently
    // Factory.ConsumptionRates.Add(TEXT("Materials"), 8.0f);
    Factory.UpgradeCosts.Add(TEXT("Materials"), 500);
    BuildingTemplates.Add(EBuildingType::Factory, Factory);

    // Laboratory
    FOutpostBuilding Laboratory;
    Laboratory.BuildingType = EBuildingType::Laboratory;
    Laboratory.Level = 1;
    Laboratory.Health = 400.0f;
    Laboratory.MaxHealth = 400.0f;
    Laboratory.PowerConsumption = 30.0f;
    Laboratory.ProductionRate = 2.0f;
    Laboratory.Production.Add(TEXT("Research"), 2.0f);
    Laboratory.UpgradeCosts.Add(TEXT("Materials"), 600);
    BuildingTemplates.Add(EBuildingType::Laboratory, Laboratory);

    // Defense Turret
    FOutpostBuilding DefenseTurret;
    DefenseTurret.BuildingType = EBuildingType::DefenseTurret;
    DefenseTurret.Level = 1;
    DefenseTurret.Health = 300.0f;
    DefenseTurret.MaxHealth = 300.0f;
    DefenseTurret.PowerConsumption = 15.0f;
    DefenseTurret.ProductionRate = 0.0f;
    DefenseTurret.UpgradeCosts.Add(TEXT("Materials"), 250);
    BuildingTemplates.Add(EBuildingType::DefenseTurret, DefenseTurret);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d building templates"), BuildingTemplates.Num());
}

bool UOutpostManager::ValidateBuildingPlacement(const FName& OutpostID, EBuildingType BuildingType, const FVector& Location)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];

    // Check distance from outpost center
    float DistanceFromCenter = FVector::Dist(Location, Outpost.Location);
    if (DistanceFromCenter > 2000.0f) // Max building distance
    {
        return false;
    }

    // Check collision with existing buildings
    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        float DistanceFromBuilding = FVector::Dist(Location, Building.Location);
        if (DistanceFromBuilding < 200.0f) // Min building spacing
        {
            return false;
        }
    }

    return true;
}

void UOutpostManager::ProcessBuildingProduction(const FName& OutpostID, FOutpostBuilding& Building, float DeltaTime)
{
    // Process building production
    for (const auto& ProductionPair : Building.Production)
    {
        AddResource(OutpostID, ProductionPair.Key, ProductionPair.Value * Building.ProductionRate * DeltaTime);
    }

    // TODO: FOutpostBuilding does not have ConsumptionRates property - needs to be added to struct
    // Process building consumption would go here once ConsumptionRates property is added
    /*
    for (const auto& ConsumptionPair : Building.ConsumptionRates)
    {
        if (!ConsumeResource(OutpostID, ConsumptionPair.Key, ConsumptionPair.Value * Building.ProductionRate * DeltaTime))
        {
            // Not enough resources, reduce efficiency
            Building.bIsOperational = false;
        }
        else
        {
            Building.bIsOperational = true;
        }
    }
    */
}

void UOutpostManager::UpdateBuildingPower(const FName& OutpostID, FOutpostBuilding& Building)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    Building.bIsPowered = Outpost.Resources.PowerGeneration >= Outpost.Resources.PowerConsumption;
}

void UOutpostManager::CalculateResourceProduction(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Resources.ProductionRates.Empty();
    Outpost.Resources.PowerGeneration = 0.0f;

    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.bIsOperational && Building.bIsPowered)
        {
            // Add production
            for (const auto& ProductionPair : Building.Production)
            {
                float CurrentRate = Outpost.Resources.ProductionRates.Contains(ProductionPair.Key) ? 
                    Outpost.Resources.ProductionRates[ProductionPair.Key] : 0.0f;
                Outpost.Resources.ProductionRates.Add(ProductionPair.Key, 
                    CurrentRate + ProductionPair.Value * Building.ProductionRate);
            }

            // Calculate power generation
            if (Building.PowerConsumption < 0.0f)
            {
                Outpost.Resources.PowerGeneration += -Building.PowerConsumption;
            }
        }
    }
}

void UOutpostManager::CalculateResourceConsumption(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Resources.ConsumptionRates.Empty();
    Outpost.Resources.PowerConsumption = 0.0f;

    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.bIsOperational)
        {
            // TODO: FOutpostBuilding does not have ConsumptionRates property - needs to be added to struct
            // Building consumption calculation would go here once ConsumptionRates property is added
            /*
            // Add consumption
            for (const auto& ConsumptionPair : Building.ConsumptionRates)
            {
                float CurrentRate = Outpost.Resources.ConsumptionRates.Contains(ConsumptionPair.Key) ?
                    Outpost.Resources.ConsumptionRates[ConsumptionPair.Key] : 0.0f;
                Outpost.Resources.ConsumptionRates.Add(ConsumptionPair.Key,
                    CurrentRate + ConsumptionPair.Value * Building.ProductionRate);
            }
            */

            // Calculate power consumption
            if (Building.PowerConsumption > 0.0f)
            {
                Outpost.Resources.PowerConsumption += Building.PowerConsumption;
            }
        }
    }

    // Population consumption
    Outpost.Resources.ConsumptionRates.Add(TEXT("Food"), Outpost.Population.TotalPopulation * 0.1f);
    Outpost.Resources.ConsumptionRates.Add(TEXT("Water"), Outpost.Population.TotalPopulation * 0.2f);
}

void UOutpostManager::ProcessResourceStorage(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Clamp resources to storage limits
    for (auto& ResourcePair : Outpost.Resources.StoredResources)
    {
        if (Outpost.Resources.MaxStorage.Contains(ResourcePair.Key))
        {
            ResourcePair.Value = FMath::Min(ResourcePair.Value, Outpost.Resources.MaxStorage[ResourcePair.Key]);
        }
    }
}

void UOutpostManager::HandleResourceShortages(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Check for critical shortages
    bool bFoodShortage = GetResourceAmount(OutpostID, TEXT("Food")) < Outpost.Population.TotalPopulation * 0.5f;
    bool bPowerShortage = GetPowerLevel(OutpostID) < 0.5f;

    if (bFoodShortage)
    {
        // Reduce happiness and population growth
        Outpost.Population.Happiness -= 0.01f;
        Outpost.Population.GrowthRate -= 0.0001f;
    }

    if (bPowerShortage)
    {
        // Deactivate non-essential buildings
        for (FOutpostBuilding& Building : Outpost.Buildings)
        {
            if (Building.BuildingType != EBuildingType::Habitat && 
                Building.BuildingType != EBuildingType::CommandCenter)
            {
                Building.bIsPowered = false;
            }
        }
    }
}

void UOutpostManager::CalculatePopulationGrowth(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Base growth rate modified by happiness and resources
    float GrowthModifier = Outpost.Population.Happiness;
    
    if (HasResource(OutpostID, TEXT("Food"), Outpost.Population.TotalPopulation))
    {
        GrowthModifier += 0.2f;
    }
    else
    {
        GrowthModifier -= 0.3f;
    }

    if (HasSufficientPower(OutpostID))
    {
        GrowthModifier += 0.1f;
    }
    else
    {
        GrowthModifier -= 0.2f;
    }

    Outpost.Population.GrowthRate = DEFAULT_POPULATION_GROWTH_RATE * GrowthModifier;
}

void UOutpostManager::UpdatePopulationNeeds(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Update profession distribution based on buildings
    TMap<FName, int32> RequiredProfessions;
    
    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        FName RequiredProfession;
        switch (Building.BuildingType)
        {
            case EBuildingType::Mine:
                RequiredProfession = TEXT("Miner");
                break;
            case EBuildingType::Factory:
                RequiredProfession = TEXT("Worker");
                break;
            case EBuildingType::Laboratory:
                RequiredProfession = TEXT("Scientist");
                break;
            case EBuildingType::MedicalCenter:
                RequiredProfession = TEXT("Doctor");
                break;
            default:
                RequiredProfession = TEXT("Worker");
                break;
        }

        int32 CurrentRequired = RequiredProfessions.Contains(RequiredProfession) ? 
            RequiredProfessions[RequiredProfession] : 0;
        RequiredProfessions.Add(RequiredProfession, CurrentRequired + 2); // 2 workers per building
    }

    // Adjust population distribution
    for (const auto& RequiredPair : RequiredProfessions)
    {
        int32 CurrentWorkers = Outpost.Population.PopulationByProfession.Contains(RequiredPair.Key) ? 
            Outpost.Population.PopulationByProfession[RequiredPair.Key] : 0;
        
        if (CurrentWorkers < RequiredPair.Value)
        {
            // Need more workers, reduce other professions
            int32 WorkersNeeded = RequiredPair.Value - CurrentWorkers;
            int32 WorkersReallocated = FMath::Min(WorkersNeeded, Outpost.Population.TotalPopulation / 10);
            
            Outpost.Population.PopulationByProfession.Add(RequiredPair.Key, CurrentWorkers + WorkersReallocated);
        }
    }
}

void UOutpostManager::ProcessPopulationMigration(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Calculate migration based on happiness and opportunities
    float Attractiveness = Outpost.Population.Happiness;
    
    if (Outpost.Wealth > 1000.0f)
    {
        Attractiveness += 0.2f;
    }

    if (Outpost.Buildings.Num() > 10)
    {
        Attractiveness += 0.1f;
    }

    // Random migration events
    if (FMath::FRand() < 0.01f && Attractiveness > 0.7f) // Immigration
    {
        AddPopulation(OutpostID, FMath::RandRange(1, 5));
    }
    else if (FMath::FRand() < 0.01f && Attractiveness < 0.3f) // Emigration
    {
        RemovePopulation(OutpostID, FMath::RandRange(1, 3));
    }
}

void UOutpostManager::CalculatePopulationHappiness(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Base happiness decay
    Outpost.Population.Happiness -= DEFAULT_HAPPINESS_DECAY_RATE;

    // Factors that increase happiness
    if (HasResource(OutpostID, TEXT("Food"), Outpost.Population.TotalPopulation))
    {
        Outpost.Population.Happiness += 0.01f;
    }

    if (HasSufficientPower(OutpostID))
    {
        Outpost.Population.Happiness += 0.005f;
    }

    if (Outpost.Wealth > 500.0f)
    {
        Outpost.Population.Happiness += 0.005f;
    }

    // Factors that decrease happiness
    if (IsUnderAttack(OutpostID))
    {
        Outpost.Population.Happiness -= 0.02f;
    }

    if (Outpost.Population.TotalPopulation >= Outpost.Population.MaxPopulation * 0.9f)
    {
        Outpost.Population.Happiness -= 0.01f;
    }

    // Clamp happiness
    Outpost.Population.Happiness = FMath::Clamp(Outpost.Population.Happiness, 0.0f, 1.0f);
}

void UOutpostManager::UpdateDefenseSystems(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];
    Outpost.Defense.TotalDefenseValue = 0.0f;
    Outpost.Defense.DefenseBuildings.Empty();

    // Calculate defense value from buildings
    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.BuildingType == EBuildingType::DefenseTurret && Building.bIsOperational)
        {
            Outpost.Defense.TotalDefenseValue += 100.0f * Building.Level;
            Outpost.Defense.DefenseBuildings.Add(Building.BuildingID);
        }
    }

    // Natural decay of alert level
    Outpost.Defense.AlertLevel = FMath::Max(0.0f, Outpost.Defense.AlertLevel - 0.001f);
}

void UOutpostManager::ProcessThreatDetection(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Simple threat detection based on random events
    if (FMath::FRand() < 0.005f) // 0.5% chance per update
    {
        Outpost.Defense.AlertLevel = FMath::Min(1.0f, Outpost.Defense.AlertLevel + 0.3f);
        Outpost.Defense.EnemyThreats.Add(FName(*FString::Printf(TEXT("Threat_%d"), FMath::RandRange(1, 100))));
    }
}

void UOutpostManager::HandleDefenseRepairs(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Repair shields if power is available
    if (HasSufficientPower(OutpostID) && Outpost.Defense.ShieldStrength < Outpost.Defense.MaxShieldStrength)
    {
        float ShieldRepair = 5.0f; // Repair rate
        Outpost.Defense.ShieldStrength = FMath::Min(Outpost.Defense.MaxShieldStrength, 
            Outpost.Defense.ShieldStrength + ShieldRepair);
        Outpost.Defense.bShieldsOnline = true;
    }

    // Repair buildings
    if (HasResource(OutpostID, TEXT("Materials"), 10.0f))
    {
        for (FOutpostBuilding& Building : Outpost.Buildings)
        {
            if (Building.Health < Building.MaxHealth)
            {
                float RepairAmount = FMath::Min(10.0f, Building.MaxHealth - Building.Health);
                Building.Health += RepairAmount;
                ConsumeResource(OutpostID, TEXT("Materials"), RepairAmount / 10.0f);
                
                if (Building.Health >= Building.MaxHealth)
                {
                    Building.bIsOperational = true;
                }
            }
        }
    }
}

void UOutpostManager::CalculateOutpostIncome(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Income from population
    float PopulationIncome = Outpost.Population.TotalPopulation * 0.1f;

    // Income from buildings
    float BuildingIncome = 0.0f;
    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.BuildingType == EBuildingType::Factory && Building.bIsOperational)
        {
            BuildingIncome += 10.0f * Building.Level;
        }
        else if (Building.BuildingType == EBuildingType::Mine && Building.bIsOperational)
        {
            BuildingIncome += 5.0f * Building.Level;
        }
    }

    Outpost.Wealth += (PopulationIncome + BuildingIncome) * 0.1f; // Per update cycle
}

void UOutpostManager::ProcessOutpostExpenses(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Maintenance costs
    float MaintenanceCost = Outpost.Buildings.Num() * 1.0f;
    float PopulationCost = Outpost.Population.TotalPopulation * 0.05f;

    Outpost.Wealth -= (MaintenanceCost + PopulationCost) * 0.1f; // Per update cycle
    Outpost.Wealth = FMath::Max(0.0f, Outpost.Wealth);
}

void UOutpostManager::UpdateLocalPrices(const FName& OutpostID)
{
    // Price updates are handled in GetOutpostPrices based on supply/demand
}

void UOutpostManager::ProcessFactionInfluence(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID) || !bEnableFactionDynamics)
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Natural influence decay
    for (auto& FactionPair : Outpost.FactionInfluence)
    {
        FactionPair.Value *= 0.999f; // Slow decay
    }

    // Influence based on population and buildings
    for (const auto& PopulationPair : Outpost.Population.PopulationByFaction)
    {
        float CurrentInfluence = Outpost.FactionInfluence.Contains(PopulationPair.Key) ? 
            Outpost.FactionInfluence[PopulationPair.Key] : 0.0f;
        float PopulationInfluence = (float)PopulationPair.Value / Outpost.Population.TotalPopulation;
        Outpost.FactionInfluence.Add(PopulationPair.Key, CurrentInfluence + PopulationInfluence * 0.1f);
    }
}

void UOutpostManager::HandleFactionConflicts(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Check for faction conflicts
    TArray<FName> ControllingFactions = GetControllingFactions(OutpostID);
    
    if (ControllingFactions.Num() > 1)
    {
        // Multiple controlling factions, potential conflict
        if (FMath::FRand() < 0.01f) // 1% chance per update
        {
            // Reduce happiness during conflicts
            Outpost.Population.Happiness -= 0.05f;
            Outpost.Defense.AlertLevel += 0.1f;
        }
    }
}

void UOutpostManager::UpdateFactionRelations(const FName& OutpostID)
{
    // Update faction relations based on outpost events
}

bool UOutpostManager::ValidateOutpost(const FOutpost& Outpost)
{
    if (Outpost.Name.IsEmpty() || Outpost.OutpostID.IsNone())
    {
        return false;
    }

    if (Outpost.Location.IsZero() || Outpost.Population.MaxPopulation <= 0)
    {
        return false;
    }

    return true;
}

bool UOutpostManager::ValidateBuilding(const FOutpostBuilding& Building)
{
    if (Building.BuildingID.IsNone() || Building.MaxHealth <= 0.0f)
    {
        return false;
    }

    return true;
}

bool UOutpostManager::CanSupportBuilding(const FName& OutpostID, const FOutpostBuilding& Building)
{
    if (!Outposts.Contains(OutpostID))
    {
        return false;
    }

    const FOutpost& Outpost = Outposts[OutpostID];

    // Check power requirements
    if (Building.PowerConsumption > 0.0f && 
        Outpost.Resources.PowerGeneration < Outpost.Resources.PowerConsumption + Building.PowerConsumption)
    {
        return false;
    }

    // Check required buildings
    for (const FName& RequiredBuilding : Building.RequiredBuildings)
    {
        bool bHasRequired = false;
        for (const FOutpostBuilding& ExistingBuilding : Outpost.Buildings)
        {
            if (ExistingBuilding.BuildingID == RequiredBuilding)
            {
                bHasRequired = true;
                break;
            }
        }
        
        if (!bHasRequired)
        {
            return false;
        }
    }

    return true;
}

FName UOutpostManager::GenerateOutpostID()
{
    static int32 OutpostCounter = 0;
    return FName(*FString::Printf(TEXT("Outpost_%d"), ++OutpostCounter));
}

FName UOutpostManager::GenerateBuildingID()
{
    static int32 BuildingCounter = 0;
    return FName(*FString::Printf(TEXT("Building_%d"), ++BuildingCounter));
}

float UOutpostManager::CalculateOutpostEfficiency(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return 0.0f;
    }

    const FOutpost& Outpost = Outposts[OutpostID];
    
    float Efficiency = 0.5f; // Base efficiency

    // Power efficiency
    if (HasSufficientPower(OutpostID))
    {
        Efficiency += 0.2f;
    }
    else
    {
        Efficiency -= 0.3f;
    }

    // Population efficiency
    if (Outpost.Population.TotalPopulation > 0)
    {
        Efficiency += Outpost.Population.Happiness * 0.2f;
        Efficiency += Outpost.Population.Productivity * 0.1f;
    }

    // Building efficiency
    int32 OperationalBuildings = 0;
    for (const FOutpostBuilding& Building : Outpost.Buildings)
    {
        if (Building.bIsOperational)
        {
            OperationalBuildings++;
        }
    }
    
    if (Outpost.Buildings.Num() > 0)
    {
        Efficiency += (float)OperationalBuildings / Outpost.Buildings.Num() * 0.2f;
    }

    return FMath::Clamp(Efficiency, 0.0f, 1.0f);
}

void UOutpostManager::UpdateOutpostSize(const FName& OutpostID)
{
    if (!Outposts.Contains(OutpostID))
    {
        return;
    }

    FOutpost& Outpost = Outposts[OutpostID];

    // Auto-upgrade size based on population
    if (Outpost.Population.TotalPopulation >= Outpost.Population.MaxPopulation * 0.9f)
    {
        if (CanUpgradeOutpost(OutpostID))
        {
            UpgradeOutpost(OutpostID);
        }
    }
}