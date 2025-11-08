// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions in inline code
#include "Components/ActorComponent.h"
#include "PlanetWeatherComponent.h"
#include "AdvancedWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EClimateZone : uint8
{
    Tropical,
    Temperate,
    Arctic,
    Desert,
    Mediterranean,
    Continental,
    Polar,
    Highland,
    Coastal,
    Mountain
};

UENUM(BlueprintType)
enum class EStormIntensity : uint8
{
    None,
    TropicalStorm,
    Category1,
    Category2,
    Category3,
    Category4,
    Category5
};

UENUM(BlueprintType)
enum class EWeatherEventType : uint8
{
    None,
    Thunderstorm,
    Blizzard,
    Sandstorm,
    Tornado,
    Hurricane,
    Heatwave,
    Coldwave,
    Flood,
    Drought,
    Aurora,
    SolarStorm
};

USTRUCT(BlueprintType)
struct FClimateZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    EClimateZone ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    FVector2D LatitudeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    FVector2D LongitudeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    float BaseTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    float TemperatureVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    float BaseHumidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    float HumidityVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    TArray<EWeatherType> CommonWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    TArray<EWeatherEventType> PossibleWeatherEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate Zone")
    float WeatherEventProbability;
};

USTRUCT(BlueprintType)
struct FStormSystem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    EStormIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    FVector2D CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float MaxWindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float Lifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    bool bIsDissipating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
    TArray<FVector2D> StormPath;
};

USTRUCT(BlueprintType)
struct FWeatherEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    EWeatherEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    FVector2D EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    TMap<FName, float> EventParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Event")
    bool bIsPredicted;
};

USTRUCT(BlueprintType)
struct FWeatherPrediction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    EWeatherType PredictedWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    EWeatherEventType PredictedEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    float PredictionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    float Confidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    FVector2D PredictedLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    float PredictedIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    TArray<FString> PredictionDetails;
};

USTRUCT(BlueprintType)
struct FWeatherPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    FName PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    TArray<EWeatherType> WeatherSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    TArray<float> DurationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float PatternProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    EClimateZone ApplicableZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    TArray<EWeatherEventType> PossibleEvents;
};

/**
 * Advanced Weather System with dynamic storm generation, weather prediction, and climate zones
 */
UCLASS(ClassGroup=(Weather), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAdvancedWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAdvancedWeatherSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ===== Climate Zone Management =====
    
    UFUNCTION(BlueprintCallable, Category = "Climate Zones")
    void InitializeClimateZones();

    UFUNCTION(BlueprintCallable, Category = "Climate Zones")
    void RegisterClimateZone(const FClimateZoneData& ClimateZone);

    UFUNCTION(BlueprintCallable, Category = "Climate Zones")
    EClimateZone GetClimateZoneAtLocation(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Climate Zones")
    FClimateZoneData GetClimateZoneData(EClimateZone ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Climate Zones")
    TArray<FClimateZoneData> GetAllClimateZones() const { return ClimateZones; }

    // ===== Storm System Management =====
    
    UFUNCTION(BlueprintCallable, Category = "Storm System")
    void GenerateStorm(const FVector2D& Location, EStormIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    void UpdateStormSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    TArray<FStormSystem> GetActiveStorms() const { return ActiveStorms; }

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    bool IsLocationInStorm(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    FStormSystem GetStormAtLocation(const FVector2D& Location);

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    float GetWindSpeedAtLocation(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Storm System")
    FVector GetWindDirectionAtLocation(const FVector2D& Location) const;

    // ===== Weather Event Management =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    void TriggerWeatherEvent(const FWeatherEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    void UpdateWeatherEvents(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    TArray<FWeatherEvent> GetActiveWeatherEvents() const { return ActiveWeatherEvents; }

    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    bool IsLocationAffectedByEvent(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    FWeatherEvent GetWeatherEventAtLocation(const FVector2D& Location);

    UFUNCTION(BlueprintCallable, Category = "Weather Events")
    void EndWeatherEvent(const FName& EventID);

    // ===== Weather Prediction System =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather Prediction")
    TArray<FWeatherPrediction> GenerateWeatherPredictions(float PredictionHours = 24.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Prediction")
    FWeatherPrediction GetPredictionAtLocation(const FVector2D& Location, float HoursAhead = 6.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Prediction")
    void UpdatePredictionSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather Prediction")
    float GetPredictionAccuracy() const { return PredictionAccuracy; }

    UFUNCTION(BlueprintCallable, Category = "Weather Prediction")
    void SetPredictionAccuracy(float NewAccuracy) { PredictionAccuracy = FMath::Clamp(NewAccuracy, 0.0f, 1.0f); }

    // ===== Weather Pattern System =====
    
    UFUNCTION(BlueprintCallable, Category = "Weather Patterns")
    void RegisterWeatherPattern(const FWeatherPattern& Pattern);

    UFUNCTION(BlueprintCallable, Category = "Weather Patterns")
    void ActivateWeatherPattern(const FName& PatternName);

    UFUNCTION(BlueprintCallable, Category = "Weather Patterns")
    void DeactivateWeatherPattern(const FName& PatternName);

    UFUNCTION(BlueprintCallable, Category = "Weather Patterns")
    TArray<FWeatherPattern> GetActivePatterns() const { return ActivePatterns; }

    UFUNCTION(BlueprintCallable, Category = "Weather Patterns")
    FWeatherPattern GetPattern(const FName& PatternName);

    // ===== Player-Affecting Weather Events =====
    
    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    void ApplyWeatherEffectsToPlayer(class ACharacter* Player, const FVector2D& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    float GetTemperatureAtLocation(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    float GetHumidityAtLocation(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    float GetVisibilityAtLocation(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    bool IsLocationSafeForPlayers(const FVector2D& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Player Effects")
    TArray<FString> GetWeatherWarningsAtLocation(const FVector2D& Location) const;

    // ===== Integration with PlanetWeatherComponent =====
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SetPlanetWeatherComponent(class UPlanetWeatherComponent* WeatherComponent);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SynchronizeWithPlanetWeather();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OverridePlanetWeatherAtLocation(const FVector2D& Location, EWeatherType WeatherType, float Duration);

    // ===== Configuration =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableStormGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableWeatherEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableWeatherPrediction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableClimateZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float StormGenerationProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WeatherEventProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PredictionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxActiveStorms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxActiveWeatherEvents;

    // ===== Events =====
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStormGenerated, const FStormSystem&, Storm, const FVector2D&, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherEventTriggered, const FWeatherEvent&, Event, const FVector2D&, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherPredictionUpdated, const TArray<FWeatherPrediction>&, Predictions);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherPatternActivated, const FWeatherPattern&, Pattern, const FName&, PatternName);

    UPROPERTY(BlueprintAssignable, Category = "Weather Events")
    FOnStormGenerated OnStormGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Weather Events")
    FOnWeatherEventTriggered OnWeatherEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Weather Events")
    FOnWeatherPredictionUpdated OnWeatherPredictionUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Weather Events")
    FOnWeatherPatternActivated OnWeatherPatternActivated;

protected:
    // ===== Climate Zones =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate Zones")
    TArray<FClimateZoneData> ClimateZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate Zones")
    TMap<EClimateZone, FClimateZoneData> ClimateZoneMap;

    // ===== Storm Systems =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Storm System")
    TArray<FStormSystem> ActiveStorms;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Storm System")
    TArray<FStormSystem> PredictedStorms;

    // ===== Weather Events =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Events")
    TArray<FWeatherEvent> ActiveWeatherEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Events")
    TArray<FWeatherEvent> PredictedEvents;

    // ===== Weather Predictions =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Prediction")
    TArray<FWeatherPrediction> CurrentPredictions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Prediction")
    float PredictionAccuracy;

    // ===== Weather Patterns =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Patterns")
    TArray<FWeatherPattern> RegisteredPatterns;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Patterns")
    TArray<FWeatherPattern> ActivePatterns;

    // ===== System References =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    class UPlanetWeatherComponent* PlanetWeatherComponent;

    // ===== Timers =====
    
    UPROPERTY()
    FTimerHandle StormUpdateTimer;

    UPROPERTY()
    FTimerHandle WeatherEventUpdateTimer;

    UPROPERTY()
    FTimerHandle PredictionUpdateTimer;

    UPROPERTY()
    FTimerHandle PatternUpdateTimer;

private:
    // ===== Internal Functions =====
    
    void InitializeDefaultClimateZones();
    void InitializeDefaultWeatherPatterns();
    void SetupTimers();
    
    // Storm Generation
    void GenerateRandomStorm();
    bool ShouldGenerateStorm();
    EStormIntensity CalculateStormIntensity(const FVector2D& Location);
    void UpdateStormMovement(FStormSystem& Storm, float DeltaTime);
    void DissipateStorm(FStormSystem& Storm);
    
    // Weather Event Generation
    void GenerateRandomWeatherEvent();
    bool ShouldGenerateWeatherEvent();
    EWeatherEventType SelectWeatherEventType(const FVector2D& Location);
    void UpdateWeatherEvent(FWeatherEvent& Event, float DeltaTime);
    
    // Weather Prediction
    void UpdatePredictions();
    FWeatherPrediction PredictWeatherAtLocation(const FVector2D& Location, float HoursAhead);
    float CalculatePredictionConfidence(const FVector2D& Location, float HoursAhead) const;
    
    // Weather Pattern Processing
    void ProcessActivePatterns(float DeltaTime);
    void ApplyWeatherPatternEffects(const FWeatherPattern& Pattern);
    
    // Climate Zone Calculations
    float CalculateTemperatureAtLocation(const FVector2D& Location) const;
    float CalculateHumidityAtLocation(const FVector2D& Location) const;
    EWeatherType CalculateWeatherAtLocation(const FVector2D& Location) const;
    
    // Utility Functions
    FVector2D GetRandomLocationInClimateZone(EClimateZone Zone) const;
    float GetDistance(const FVector2D& Point1, const FVector2D& Point2) const;
    bool IsPointInClimateZone(const FVector2D& Point, const FClimateZoneData& Zone) const;
    
    // VFX Spawning
    void SpawnStormVisuals(const FStormSystem& Storm);
    
    // Constants
    static constexpr float DEFAULT_STORM_GENERATION_PROBABILITY = 0.01f;
    static constexpr float DEFAULT_WEATHER_EVENT_PROBABILITY = 0.005f;
    static constexpr float DEFAULT_PREDICTION_UPDATE_INTERVAL = 300.0f; // 5 minutes
    static constexpr float DEFAULT_PREDICTION_ACCURACY = 0.75f;
    static constexpr int32 DEFAULT_MAX_ACTIVE_STORMS = 5;
    static constexpr int32 DEFAULT_MAX_ACTIVE_WEATHER_EVENTS = 10;
    static constexpr float STORM_UPDATE_INTERVAL = 1.0f;
    static constexpr float WEATHER_EVENT_UPDATE_INTERVAL = 0.5f;
    static constexpr float PATTERN_UPDATE_INTERVAL = 10.0f;
};