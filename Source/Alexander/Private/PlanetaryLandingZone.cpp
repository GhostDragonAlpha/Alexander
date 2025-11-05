#include "PlanetaryLandingZone.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "Net/UnrealNetwork.h"

APlanetaryLandingZone::APlanetaryLandingZone()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;

    // Set default values
    CurrentStatus = ELandingZoneStatus::Active;
    ServiceUpdateInterval = DEFAULT_SERVICE_UPDATE_INTERVAL;
    EnvironmentUpdateInterval = DEFAULT_ENVIRONMENT_UPDATE_INTERVAL;
    EconomyUpdateInterval = DEFAULT_ECONOMY_UPDATE_INTERVAL;
    bEnableEnvironmentalHazards = true;
    bEnableDynamicPricing = true;
    bEnableFactionInfluence = true;

    // Create components
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    ZoneBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBounds"));
    ZoneBounds->SetupAttachment(RootComponent);
    ZoneBounds->SetBoxExtent(FVector(DEFAULT_ZONE_RADIUS, DEFAULT_ZONE_RADIUS, 1000.0f));
    ZoneBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ZoneBounds->SetCollisionObjectType(ECC_WorldDynamic);
    ZoneBounds->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void APlanetaryLandingZone::BeginPlay()
{
    Super::BeginPlay();

    // Initialize landing zone if data is set
    if (LandingZoneData.ZoneName != NAME_None)
    {
        InitializeLandingZone(LandingZoneData);
    }

    // Set up timers
    if (GetWorld() && GetWorldTimerManager().IsValid())
    {
        GetWorldTimerManager().SetTimer(ServiceUpdateTimer, this, &APlanetaryLandingZone::UpdateZoneServices, ServiceUpdateInterval, true);
        GetWorldTimerManager().SetTimer(EnvironmentUpdateTimer, this, &APlanetaryLandingZone::UpdateEnvironmentalConditions, EnvironmentUpdateInterval, true);
        GetWorldTimerManager().SetTimer(EconomyUpdateTimer, this, &APlanetaryLandingZone::UpdateLocalEconomy, EconomyUpdateInterval, true);
    }

    // Bind overlap events
    ZoneBounds->OnComponentBeginOverlap.AddDynamic(this, &APlanetaryLandingZone::OnZoneBeginOverlap);
    ZoneBounds->OnComponentEndOverlap.AddDynamic(this, &APlanetaryLandingZone::OnZoneEndOverlap);
}

void APlanetaryLandingZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentStatus == ELandingZoneStatus::Active)
    {
        ProcessEnvironmentalEffects(DeltaTime);
        HandleFactions(DeltaTime);
    }
}

void APlanetaryLandingZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timers
    if (GetWorldTimerManager().IsValid())
    {
        GetWorldTimerManager().ClearTimer(ServiceUpdateTimer);
        GetWorldTimerManager().ClearTimer(EnvironmentUpdateTimer);
        GetWorldTimerManager().ClearTimer(EconomyUpdateTimer);
        GetWorldTimerManager().ClearTimer(PopulationUpdateTimer);
    }

    ShutdownLandingZone();
    Super::EndPlay(EndPlayReason);
}

void APlanetaryLandingZone::InitializeLandingZone(const FLandingZone& ZoneData)
{
    LandingZoneData = ZoneData;
    CurrentStatus = ZoneData.Status;
    CurrentEnvironment = ZoneData.Environment;

    UE_LOG(LogTemp, Log, TEXT("Initializing Planetary Landing Zone: %s"), *ZoneData.ZoneName.ToString());

    // Initialize landing pads
    InitializeLandingPads();

    // Set up zone bounds
    SetupZoneBounds();

    // Initialize services
    UpdateServices();

    // Initialize local economy
    UpdateLocalEconomy();

    // Initialize population
    UpdatePopulation();

    UE_LOG(LogTemp, Log, TEXT("Landing Zone %s initialized with %d landing pads"), 
        *ZoneData.ZoneName.ToString(), LandingZoneData.LandingPads.Num());
}

void APlanetaryLandingZone::ShutdownLandingZone()
{
    UE_LOG(LogTemp, Log, TEXT("Shutting down Planetary Landing Zone: %s"), *LandingZoneData.ZoneName.ToString());

    // Clear all ships
    for (AActor* Ship : ShipsInZone)
    {
        if (Ship)
        {
            AbortLanding(Ship);
        }
    }

    // Release all landing pads
    for (auto& PadPair : LandingPadMap)
    {
        ReleaseLandingPad(PadPair.Key);
    }

    SetZoneStatus(ELandingZoneStatus::Offline);
}

FLandingPad* APlanetaryLandingZone::GetAvailableLandingPad(float ShipSize, const FName& Faction)
{
    for (auto& PadPair : LandingPadMap)
    {
        FLandingPad& Pad = PadPair.Value;
        
        if (!Pad.bIsOccupied && Pad.MaxShipSize >= ShipSize)
        {
            if (Pad.bRequiresPermission && !Pad.AllowedFactions.IsEmpty())
            {
                if (Pad.AllowedFactions.Contains(Faction))
                {
                    return &Pad;
                }
            }
            else
            {
                return &Pad;
            }
        }
    }
    return nullptr;
}

bool APlanetaryLandingZone::ReserveLandingPad(const FName& PadID, AActor* Ship)
{
    if (!LandingPadMap.Contains(PadID) || !Ship)
    {
        return false;
    }

    FLandingPad& Pad = LandingPadMap[PadID];
    
    if (Pad.bIsOccupied)
    {
        return false;
    }

    Pad.bIsOccupied = true;
    Pad.OccupyingShip = Ship;
    ShipPadAssignments.Add(Ship, PadID);
    ShipsInZone.Add(Ship);

    UE_LOG(LogTemp, Log, TEXT("Reserved landing pad %s for ship %s"), *PadID.ToString(), *Ship->GetName());
    return true;
}

void APlanetaryLandingZone::ReleaseLandingPad(const FName& PadID)
{
    if (!LandingPadMap.Contains(PadID))
    {
        return;
    }

    FLandingPad& Pad = LandingPadMap[PadID];
    
    if (Pad.OccupyingShip)
    {
        ShipsInZone.Remove(Pad.OccupyingShip);
        ShipPadAssignments.Remove(Pad.OccupyingShip);
        OnShipTakeoff.Broadcast(Pad.OccupyingShip, PadID);
    }

    Pad.bIsOccupied = false;
    Pad.OccupyingShip = nullptr;

    UE_LOG(LogTemp, Log, TEXT("Released landing pad %s"), *PadID.ToString());
}

bool APlanetaryLandingZone::IsPadAvailable(const FName& PadID)
{
    if (!LandingPadMap.Contains(PadID))
    {
        return false;
    }
    return !LandingPadMap[PadID].bIsOccupied;
}

TArray<FLandingPad> APlanetaryLandingZone::GetAllLandingPads()
{
    TArray<FLandingPad> Result;
    for (const auto& PadPair : LandingPadMap)
    {
        Result.Add(PadPair.Value);
    }
    return Result;
}

TArray<FLandingPad> APlanetaryLandingZone::GetOccupiedPads()
{
    TArray<FLandingPad> Result;
    for (const auto& PadPair : LandingPadMap)
    {
        if (PadPair.Value.bIsOccupied)
        {
            Result.Add(PadPair.Value);
        }
    }
    return Result;
}

bool APlanetaryLandingZone::CanShipLand(AActor* Ship, const FName& Faction)
{
    if (!Ship || CurrentStatus != ELandingZoneStatus::Active)
    {
        return false;
    }

    // Check if landing requires clearance
    if (LandingZoneData.bRequiresLandingClearance)
    {
        // Check permits here
        // if (!HasRequiredPermits(Faction, LandingZoneData.RequiredPermits))
        // {
        //     return false;
        // }
    }

    // Check faction welcome status
    if (!IsFactionWelcome(Faction))
    {
        return false;
    }

    // Check for available landing pad
    FLandingPad* AvailablePad = GetAvailableLandingPad(100.0f, Faction); // Default ship size
    return AvailablePad != nullptr;
}

bool APlanetaryLandingZone::RequestLandingClearance(AActor* Ship, const FName& Faction)
{
    if (!CanShipLand(Ship, Faction))
    {
        return false;
    }

    FLandingPad* AvailablePad = GetAvailableLandingPad(100.0f, Faction); // Default ship size
    if (AvailablePad)
    {
        return ReserveLandingPad(AvailablePad->PadID, Ship);
    }

    return false;
}

void APlanetaryLandingZone::ProcessLanding(AActor* Ship, const FName& PadID)
{
    if (!Ship || !LandingPadMap.Contains(PadID))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Processing landing for ship %s on pad %s"), *Ship->GetName(), *PadID.ToString());

    // Play landing effects
    if (LandingZoneData.Environment.WeatherEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LandingZoneData.Environment.WeatherEffect,
            LandingPadMap[PadID].Location);
    }

    // Notify systems
    OnShipLanded.Broadcast(Ship, PadID);

    // Update local economy with landing activity
    ProcessTradeActivity(TEXT("Landing_Fee"), 1, true);
}

void APlanetaryLandingZone::ProcessTakeoff(AActor* Ship, const FName& PadID)
{
    if (!Ship)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Processing takeoff for ship %s from pad %s"), *Ship->GetName(), *PadID.ToString());

    // Play takeoff effects
    if (LandingZoneData.Environment.WeatherEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LandingZoneData.Environment.WeatherEffect,
            Ship->GetActorLocation());
    }

    // Release the landing pad
    ReleaseLandingPad(PadID);
}

void APlanetaryLandingZone::AbortLanding(AActor* Ship)
{
    if (!Ship)
    {
        return;
    }

    if (ShipPadAssignments.Contains(Ship))
    {
        FName PadID = ShipPadAssignments[Ship];
        ReleaseLandingPad(PadID);
    }
    else
    {
        ShipsInZone.Remove(Ship);
    }

    UE_LOG(LogTemp, Log, TEXT("Aborted landing for ship %s"), *Ship->GetName());
}

bool APlanetaryLandingZone::IsServiceAvailable(const FName& ServiceType)
{
    if (ServiceType == TEXT("Refueling"))
    {
        return LandingZoneData.Services.bRefuelingAvailable;
    }
    else if (ServiceType == TEXT("Repair"))
    {
        return LandingZoneData.Services.bRepairAvailable;
    }
    else if (ServiceType == TEXT("CargoTransfer"))
    {
        return LandingZoneData.Services.bCargoTransfer;
    }
    else if (ServiceType == TEXT("Customization"))
    {
        return LandingZoneData.Services.bCustomization;
    }
    else if (ServiceType == TEXT("Medical"))
    {
        return LandingZoneData.Services.bMedicalServices;
    }
    else if (ServiceType == TEXT("Trading"))
    {
        return LandingZoneData.Services.bTradingPost;
    }
    else if (ServiceType == TEXT("Missions"))
    {
        return LandingZoneData.Services.bMissionBoard;
    }
    else if (ServiceType == TEXT("Accommodation"))
    {
        return LandingZoneData.Services.bAccommodation;
    }

    return LandingZoneData.Services.ServiceCosts.Contains(ServiceType);
}

float APlanetaryLandingZone::GetServiceCost(const FName& ServiceType)
{
    if (ServiceType == TEXT("Refueling"))
    {
        return LandingZoneData.Services.RefuelCost;
    }
    else if (ServiceType == TEXT("Repair"))
    {
        return LandingZoneData.Services.RepairCost;
    }
    else if (LandingZoneData.Services.ServiceCosts.Contains(ServiceType))
    {
        return LandingZoneData.Services.ServiceCosts[ServiceType];
    }

    return 0.0f;
}

bool APlanetaryLandingZone::UseService(APlayerState* Player, const FName& ServiceType, AActor* Ship)
{
    if (!Player || !IsServiceAvailable(ServiceType))
    {
        return false;
    }

    float Cost = GetServiceCost(ServiceType);
    
    // Check if player can afford service
    // if (!Player->CanAfford(Cost))
    // {
    //     return false;
    // }

    // Process service based on type
    if (ServiceType == TEXT("Refueling") && Ship)
    {
        // Process refueling
        ProcessRefuelingService();
    }
    else if (ServiceType == TEXT("Repair") && Ship)
    {
        // Process repair
        ProcessRepairService();
    }
    else if (ServiceType == TEXT("CargoTransfer"))
    {
        // Process cargo transfer
        ProcessCargoTransfer();
    }

    // Charge player
    // Player->DeductCredits(Cost);

    // Update local economy
    ProcessTradeActivity(ServiceType, 1, true);

    UE_LOG(LogTemp, Log, TEXT("Player %s used service %s for cost %.0f"), 
        *Player->GetPlayerName(), *ServiceType.ToString(), Cost);

    return true;
}

void APlanetaryLandingZone::UpdateServices()
{
    // Update service availability based on zone status and population
    if (CurrentStatus != ELandingZoneStatus::Active)
    {
        // Disable all services when zone is inactive
        LandingZoneData.Services.bRefuelingAvailable = false;
        LandingZoneData.Services.bRepairAvailable = false;
        LandingZoneData.Services.bCargoTransfer = false;
        LandingZoneData.Services.bCustomization = false;
        LandingZoneData.Services.bMedicalServices = false;
        LandingZoneData.Services.bTradingPost = false;
        LandingZoneData.Services.bMissionBoard = false;
        LandingZoneData.Services.bAccommodation = false;
        return;
    }

    // Enable services based on population and facilities
    bool bHasBasicServices = LandingZoneData.CurrentPopulation > 0;
    bool bHasAdvancedServices = LandingZoneData.CurrentPopulation > 50;
    bool bHasSpecializedServices = LandingZoneData.CurrentPopulation > 200;

    LandingZoneData.Services.bRefuelingAvailable = bHasBasicServices;
    LandingZoneData.Services.bRepairAvailable = bHasBasicServices;
    LandingZoneData.Services.bCargoTransfer = bHasBasicServices;
    LandingZoneData.Services.bMedicalServices = bHasAdvancedServices;
    LandingZoneData.Services.bTradingPost = bHasAdvancedServices;
    LandingZoneData.Services.bMissionBoard = bHasAdvancedServices;
    LandingZoneData.Services.bCustomization = bHasSpecializedServices;
    LandingZoneData.Services.bAccommodation = bHasSpecializedServices;

    // Update service costs based on local economy
    UpdateServiceCosts();
}

void APlanetaryLandingZone::UpdateEnvironmentalConditions(float DeltaTime)
{
    if (!bEnableEnvironmentalHazards)
    {
        return;
    }

    // Update weather conditions
    ProcessAtmosphericConditions();

    // Update terrain conditions
    UpdateTerrainConditions();

    // Apply environmental effects
    ApplyWeatherEffects();

    // Apply radiation damage if present
    if (CurrentEnvironment.bHasRadiation)
    {
        ApplyRadiationDamage();
    }

    // Check for new environmental events
    if (FMath::FRand() < 0.001f) // 0.1% chance per update
    {
        TriggerEnvironmentalEvent(TEXT("Random_Weather_Event"));
    }
}

FLandingZoneEnvironment APlanetaryLandingZone::GetCurrentEnvironment()
{
    return CurrentEnvironment;
}

bool APlanetaryLandingZone::IsEnvironmentSafe()
{
    return !CurrentEnvironment.bHasRadiation && 
           CurrentEnvironment.RadiationLevel < 0.5f &&
           ActiveHazards.Num() == 0;
}

TArray<FName> APlanetaryLandingZone::GetActiveHazards()
{
    return ActiveHazards;
}

void APlanetaryLandingZone::TriggerEnvironmentalEvent(const FName& EventType)
{
    UE_LOG(LogTemp, Log, TEXT("Environmental event triggered: %s in zone %s"), 
        *EventType.ToString(), *LandingZoneData.ZoneName.ToString());

    if (EventType == TEXT("Radiation_Storm"))
    {
        CurrentEnvironment.bHasRadiation = true;
        CurrentEnvironment.RadiationLevel = FMath::RandRange(0.5f, 2.0f);
        ActiveHazards.AddUnique(TEXT("Radiation_Storm"));
    }
    else if (EventType == TEXT("Acid_Rain"))
    {
        ActiveHazards.AddUnique(TEXT("Acid_Rain"));
    }
    else if (EventType == TEXT("Sandstorm"))
    {
        CurrentEnvironment.Visibility = FMath::RandRange(0.1f, 0.5f);
        CurrentEnvironment.WindSpeed = FMath::RandRange(20.0f, 50.0f);
        ActiveHazards.AddUnique(TEXT("Sandstorm"));
    }
    else if (EventType == TEXT("Meteor_Shower"))
    {
        ActiveHazards.AddUnique(TEXT("Meteor_Shower"));
    }

    OnEnvironmentalEvent.Broadcast(EventType);
}

void APlanetaryLandingZone::UpdatePopulation()
{
    if (CurrentStatus != ELandingZoneStatus::Active)
    {
        return;
    }

    CalculatePopulationGrowth();
    UpdateFactionDemographics();
    ProcessMigration();

    // Update services based on new population
    UpdateServices();
}

bool APlanetaryLandingZone::CanAcceptNewResidents()
{
    return LandingZoneData.CurrentPopulation < LandingZoneData.MaxPopulation;
}

void APlanetaryLandingZone::AddResidents(int32 Count, const FName& Faction)
{
    if (!CanAcceptNewResidents() || Count <= 0)
    {
        return;
    }

    int32 ActualCount = FMath::Min(Count, LandingZoneData.MaxPopulation - LandingZoneData.CurrentPopulation);
    LandingZoneData.CurrentPopulation += ActualCount;

    // Update faction presence
    if (!Faction.IsNone())
    {
        float CurrentInfluence = LandingZoneData.FactionPresence.Contains(Faction) ? 
            LandingZoneData.FactionPresence[Faction] : 0.0f;
        LandingZoneData.FactionPresence.Add(Faction, CurrentInfluence + ActualCount);
    }

    UE_LOG(LogTemp, Log, TEXT("Added %d residents to zone %s. New population: %d"), 
        ActualCount, *LandingZoneData.ZoneName.ToString(), LandingZoneData.CurrentPopulation);
}

void APlanetaryLandingZone::RemoveResidents(int32 Count)
{
    if (Count <= 0 || LandingZoneData.CurrentPopulation <= 0)
    {
        return;
    }

    int32 ActualCount = FMath::Min(Count, LandingZoneData.CurrentPopulation);
    LandingZoneData.CurrentPopulation -= ActualCount;

    // Update faction presence (reduce proportionally)
    for (auto& FactionPair : LandingZoneData.FactionPresence)
    {
        float Reduction = (FactionPair.Value / (LandingZoneData.CurrentPopulation + ActualCount)) * ActualCount;
        FactionPair.Value = FMath::Max(0.0f, FactionPair.Value - Reduction);
    }

    UE_LOG(LogTemp, Log, TEXT("Removed %d residents from zone %s. New population: %d"), 
        ActualCount, *LandingZoneData.ZoneName.ToString(), LandingZoneData.CurrentPopulation);
}

TMap<FName, float> APlanetaryLandingZone::GetFactionPresence()
{
    return LandingZoneData.FactionPresence;
}

float APlanetaryLandingZone::GetFactionInfluence(const FName& Faction)
{
    if (LandingZoneData.FactionPresence.Contains(Faction))
    {
        return LandingZoneData.FactionPresence[Faction];
    }
    return 0.0f;
}

void APlanetaryLandingZone::UpdateFactionInfluence(const FName& Faction, float Change)
{
    float CurrentInfluence = LandingZoneData.FactionPresence.Contains(Faction) ? 
        LandingZoneData.FactionPresence[Faction] : 0.0f;
    LandingZoneData.FactionPresence.Add(Faction, FMath::Max(0.0f, CurrentInfluence + Change));

    OnFactionInfluenceChanged.Broadcast(Faction, LandingZoneData.FactionPresence[Faction]);
}

bool APlanetaryLandingZone::IsFactionWelcome(const FName& Faction)
{
    // Check if faction is not hostile
    // This would integrate with your faction system
    return true; // Placeholder
}

TArray<FName> APlanetaryLandingZone::GetControllingFactions()
{
    TArray<FName> ControllingFactions;
    float TotalInfluence = 0.0f;

    // Calculate total influence
    for (const auto& FactionPair : LandingZoneData.FactionPresence)
    {
        TotalInfluence += FactionPair.Value;
    }

    if (TotalInfluence <= 0.0f)
    {
        return ControllingFactions;
    }

    // Add factions with significant influence (>20%)
    for (const auto& FactionPair : LandingZoneData.FactionPresence)
    {
        float InfluencePercentage = (FactionPair.Value / TotalInfluence) * 100.0f;
        if (InfluencePercentage > 20.0f)
        {
            ControllingFactions.Add(FactionPair.Key);
        }
    }

    return ControllingFactions;
}

void APlanetaryLandingZone::SetZoneStatus(ELandingZoneStatus NewStatus)
{
    ELandingZoneStatus OldStatus = CurrentStatus;
    CurrentStatus = NewStatus;

    OnZoneStatusChanged.Broadcast(OldStatus, NewStatus);

    UE_LOG(LogTemp, Log, TEXT("Zone %s status changed from %d to %d"), 
        *LandingZoneData.ZoneName.ToString(), (int32)OldStatus, (int32)NewStatus);
}

void APlanetaryLandingZone::HandleZoneDamage(float DamageAmount)
{
    if (CurrentStatus == ELandingZoneStatus::Active)
    {
        // Zone takes damage
        if (DamageAmount > 100.0f)
        {
            SetZoneStatus(ELandingZoneStatus::Damaged);
        }
    }
    else if (CurrentStatus == ELandingZoneStatus::Damaged)
    {
        // Additional damage could make zone abandoned
        if (DamageAmount > 500.0f)
        {
            SetZoneStatus(ELandingZoneStatus::Abandoned);
        }
    }
}

void APlanetaryLandingZone::RepairZone(float RepairAmount)
{
    if (CurrentStatus == ELandingZoneStatus::Damaged)
    {
        // Repair the zone
        if (RepairAmount > 100.0f)
        {
            SetZoneStatus(ELandingZoneStatus::Active);
        }
    }
}

void APlanetaryLandingZone::UpdateLocalEconomy()
{
    if (!bEnableDynamicPricing)
    {
        return;
    }

    CalculateSupplyDemand();
    UpdatePriceModifiers();
    ProcessTradeFlow();
}

TMap<FName, float> APlanetaryLandingZone::GetLocalPrices()
{
    return LocalPrices;
}

void APlanetaryLandingZone::ProcessTradeActivity(const FName& ItemID, int32 Quantity, bool bBuy)
{
    // Update demand based on trade activity
    int32 CurrentDemand = LocalDemand.Contains(ItemID) ? LocalDemand[ItemID] : 0;
    
    if (bBuy)
    {
        LocalDemand.Add(ItemID, FMath::Max(0, CurrentDemand - Quantity));
    }
    else
    {
        LocalDemand.Add(ItemID, CurrentDemand + Quantity);
    }

    // Update price based on supply/demand
    UpdateItemPrice(ItemID);
}

void APlanetaryLandingZone::GenerateLocalMissions()
{
    if (!LandingZoneData.Services.bMissionBoard)
    {
        return;
    }

    // Generate missions based on zone characteristics
    TArray<FName> AvailableMissionTypes = GetAvailableMissionTypes();

    for (const FName& MissionType : AvailableMissionTypes)
    {
        // Generate missions of this type
        // This would integrate with your mission system
    }
}

TArray<FName> APlanetaryLandingZone::GetAvailableMissionTypes()
{
    TArray<FName> MissionTypes;

    if (LandingZoneData.Services.bTradingPost)
    {
        MissionTypes.Add(TEXT("Delivery"));
        MissionTypes.Add(TEXT("Transport"));
    }

    if (LandingZoneData.ZoneType == ELandingZoneType::Mining)
    {
        MissionTypes.Add(TEXT("Mining"));
        MissionTypes.Add(TEXT("Salvage"));
    }

    if (LandingZoneData.ZoneType == ELandingZoneType::Research)
    {
        MissionTypes.Add(TEXT("Exploration"));
        MissionTypes.Add(TEXT("Investigation"));
    }

    if (LandingZoneData.ZoneType == ELandingZoneType::Military)
    {
        MissionTypes.Add(TEXT("Patrol"));
        MissionTypes.Add(TEXT("Bounty"));
        MissionTypes.Add(TEXT("Escort"));
    }

    return MissionTypes;
}

bool APlanetaryLandingZone::SupportsMissionType(EMissionType MissionType)
{
    TArray<FName> AvailableTypes = GetAvailableMissionTypes();
    
    switch (MissionType)
    {
        case EMissionType::Delivery:
        case EMissionType::Transport:
            return AvailableTypes.Contains(TEXT("Delivery")) || AvailableTypes.Contains(TEXT("Transport"));
        case EMissionType::Mining:
        case EMissionType::Salvage:
            return AvailableTypes.Contains(TEXT("Mining")) || AvailableTypes.Contains(TEXT("Salvage"));
        case EMissionType::Exploration:
        case EMissionType::Investigation:
            return AvailableTypes.Contains(TEXT("Exploration")) || AvailableTypes.Contains(TEXT("Investigation"));
        case EMissionType::Patrol:
        case EMissionType::Bounty:
        case EMissionType::Escort:
            return AvailableTypes.Contains(TEXT("Patrol")) || AvailableTypes.Contains(TEXT("Bounty")) || AvailableTypes.Contains(TEXT("Escort"));
        default:
            return false;
    }
}

float APlanetaryLandingZone::GetDistanceToZone(const FVector& Location)
{
    return FVector::Dist(Location, LandingZoneData.CenterLocation);
}

bool APlanetaryLandingZone::IsLocationInZone(const FVector& Location)
{
    return GetDistanceToZone(Location) <= LandingZoneData.ZoneRadius;
}

FVector APlanetaryLandingZone::GetNearestSafeLocation(const FVector& DangerousLocation)
{
    // Find nearest landing pad or safe area
    FVector NearestSafeLocation = LandingZoneData.CenterLocation;
    float NearestDistance = FVector::Dist(DangerousLocation, NearestSafeLocation);

    for (const auto& PadPair : LandingPadMap)
    {
        const FVector& PadLocation = PadPair.Value.Location;
        float Distance = FVector::Dist(DangerousLocation, PadLocation);
        
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestSafeLocation = PadLocation;
        }
    }

    return NearestSafeLocation;
}

TArray<AActor*> APlanetaryLandingZone::GetShipsInZone()
{
    return ShipsInZone.Array();
}

TArray<APlayerState*> APlanetaryLandingZone::GetPlayersInZone()
{
    TArray<APlayerState*> Players;
    
    for (AActor* Ship : ShipsInZone)
    {
        if (Ship)
        {
            // Get players from ship
            // This depends on your ship implementation
        }
    }
    
    return Players;
}

void APlanetaryLandingZone::InitializeLandingPads()
{
    LandingPadMap.Empty();

    // Create landing pads from zone data
    for (const FLandingPad& Pad : LandingZoneData.LandingPads)
    {
        LandingPadMap.Add(Pad.PadID, Pad);
    }

    // Generate additional landing pads if needed
    int32 PadsToGenerate = FMath::Min(MAX_LANDING_PADS - LandingZoneData.LandingPads.Num(), 10);
    
    for (int32 i = 0; i < PadsToGenerate; i++)
    {
        FVector PadLocation = LandingZoneData.CenterLocation + 
            FVector(FMath::RandRange(-2000.0f, 2000.0f), FMath::RandRange(-2000.0f, 2000.0f), 0.0f);
        FRotator PadRotation = FRotator::ZeroRotator;
        float MaxSize = FMath::RandRange(50.0f, 200.0f);
        
        CreateLandingPad(PadLocation, PadRotation, MaxSize);
    }

    UE_LOG(LogTemp, Log, TEXT("Initialized %d landing pads for zone %s"), 
        LandingPadMap.Num(), *LandingZoneData.ZoneName.ToString());
}

void APlanetaryLandingZone::SetupZoneBounds()
{
    if (ZoneBounds)
    {
        ZoneBounds->SetBoxExtent(FVector(LandingZoneData.ZoneRadius, LandingZoneData.ZoneRadius, 1000.0f));
        ZoneBounds->SetWorldLocation(LandingZoneData.CenterLocation);
    }
}

void APlanetaryLandingZone::UpdateZoneServices(float DeltaTime)
{
    UpdateServices();
}

void APlanetaryLandingZone::ProcessEnvironmentalEffects(float DeltaTime)
{
    // Process ongoing environmental effects
    for (int32 i = ActiveHazards.Num() - 1; i >= 0; i--)
    {
        const FName& Hazard = ActiveHazards[i];
        
        if (Hazard == TEXT("Radiation_Storm"))
        {
            // Radiation damage over time
            ApplyRadiationDamage();
        }
        else if (Hazard == TEXT("Acid_Rain"))
        {
            // Acid damage to ships and structures
            // Implementation depends on your damage system
        }
        else if (Hazard == TEXT("Sandstorm"))
        {
            // Visibility and movement effects
            // Implementation depends on your player controller
        }
    }
}

void APlanetaryLandingZone::HandleFactions(float DeltaTime)
{
    if (!bEnableFactionInfluence)
    {
        return;
    }

    // Update faction influence based on activity
    for (auto& FactionPair : LandingZoneData.FactionPresence)
    {
        // Natural influence decay/growth
        float InfluenceChange = FMath::RandRange(-0.1f, 0.1f);
        UpdateFactionInfluence(FactionPair.Key, InfluenceChange);
    }
}

FName APlanetaryLandingZone::GeneratePadID()
{
    static int32 PadCounter = 0;
    return FName(*FString::Printf(TEXT("Pad_%d"), ++PadCounter));
}

void APlanetaryLandingZone::CreateLandingPad(const FVector& Location, const FRotator& Rotation, float MaxSize)
{
    FLandingPad NewPad;
    NewPad.PadID = GeneratePadID();
    NewPad.Location = Location;
    NewPad.Rotation = Rotation;
    NewPad.MaxShipSize = MaxSize;
    NewPad.bIsOccupied = false;
    NewPad.OccupyingShip = nullptr;
    NewPad.bRequiresPermission = false;
    
    LandingPadMap.Add(NewPad.PadID, NewPad);
}

void APlanetaryLandingZone::ApplyWeatherEffects()
{
    // Apply weather effects based on current conditions
    if (CurrentEnvironment.WindSpeed > 30.0f)
    {
        // Strong wind effects
    }

    if (CurrentEnvironment.Visibility < 0.5f)
    {
        // Low visibility effects
    }
}

void APlanetaryLandingZone::ApplyRadiationDamage()
{
    if (!CurrentEnvironment.bHasRadiation || CurrentEnvironment.RadiationLevel <= 0.0f)
    {
        return;
    }

    // Apply radiation damage to ships and players in zone
    for (AActor* Ship : ShipsInZone)
    {
        if (Ship)
        {
            // Apply radiation damage based on level
            float Damage = CurrentEnvironment.RadiationLevel * 0.1f; // Damage per second
            // Ship->TakeDamage(Damage, ...);
        }
    }
}

void APlanetaryLandingZone::ProcessAtmosphericConditions()
{
    // Update atmospheric conditions over time
    CurrentEnvironment.Temperature += FMath::RandRange(-0.1f, 0.1f);
    CurrentEnvironment.WindSpeed += FMath::RandRange(-1.0f, 1.0f);
    CurrentEnvironment.Visibility += FMath::RandRange(-0.01f, 0.01f);

    // Clamp values
    CurrentEnvironment.Temperature = FMath::Clamp(CurrentEnvironment.Temperature, -50.0f, 50.0f);
    CurrentEnvironment.WindSpeed = FMath::Clamp(CurrentEnvironment.WindSpeed, 0.0f, 100.0f);
    CurrentEnvironment.Visibility = FMath::Clamp(CurrentEnvironment.Visibility, 0.0f, 1.0f);
}

void APlanetaryLandingZone::UpdateTerrainConditions()
{
    // Update terrain conditions based on weather
    if (CurrentEnvironment.Temperature < 0.0f)
    {
        // Ice formation
    }
    else if (CurrentEnvironment.Temperature > 40.0f)
    {
        // Heat effects
    }
}

void APlanetaryLandingZone::ProcessRefuelingService()
{
    // Process refueling for ships
    UE_LOG(LogTemp, Log, TEXT("Processing refueling service"));
}

void APlanetaryLandingZone::ProcessRepairService()
{
    // Process repair for ships
    UE_LOG(LogTemp, Log, TEXT("Processing repair service"));
}

void APlanetaryLandingZone::ProcessCargoTransfer()
{
    // Process cargo transfer
    UE_LOG(LogTemp, Log, TEXT("Processing cargo transfer"));
}

void APlanetaryLandingZone::UpdateServiceCosts()
{
    // Update service costs based on local economy and population
    float PopulationModifier = 1.0f + (LandingZoneData.CurrentPopulation / 1000.0f);
    
    LandingZoneData.Services.RefuelCost = 100.0f * PopulationModifier;
    LandingZoneData.Services.RepairCost = 150.0f * PopulationModifier;
}

void APlanetaryLandingZone::CalculatePopulationGrowth()
{
    if (LandingZoneData.CurrentPopulation >= LandingZoneData.MaxPopulation)
    {
        return;
    }

    // Calculate natural population growth
    float GrowthRate = 0.001f; // 0.1% per update cycle
    int32 Growth = FMath::FloorToInt(LandingZoneData.CurrentPopulation * GrowthRate);
    
    if (Growth > 0)
    {
        AddResidents(Growth);
    }
}

void APlanetaryLandingZone::UpdateFactionDemographics()
{
    // Update faction demographics based on various factors
    float TotalInfluence = 0.0f;
    
    for (const auto& FactionPair : LandingZoneData.FactionPresence)
    {
        TotalInfluence += FactionPair.Value;
    }

    if (TotalInfluence > 0.0f)
    {
        // Normalize faction presence
        for (auto& FactionPair : LandingZoneData.FactionPresence)
        {
            FactionPair.Value = (FactionPair.Value / TotalInfluence) * LandingZoneData.CurrentPopulation;
        }
    }
}

void APlanetaryLandingZone::ProcessMigration()
{
    // Process population migration based on zone attractiveness
    float Attractiveness = CalculateZoneAttractiveness();
    
    if (Attractiveness > 0.7f && CanAcceptNewResidents())
    {
        // Immigration
        int32 Immigrants = FMath::RandRange(1, 5);
        AddResidents(Immigrants);
    }
    else if (Attractiveness < 0.3f)
    {
        // Emigration
        int32 Emigrants = FMath::RandRange(1, 3);
        RemoveResidents(Emigrants);
    }
}

float APlanetaryLandingZone::CalculateZoneAttractiveness()
{
    float Attractiveness = 0.5f; // Base attractiveness

    // Add factors for services
    if (LandingZoneData.Services.bRefuelingAvailable) Attractiveness += 0.1f;
    if (LandingZoneData.Services.bRepairAvailable) Attractiveness += 0.1f;
    if (LandingZoneData.Services.bTradingPost) Attractiveness += 0.15f;
    if (LandingZoneData.Services.bMedicalServices) Attractiveness += 0.1f;
    if (LandingZoneData.Services.bMissionBoard) Attractiveness += 0.1f;

    // Subtract factors for hazards
    if (CurrentEnvironment.bHasRadiation) Attractiveness -= 0.2f;
    if (ActiveHazards.Num() > 0) Attractiveness -= 0.1f * ActiveHazards.Num();

    // Factor in safety
    if (CurrentStatus == ELandingZoneStatus::Active) Attractiveness += 0.1f;
    if (CurrentStatus == ELandingZoneStatus::Damaged) Attractiveness -= 0.2f;
    if (CurrentStatus == ELandingZoneStatus::Abandoned) Attractiveness -= 0.4f;

    return FMath::Clamp(Attractiveness, 0.0f, 1.0f);
}

void APlanetaryLandingZone::CalculateSupplyDemand()
{
    // Calculate supply and demand for various goods
    // This would integrate with your economy system
}

void APlanetaryLandingZone::UpdatePriceModifiers()
{
    // Update prices based on supply/demand
    for (auto& PricePair : LocalPrices)
    {
        const FName& ItemID = PricePair.Key;
        float& Price = PricePair.Value;
        
        if (LocalDemand.Contains(ItemID))
        {
            float Demand = LocalDemand[ItemID];
            float DemandModifier = 1.0f + (Demand / 100.0f); // Simple demand scaling
            Price *= DemandModifier;
        }
    }
}

void APlanetaryLandingZone::ProcessTradeFlow()
{
    // Process trade flow between zones
    // This would integrate with your economy system
}

void APlanetaryLandingZone::UpdateItemPrice(const FName& ItemID)
{
    if (!LocalPrices.Contains(ItemID))
    {
        LocalPrices.Add(ItemID, 100.0f); // Base price
    }

    float Demand = LocalDemand.Contains(ItemID) ? LocalDemand[ItemID] : 0.0f;
    float DemandModifier = 1.0f + (Demand / 100.0f);
    LocalPrices[ItemID] = 100.0f * DemandModifier;
}

void APlanetaryLandingZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Handle ship entering zone
    if (OtherActor && OtherActor->ActorHasTag(TEXT("Ship")))
    {
        ShipsInZone.Add(OtherActor);
        UE_LOG(LogTemp, Log, TEXT("Ship %s entered landing zone %s"), 
            *OtherActor->GetName(), *LandingZoneData.ZoneName.ToString());
    }
}

void APlanetaryLandingZone::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Handle ship leaving zone
    if (OtherActor && OtherActor->ActorHasTag(TEXT("Ship")))
    {
        ShipsInZone.Remove(OtherActor);
        
        if (ShipPadAssignments.Contains(OtherActor))
        {
            FName PadID = ShipPadAssignments[OtherActor];
            ReleaseLandingPad(PadID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Ship %s left landing zone %s"), 
            *OtherActor->GetName(), *LandingZoneData.ZoneName.ToString());
    }
}

void APlanetaryLandingZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlanetaryLandingZone, CurrentStatus);
    DOREPLIFETIME(APlanetaryLandingZone, LandingZoneData);
    DOREPLIFETIME(APlanetaryLandingZone, CurrentEnvironment);
    DOREPLIFETIME(APlanetaryLandingZone, LandingPadMap);
}