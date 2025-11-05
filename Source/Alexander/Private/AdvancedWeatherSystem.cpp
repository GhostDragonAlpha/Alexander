// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedWeatherSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UAdvancedWeatherSystem::UAdvancedWeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz update for smooth weather changes

    // Configuration defaults
    bEnableStormGeneration = true;
    bEnableWeatherEvents = true;
    bEnableWeatherPrediction = true;
    bEnableClimateZones = true;
    StormGenerationProbability = DEFAULT_STORM_GENERATION_PROBABILITY;
    WeatherEventProbability = DEFAULT_WEATHER_EVENT_PROBABILITY;
    PredictionUpdateInterval = DEFAULT_PREDICTION_UPDATE_INTERVAL;
    MaxActiveStorms = DEFAULT_MAX_ACTIVE_STORMS;
    MaxActiveWeatherEvents = DEFAULT_MAX_ACTIVE_WEATHER_EVENTS;
    PredictionAccuracy = DEFAULT_PREDICTION_ACCURACY;
}

void UAdvancedWeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeClimateZones();
    InitializeDefaultWeatherPatterns();
    SetupTimers();
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedWeatherSystem initialized"));
}

void UAdvancedWeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableStormGeneration)
    {
        UpdateStormSystem(DeltaTime);
    }
    
    if (bEnableWeatherEvents)
    {
        UpdateWeatherEvents(DeltaTime);
    }
    
    if (bEnableWeatherPrediction)
    {
        UpdatePredictionSystem(DeltaTime);
    }
    
    ProcessActivePatterns(DeltaTime);
    
    // Synchronize with planet weather component
    if (PlanetWeatherComponent)
    {
        SynchronizeWithPlanetWeather();
    }
}

void UAdvancedWeatherSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(StormUpdateTimer);
        World->GetTimerManager().ClearTimer(WeatherEventUpdateTimer);
        World->GetTimerManager().ClearTimer(PredictionUpdateTimer);
        World->GetTimerManager().ClearTimer(PatternUpdateTimer);
    }

    // Clear all data
    ClimateZones.Empty();
    ClimateZoneMap.Empty();
    ActiveStorms.Empty();
    PredictedStorms.Empty();
    ActiveWeatherEvents.Empty();
    PredictedEvents.Empty();
    CurrentPredictions.Empty();
    RegisteredPatterns.Empty();
    ActivePatterns.Empty();

    Super::EndPlay(EndPlayReason);
}

// ===== Climate Zone Management =====

void UAdvancedWeatherSystem::InitializeClimateZones()
{
    if (!bEnableClimateZones)
    {
        return;
    }
    
    InitializeDefaultClimateZones();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d climate zones"), ClimateZones.Num());
}

void UAdvancedWeatherSystem::RegisterClimateZone(const FClimateZoneData& ClimateZone)
{
    if (ClimateZoneMap.Contains(ClimateZone.ZoneType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Climate zone %d already registered"), (int32)ClimateZone.ZoneType);
        return;
    }
    
    ClimateZones.Add(ClimateZone);
    ClimateZoneMap.Add(ClimateZone.ZoneType, ClimateZone);
    
    UE_LOG(LogTemp, Log, TEXT("Registered climate zone: %d"), (int32)ClimateZone.ZoneType);
}

EClimateZone UAdvancedWeatherSystem::GetClimateZoneAtLocation(const FVector2D& Location) const
{
    for (const FClimateZoneData& Zone : ClimateZones)
    {
        if (IsPointInClimateZone(Location, Zone))
        {
            return Zone.ZoneType;
        }
    }
    
    return EClimateZone::Temperate; // Default fallback
}

FClimateZoneData* UAdvancedWeatherSystem::GetClimateZoneData(EClimateZone ZoneType)
{
    return ClimateZoneMap.Find(ZoneType);
}

// ===== Storm System Management =====

void UAdvancedWeatherSystem::GenerateStorm(const FVector2D& Location, EStormIntensity Intensity)
{
    if (ActiveStorms.Num() >= MaxActiveStorms)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum number of active storms reached"));
        return;
    }
    
    FStormSystem NewStorm;
    NewStorm.Intensity = Intensity;
    NewStorm.CenterLocation = Location;
    NewStorm.Radius = FMath::FRandRange(5000.0f, 20000.0f) * (1.0f + (int32)Intensity * 0.5f);
    NewStorm.MaxWindSpeed = 50.0f * (1.0f + (int32)Intensity * 0.8f);
    NewStorm.MovementSpeed = FMath::FRandRange(5.0f, 20.0f);
    NewStorm.MovementDirection = FMath::FRandRange(0.0f, 360.0f);
    NewStorm.Lifetime = FMath::FRandRange(3600.0f, 14400.0f); // 1-4 hours
    NewStorm.ElapsedTime = 0.0f;
    NewStorm.bIsDissipating = false;
    
    // Generate storm path
    int32 PathPoints = FMath::RandRange(5, 15);
    for (int32 i = 0; i < PathPoints; i++)
    {
        FVector2D PathPoint = NewStorm.CenterLocation;
        PathPoint.X += FMath::FRandRange(-50000.0f, 50000.0f);
        PathPoint.Y += FMath::FRandRange(-50000.0f, 50000.0f);
        NewStorm.StormPath.Add(PathPoint);
    }
    
    ActiveStorms.Add(NewStorm);
    
    OnStormGenerated.Broadcast(NewStorm, Location);
    
    UE_LOG(LogTemp, Log, TEXT("Generated storm with intensity %d at location (%f, %f)"), 
           (int32)Intensity, Location.X, Location.Y);
}

void UAdvancedWeatherSystem::UpdateStormSystem(float DeltaTime)
{
    // Update existing storms
    for (int32 i = ActiveStorms.Num() - 1; i >= 0; i--)
    {
        FStormSystem& Storm = ActiveStorms[i];
        UpdateStormMovement(Storm, DeltaTime);
        
        Storm.ElapsedTime += DeltaTime;
        
        // Check if storm should dissipate
        if (Storm.ElapsedTime >= Storm.Lifetime || Storm.bIsDissipating)
        {
            DissipateStorm(Storm);
            ActiveStorms.RemoveAt(i);
        }
    }
    
    // Generate new storms
    if (ShouldGenerateStorm())
    {
        GenerateRandomStorm();
    }
}

TArray<FStormSystem> UAdvancedWeatherSystem::GetActiveStorms() const
{
    return ActiveStorms;
}

bool UAdvancedWeatherSystem::IsLocationInStorm(const FVector2D& Location) const
{
    for (const FStormSystem& Storm : ActiveStorms)
    {
        if (GetDistance(Location, Storm.CenterLocation) <= Storm.Radius)
        {
            return true;
        }
    }
    return false;
}

FStormSystem* UAdvancedWeatherSystem::GetStormAtLocation(const FVector2D& Location)
{
    for (FStormSystem& Storm : ActiveStorms)
    {
        if (GetDistance(Location, Storm.CenterLocation) <= Storm.Radius)
        {
            return &Storm;
        }
    }
    return nullptr;
}

float UAdvancedWeatherSystem::GetWindSpeedAtLocation(const FVector2D& Location) const
{
    float MaxWindSpeed = 0.0f;
    
    for (const FStormSystem& Storm : ActiveStorms)
    {
        float Distance = GetDistance(Location, Storm.CenterLocation);
        if (Distance <= Storm.Radius)
        {
            float Influence = 1.0f - (Distance / Storm.Radius);
            float WindSpeed = Storm.MaxWindSpeed * Influence;
            MaxWindSpeed = FMath::Max(MaxWindSpeed, WindSpeed);
        }
    }
    
    return MaxWindSpeed;
}

FVector UAdvancedWeatherSystem::GetWindDirectionAtLocation(const FVector2D& Location) const
{
    FVector WindDirection = FVector::ZeroVector;
    
    for (const FStormSystem& Storm : ActiveStorms)
    {
        float Distance = GetDistance(Location, Storm.CenterLocation);
        if (Distance <= Storm.Radius)
        {
            float Influence = 1.0f - (Distance / Storm.Radius);
            FVector StormDirection = FVector(FMath::Cos(FMath::DegreesToRadians(Storm.MovementDirection)),
                                         FMath::Sin(FMath::DegreesToRadians(Storm.MovementDirection)), 0.0f);
            WindDirection += StormDirection * Influence;
        }
    }
    
    return WindDirection.GetSafeNormal();
}

// ===== Weather Event Management =====

void UAdvancedWeatherSystem::TriggerWeatherEvent(const FWeatherEvent& Event)
{
    if (ActiveWeatherEvents.Num() >= MaxActiveWeatherEvents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum number of active weather events reached"));
        return;
    }
    
    FWeatherEvent NewEvent = Event;
    NewEvent.StartTime = GetWorld()->GetTimeSeconds();
    NewEvent.bIsActive = true;
    
    ActiveWeatherEvents.Add(NewEvent);
    
    OnWeatherEventTriggered.Broadcast(NewEvent, Event.EventLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered weather event %d at location (%f, %f)"), 
           (int32)Event.EventType, Event.EventLocation.X, Event.EventLocation.Y);
}

void UAdvancedWeatherSystem::UpdateWeatherEvents(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update existing events
    for (int32 i = ActiveWeatherEvents.Num() - 1; i >= 0; i--)
    {
        FWeatherEvent& Event = ActiveWeatherEvents[i];
        UpdateWeatherEvent(Event, DeltaTime);
        
        // Check if event should end
        if (CurrentTime - Event.StartTime >= Event.Duration)
        {
            Event.bIsActive = false;
            ActiveWeatherEvents.RemoveAt(i);
        }
    }
    
    // Generate new events
    if (ShouldGenerateWeatherEvent())
    {
        GenerateRandomWeatherEvent();
    }
}

TArray<FWeatherEvent> UAdvancedWeatherSystem::GetActiveWeatherEvents() const
{
    return ActiveWeatherEvents;
}

bool UAdvancedWeatherSystem::IsLocationAffectedByEvent(const FVector2D& Location) const
{
    for (const FWeatherEvent& Event : ActiveWeatherEvents)
    {
        if (Event.bIsActive && GetDistance(Location, Event.EventLocation) <= Event.Radius)
        {
            return true;
        }
    }
    return false;
}

FWeatherEvent* UAdvancedWeatherSystem::GetWeatherEventAtLocation(const FVector2D& Location)
{
    for (FWeatherEvent& Event : ActiveWeatherEvents)
    {
        if (Event.bIsActive && GetDistance(Location, Event.EventLocation) <= Event.Radius)
        {
            return &Event;
        }
    }
    return nullptr;
}

void UAdvancedWeatherSystem::EndWeatherEvent(const FName& EventID)
{
    for (int32 i = 0; i < ActiveWeatherEvents.Num(); i++)
    {
        // Note: FWeatherEvent doesn't have an ID field, so this would need to be added
        // For now, we'll end the first event we find
        if (ActiveWeatherEvents[i].bIsActive)
        {
            ActiveWeatherEvents[i].bIsActive = false;
            ActiveWeatherEvents.RemoveAt(i);
            break;
        }
    }
}

// ===== Weather Prediction System =====

TArray<FWeatherPrediction> UAdvancedWeatherSystem::GenerateWeatherPredictions(float PredictionHours)
{
    TArray<FWeatherPrediction> Predictions;
    
    // Generate predictions for different time intervals
    TArray<float> TimeIntervals = {1.0f, 3.0f, 6.0f, 12.0f, 24.0f};
    
    for (float Interval : TimeIntervals)
    {
        if (Interval <= PredictionHours)
        {
            // Generate predictions for key locations
            for (const FClimateZoneData& Zone : ClimateZones)
            {
                FVector2D ZoneCenter = GetRandomLocationInClimateZone(Zone.ZoneType);
                FWeatherPrediction Prediction = PredictWeatherAtLocation(ZoneCenter, Interval);
                Predictions.Add(Prediction);
            }
        }
    }
    
    CurrentPredictions = Predictions;
    OnWeatherPredictionUpdated.Broadcast(Predictions);
    
    return Predictions;
}

FWeatherPrediction UAdvancedWeatherSystem::GetPredictionAtLocation(const FVector2D& Location, float HoursAhead)
{
    return PredictWeatherAtLocation(Location, HoursAhead);
}

void UAdvancedWeatherSystem::UpdatePredictionSystem(float DeltaTime)
{
    // Update predictions periodically
    static float AccumulatedTime = 0.0f;
    AccumulatedTime += DeltaTime;
    
    if (AccumulatedTime >= PredictionUpdateInterval)
    {
        GenerateWeatherPredictions(24.0f);
        AccumulatedTime = 0.0f;
    }
}

// ===== Weather Pattern System =====

void UAdvancedWeatherSystem::RegisterWeatherPattern(const FWeatherPattern& Pattern)
{
    for (const FWeatherPattern& ExistingPattern : RegisteredPatterns)
    {
        if (ExistingPattern.PatternName == Pattern.PatternName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Weather pattern '%s' already registered"), *Pattern.PatternName.ToString());
            return;
        }
    }
    
    RegisteredPatterns.Add(Pattern);
    UE_LOG(LogTemp, Log, TEXT("Registered weather pattern: %s"), *Pattern.PatternName.ToString());
}

void UAdvancedWeatherSystem::ActivateWeatherPattern(const FName& PatternName)
{
    for (const FWeatherPattern& Pattern : RegisteredPatterns)
    {
        if (Pattern.PatternName == PatternName)
        {
            ActivePatterns.Add(Pattern);
            OnWeatherPatternActivated.Broadcast(Pattern, PatternName);
            UE_LOG(LogTemp, Log, TEXT("Activated weather pattern: %s"), *PatternName.ToString());
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Weather pattern '%s' not found"), *PatternName.ToString());
}

void UAdvancedWeatherSystem::DeactivateWeatherPattern(const FName& PatternName)
{
    for (int32 i = 0; i < ActivePatterns.Num(); i++)
    {
        if (ActivePatterns[i].PatternName == PatternName)
        {
            ActivePatterns.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Deactivated weather pattern: %s"), *PatternName.ToString());
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Active weather pattern '%s' not found"), *PatternName.ToString());
}

TArray<FWeatherPattern> UAdvancedWeatherSystem::GetActivePatterns() const
{
    return ActivePatterns;
}

FWeatherPattern* UAdvancedWeatherSystem::GetPattern(const FName& PatternName)
{
    for (FWeatherPattern& Pattern : RegisteredPatterns)
    {
        if (Pattern.PatternName == PatternName)
        {
            return &Pattern;
        }
    }
    return nullptr;
}

// ===== Player-Affecting Weather Events =====

void UAdvancedWeatherSystem::ApplyWeatherEffectsToPlayer(class ACharacter* Player, const FVector2D& PlayerLocation)
{
    if (!Player)
    {
        return;
    }
    
    // Check for storm effects
    if (IsLocationInStorm(PlayerLocation))
    {
        FStormSystem* Storm = GetStormAtLocation(PlayerLocation);
        if (Storm)
        {
            // Apply wind force to player
            FVector WindDirection = GetWindDirectionAtLocation(PlayerLocation);
            float WindSpeed = GetWindSpeedAtLocation(PlayerLocation);
            
            if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
            {
                FVector WindForce = WindDirection * WindSpeed * 10.0f;
                MovementComp->AddForce(WindForce);
            }
        }
    }
    
    // Check for weather event effects
    if (FWeatherEvent* Event = GetWeatherEventAtLocation(PlayerLocation))
    {
        switch (Event->EventType)
        {
        case EWeatherEventType::Blizzard:
            // Reduce visibility and movement speed
            if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed *= 0.5f;
            }
            break;
            
        case EWeatherEventType::Heatwave:
            // Apply heat damage over time
            {
                float Temperature = GetTemperatureAtLocation(PlayerLocation);
                float HeatDamage = 0.0f;

                // Calculate damage based on temperature
                if (Temperature > 40.0f) // Above 40°C starts dealing damage
                {
                    HeatDamage = (Temperature - 40.0f) * 0.1f; // Scale damage with temperature

                    // Check for heat protection equipment
                    // In a real implementation, check player's equipment/suit
                    bool bHasHeatProtection = false; // Placeholder

                    if (!bHasHeatProtection)
                    {
                        // Apply heat damage to player
                        if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
                        {
                            // Reduce stamina/speed in extreme heat
                            MovementComp->MaxWalkSpeed *= 0.8f;
                        }

                        // Apply actual damage (would integrate with health system)
                        UE_LOG(LogTemp, Warning, TEXT("Player taking heat damage: %.2f (Temp: %.1f°C)"), HeatDamage, Temperature);
                    }
                }
            }
            break;

        case EWeatherEventType::Coldwave:
            // Apply cold damage and effects
            {
                float Temperature = GetTemperatureAtLocation(PlayerLocation);
                float ColdDamage = 0.0f;

                // Calculate damage based on temperature
                if (Temperature < -10.0f) // Below -10°C starts dealing damage
                {
                    ColdDamage = (-10.0f - Temperature) * 0.1f; // Scale damage with cold

                    // Check for cold protection equipment
                    bool bHasColdProtection = false; // Placeholder

                    if (!bHasColdProtection)
                    {
                        // Apply cold effects to player
                        if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
                        {
                            // Slow movement in extreme cold
                            MovementComp->MaxWalkSpeed *= 0.6f;
                        }

                        // Apply hypothermia effects
                        UE_LOG(LogTemp, Warning, TEXT("Player taking cold damage: %.2f (Temp: %.1f°C)"), ColdDamage, Temperature);
                    }
                }
            }
            break;
            
        default:
            break;
        }
    }
}

float UAdvancedWeatherSystem::GetTemperatureAtLocation(const FVector2D& Location) const
{
    return CalculateTemperatureAtLocation(Location);
}

float UAdvancedWeatherSystem::GetHumidityAtLocation(const FVector2D& Location) const
{
    return CalculateHumidityAtLocation(Location);
}

float UAdvancedWeatherSystem::GetVisibilityAtLocation(const FVector2D& Location) const
{
    float Visibility = 1.0f;
    
    // Reduce visibility in storms
    if (IsLocationInStorm(Location))
    {
        Visibility *= 0.3f;
    }
    
    // Reduce visibility in weather events
    if (IsLocationAffectedByEvent(Location))
    {
        if (FWeatherEvent* Event = GetWeatherEventAtLocation(Location))
        {
            switch (Event->EventType)
            {
            case EWeatherEventType::Blizzard:
            case EWeatherEventType::Sandstorm:
                Visibility *= 0.1f;
                break;
            case EWeatherEventType::Fog:
                Visibility *= 0.2f;
                break;
            default:
                break;
            }
        }
    }
    
    return Visibility;
}

bool UAdvancedWeatherSystem::IsLocationSafeForPlayers(const FVector2D& Location) const
{
    // Check for dangerous weather events
    if (FWeatherEvent* Event = GetWeatherEventAtLocation(Location))
    {
        switch (Event->EventType)
        {
        case EWeatherEventType::Hurricane:
        case EWeatherEventType::Tornado:
        case EWeatherEventType::SolarStorm:
            return false;
        default:
            break;
        }
    }
    
    // Check for extreme storms
    if (FStormSystem* Storm = GetStormAtLocation(Location))
    {
        if (Storm->Intensity >= EStormIntensity::Category3)
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UAdvancedWeatherSystem::GetWeatherWarningsAtLocation(const FVector2D& Location) const
{
    TArray<FString> Warnings;
    
    // Check for storm warnings
    if (IsLocationInStorm(Location))
    {
        FStormSystem* Storm = GetStormAtLocation(Location);
        if (Storm)
        {
            Warnings.Add(FString::Printf(TEXT("Storm Warning: Category %d storm with wind speeds up to %.1f km/h"), 
                                      (int32)Storm->Intensity, Storm->MaxWindSpeed));
        }
    }
    
    // Check for weather event warnings
    if (FWeatherEvent* Event = GetWeatherEventAtLocation(Location))
    {
        switch (Event->EventType)
        {
        case EWeatherEventType::Blizzard:
            Warnings.Add(TEXT("Blizzard Warning: Extreme snow and reduced visibility"));
            break;
        case EWeatherEventType::Heatwave:
            Warnings.Add(TEXT("Heat Warning: Extreme temperatures - stay hydrated"));
            break;
        case EWeatherEventType::Coldwave:
            Warnings.Add(TEXT("Cold Warning: Extreme cold - seek shelter"));
            break;
        case EWeatherEventType::Flood:
            Warnings.Add(TEXT("Flood Warning: Rising water levels"));
            break;
        default:
            break;
        }
    }
    
    return Warnings;
}

// ===== Integration with PlanetWeatherComponent =====

void UAdvancedWeatherSystem::SetPlanetWeatherComponent(class UPlanetWeatherComponent* WeatherComponent)
{
    PlanetWeatherComponent = WeatherComponent;
}

void UAdvancedWeatherSystem::SynchronizeWithPlanetWeather()
{
    if (!PlanetWeatherComponent)
    {
        return;
    }
    
    // This would synchronize the advanced weather system with the base planet weather
    // Implementation depends on the specific requirements of the integration
}

void UAdvancedWeatherSystem::OverridePlanetWeatherAtLocation(const FVector2D& Location, EWeatherType WeatherType, float Duration)
{
    if (PlanetWeatherComponent)
    {
        // Override weather at specific location
        // This would require extending the PlanetWeatherComponent to support location-based overrides
    }
}

// ===== Private Functions =====

void UAdvancedWeatherSystem::InitializeDefaultClimateZones()
{
    // Tropical Zone
    FClimateZoneData TropicalZone;
    TropicalZone.ZoneType = EClimateZone::Tropical;
    TropicalZone.LatitudeRange = FVector2D(-23.5f, 23.5f);
    TropicalZone.LongitudeRange = FVector2D(-180.0f, 180.0f);
    TropicalZone.BaseTemperature = 25.0f;
    TropicalZone.TemperatureVariation = 5.0f;
    TropicalZone.BaseHumidity = 0.8f;
    TropicalZone.HumidityVariation = 0.2f;
    TropicalZone.CommonWeatherTypes = {EWeatherType::Clear, EWeatherType::PartlyCloudy, EWeatherType::Rain};
    TropicalZone.PossibleWeatherEvents = {EWeatherEventType::Thunderstorm, EWeatherEventType::Hurricane};
    TropicalZone.WeatherEventProbability = 0.02f;
    RegisterClimateZone(TropicalZone);
    
    // Temperate Zone
    FClimateZoneData TemperateZone;
    TemperateZone.ZoneType = EClimateZone::Temperate;
    TemperateZone.LatitudeRange = FVector2D(23.5f, 66.5f);
    TemperateZone.LongitudeRange = FVector2D(-180.0f, 180.0f);
    TemperateZone.BaseTemperature = 15.0f;
    TemperateZone.TemperatureVariation = 10.0f;
    TemperateZone.BaseHumidity = 0.6f;
    TemperateZone.HumidityVariation = 0.3f;
    TemperateZone.CommonWeatherTypes = {EWeatherType::Clear, EWeatherType::PartlyCloudy, EWeatherType::Cloudy, EWeatherType::Rain};
    TemperateZone.PossibleWeatherEvents = {EWeatherEventType::Thunderstorm, EWeatherEventType::Flood};
    TemperateZone.WeatherEventProbability = 0.015f;
    RegisterClimateZone(TemperateZone);
    
    // Arctic Zone
    FClimateZoneData ArcticZone;
    ArcticZone.ZoneType = EClimateZone::Arctic;
    ArcticZone.LatitudeRange = FVector2D(66.5f, 90.0f);
    ArcticZone.LongitudeRange = FVector2D(-180.0f, 180.0f);
    ArcticZone.BaseTemperature = -10.0f;
    ArcticZone.TemperatureVariation = 15.0f;
    ArcticZone.BaseHumidity = 0.3f;
    ArcticZone.HumidityVariation = 0.2f;
    ArcticZone.CommonWeatherTypes = {EWeatherType::Cloudy, EWeatherType::Snow};
    ArcticZone.PossibleWeatherEvents = {EWeatherEventType::Blizzard, EWeatherEventType::Coldwave};
    ArcticZone.WeatherEventProbability = 0.025f;
    RegisterClimateZone(ArcticZone);
    
    // Desert Zone
    FClimateZoneData DesertZone;
    DesertZone.ZoneType = EClimateZone::Desert;
    DesertZone.LatitudeRange = FVector2D(15.0f, 35.0f);
    DesertZone.LongitudeRange = FVector2D(-180.0f, 180.0f);
    DesertZone.BaseTemperature = 30.0f;
    DesertZone.TemperatureVariation = 15.0f;
    DesertZone.BaseHumidity = 0.2f;
    DesertZone.HumidityVariation = 0.1f;
    DesertZone.CommonWeatherTypes = {EWeatherType::Clear, EWeatherType::PartlyCloudy};
    DesertZone.PossibleWeatherEvents = {EWeatherEventType::Sandstorm, EWeatherEventType::Heatwave};
    DesertZone.WeatherEventProbability = 0.01f;
    RegisterClimateZone(DesertZone);
}

void UAdvancedWeatherSystem::InitializeDefaultWeatherPatterns()
{
    // Monsoon Pattern
    FWeatherPattern MonsoonPattern;
    MonsoonPattern.PatternName = TEXT("Monsoon");
    MonsoonPattern.WeatherSequence = {EWeatherType::PartlyCloudy, EWeatherType::Cloudy, EWeatherType::Rain, EWeatherType::HeavyRain, EWeatherType::Rain, EWeatherType::Cloudy, EWeatherType::PartlyCloudy};
    MonsoonPattern.DurationSequence = {6.0f, 4.0f, 8.0f, 12.0f, 8.0f, 4.0f, 6.0f};
    MonsoonPattern.PatternProbability = 0.1f;
    MonsoonPattern.ApplicableZone = EClimateZone::Tropical;
    MonsoonPattern.PossibleEvents = {EWeatherEventType::Thunderstorm, EWeatherEventType::Flood};
    RegisterWeatherPattern(MonsoonPattern);
    
    // Blizzard Pattern
    FWeatherPattern BlizzardPattern;
    BlizzardPattern.PatternName = TEXT("Blizzard");
    BlizzardPattern.WeatherSequence = {EWeatherType::Cloudy, EWeatherType::Snow, EWeatherType::HeavySnow, EWeatherType::HeavySnow, EWeatherType::Snow, EWeatherType::Cloudy};
    BlizzardPattern.DurationSequence = {4.0f, 6.0f, 12.0f, 12.0f, 6.0f, 4.0f};
    BlizzardPattern.PatternProbability = 0.05f;
    BlizzardPattern.ApplicableZone = EClimateZone::Arctic;
    BlizzardPattern.PossibleEvents = {EWeatherEventType::Blizzard, EWeatherEventType::Coldwave};
    RegisterWeatherPattern(BlizzardPattern);
}

void UAdvancedWeatherSystem::SetupTimers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Storm update timer
    World->GetTimerManager().SetTimer(StormUpdateTimer, [this]()
    {
        float DeltaTime = STORM_UPDATE_INTERVAL;
        UpdateStormSystem(DeltaTime);
    }, STORM_UPDATE_INTERVAL, true);
    
    // Weather event update timer
    World->GetTimerManager().SetTimer(WeatherEventUpdateTimer, [this]()
    {
        float DeltaTime = WEATHER_EVENT_UPDATE_INTERVAL;
        UpdateWeatherEvents(DeltaTime);
    }, WEATHER_EVENT_UPDATE_INTERVAL, true);
    
    // Prediction update timer
    World->GetTimerManager().SetTimer(PredictionUpdateTimer, [this]()
    {
        UpdatePredictionSystem(PredictionUpdateInterval);
    }, PredictionUpdateInterval, true);
    
    // Pattern update timer
    World->GetTimerManager().SetTimer(PatternUpdateTimer, [this]()
    {
        float DeltaTime = PATTERN_UPDATE_INTERVAL;
        ProcessActivePatterns(DeltaTime);
    }, PATTERN_UPDATE_INTERVAL, true);
}

void UAdvancedWeatherSystem::GenerateRandomStorm()
{
    EClimateZone RandomZone = (EClimateZone)FMath::RandRange(0, (int32)EClimateZone::Mountain);
    FVector2D Location = GetRandomLocationInClimateZone(RandomZone);
    EStormIntensity Intensity = CalculateStormIntensity(Location);
    
    GenerateStorm(Location, Intensity);
}

bool UAdvancedWeatherSystem::ShouldGenerateStorm()
{
    return FMath::FRand() <= StormGenerationProbability;
}

EStormIntensity UAdvancedWeatherSystem::CalculateStormIntensity(const FVector2D& Location)
{
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    
    // Different zones have different storm intensity probabilities
    switch (Zone)
    {
    case EClimateZone::Tropical:
        if (FMath::FRand() < 0.3f) return EStormIntensity::Category3;
        if (FMath::FRand() < 0.5f) return EStormIntensity::Category2;
        if (FMath::FRand() < 0.7f) return EStormIntensity::Category1;
        return EStormIntensity::TropicalStorm;
        
    case EClimateZone::Temperate:
        if (FMath::FRand() < 0.1f) return EStormIntensity::Category2;
        if (FMath::FRand() < 0.3f) return EStormIntensity::Category1;
        return EStormIntensity::TropicalStorm;
        
    default:
        return EStormIntensity::TropicalStorm;
    }
}

void UAdvancedWeatherSystem::UpdateStormMovement(FStormSystem& Storm, float DeltaTime)
{
    // Move storm along its path
    if (Storm.StormPath.Num() > 0)
    {
        FVector2D TargetLocation = Storm.StormPath[0];
        FVector2D Direction = TargetLocation - Storm.CenterLocation;
        float Distance = Direction.Size();
        
        if (Distance < 1000.0f) // Close enough to target
        {
            Storm.StormPath.RemoveAt(0);
        }
        else
        {
            Direction = Direction.GetSafeNormal();
            Storm.CenterLocation += Direction * Storm.MovementSpeed * DeltaTime;
        }
    }
    else
    {
        // Random movement if no path
        float MovementRadians = FMath::DegreesToRadians(Storm.MovementDirection);
        FVector2D Movement = FVector2D(FMath::Cos(MovementRadians), FMath::Sin(MovementRadians)) * Storm.MovementSpeed * DeltaTime;
        Storm.CenterLocation += Movement;
    }
    
    // Gradually change direction
    Storm.MovementDirection += FMath::FRandRange(-10.0f, 10.0f) * DeltaTime;
    Storm.MovementDirection = FMath::Fmod(Storm.MovementDirection, 360.0f);
}

void UAdvancedWeatherSystem::DissipateStorm(FStormSystem& Storm)
{
    Storm.bIsDissipating = true;
    Storm.Radius *= 0.95f; // Gradually shrink
    Storm.MaxWindSpeed *= 0.95f; // Gradually weaken
}

void UAdvancedWeatherSystem::GenerateRandomWeatherEvent()
{
    EClimateZone RandomZone = (EClimateZone)FMath::RandRange(0, (int32)EClimateZone::Mountain);
    FVector2D Location = GetRandomLocationInClimateZone(RandomZone);
    EWeatherEventType EventType = SelectWeatherEventType(Location);
    
    FWeatherEvent Event;
    Event.EventType = EventType;
    Event.EventLocation = Location;
    Event.Radius = FMath::FRandRange(5000.0f, 20000.0f);
    Event.Duration = FMath::FRandRange(1800.0f, 7200.0f); // 30 minutes to 2 hours
    Event.Intensity = FMath::FRandRange(0.3f, 1.0f);
    Event.bIsActive = true;
    Event.bIsPredicted = false;
    
    TriggerWeatherEvent(Event);
}

bool UAdvancedWeatherSystem::ShouldGenerateWeatherEvent()
{
    return FMath::FRand() <= WeatherEventProbability;
}

EWeatherEventType UAdvancedWeatherSystem::SelectWeatherEventType(const FVector2D& Location)
{
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    
    if (FClimateZoneData* ZoneData = GetClimateZoneData(Zone))
    {
        if (ZoneData->PossibleWeatherEvents.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, ZoneData->PossibleWeatherEvents.Num() - 1);
            return ZoneData->PossibleWeatherEvents[RandomIndex];
        }
    }
    
    // Fallback to random event
    return (EWeatherEventType)FMath::RandRange(1, (int32)EWeatherEventType::SolarStorm);
}

void UAdvancedWeatherSystem::UpdateWeatherEvent(FWeatherEvent& Event, float DeltaTime)
{
    // Update event-specific logic
    switch (Event.EventType)
    {
    case EWeatherEventType::Thunderstorm:
        // Lightning strikes, etc.
        break;
        
    case EWeatherEventType::Blizzard:
        // Snow accumulation, etc.
        break;
        
    case EWeatherEventType::Sandstorm:
        // Sand particle effects, etc.
        break;
        
    default:
        break;
    }
}

void UAdvancedWeatherSystem::UpdatePredictions()
{
    GenerateWeatherPredictions(24.0f);
}

FWeatherPrediction UAdvancedWeatherSystem::PredictWeatherAtLocation(const FVector2D& Location, float HoursAhead)
{
    FWeatherPrediction Prediction;
    
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    EWeatherType PredictedWeather = CalculateWeatherAtLocation(Location);
    
    Prediction.PredictedWeather = PredictedWeather;
    Prediction.PredictionTime = HoursAhead;
    Prediction.PredictedLocation = Location;
    Prediction.Confidence = CalculatePredictionConfidence(Location, HoursAhead);
    
    // Add prediction details
    Prediction.PredictionDetails.Add(FString::Printf(TEXT("Temperature: %.1f°C"), GetTemperatureAtLocation(Location)));
    Prediction.PredictionDetails.Add(FString::Printf(TEXT("Humidity: %.0f%%"), GetHumidityAtLocation(Location) * 100.0f));
    
    return Prediction;
}

float UAdvancedWeatherSystem::CalculatePredictionConfidence(const FVector2D& Location, float HoursAhead) const
{
    // Confidence decreases with time ahead
    float TimeConfidence = FMath::Max(0.1f, 1.0f - (HoursAhead / 48.0f)); // Lose confidence over 48 hours
    
    // Confidence varies by climate zone stability
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    float ZoneConfidence = 0.8f; // Default
    
    switch (Zone)
    {
    case EClimateZone::Tropical:
        ZoneConfidence = 0.7f; // More variable
        break;
    case EClimateZone::Temperate:
        ZoneConfidence = 0.8f; // Moderately predictable
        break;
    case EClimateZone::Desert:
        ZoneConfidence = 0.9f; // Very predictable
        break;
    default:
        break;
    }
    
    return TimeConfidence * ZoneConfidence * PredictionAccuracy;
}

void UAdvancedWeatherSystem::ProcessActivePatterns(float DeltaTime)
{
    for (FWeatherPattern& Pattern : ActivePatterns)
    {
        ApplyWeatherPatternEffects(Pattern);
    }
}

void UAdvancedWeatherSystem::ApplyWeatherPatternEffects(const FWeatherPattern& Pattern)
{
    // Apply pattern effects to appropriate climate zones
    // This would override normal weather generation in affected areas
}

float UAdvancedWeatherSystem::CalculateTemperatureAtLocation(const FVector2D& Location) const
{
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    
    if (const FClimateZoneData* ZoneData = GetClimateZoneData(Zone))
    {
        float BaseTemp = ZoneData->BaseTemperature;
        float Variation = ZoneData->TemperatureVariation;
        
        // Add time-based variation
        float TimeOfDay = FMath::Fmod(GetWorld()->GetTimeSeconds(), 86400.0f) / 86400.0f; // 0-1 over 24 hours
        float TimeVariation = FMath::Sin(TimeOfDay * 2.0f * PI) * Variation * 0.5f;
        
        // Add random variation
        float RandomVariation = FMath::FRandRange(-Variation, Variation) * 0.3f;
        
        return BaseTemp + TimeVariation + RandomVariation;
    }
    
    return 20.0f; // Default temperature
}

float UAdvancedWeatherSystem::CalculateHumidityAtLocation(const FVector2D& Location) const
{
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    
    if (const FClimateZoneData* ZoneData = GetClimateZoneData(Zone))
    {
        float BaseHumidity = ZoneData->BaseHumidity;
        float Variation = ZoneData->HumidityVariation;
        
        // Add random variation
        float RandomVariation = FMath::FRandRange(-Variation, Variation);
        
        return FMath::Clamp(BaseHumidity + RandomVariation, 0.0f, 1.0f);
    }
    
    return 0.5f; // Default humidity
}

EWeatherType UAdvancedWeatherSystem::CalculateWeatherAtLocation(const FVector2D& Location) const
{
    EClimateZone Zone = GetClimateZoneAtLocation(Location);
    
    if (const FClimateZoneData* ZoneData = GetClimateZoneData(Zone))
    {
        if (ZoneData->CommonWeatherTypes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, ZoneData->CommonWeatherTypes.Num() - 1);
            return ZoneData->CommonWeatherTypes[RandomIndex];
        }
    }
    
    return EWeatherType::Clear; // Default weather
}

FVector2D UAdvancedWeatherSystem::GetRandomLocationInClimateZone(EClimateZone Zone) const
{
    if (const FClimateZoneData* ZoneData = GetClimateZoneData(Zone))
    {
        float Latitude = FMath::FRandRange(ZoneData->LatitudeRange.X, ZoneData->LatitudeRange.Y);
        float Longitude = FMath::FRandRange(ZoneData->LongitudeRange.X, ZoneData->LongitudeRange.Y);
        
        // Convert to world coordinates (simplified)
        return FVector2D(Latitude * 111000.0f, Longitude * 111000.0f); // Rough conversion to meters
    }
    
    return FVector2D::ZeroVector;
}

float UAdvancedWeatherSystem::GetDistance(const FVector2D& Point1, const FVector2D& Point2) const
{
    return FVector2D::Distance(Point1, Point2);
}

bool UAdvancedWeatherSystem::IsPointInClimateZone(const FVector2D& Point, const FClimateZoneData& Zone) const
{
    // Simplified check - in reality would need proper coordinate conversion
    return Point.X >= Zone.LatitudeRange.X && Point.X <= Zone.LatitudeRange.Y &&
           Point.Y >= Zone.LongitudeRange.X && Point.Y <= Zone.LongitudeRange.Y;
}