// Fill out your copyright notice in the Description page of Project Settings.

#include "EnvironmentalHazardsSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UEnvironmentalHazardsSystem::UEnvironmentalHazardsSystem()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;
    
    // Set replication
    SetIsReplicatedByDefault(true);
    
    // Initialize configuration
    HazardUpdateInterval = DEFAULT_HAZARD_UPDATE_INTERVAL;
    WeatherUpdateInterval = DEFAULT_WEATHER_UPDATE_INTERVAL;
    PredictionUpdateInterval = DEFAULT_PREDICTION_UPDATE_INTERVAL;
    MaxPredictionTimeWindow = MAX_PREDICTION_TIME_WINDOW;
    DefaultHazardDuration = DEFAULT_HAZARD_DURATION;
    bEnableAutoPrediction = true;
    bEnableAutoDefense = true;
    
    // Initialize internal state
    LastHazardUpdateTime = 0.0f;
    LastWeatherUpdateTime = 0.0f;
    LastPredictionUpdateTime = 0.0f;
    NextHazardEventID = 1;
    NextDefenseSystemID = 1;
    NextPredictionID = 1;
    NextZoneID = 1;
    
    // Initialize data structures
    ActiveHazards.Empty();
    CurrentWeather = FWeatherData();
    EnvironmentalZones.Empty();
    DefenseSystems.Empty();
    HazardPredictions.Empty();
    EnvironmentalStats = FEnvironmentalStatistics();
}

// Called when the game starts
void UEnvironmentalHazardsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize environmental system
    if (GetOwner()->HasAuthority())
    {
        InitializeEnvironmentalSystem();
    }
    
    // Set up timers
    SetupEnvironmentalTimers();
}

// Called every frame
void UEnvironmentalHazardsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update environmental systems
    if (GetOwner()->HasAuthority())
    {
        UpdateHazardEvents(DeltaTime);
        UpdateWeatherSystem(DeltaTime);
        UpdatePredictionSystem(DeltaTime);
        UpdateDefenseSystems(DeltaTime);
        UpdateEnvironmentalStatistics();
    }
}

// Replicated properties
void UEnvironmentalHazardsSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UEnvironmentalHazardsSystem, ActiveHazards);
    DOREPLIFETIME(UEnvironmentalHazardsSystem, CurrentWeather);
    DOREPLIFETIME(UEnvironmentalHazardsSystem, EnvironmentalZones);
    DOREPLIFETIME(UEnvironmentalHazardsSystem, DefenseSystems);
    DOREPLIFETIME(UEnvironmentalHazardsSystem, HazardPredictions);
    DOREPLIFETIME(UEnvironmentalHazardsSystem, EnvironmentalStats);
}

// Hazard management
bool UEnvironmentalHazardsSystem::GetHazardEvent(int32 EventID, FHazardEvent& OutEvent)
{
    FHazardEvent* HazardEvent = FindHazardEvent(EventID);
    if (HazardEvent)
    {
        OutEvent = *HazardEvent;
        return true;
    }
    return false;
}

int32 UEnvironmentalHazardsSystem::CreateHazardEvent(EHazardType HazardType, EHazardSeverity Severity, const FVector& Location, float Radius)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    if (!IsValidHazardType(HazardType) || !IsValidSeverity(Severity))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid hazard type or severity"));
        return -1;
    }
    
    // Create new hazard event
    FHazardEvent NewEvent;
    NewEvent.EventID = NextHazardEventID++;
    NewEvent.HazardType = HazardType;
    NewEvent.Severity = Severity;
    NewEvent.Location = Location;
    NewEvent.Radius = FMath::Clamp(Radius, MIN_HAZARD_RADIUS, MAX_HAZARD_RADIUS);
    NewEvent.StartTime = FDateTime::Now();
    NewEvent.EndTime = NewEvent.StartTime + FTimespan::FromSeconds(DefaultHazardDuration);
    NewEvent.Duration = DefaultHazardDuration;
    NewEvent.bIsActive = false;
    NewEvent.bIsPredicted = false;
    NewEvent.DamagePerSecond = CalculateHazardDamage(HazardType, Severity);
    NewEvent.WarningMessage = GenerateHazardWarning(HazardType, Severity);
    NewEvent.Description = GenerateHazardDescription(HazardType, Severity);
    
    // Set required defenses based on hazard type
    NewEvent.RequiredDefenses = GetRequiredDefenses(HazardType);
    
    // Calculate resource costs for mitigation
    NewEvent.ResourceCosts = CalculateMitigationCosts(HazardType, Severity);
    
    ActiveHazards.Add(NewEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Created hazard event %d: %s at %s"), 
           NewEvent.EventID, *GetHazardName(HazardType), *Location.ToString());
    
    return NewEvent.EventID;
}

bool UEnvironmentalHazardsSystem::TriggerHazardEvent(int32 EventID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardEvent* HazardEvent = FindHazardEvent(EventID);
    if (!HazardEvent || HazardEvent->bIsActive)
    {
        return false;
    }
    
    // Activate hazard event
    HazardEvent->bIsActive = true;
    HazardEvent->StartTime = FDateTime::Now();
    HazardEvent->EndTime = HazardEvent->StartTime + FTimespan::FromSeconds(HazardEvent->Duration);
    
    // Apply hazard effects
    ApplyHazardEffects(*HazardEvent);
    
    // Update statistics
    EnvironmentalStats.TotalHazardsEncountered++;
    EnvironmentalStats.ActiveHazards++;
    EnvironmentalStats.HazardFrequency.FindOrAdd(HazardEvent->HazardType)++;
    EnvironmentalStats.SeverityDistribution.FindOrAdd(HazardEvent->Severity)++;
    
    // Broadcast warning
    BroadcastHazardWarning(*HazardEvent);
    
    // Trigger event
    OnHazardEventStarted.Broadcast(*HazardEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered hazard event %d: %s"), 
           EventID, *GetHazardName(HazardEvent->HazardType));
    
    return true;
}

bool UEnvironmentalHazardsSystem::EndHazardEvent(int32 EventID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardEvent* HazardEvent = FindHazardEvent(EventID);
    if (!HazardEvent || !HazardEvent->bIsActive)
    {
        return false;
    }
    
    // Deactivate hazard event
    HazardEvent->bIsActive = false;
    HazardEvent->EndTime = FDateTime::Now();
    
    // Update statistics
    EnvironmentalStats.ActiveHazards--;
    
    // Trigger event
    OnHazardEventEnded.Broadcast(*HazardEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Ended hazard event %d: %s"), 
           EventID, *GetHazardName(HazardEvent->HazardType));
    
    return true;
}

TArray<FHazardEvent> UEnvironmentalHazardsSystem::GetHazardsInRadius(const FVector& Location, float Radius) const
{
    TArray<FHazardEvent> HazardsInRadius;
    
    for (const FHazardEvent& HazardEvent : ActiveHazards)
    {
        if (HazardEvent.bIsActive)
        {
            float Distance = FVector::Dist(Location, HazardEvent.Location);
            if (Distance <= (Radius + HazardEvent.Radius))
            {
                HazardsInRadius.Add(HazardEvent);
            }
        }
    }
    
    return HazardsInRadius;
}

bool UEnvironmentalHazardsSystem::IsLocationSafe(const FVector& Location) const
{
    // Check for active hazards
    TArray<FHazardEvent> NearbyHazards = GetHazardsInRadius(Location, 100.0f);
    
    for (const FHazardEvent& HazardEvent : NearbyHazards)
    {
        // Check if location is protected
        float ProtectionLevel = CalculateTotalDefenseEffectiveness(
            GetActiveDefensesInRadius(Location, 100.0f), 
            HazardEvent.HazardType
        );
        
        // If protection is insufficient, location is not safe
        if (ProtectionLevel < 0.8f) // 80% protection required for safety
        {
            return false;
        }
    }
    
    return true;
}

// Weather system
FWeatherData UEnvironmentalHazardsSystem::GetWeatherForecast() const
{
    FWeatherData Forecast = CurrentWeather;
    Forecast.CurrentCondition = Forecast.ForecastCondition;
    Forecast.CurrentTemperature = Forecast.ForecastTemperature;
    return Forecast;
}

void UEnvironmentalHazardsSystem::UpdateWeather()
{
    if (!GetOwner()->HasAuthority()) return;
    
    // Calculate next weather condition
    EWeatherCondition NextCondition = CalculateNextWeatherCondition();
    
    // Update weather data
    CurrentWeather.LastUpdateTime = FDateTime::Now();
    CurrentWeather.CurrentCondition = NextCondition;
    
    // Update temperature based on weather
    UpdateTemperatureForWeather(NextCondition);
    
    // Update other weather parameters
    UpdateWeatherParameters(NextCondition);
    
    // Apply weather effects
    ApplyWeatherEffects(CurrentWeather);
    
    // Generate new forecast
    GenerateWeatherForecast();
    
    // Update statistics
    EnvironmentalStats.RecentWeather.Add(NextCondition);
    if (EnvironmentalStats.RecentWeather.Num() > 24) // Keep last 24 hours
    {
        EnvironmentalStats.RecentWeather.RemoveAt(0);
    }
    
    // Trigger event
    OnWeatherChanged.Broadcast(CurrentWeather);
    
    UE_LOG(LogTemp, Log, TEXT("Weather updated to: %s"), *GetWeatherConditionName(NextCondition));
}

bool UEnvironmentalHazardsSystem::PredictWeatherChange(EWeatherCondition& PredictedCondition, float& Confidence)
{
    // Simple weather prediction based on current conditions and patterns
    TArray<EWeatherCondition> PossibleConditions;
    TArray<float> Probabilities;

    // Get possible transitions from current weather
    for (const auto& Transition : WeatherTransitionProbabilities)
    {
        if (Transition.Key == CurrentWeather.CurrentCondition)
        {
            for (const auto& ConditionProb : Transition.Value.Transitions)
            {
                PossibleConditions.Add(ConditionProb.Key);
                Probabilities.Add(ConditionProb.Value);
            }
            break;
        }
    }
    
    if (PossibleConditions.Num() == 0)
    {
        PredictedCondition = EWeatherCondition::Clear;
        Confidence = 0.5f;
        return false;
    }
    
    // Select condition based on probability
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    float CumulativeProbability = 0.0f;
    
    for (int32 i = 0; i < PossibleConditions.Num(); i++)
    {
        CumulativeProbability += Probabilities[i];
        if (RandomValue <= CumulativeProbability)
        {
            PredictedCondition = PossibleConditions[i];
            Confidence = Probabilities[i];
            return true;
        }
    }
    
    PredictedCondition = PossibleConditions.Last();
    Confidence = Probabilities.Last();
    return true;
}

void UEnvironmentalHazardsSystem::SetWeatherCondition(EWeatherCondition NewCondition)
{
    if (!GetOwner()->HasAuthority()) return;
    
    if (!IsValidWeatherCondition(NewCondition)) return;
    
    CurrentWeather.CurrentCondition = NewCondition;
    UpdateTemperatureForWeather(NewCondition);
    UpdateWeatherParameters(NewCondition);
    ApplyWeatherEffects(CurrentWeather);
    
    OnWeatherChanged.Broadcast(CurrentWeather);
}

// Environmental zones
bool UEnvironmentalHazardsSystem::GetEnvironmentalZone(int32 ZoneID, FEnvironmentalZone& OutZone)
{
    FEnvironmentalZone* Zone = FindEnvironmentalZone(ZoneID);
    if (Zone)
    {
        OutZone = *Zone;
        return true;
    }
    return false;
}

bool UEnvironmentalHazardsSystem::GetZoneAtLocation(const FVector& Location, FEnvironmentalZone& OutZone)
{
    for (FEnvironmentalZone& Zone : EnvironmentalZones)
    {
        float Distance = FVector::Dist(Location, Zone.CenterLocation);
        if (Distance <= Zone.ZoneRadius)
        {
            OutZone = Zone;
            return true;
        }
    }
    return false;
}

int32 UEnvironmentalHazardsSystem::CreateEnvironmentalZone(const FString& ZoneName, const FVector& Center, float Radius)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    FEnvironmentalZone NewZone;
    NewZone.ZoneID = NextZoneID++;
    NewZone.ZoneName = ZoneName;
    NewZone.CenterLocation = Center;
    NewZone.ZoneRadius = Radius;
    
    // Determine zone properties based on location
    DetermineZoneProperties(NewZone);
    
    EnvironmentalZones.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("Created environmental zone %d: %s"), 
           NewZone.ZoneID, *ZoneName);
    
    return NewZone.ZoneID;
}

bool UEnvironmentalHazardsSystem::UpdateZoneProperties(int32 ZoneID, const FEnvironmentalZone& NewProperties)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FEnvironmentalZone* Zone = FindEnvironmentalZone(ZoneID);
    if (!Zone) return false;
    
    *Zone = NewProperties;
    Zone->ZoneID = ZoneID; // Preserve ID
    
    UE_LOG(LogTemp, Log, TEXT("Updated environmental zone %d"), ZoneID);
    return true;
}

// Defense systems
bool UEnvironmentalHazardsSystem::GetDefenseSystem(int32 DefenseID, FHazardDefense& OutDefense)
{
    FHazardDefense* Defense = FindDefenseSystem(DefenseID);
    if (Defense)
    {
        OutDefense = *Defense;
        return true;
    }
    return false;
}

int32 UEnvironmentalHazardsSystem::InstallDefenseSystem(const FString& DefenseName, EHazardType ProtectedHazard, const FVector& Location, float Radius)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    if (!IsValidHazardType(ProtectedHazard)) return -1;
    
    FHazardDefense NewDefense;
    NewDefense.DefenseID = NextDefenseSystemID++;
    NewDefense.DefenseName = DefenseName;
    NewDefense.ProtectedHazardType = ProtectedHazard;
    NewDefense.ProtectedHazards.Add(ProtectedHazard);
    NewDefense.Location = Location;
    NewDefense.ProtectionRadius = Radius;
    NewDefense.Effectiveness = 1.0f;
    NewDefense.PowerConsumption = CalculateDefensePowerConsumption(ProtectedHazard, Radius);
    NewDefense.MaintenanceCost = CalculateDefenseMaintenanceCost(ProtectedHazard, Radius);
    NewDefense.bIsActive = true;
    NewDefense.bIsOperational = true;
    NewDefense.LastMaintenanceTime = FDateTime::Now();
    NewDefense.CurrentHealth = 100.0f;
    NewDefense.MaxHealth = 100.0f;
    
    DefenseSystems.Add(NewDefense);
    
    // Update statistics
    EnvironmentalStats.ActiveDefenseSystems++;
    
    // Trigger event
    OnDefenseSystemActivated.Broadcast(NewDefense.DefenseID, true);
    
    UE_LOG(LogTemp, Log, TEXT("Installed defense system %d: %s for %s"), 
           NewDefense.DefenseID, *DefenseName, *GetHazardName(ProtectedHazard));
    
    return NewDefense.DefenseID;
}

bool UEnvironmentalHazardsSystem::ActivateDefenseSystem(int32 DefenseID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardDefense* Defense = FindDefenseSystem(DefenseID);
    if (!Defense) return false;
    
    Defense->bIsActive = true;
    
    // Update statistics
    EnvironmentalStats.ActiveDefenseSystems++;
    
    // Trigger event
    OnDefenseSystemActivated.Broadcast(DefenseID, true);
    
    UE_LOG(LogTemp, Log, TEXT("Activated defense system %d"), DefenseID);
    return true;
}

bool UEnvironmentalHazardsSystem::DeactivateDefenseSystem(int32 DefenseID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardDefense* Defense = FindDefenseSystem(DefenseID);
    if (!Defense) return false;
    
    Defense->bIsActive = false;
    
    // Update statistics
    EnvironmentalStats.ActiveDefenseSystems--;
    
    // Trigger event
    OnDefenseSystemActivated.Broadcast(DefenseID, false);
    
    UE_LOG(LogTemp, Log, TEXT("Deactivated defense system %d"), DefenseID);
    return true;
}

bool UEnvironmentalHazardsSystem::UpgradeDefenseSystem(int32 DefenseID, const FString& UpgradeType)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardDefense* Defense = FindDefenseSystem(DefenseID);
    if (!Defense) return false;
    
    // Apply upgrade effects
    ApplyDefenseUpgrade(*Defense, UpgradeType);
    
    Defense->Upgrades.Add(UpgradeType);
    
    UE_LOG(LogTemp, Log, TEXT("Upgraded defense system %d with %s"), DefenseID, *UpgradeType);
    return true;
}

bool UEnvironmentalHazardsSystem::MaintainDefenseSystem(int32 DefenseID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardDefense* Defense = FindDefenseSystem(DefenseID);
    if (!Defense) return false;
    
    // Restore health and effectiveness
    Defense->CurrentHealth = Defense->MaxHealth;
    Defense->bIsOperational = true;
    Defense->LastMaintenanceTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Maintained defense system %d"), DefenseID);
    return true;
}

// Hazard prediction
bool UEnvironmentalHazardsSystem::GetHazardPrediction(int32 PredictionID, FHazardPrediction& OutPrediction)
{
    FHazardPrediction* Prediction = FindHazardPrediction(PredictionID);
    if (Prediction)
    {
        OutPrediction = *Prediction;
        return true;
    }
    return false;
}

int32 UEnvironmentalHazardsSystem::PredictHazard(EHazardType HazardType, const FVector& Location, float TimeWindow)
{
    if (!GetOwner()->HasAuthority()) return -1;
    
    if (!IsValidHazardType(HazardType)) return -1;
    
    // Calculate prediction parameters
    float Probability = CalculateHazardProbability(HazardType, Location);
    if (Probability < 0.3f) // Minimum 30% probability to create prediction
    {
        return -1;
    }
    
    FHazardPrediction NewPrediction;
    NewPrediction.PredictionID = NextPredictionID++;
    NewPrediction.PredictedHazardType = HazardType;
    NewPrediction.PredictedSeverity = CalculateHazardSeverity(HazardType, Location);
    NewPrediction.PredictedLocation = Location;
    NewPrediction.PredictedRadius = FMath::RandRange(MIN_HAZARD_RADIUS, MAX_HAZARD_RADIUS);
    NewPrediction.PredictionTime = FDateTime::Now();
    NewPrediction.PredictedStartTime = NewPrediction.PredictionTime + FTimespan::FromSeconds(TimeWindow);
    NewPrediction.ConfidenceLevel = Probability;
    NewPrediction.PredictionSources = GetPredictionSources(HazardType);
    NewPrediction.PredictionMethod = GetPredictionMethod(HazardType);
    NewPrediction.bIsVerified = false;
    
    HazardPredictions.Add(NewPrediction);
    
    // Update statistics
    EnvironmentalStats.HazardsPredicted++;
    
    // Trigger event
    OnHazardPredicted.Broadcast(NewPrediction);
    
    UE_LOG(LogTemp, Log, TEXT("Created hazard prediction %d: %s with confidence %f"), 
           NewPrediction.PredictionID, *GetHazardName(HazardType), Probability);
    
    return NewPrediction.PredictionID;
}

bool UEnvironmentalHazardsSystem::VerifyPrediction(int32 PredictionID)
{
    if (!GetOwner()->HasAuthority()) return false;
    
    FHazardPrediction* Prediction = FindHazardPrediction(PredictionID);
    if (!Prediction) return false;
    
    // Check if prediction came true
    bool bPredictionAccurate = VerifyPredictionAccuracy(*Prediction);
    Prediction->bIsVerified = true;
    
    if (bPredictionAccurate)
    {
        UE_LOG(LogTemp, Log, TEXT("Prediction %d verified as accurate"), PredictionID);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Prediction %d verified as inaccurate"), PredictionID);
    }
    
    return bPredictionAccurate;
}

TArray<FHazardPrediction> UEnvironmentalHazardsSystem::GetPredictionsForTimeWindow(const FDateTime& StartTime, const FDateTime& EndTime) const
{
    TArray<FHazardPrediction> PredictionsInWindow;
    
    for (const FHazardPrediction& Prediction : HazardPredictions)
    {
        if (Prediction.PredictedStartTime >= StartTime && Prediction.PredictedStartTime <= EndTime)
        {
            PredictionsInWindow.Add(Prediction);
        }
    }
    
    return PredictionsInWindow;
}

// Environmental analysis
float UEnvironmentalHazardsSystem::CalculateHazardRisk(EHazardType HazardType, const FVector& Location) const
{
    float BaseRisk = CalculateHazardProbability(HazardType, Location);

    // Modify risk based on environmental zone
    FEnvironmentalZone Zone;
    if (const_cast<UEnvironmentalHazardsSystem*>(this)->GetZoneAtLocation(Location, Zone))
    {
        if (Zone.HazardProbabilities.Contains(HazardType))
        {
            BaseRisk *= Zone.HazardProbabilities[HazardType];
        }
    }

    // Modify risk based on existing defenses
    TArray<FHazardDefense> Defenses = GetActiveDefensesInRadius(Location, 1000.0f);
    float DefenseEffectiveness = CalculateTotalDefenseEffectiveness(Defenses, HazardType);
    BaseRisk *= (1.0f - DefenseEffectiveness);

    return FMath::Clamp(BaseRisk, 0.0f, 1.0f);
}

TArray<EHazardType> UEnvironmentalHazardsSystem::GetLikelyHazards(const FVector& Location, float TimeWindow) const
{
    TArray<EHazardType> LikelyHazards;
    TArray<float> HazardRisks;
    
    // Calculate risk for each hazard type
    TArray<EHazardType> AllHazardTypes = {
        EHazardType::RadiationStorm, EHazardType::SolarFlare, EHazardType::MeteorShower,
        EHazardType::DustStorm, EHazardType::ExtremeTemperature, EHazardType::AtmosphericPressure,
        EHazardType::SeismicActivity, EHazardType::AcidRain, EHazardType::MagneticStorm,
        EHazardType::GravityAnomaly, EHazardType::BiologicalHazard
    };
    
    for (EHazardType HazardType : AllHazardTypes)
    {
        float Risk = CalculateHazardRisk(HazardType, Location);
        if (Risk > 0.2f) // Minimum 20% risk
        {
            LikelyHazards.Add(HazardType);
            HazardRisks.Add(Risk);
        }
    }
    
    // Sort by risk (descending)
    for (int32 i = 0; i < LikelyHazards.Num() - 1; i++)
    {
        for (int32 j = i + 1; j < LikelyHazards.Num(); j++)
        {
            if (HazardRisks[j] > HazardRisks[i])
            {
                LikelyHazards.Swap(i, j);
                HazardRisks.Swap(i, j);
            }
        }
    }
    
    return LikelyHazards;
}

float UEnvironmentalHazardsSystem::GetEnvironmentalSafetyRating(const FVector& Location) const
{
    float SafetyRating = 1.0f; // Start with perfect safety

    // Check active hazards
    TArray<FHazardEvent> NearbyHazards = GetHazardsInRadius(Location, 500.0f);
    for (const FHazardEvent& HazardEvent : NearbyHazards)
    {
        float HazardImpact = CalculateHazardImpact(HazardEvent, Location);
        SafetyRating -= HazardImpact;
    }

    // Check weather conditions
    float WeatherImpact = CalculateWeatherImpact(CurrentWeather, Location);
    SafetyRating -= WeatherImpact;

    // Check environmental zone
    FEnvironmentalZone Zone;
    if (const_cast<UEnvironmentalHazardsSystem*>(this)->GetZoneAtLocation(Location, Zone))
    {
        if (!Zone.bIsHabitable)
        {
            SafetyRating -= 0.5f;
        }
    }

    // Add defense bonus
    TArray<FHazardDefense> Defenses = GetActiveDefensesInRadius(Location, 500.0f);
    float DefenseBonus = Defenses.Num() * 0.1f; // 10% bonus per defense system
    SafetyRating += DefenseBonus;

    return FMath::Clamp(SafetyRating, 0.0f, 1.0f);
}

TArray<FString> UEnvironmentalHazardsSystem::GetRecommendedDefenses(const FVector& Location) const
{
    TArray<FString> RecommendedDefenses;
    
    // Get likely hazards for this location
    TArray<EHazardType> LikelyHazards = GetLikelyHazards(Location, 3600.0f); // 1 hour window
    
    // Get existing defenses
    TArray<FHazardDefense> ExistingDefenses = GetActiveDefensesInRadius(Location, 1000.0f);
    TSet<EHazardType> ProtectedHazards;
    
    for (const FHazardDefense& Defense : ExistingDefenses)
    {
        for (EHazardType ProtectedHazard : Defense.ProtectedHazards)
        {
            ProtectedHazards.Add(ProtectedHazard);
        }
    }
    
    // Recommend defenses for unprotected hazards
    for (EHazardType HazardType : LikelyHazards)
    {
        if (!ProtectedHazards.Contains(HazardType))
        {
            FString DefenseName = GetDefenseNameForHazard(HazardType);
            if (!DefenseName.IsEmpty())
            {
                RecommendedDefenses.Add(DefenseName);
            }
        }
    }
    
    return RecommendedDefenses;
}

// Utilities
FString UEnvironmentalHazardsSystem::GetHazardName(EHazardType HazardType) const
{
    switch (HazardType)
    {
        case EHazardType::RadiationStorm: return TEXT("Radiation Storm");
        case EHazardType::SolarFlare: return TEXT("Solar Flare");
        case EHazardType::MeteorShower: return TEXT("Meteor Shower");
        case EHazardType::DustStorm: return TEXT("Dust Storm");
        case EHazardType::ExtremeTemperature: return TEXT("Extreme Temperature");
        case EHazardType::AtmosphericPressure: return TEXT("Atmospheric Pressure Anomaly");
        case EHazardType::SeismicActivity: return TEXT("Seismic Activity");
        case EHazardType::AcidRain: return TEXT("Acid Rain");
        case EHazardType::MagneticStorm: return TEXT("Magnetic Storm");
        case EHazardType::GravityAnomaly: return TEXT("Gravity Anomaly");
        case EHazardType::BiologicalHazard: return TEXT("Biological Hazard");
        default: return TEXT("Unknown Hazard");
    }
}

FString UEnvironmentalHazardsSystem::GetSeverityName(EHazardSeverity Severity) const
{
    switch (Severity)
    {
        case EHazardSeverity::Low: return TEXT("Low");
        case EHazardSeverity::Medium: return TEXT("Medium");
        case EHazardSeverity::High: return TEXT("High");
        case EHazardSeverity::Extreme: return TEXT("Extreme");
        case EHazardSeverity::Catastrophic: return TEXT("Catastrophic");
        default: return TEXT("Unknown");
    }
}

FString UEnvironmentalHazardsSystem::GetWeatherConditionName(EWeatherCondition Condition) const
{
    switch (Condition)
    {
        case EWeatherCondition::Clear: return TEXT("Clear");
        case EWeatherCondition::Cloudy: return TEXT("Cloudy");
        case EWeatherCondition::Overcast: return TEXT("Overcast");
        case EWeatherCondition::LightRain: return TEXT("Light Rain");
        case EWeatherCondition::HeavyRain: return TEXT("Heavy Rain");
        case EWeatherCondition::Storm: return TEXT("Storm");
        case EWeatherCondition::Snow: return TEXT("Snow");
        case EWeatherCondition::Blizzard: return TEXT("Blizzard");
        case EWeatherCondition::Fog: return TEXT("Fog");
        case EWeatherCondition::Sandstorm: return TEXT("Sandstorm");
        case EWeatherCondition::AcidRain: return TEXT("Acid Rain");
        case EWeatherCondition::ToxicFog: return TEXT("Toxic Fog");
        default: return TEXT("Unknown");
    }
}

float UEnvironmentalHazardsSystem::GetHazardDamageMultiplier(EHazardType HazardType, EHazardSeverity Severity) const
{
    float BaseMultiplier = 1.0f;
    
    // Hazard type multipliers
    switch (HazardType)
    {
        case EHazardType::RadiationStorm: BaseMultiplier = 2.0f; break;
        case EHazardType::SolarFlare: BaseMultiplier = 3.0f; break;
        case EHazardType::MeteorShower: BaseMultiplier = 4.0f; break;
        case EHazardType::DustStorm: BaseMultiplier = 1.5f; break;
        case EHazardType::ExtremeTemperature: BaseMultiplier = 2.5f; break;
        case EHazardType::AtmosphericPressure: BaseMultiplier = 1.8f; break;
        case EHazardType::SeismicActivity: BaseMultiplier = 3.5f; break;
        case EHazardType::AcidRain: BaseMultiplier = 2.2f; break;
        case EHazardType::MagneticStorm: BaseMultiplier = 1.7f; break;
        case EHazardType::GravityAnomaly: BaseMultiplier = 2.8f; break;
        case EHazardType::BiologicalHazard: BaseMultiplier = 3.2f; break;
        default: BaseMultiplier = 1.0f; break;
    }
    
    // Severity multipliers
    float SeverityMultiplier = 1.0f;
    switch (Severity)
    {
        case EHazardSeverity::Low: SeverityMultiplier = 0.5f; break;
        case EHazardSeverity::Medium: SeverityMultiplier = 1.0f; break;
        case EHazardSeverity::High: SeverityMultiplier = 2.0f; break;
        case EHazardSeverity::Extreme: SeverityMultiplier = 3.0f; break;
        case EHazardSeverity::Catastrophic: SeverityMultiplier = 5.0f; break;
        default: SeverityMultiplier = 1.0f; break;
    }
    
    return BaseMultiplier * SeverityMultiplier;
}

bool UEnvironmentalHazardsSystem::CanWithstandHazard(EHazardType HazardType, EHazardSeverity Severity, const FVector& Location) const
{
    // Check if location has adequate defenses
    TArray<FHazardDefense> Defenses = GetActiveDefensesInRadius(Location, 500.0f);
    float DefenseEffectiveness = CalculateTotalDefenseEffectiveness(Defenses, HazardType);
    
    // Calculate required effectiveness based on severity
    float RequiredEffectiveness = GetRequiredEffectiveness(Severity);
    
    return DefenseEffectiveness >= RequiredEffectiveness;
}

// Internal functions
void UEnvironmentalHazardsSystem::InitializeEnvironmentalSystem()
{
    // Initialize environmental zones
    InitializeEnvironmentalZones();
    
    // Initialize weather patterns
    InitializeWeatherPatterns();
    
    // Initialize hazard probabilities
    InitializeHazardProbabilities();
    
    // Set initial weather
    SetInitialWeather();
    
    UE_LOG(LogTemp, Log, TEXT("Environmental hazards system initialized"));
}

void UEnvironmentalHazardsSystem::UpdateHazardEvents(float DeltaTime)
{
    for (FHazardEvent& HazardEvent : ActiveHazards)
    {
        if (HazardEvent.bIsActive)
        {
            ProcessHazardEvent(HazardEvent, DeltaTime);
        }
    }
    
    // Check for expired hazards
    FDateTime CurrentTime = FDateTime::Now();
    for (FHazardEvent& HazardEvent : ActiveHazards)
    {
        if (HazardEvent.bIsActive && HazardEvent.EndTime <= CurrentTime)
        {
            EndHazardEvent(HazardEvent.EventID);
        }
    }
    
    // Generate random hazards
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per update
    {
        GenerateRandomHazard();
    }
}

void UEnvironmentalHazardsSystem::UpdateWeatherSystem(float DeltaTime)
{
    // Update weather based on timer
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastWeatherUpdateTime >= WeatherUpdateInterval)
    {
        UpdateWeather();
        LastWeatherUpdateTime = CurrentTime;
    }
}

void UEnvironmentalHazardsSystem::UpdatePredictionSystem(float DeltaTime)
{
    if (!bEnableAutoPrediction) return;
    
    // Update predictions based on timer
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPredictionUpdateTime >= PredictionUpdateInterval)
    {
        GenerateHazardPredictions();
        ProcessPredictionAccuracy();
        LastPredictionUpdateTime = CurrentTime;
    }
}

void UEnvironmentalHazardsSystem::UpdateDefenseSystems(float DeltaTime)
{
    for (FHazardDefense& Defense : DefenseSystems)
    {
        if (Defense.bIsActive && Defense.bIsOperational)
        {
            ProcessDefenseEffectiveness(Defense, DeltaTime);
        }
    }
}

void UEnvironmentalHazardsSystem::UpdateEnvironmentalStatistics()
{
    // Update average values
    UpdateAverageStatistics();
    
    // Update defense system effectiveness
    EnvironmentalStats.DefenseSystemEffectiveness = CalculateAverageDefenseEffectiveness();
}

void UEnvironmentalHazardsSystem::ProcessHazardEvent(FHazardEvent& HazardEvent, float DeltaTime)
{
    // Apply damage over time
    float Damage = HazardEvent.DamagePerSecond * DeltaTime;
    
    // Apply defense protection
    ApplyDefenseProtection(HazardEvent);
    
    // Apply hazard effects
    ApplyHazardEffects(HazardEvent);
    
    // Update affected buildings
    UpdateAffectedBuildings(HazardEvent, Damage);
}

void UEnvironmentalHazardsSystem::ProcessWeatherChanges()
{
    // Process weather transitions and effects
    ApplyWeatherEffects(CurrentWeather);
}

void UEnvironmentalHazardsSystem::ProcessWeatherEffects()
{
    // Apply current weather effects to systems
    ApplyWeatherEffects(CurrentWeather);
}

void UEnvironmentalHazardsSystem::ProcessPredictionAccuracy()
{
    // Check if any predictions have come true
    for (FHazardPrediction& Prediction : HazardPredictions)
    {
        if (!Prediction.bIsVerified && Prediction.PredictedStartTime <= FDateTime::Now())
        {
            VerifyPrediction(Prediction.PredictionID);
        }
    }
}

void UEnvironmentalHazardsSystem::ProcessDefenseEffectiveness(FHazardDefense& Defense, float DeltaTime)
{
    // Apply maintenance decay
    FTimespan TimeSinceMaintenance = FDateTime::Now() - Defense.LastMaintenanceTime;
    float MaintenanceDecay = TimeSinceMaintenance.GetTotalDays() * 0.01f; // 1% decay per day
    
    Defense.Effectiveness = FMath::Max(0.1f, Defense.Effectiveness - MaintenanceDecay);
    
    // Apply damage if effectiveness is low
    if (Defense.Effectiveness < 0.5f)
    {
        Defense.CurrentHealth -= DeltaTime * 5.0f; // 5 damage per second
        if (Defense.CurrentHealth <= 0.0f)
        {
            Defense.bIsOperational = false;
        }
    }
}

void UEnvironmentalHazardsSystem::GenerateRandomHazard()
{
    // Select random hazard type
    TArray<EHazardType> HazardTypes = {
        EHazardType::RadiationStorm, EHazardType::SolarFlare, EHazardType::MeteorShower,
        EHazardType::DustStorm, EHazardType::ExtremeTemperature
    };
    
    EHazardType RandomHazardType = HazardTypes[FMath::RandRange(0, HazardTypes.Num() - 1)];
    EHazardSeverity RandomSeverity = static_cast<EHazardSeverity>(FMath::RandRange(0, 4));
    
    // Generate random location
    FVector Center = GetOwner()->GetActorLocation();
    FVector RandomLocation = Center + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(1000.0f, 3000.0f);
    float RandomRadius = FMath::RandRange(500.0f, 2000.0f);
    
    // Create hazard event
    int32 EventID = CreateHazardEvent(RandomHazardType, RandomSeverity, RandomLocation, RandomRadius);
    
    // Trigger the hazard
    if (EventID != -1)
    {
        TriggerHazardEvent(EventID);
    }
}

void UEnvironmentalHazardsSystem::GenerateWeatherForecast()
{
    // Generate forecast for next weather update
    EWeatherCondition PredictedCondition;
    float Confidence;
    
    if (PredictWeatherChange(PredictedCondition, Confidence))
    {
        CurrentWeather.ForecastCondition = PredictedCondition;
        CurrentWeather.NextForecastTime = FDateTime::Now() + FTimespan::FromSeconds(WeatherUpdateInterval);
        
        // Update forecast temperature
        CurrentWeather.ForecastTemperature = CalculateForecastTemperature(PredictedCondition);
    }
}

void UEnvironmentalHazardsSystem::GenerateHazardPredictions()
{
    // Generate predictions for likely hazards
    FVector Center = GetOwner()->GetActorLocation();
    TArray<EHazardType> LikelyHazards = GetLikelyHazards(Center, MaxPredictionTimeWindow);
    
    for (EHazardType HazardType : LikelyHazards)
    {
        float TimeWindow = FMath::RandRange(300.0f, MaxPredictionTimeWindow); // 5 minutes to 24 hours
        FVector PredictionLocation = Center + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(500.0f, 2000.0f);
        
        PredictHazard(HazardType, PredictionLocation, TimeWindow);
    }
}

void UEnvironmentalHazardsSystem::ApplyHazardEffects(const FHazardEvent& HazardEvent)
{
    // Apply hazard effects to buildings, resources, and colonists
    float Damage = HazardEvent.DamagePerSecond;
    TArray<FString> AffectedSystems;
    
    CalculateEnvironmentalImpact(HazardEvent, Damage, AffectedSystems);
    
    // Apply damage to affected systems
    for (const FString& SystemName : AffectedSystems)
    {
        ApplyDamageToSystem(SystemName, Damage);
    }
    
    // Update statistics
    EnvironmentalStats.TotalDamageTaken += Damage;
}

void UEnvironmentalHazardsSystem::ApplyWeatherEffects(const FWeatherData& Weather)
{
    // Apply weather effects to efficiency and systems
    float EfficiencyModifier;
    TArray<FString> AffectedSystems;
    
    CalculateWeatherImpact(Weather, EfficiencyModifier, AffectedSystems);
    
    // Apply efficiency modifier to affected systems
    for (const FString& SystemName : AffectedSystems)
    {
        ApplyEfficiencyModifier(SystemName, EfficiencyModifier);
    }
}

void UEnvironmentalHazardsSystem::ApplyDefenseProtection(FHazardEvent& HazardEvent)
{
    // Get active defenses in hazard radius
    TArray<FHazardDefense> Defenses = GetActiveDefensesInRadius(HazardEvent.Location, HazardEvent.Radius);
    float TotalEffectiveness = CalculateTotalDefenseEffectiveness(Defenses, HazardEvent.HazardType);
    
    // Reduce damage based on defense effectiveness
    HazardEvent.DamagePerSecond *= (1.0f - TotalEffectiveness);
    
    // Update statistics
    float DamagePrevented = HazardEvent.DamagePerSecond * TotalEffectiveness;
    EnvironmentalStats.TotalDamagePrevented += DamagePrevented;
    EnvironmentalStats.HazardsMitigated++;
}

bool UEnvironmentalHazardsSystem::ValidateHazardEvent(const FHazardEvent& HazardEvent) const
{
    return IsValidHazardType(HazardEvent.HazardType) && 
           IsValidSeverity(HazardEvent.Severity) &&
           HazardEvent.Radius > 0.0f &&
           HazardEvent.Duration > 0.0f;
}

bool UEnvironmentalHazardsSystem::ValidateDefenseSystem(const FHazardDefense& Defense) const
{
    return Defense.ProtectionRadius > 0.0f &&
           Defense.Effectiveness > 0.0f &&
           Defense.MaxHealth > 0.0f;
}

bool UEnvironmentalHazardsSystem::ValidatePrediction(const FHazardPrediction& Prediction) const
{
    return IsValidHazardType(Prediction.PredictedHazardType) &&
           Prediction.PredictedRadius > 0.0f &&
           Prediction.ConfidenceLevel > 0.0f &&
           Prediction.ConfidenceLevel <= 1.0f;
}

void UEnvironmentalHazardsSystem::CalculateEnvironmentalImpact(const FHazardEvent& HazardEvent, float& Damage, TArray<FString>& AffectedSystems)
{
    // Calculate damage based on hazard type and severity
    Damage = GetHazardDamageMultiplier(HazardEvent.HazardType, HazardEvent.Severity) * 10.0f; // Base 10 damage
    
    // Determine affected systems based on hazard type
    switch (HazardEvent.HazardType)
    {
        case EHazardType::RadiationStorm:
            AffectedSystems.Add(TEXT("PowerSystems"));
            AffectedSystems.Add(TEXT("LifeSupport"));
            AffectedSystems.Add(TEXT("Communication"));
            break;
        case EHazardType::SolarFlare:
            AffectedSystems.Add(TEXT("PowerSystems"));
            AffectedSystems.Add(TEXT("Communication"));
            AffectedSystems.Add(TEXT("Electronics"));
            break;
        case EHazardType::MeteorShower:
            AffectedSystems.Add(TEXT("Buildings"));
            AffectedSystems.Add(TEXT("DefenseSystems"));
            AffectedSystems.Add(TEXT("ExternalEquipment"));
            break;
        case EHazardType::DustStorm:
            AffectedSystems.Add(TEXT("SolarPanels"));
            AffectedSystems.Add(TEXT("ExternalEquipment"));
            AffectedSystems.Add(TEXT("LifeSupport"));
            break;
        case EHazardType::ExtremeTemperature:
            AffectedSystems.Add(TEXT("LifeSupport"));
            AffectedSystems.Add(TEXT("PowerSystems"));
            AffectedSystems.Add(TEXT("Colonists"));
            break;
        default:
            AffectedSystems.Add(TEXT("AllSystems"));
            break;
    }
}

void UEnvironmentalHazardsSystem::CalculateWeatherImpact(const FWeatherData& Weather, float& EfficiencyModifier, TArray<FString>& AffectedSystems)
{
    EfficiencyModifier = 1.0f; // Start with no modifier
    
    // Calculate efficiency modifier based on weather
    switch (Weather.CurrentCondition)
    {
        case EWeatherCondition::Clear:
            EfficiencyModifier = 1.1f; // 10% bonus
            AffectedSystems.Add(TEXT("SolarPanels"));
            break;
        case EWeatherCondition::Cloudy:
            EfficiencyModifier = 0.9f; // 10% penalty
            AffectedSystems.Add(TEXT("SolarPanels"));
            break;
        case EWeatherCondition::Storm:
            EfficiencyModifier = 0.7f; // 30% penalty
            AffectedSystems.Add(TEXT("ExternalEquipment"));
            AffectedSystems.Add(TEXT("Communication"));
            break;
        case EWeatherCondition::Blizzard:
            EfficiencyModifier = 0.5f; // 50% penalty
            AffectedSystems.Add(TEXT("AllSystems"));
            break;
        case EWeatherCondition::AcidRain:
            EfficiencyModifier = 0.8f; // 20% penalty
            AffectedSystems.Add(TEXT("ExternalEquipment"));
            AffectedSystems.Add(TEXT("Buildings"));
            break;
        default:
            EfficiencyModifier = 1.0f;
            break;
    }
    
    // Temperature effects
    if (Weather.CurrentTemperature < -20.0f || Weather.CurrentTemperature > 45.0f)
    {
        EfficiencyModifier *= 0.8f; // Additional 20% penalty for extreme temperatures
        AffectedSystems.Add(TEXT("LifeSupport"));
        AffectedSystems.Add(TEXT("PowerSystems"));
    }
}

void UEnvironmentalHazardsSystem::TriggerEnvironmentalAlert(const FString& AlertMessage, EHazardSeverity Severity)
{
    // Broadcast alert to all players
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, AlertMessage);
    }
    
    // Trigger event
    OnEnvironmentalAlert.Broadcast(AlertMessage);
}

void UEnvironmentalHazardsSystem::BroadcastHazardWarning(const FHazardEvent& HazardEvent)
{
    FString WarningMessage = FString::Printf(
        TEXT("HAZARD WARNING: %s (Severity: %s) detected at location. %s"),
        *GetHazardName(HazardEvent.HazardType),
        *GetSeverityName(HazardEvent.Severity),
        *HazardEvent.WarningMessage
    );
    
    TriggerEnvironmentalAlert(WarningMessage, HazardEvent.Severity);
}

void UEnvironmentalHazardsSystem::InitializeEnvironmentalZones()
{
    // Create default environmental zones around the planet
    FVector Center = GetOwner()->GetActorLocation();
    
    // Create temperate zone (center)
    CreateEnvironmentalZone(TEXT("Temperate Zone"), Center, 2000.0f);
    
    // Create surrounding zones
    for (int32 i = 0; i < 6; i++)
    {
        FVector ZoneLocation = Center + UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(2500.0f, 4000.0f);
        FString ZoneName = FString::Printf(TEXT("Environmental Zone %d"), i + 1);
        CreateEnvironmentalZone(ZoneName, ZoneLocation, 1500.0f);
    }
}

void UEnvironmentalHazardsSystem::InitializeWeatherPatterns()
{
    // Initialize weather transition probabilities
    FWeatherTransitionProbabilities ClearTransitions;
    ClearTransitions.Transitions.Add(EWeatherCondition::Clear, 0.6f);
    ClearTransitions.Transitions.Add(EWeatherCondition::Cloudy, 0.3f);
    ClearTransitions.Transitions.Add(EWeatherCondition::Overcast, 0.1f);
    WeatherTransitionProbabilities.Add(EWeatherCondition::Clear, ClearTransitions);

    FWeatherTransitionProbabilities CloudyTransitions;
    CloudyTransitions.Transitions.Add(EWeatherCondition::Clear, 0.2f);
    CloudyTransitions.Transitions.Add(EWeatherCondition::Cloudy, 0.4f);
    CloudyTransitions.Transitions.Add(EWeatherCondition::Overcast, 0.3f);
    CloudyTransitions.Transitions.Add(EWeatherCondition::LightRain, 0.1f);
    WeatherTransitionProbabilities.Add(EWeatherCondition::Cloudy, CloudyTransitions);

    FWeatherTransitionProbabilities StormTransitions;
    StormTransitions.Transitions.Add(EWeatherCondition::Overcast, 0.3f);
    StormTransitions.Transitions.Add(EWeatherCondition::Storm, 0.4f);
    StormTransitions.Transitions.Add(EWeatherCondition::HeavyRain, 0.2f);
    StormTransitions.Transitions.Add(EWeatherCondition::Cloudy, 0.1f);
    WeatherTransitionProbabilities.Add(EWeatherCondition::Storm, StormTransitions);
}

void UEnvironmentalHazardsSystem::InitializeHazardProbabilities()
{
    // Initialize base hazard probabilities
    BaseHazardProbabilities.Add(EHazardType::RadiationStorm, 0.1f);
    BaseHazardProbabilities.Add(EHazardType::SolarFlare, 0.05f);
    BaseHazardProbabilities.Add(EHazardType::MeteorShower, 0.08f);
    BaseHazardProbabilities.Add(EHazardType::DustStorm, 0.15f);
    BaseHazardProbabilities.Add(EHazardType::ExtremeTemperature, 0.12f);
    BaseHazardProbabilities.Add(EHazardType::AtmosphericPressure, 0.06f);
    BaseHazardProbabilities.Add(EHazardType::SeismicActivity, 0.04f);
    BaseHazardProbabilities.Add(EHazardType::AcidRain, 0.07f);
    BaseHazardProbabilities.Add(EHazardType::MagneticStorm, 0.09f);
    BaseHazardProbabilities.Add(EHazardType::GravityAnomaly, 0.03f);
    BaseHazardProbabilities.Add(EHazardType::BiologicalHazard, 0.05f);
}

// Utility functions implementation
bool UEnvironmentalHazardsSystem::IsValidHazardType(EHazardType HazardType) const
{
    return HazardType != EHazardType::None;
}

bool UEnvironmentalHazardsSystem::IsValidWeatherCondition(EWeatherCondition Condition) const
{
    return Condition != EWeatherCondition::Clear || true; // All conditions are valid
}

bool UEnvironmentalHazardsSystem::IsValidSeverity(EHazardSeverity Severity) const
{
    return Severity >= EHazardSeverity::Low && Severity <= EHazardSeverity::Catastrophic;
}

float UEnvironmentalHazardsSystem::CalculateHazardProbability(EHazardType HazardType, const FVector& Location) const
{
    float BaseProbability = 0.1f; // 10% base probability

    if (BaseHazardProbabilities.Contains(HazardType))
    {
        BaseProbability = BaseHazardProbabilities[HazardType];
    }

    // Modify based on environmental zone
    FEnvironmentalZone Zone;
    if (const_cast<UEnvironmentalHazardsSystem*>(this)->GetZoneAtLocation(Location, Zone))
    {
        if (Zone.HazardProbabilities.Contains(HazardType))
        {
            BaseProbability *= Zone.HazardProbabilities[HazardType];
        }
    }

    // Add some randomness
    BaseProbability *= FMath::RandRange(0.5f, 1.5f);

    return FMath::Clamp(BaseProbability, 0.0f, 1.0f);
}

EHazardSeverity UEnvironmentalHazardsSystem::CalculateHazardSeverity(EHazardType HazardType, const FVector& Location) const
{
    // Calculate severity based on location and hazard type
    float SeverityRoll = FMath::RandRange(0.0f, 1.0f);
    
    // Some hazards are more likely to be severe
    float SeverityModifier = 1.0f;
    switch (HazardType)
    {
        case EHazardType::SolarFlare:
        case EHazardType::MeteorShower:
            SeverityModifier = 1.5f;
            break;
        case EHazardType::RadiationStorm:
        case EHazardType::SeismicActivity:
            SeverityModifier = 1.2f;
            break;
        default:
            SeverityModifier = 1.0f;
            break;
    }
    
    SeverityRoll *= SeverityModifier;
    
    if (SeverityRoll < 0.3f) return EHazardSeverity::Low;
    if (SeverityRoll < 0.6f) return EHazardSeverity::Medium;
    if (SeverityRoll < 0.8f) return EHazardSeverity::High;
    if (SeverityRoll < 0.95f) return EHazardSeverity::Extreme;
    return EHazardSeverity::Catastrophic;
}

EWeatherCondition UEnvironmentalHazardsSystem::CalculateNextWeatherCondition() const
{
    EWeatherCondition CurrentCondition = CurrentWeather.CurrentCondition;

    if (WeatherTransitionProbabilities.Contains(CurrentCondition))
    {
        const FWeatherTransitionProbabilities& TransitionData = WeatherTransitionProbabilities[CurrentCondition];
        const TMap<EWeatherCondition, float>& Transitions = TransitionData.Transitions;

        float RandomValue = FMath::RandRange(0.0f, 1.0f);
        float CumulativeProbability = 0.0f;

        for (const auto& Transition : Transitions)
        {
            CumulativeProbability += Transition.Value;
            if (RandomValue <= CumulativeProbability)
            {
                return Transition.Key;
            }
        }
    }

    // Default to clear weather if no transitions defined
    return EWeatherCondition::Clear;
}

TArray<FHazardDefense> UEnvironmentalHazardsSystem::GetActiveDefensesInRadius(const FVector& Location, float Radius) const
{
    TArray<FHazardDefense> DefensesInRadius;
    
    for (const FHazardDefense& Defense : DefenseSystems)
    {
        if (Defense.bIsActive && Defense.bIsOperational)
        {
            float Distance = FVector::Dist(Location, Defense.Location);
            if (Distance <= (Radius + Defense.ProtectionRadius))
            {
                DefensesInRadius.Add(Defense);
            }
        }
    }
    
    return DefensesInRadius;
}

float UEnvironmentalHazardsSystem::CalculateTotalDefenseEffectiveness(const TArray<FHazardDefense>& Defenses, EHazardType HazardType) const
{
    float TotalEffectiveness = 0.0f;
    
    for (const FHazardDefense& Defense : Defenses)
    {
        if (Defense.ProtectedHazards.Contains(HazardType))
        {
            TotalEffectiveness += Defense.Effectiveness;
        }
    }
    
    return FMath::Clamp(TotalEffectiveness, 0.0f, 1.0f);
}

// Find functions
FHazardEvent* UEnvironmentalHazardsSystem::FindHazardEvent(int32 EventID)
{
    for (FHazardEvent& HazardEvent : ActiveHazards)
    {
        if (HazardEvent.EventID == EventID)
        {
            return &HazardEvent;
        }
    }
    return nullptr;
}

FHazardDefense* UEnvironmentalHazardsSystem::FindDefenseSystem(int32 DefenseID)
{
    for (FHazardDefense& Defense : DefenseSystems)
    {
        if (Defense.DefenseID == DefenseID)
        {
            return &Defense;
        }
    }
    return nullptr;
}

FHazardPrediction* UEnvironmentalHazardsSystem::FindHazardPrediction(int32 PredictionID)
{
    for (FHazardPrediction& Prediction : HazardPredictions)
    {
        if (Prediction.PredictionID == PredictionID)
        {
            return &Prediction;
        }
    }
    return nullptr;
}

FEnvironmentalZone* UEnvironmentalHazardsSystem::FindEnvironmentalZone(int32 ZoneID)
{
    for (FEnvironmentalZone& Zone : EnvironmentalZones)
    {
        if (Zone.ZoneID == ZoneID)
        {
            return &Zone;
        }
    }
    return nullptr;
}

// Helper functions for timers and setup
void UEnvironmentalHazardsSystem::SetupEnvironmentalTimers()
{
    if (!GetOwner()->HasAuthority()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Note: UpdateHazardEvents is called from TickComponent, not from timer
    // Timers are set up but not used for UpdateHazardEvents in this implementation
    // as it's already called from TickComponent

    // Set up weather update timer
    World->GetTimerManager().SetTimer(
        WeatherUpdateTimer,
        this,
        &UEnvironmentalHazardsSystem::UpdateWeather,
        WeatherUpdateInterval,
        true
    );

    // Set up prediction update timer
    World->GetTimerManager().SetTimer(
        PredictionUpdateTimer,
        this,
        &UEnvironmentalHazardsSystem::GenerateHazardPredictions,
        PredictionUpdateInterval,
        true
    );
}

void UEnvironmentalHazardsSystem::SetInitialWeather()
{
    CurrentWeather.CurrentCondition = EWeatherCondition::Clear;
    CurrentWeather.CurrentTemperature = 20.0f;
    CurrentWeather.CurrentHumidity = 50.0f;
    CurrentWeather.CurrentPressure = 1013.25f;
    CurrentWeather.CurrentWindSpeed = 5.0f;
    CurrentWeather.CurrentVisibility = 10.0f;
    CurrentWeather.TemperatureZone = ETemperatureZone::Temperate;
    CurrentWeather.LastUpdateTime = FDateTime::Now();
    CurrentWeather.NextForecastTime = FDateTime::Now() + FTimespan::FromSeconds(WeatherUpdateInterval);
}

// Additional helper functions
float UEnvironmentalHazardsSystem::CalculateHazardDamage(EHazardType HazardType, EHazardSeverity Severity) const
{
    return GetHazardDamageMultiplier(HazardType, Severity) * 10.0f;
}

FString UEnvironmentalHazardsSystem::GenerateHazardWarning(EHazardType HazardType, EHazardSeverity Severity) const
{
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            return TEXT("Seek shelter immediately. Radiation levels are dangerous.");
        case EHazardType::SolarFlare:
            return TEXT("Communication systems may be disrupted. Power fluctuations expected.");
        case EHazardType::MeteorShower:
            return TEXT("Take cover in reinforced structures. Impact damage likely.");
        case EHazardType::DustStorm:
            return TEXT("Visibility reduced. External equipment may be damaged.");
        case EHazardType::ExtremeTemperature:
            return TEXT("Temperature control systems under stress. Conserve power.");
        default:
            return TEXT("Hazard detected. Take appropriate precautions.");
    }
}

FString UEnvironmentalHazardsSystem::GenerateHazardDescription(EHazardType HazardType, EHazardSeverity Severity) const
{
    return FString::Printf(TEXT("A %s severity %s has been detected in the area."), 
                           *GetSeverityName(Severity), *GetHazardName(HazardType));
}

TArray<FString> UEnvironmentalHazardsSystem::GetRequiredDefenses(EHazardType HazardType) const
{
    TArray<FString> RequiredDefenses;
    
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            RequiredDefenses.Add(TEXT("Radiation Shielding"));
            RequiredDefenses.Add(TEXT("Lead Lining"));
            break;
        case EHazardType::SolarFlare:
            RequiredDefenses.Add(TEXT("EMP Protection"));
            RequiredDefenses.Add(TEXT("Power Surge Protectors"));
            break;
        case EHazardType::MeteorShower:
            RequiredDefenses.Add(TEXT("Impact Shields"));
            RequiredDefenses.Add(TEXT("Reinforced Structures"));
            break;
        case EHazardType::DustStorm:
            RequiredDefenses.Add(TEXT("Dust Filters"));
            RequiredDefenses.Add(TEXT("Sealed Structures"));
            break;
        default:
            RequiredDefenses.Add(TEXT("General Protection"));
            break;
    }
    
    return RequiredDefenses;
}

TMap<FString, float> UEnvironmentalHazardsSystem::CalculateMitigationCosts(EHazardType HazardType, EHazardSeverity Severity) const
{
    TMap<FString, float> Costs;
    
    float BaseCost = GetHazardDamageMultiplier(HazardType, Severity) * 100.0f;
    
    Costs.Add(TEXT("Metals"), BaseCost);
    Costs.Add(TEXT("Crystals"), BaseCost * 0.5f);
    Costs.Add(TEXT("Energy"), BaseCost * 2.0f);
    
    return Costs;
}

void UEnvironmentalHazardsSystem::UpdateTemperatureForWeather(EWeatherCondition Condition)
{
    switch (Condition)
    {
        case EWeatherCondition::Clear:
            CurrentWeather.CurrentTemperature = FMath::RandRange(15.0f, 25.0f);
            break;
        case EWeatherCondition::Cloudy:
            CurrentWeather.CurrentTemperature = FMath::RandRange(12.0f, 22.0f);
            break;
        case EWeatherCondition::Storm:
            CurrentWeather.CurrentTemperature = FMath::RandRange(8.0f, 18.0f);
            break;
        case EWeatherCondition::Snow:
        case EWeatherCondition::Blizzard:
            CurrentWeather.CurrentTemperature = FMath::RandRange(-10.0f, 5.0f);
            break;
        default:
            CurrentWeather.CurrentTemperature = FMath::RandRange(10.0f, 30.0f);
            break;
    }
}

void UEnvironmentalHazardsSystem::UpdateWeatherParameters(EWeatherCondition Condition)
{
    switch (Condition)
    {
        case EWeatherCondition::Clear:
            CurrentWeather.CurrentHumidity = FMath::RandRange(30.0f, 60.0f);
            CurrentWeather.CurrentWindSpeed = FMath::RandRange(2.0f, 8.0f);
            CurrentWeather.CurrentVisibility = FMath::RandRange(8.0f, 15.0f);
            break;
        case EWeatherCondition::Storm:
            CurrentWeather.CurrentHumidity = FMath::RandRange(70.0f, 95.0f);
            CurrentWeather.CurrentWindSpeed = FMath::RandRange(15.0f, 35.0f);
            CurrentWeather.CurrentVisibility = FMath::RandRange(1.0f, 5.0f);
            break;
        case EWeatherCondition::Fog:
            CurrentWeather.CurrentHumidity = FMath::RandRange(85.0f, 100.0f);
            CurrentWeather.CurrentWindSpeed = FMath::RandRange(0.5f, 3.0f);
            CurrentWeather.CurrentVisibility = FMath::RandRange(0.1f, 1.0f);
            break;
        default:
            CurrentWeather.CurrentHumidity = FMath::RandRange(40.0f, 80.0f);
            CurrentWeather.CurrentWindSpeed = FMath::RandRange(5.0f, 15.0f);
            CurrentWeather.CurrentVisibility = FMath::RandRange(5.0f, 12.0f);
            break;
    }
}

void UEnvironmentalHazardsSystem::DetermineZoneProperties(FEnvironmentalZone& Zone)
{
    // Determine zone properties based on distance from center
    float DistanceFromCenter = FVector::Dist(Zone.CenterLocation, GetOwner()->GetActorLocation());
    
    if (DistanceFromCenter < 1500.0f)
    {
        Zone.TemperatureZone = ETemperatureZone::Temperate;
        Zone.TypicalWeather = EWeatherCondition::Clear;
        Zone.bIsHabitable = true;
        Zone.BaseRadiationLevel = 0.1f;
    }
    else if (DistanceFromCenter < 3000.0f)
    {
        Zone.TemperatureZone = ETemperatureZone::Cold;
        Zone.TypicalWeather = EWeatherCondition::Cloudy;
        Zone.bIsHabitable = true;
        Zone.bRequiresSpecialEquipment = true;
        Zone.BaseRadiationLevel = 0.2f;
    }
    else
    {
        Zone.TemperatureZone = ETemperatureZone::Frozen;
        Zone.TypicalWeather = EWeatherCondition::Blizzard;
        Zone.bIsHabitable = false;
        Zone.bRequiresSpecialEquipment = true;
        Zone.BaseRadiationLevel = 0.5f;
    }
    
    // Set common hazards for this zone
    Zone.CommonHazards.Add(EHazardType::ExtremeTemperature);
    Zone.CommonHazards.Add(EHazardType::RadiationStorm);
    
    // Set hazard probabilities
    for (EHazardType HazardType : Zone.CommonHazards)
    {
        Zone.HazardProbabilities.Add(HazardType, FMath::RandRange(0.1f, 0.3f));
    }
}

float UEnvironmentalHazardsSystem::CalculateForecastTemperature(EWeatherCondition Condition) const
{
    switch (Condition)
    {
        case EWeatherCondition::Clear:
            return FMath::RandRange(18.0f, 28.0f);
        case EWeatherCondition::Cloudy:
            return FMath::RandRange(15.0f, 25.0f);
        case EWeatherCondition::Storm:
            return FMath::RandRange(10.0f, 20.0f);
        case EWeatherCondition::Snow:
            return FMath::RandRange(-5.0f, 5.0f);
        default:
            return CurrentWeather.CurrentTemperature;
    }
}

float UEnvironmentalHazardsSystem::CalculateDefensePowerConsumption(EHazardType HazardType, float Radius) const
{
    float BaseConsumption = 10.0f;
    
    // Modify based on hazard type
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            BaseConsumption = 25.0f;
            break;
        case EHazardType::SolarFlare:
            BaseConsumption = 30.0f;
            break;
        case EHazardType::MeteorShower:
            BaseConsumption = 20.0f;
            break;
        default:
            BaseConsumption = 15.0f;
            break;
    }
    
    // Scale by radius
    return BaseConsumption * (Radius / 500.0f);
}

float UEnvironmentalHazardsSystem::CalculateDefenseMaintenanceCost(EHazardType HazardType, float Radius) const
{
    float BaseCost = 5.0f;
    
    // Modify based on hazard type
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            BaseCost = 12.0f;
            break;
        case EHazardType::SolarFlare:
            BaseCost = 15.0f;
            break;
        case EHazardType::MeteorShower:
            BaseCost = 10.0f;
            break;
        default:
            BaseCost = 8.0f;
            break;
    }
    
    // Scale by radius
    return BaseCost * (Radius / 500.0f);
}

void UEnvironmentalHazardsSystem::ApplyDefenseUpgrade(FHazardDefense& Defense, const FString& UpgradeType)
{
    if (UpgradeType == TEXT("Enhanced Shielding"))
    {
        Defense.Effectiveness = FMath::Min(2.0f, Defense.Effectiveness * 1.3f);
        Defense.MaxHealth *= 1.2f;
        Defense.CurrentHealth = Defense.MaxHealth;
    }
    else if (UpgradeType == TEXT("Extended Range"))
    {
        Defense.ProtectionRadius *= 1.5f;
        Defense.PowerConsumption *= 1.2f;
    }
    else if (UpgradeType == TEXT("Efficiency Upgrade"))
    {
        Defense.PowerConsumption *= 0.7f;
        Defense.MaintenanceCost *= 0.8f;
    }
}

float UEnvironmentalHazardsSystem::GetRequiredEffectiveness(EHazardSeverity Severity) const
{
    switch (Severity)
    {
        case EHazardSeverity::Low: return 0.3f;
        case EHazardSeverity::Medium: return 0.5f;
        case EHazardSeverity::High: return 0.7f;
        case EHazardSeverity::Extreme: return 0.85f;
        case EHazardSeverity::Catastrophic: return 0.95f;
        default: return 0.5f;
    }
}

TArray<FString> UEnvironmentalHazardsSystem::GetPredictionSources(EHazardType HazardType) const
{
    TArray<FString> Sources;
    
    Sources.Add(TEXT("Environmental Sensors"));
    Sources.Add(TEXT("Satellite Monitoring"));
    
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            Sources.Add(TEXT("Radiation Detectors"));
            break;
        case EHazardType::SolarFlare:
            Sources.Add(TEXT("Solar Observatory"));
            break;
        case EHazardType::MeteorShower:
            Sources.Add(TEXT("Asteroid Tracking"));
            break;
        case EHazardType::SeismicActivity:
            Sources.Add(TEXT("Seismic Sensors"));
            break;
        default:
            break;
    }
    
    return Sources;
}

FString UEnvironmentalHazardsSystem::GetPredictionMethod(EHazardType HazardType) const
{
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            return TEXT("Radiation Level Analysis");
        case EHazardType::SolarFlare:
            return TEXT("Solar Activity Monitoring");
        case EHazardType::MeteorShower:
            return TEXT("Orbital Mechanics Calculation");
        case EHazardType::DustStorm:
            return TEXT("Atmospheric Pattern Analysis");
        case EHazardType::SeismicActivity:
            return TEXT("Seismic Pattern Recognition");
        default:
            return TEXT("Statistical Analysis");
    }
}

bool UEnvironmentalHazardsSystem::VerifyPredictionAccuracy(const FHazardPrediction& Prediction) const
{
    // Check if a similar hazard event occurred around the predicted time
    FDateTime PredictionWindowStart = Prediction.PredictedStartTime - FTimespan::FromMinutes(30);
    FDateTime PredictionWindowEnd = Prediction.PredictedStartTime + FTimespan::FromMinutes(30);
    
    for (const FHazardEvent& HazardEvent : ActiveHazards)
    {
        if (HazardEvent.HazardType == Prediction.PredictedHazardType &&
            HazardEvent.StartTime >= PredictionWindowStart &&
            HazardEvent.StartTime <= PredictionWindowEnd)
        {
            float LocationError = FVector::Dist(HazardEvent.Location, Prediction.PredictedLocation);
            if (LocationError <= Prediction.PredictedRadius * 2.0f) // Within 2x predicted radius
            {
                return true;
            }
        }
    }
    
    return false;
}

void UEnvironmentalHazardsSystem::UpdateAverageStatistics()
{
    // Update average temperature
    EnvironmentalStats.AverageTemperature = CurrentWeather.CurrentTemperature;
    
    // Update average radiation level
    float TotalRadiation = 0.1f; // Base radiation level
    for (const FEnvironmentalZone& Zone : EnvironmentalZones)
    {
        TotalRadiation += Zone.BaseRadiationLevel;
    }
    EnvironmentalStats.AverageRadiationLevel = TotalRadiation / EnvironmentalZones.Num();
}

float UEnvironmentalHazardsSystem::CalculateAverageDefenseEffectiveness() const
{
    if (DefenseSystems.Num() == 0) return 0.0f;
    
    float TotalEffectiveness = 0.0f;
    for (const FHazardDefense& Defense : DefenseSystems)
    {
        if (Defense.bIsActive && Defense.bIsOperational)
        {
            TotalEffectiveness += Defense.Effectiveness;
        }
    }
    
    return TotalEffectiveness / DefenseSystems.Num();
}

void UEnvironmentalHazardsSystem::ApplyDamageToSystem(const FString& SystemName, float Damage)
{
    // This would interface with other systems to apply damage
    // For now, just log the damage
    UE_LOG(LogTemp, Log, TEXT("Applied %f damage to system: %s"), Damage, *SystemName);
}

void UEnvironmentalHazardsSystem::ApplyEfficiencyModifier(const FString& SystemName, float Modifier)
{
    // This would interface with other systems to apply efficiency modifiers
    // For now, just log the modifier
    UE_LOG(LogTemp, Log, TEXT("Applied %f efficiency modifier to system: %s"), Modifier, *SystemName);
}

void UEnvironmentalHazardsSystem::UpdateAffectedBuildings(FHazardEvent& HazardEvent, float Damage)
{
    // Update list of affected buildings
    HazardEvent.AffectedBuildings.Empty();
    
    // This would check for buildings in the hazard radius
    // For now, add placeholder building names
    HazardEvent.AffectedBuildings.Add(TEXT("Building_1"));
    HazardEvent.AffectedBuildings.Add(TEXT("Building_2"));
}

float UEnvironmentalHazardsSystem::CalculateHazardImpact(const FHazardEvent& HazardEvent, const FVector& Location) const
{
    float Distance = FVector::Dist(Location, HazardEvent.Location);
    if (Distance > HazardEvent.Radius)
    {
        return 0.0f;
    }
    
    // Calculate impact based on distance from hazard center
    float DistanceRatio = Distance / HazardEvent.Radius;
    float Impact = (1.0f - DistanceRatio) * GetHazardDamageMultiplier(HazardEvent.HazardType, HazardEvent.Severity);
    
    return Impact;
}

float UEnvironmentalHazardsSystem::CalculateWeatherImpact(const FWeatherData& Weather, const FVector& Location) const
{
    float Impact = 0.0f;
    
    // Calculate impact based on weather condition
    switch (Weather.CurrentCondition)
    {
        case EWeatherCondition::Storm:
            Impact = 0.3f;
            break;
        case EWeatherCondition::Blizzard:
            Impact = 0.5f;
            break;
        case EWeatherCondition::AcidRain:
            Impact = 0.2f;
            break;
        case EWeatherCondition::ToxicFog:
            Impact = 0.4f;
            break;
        default:
            Impact = 0.0f;
            break;
    }
    
    // Modify impact based on temperature extremes
    if (Weather.CurrentTemperature < -20.0f || Weather.CurrentTemperature > 45.0f)
    {
        Impact += 0.2f;
    }
    
    return Impact;
}

FString UEnvironmentalHazardsSystem::GetDefenseNameForHazard(EHazardType HazardType) const
{
    switch (HazardType)
    {
        case EHazardType::RadiationStorm:
            return TEXT("Radiation Shielding Array");
        case EHazardType::SolarFlare:
            return TEXT("EMP Protection System");
        case EHazardType::MeteorShower:
            return TEXT("Impact Defense Grid");
        case EHazardType::DustStorm:
            return TEXT("Dust Protection Field");
        case EHazardType::ExtremeTemperature:
            return TEXT("Thermal Regulation System");
        case EHazardType::AtmosphericPressure:
            return TEXT("Pressure Stabilizer");
        case EHazardType::SeismicActivity:
            return TEXT("Seismic Dampeners");
        case EHazardType::AcidRain:
            return TEXT("Acid Repellent Coating");
        case EHazardType::MagneticStorm:
            return TEXT("Magnetic Field Generator");
        case EHazardType::GravityAnomaly:
            return TEXT("Gravity Stabilizer");
        case EHazardType::BiologicalHazard:
            return TEXT("Bio-Containment Field");
        default:
            return TEXT("General Defense System");
    }
}

