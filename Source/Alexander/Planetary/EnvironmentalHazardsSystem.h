// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Delegates/DelegateCombinations.h"
#include "EnvironmentalHazardsSystem.generated.h"

// Hazard types
UENUM(BlueprintType)
enum class EHazardType : uint8
{
    None                    UMETA(DisplayName = "None"),
    RadiationStorm          UMETA(DisplayName = "Radiation Storm"),
    SolarFlare              UMETA(DisplayName = "Solar Flare"),
    MeteorShower            UMETA(DisplayName = "Meteor Shower"),
    DustStorm               UMETA(DisplayName = "Dust Storm"),
    ExtremeTemperature      UMETA(DisplayName = "Extreme Temperature"),
    AtmosphericPressure     UMETA(DisplayName = "Atmospheric Pressure"),
    SeismicActivity         UMETA(DisplayName = "Seismic Activity"),
    AcidRain                UMETA(DisplayName = "Acid Rain"),
    MagneticStorm           UMETA(DisplayName = "Magnetic Storm"),
    GravityAnomaly          UMETA(DisplayName = "Gravity Anomaly"),
    BiologicalHazard        UMETA(DisplayName = "Biological Hazard")
};

// Hazard severity levels
UENUM(BlueprintType)
enum class EHazardSeverity : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme"),
    Catastrophic UMETA(DisplayName = "Catastrophic")
};

// Weather conditions
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Cloudy          UMETA(DisplayName = "Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Snow            UMETA(DisplayName = "Snow"),
    Blizzard        UMETA(DisplayName = "Blizzard"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm"),
    AcidRain        UMETA(DisplayName = "Acid Rain"),
    ToxicFog        UMETA(DisplayName = "Toxic Fog")
};

// Temperature zones
UENUM(BlueprintType)
enum class ETemperatureZone : uint8
{
    Frozen          UMETA(DisplayName = "Frozen"),
    Cold            UMETA(DisplayName = "Cold"),
    Temperate       UMETA(DisplayName = "Temperate"),
    Warm            UMETA(DisplayName = "Warm"),
    Hot             UMETA(DisplayName = "Hot"),
    ExtremeHeat     UMETA(DisplayName = "Extreme Heat")
};

// Hazard event data
USTRUCT(BlueprintType)
struct FHazardEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHazardType HazardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHazardSeverity Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredicted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamagePerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AffectedBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredDefenses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> ResourceCosts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WarningMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    FHazardEvent()
    {
        EventID = -1;
        HazardType = EHazardType::None;
        Severity = EHazardSeverity::Low;
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        Duration = 300.0f; // 5 minutes default
        bIsActive = false;
        bIsPredicted = false;
        DamagePerSecond = 0.0f;
        AffectedBuildings.Empty();
        RequiredDefenses.Empty();
        ResourceCosts.Empty();
        WarningMessage = TEXT("");
        Description = TEXT("");
    }
};

// Weather data
USTRUCT(BlueprintType)
struct FWeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherCondition CurrentCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherCondition ForecastCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ForecastTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHumidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentPressure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentWindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentVisibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETemperatureZone TemperatureZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime NextForecastTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EWeatherCondition> UpcomingConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> TemperatureTrend;

    FWeatherData()
    {
        CurrentCondition = EWeatherCondition::Clear;
        ForecastCondition = EWeatherCondition::Clear;
        CurrentTemperature = 20.0f; // Celsius
        ForecastTemperature = 20.0f;
        CurrentHumidity = 50.0f; // Percentage
        CurrentPressure = 1013.25f; // Millibars
        CurrentWindSpeed = 5.0f; // m/s
        CurrentVisibility = 10.0f; // km
        TemperatureZone = ETemperatureZone::Temperate;
        LastUpdateTime = FDateTime::Now();
        NextForecastTime = FDateTime::Now() + FTimespan::FromHours(1);
        UpcomingConditions.Empty();
        TemperatureTrend.Empty();
    }
};

// Environmental zone data
USTRUCT(BlueprintType)
struct FEnvironmentalZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETemperatureZone TemperatureZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherCondition TypicalWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EHazardType> CommonHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseRadiationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseAtmosphericPressure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseGravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EHazardType, float> HazardProbabilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHabitable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSpecialEquipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredEquipment;

    FEnvironmentalZone()
    {
        ZoneID = -1;
        ZoneName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        TemperatureZone = ETemperatureZone::Temperate;
        TypicalWeather = EWeatherCondition::Clear;
        CommonHazards.Empty();
        BaseRadiationLevel = 0.1f;
        BaseAtmosphericPressure = 1013.25f;
        BaseGravity = 1.0f;
        HazardProbabilities.Empty();
        bIsHabitable = true;
        bRequiresSpecialEquipment = false;
        RequiredEquipment.Empty();
    }
};

// Defense system data
USTRUCT(BlueprintType)
struct FHazardDefense
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefenseID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DefenseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHazardType ProtectedHazardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EHazardType> ProtectedHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProtectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Effectiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaintenanceCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOperational;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastMaintenanceTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Upgrades;

    FHazardDefense()
    {
        DefenseID = -1;
        DefenseName = TEXT("");
        ProtectedHazardType = EHazardType::None;
        ProtectedHazards.Empty();
        Location = FVector::ZeroVector;
        ProtectionRadius = 500.0f;
        Effectiveness = 1.0f;
        PowerConsumption = 10.0f;
        MaintenanceCost = 5.0f;
        bIsActive = true;
        bIsOperational = true;
        LastMaintenanceTime = FDateTime::Now();
        CurrentHealth = 100.0f;
        MaxHealth = 100.0f;
        Upgrades.Empty();
    }
};

// Hazard prediction data
USTRUCT(BlueprintType)
struct FHazardPrediction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PredictionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHazardType PredictedHazardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHazardSeverity PredictedSeverity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PredictedLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictedRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PredictionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PredictedStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConfidenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PredictionSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PredictionMethod;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVerified;

    FHazardPrediction()
    {
        PredictionID = -1;
        PredictedHazardType = EHazardType::None;
        PredictedSeverity = EHazardSeverity::Low;
        PredictedLocation = FVector::ZeroVector;
        PredictedRadius = 1000.0f;
        PredictionTime = FDateTime::Now();
        PredictedStartTime = FDateTime::Now();
        ConfidenceLevel = 0.5f;
        PredictionSources.Empty();
        PredictionMethod = TEXT("");
        bIsVerified = false;
    }
};

// Environmental statistics
USTRUCT(BlueprintType)
struct FEnvironmentalStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalHazardsEncountered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HazardsPredicted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HazardsMitigated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDamagePrevented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDamageTaken;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageResponseTime;

    TMap<EHazardType, int32> HazardFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EHazardSeverity, int32> SeverityDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EWeatherCondition> RecentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageRadiationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveDefenseSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefenseSystemEffectiveness;

    FEnvironmentalStatistics()
    {
        TotalHazardsEncountered = 0;
        ActiveHazards = 0;
        HazardsPredicted = 0;
        HazardsMitigated = 0;
        TotalDamagePrevented = 0.0f;
        TotalDamageTaken = 0.0f;
        AverageResponseTime = 0.0f;
        HazardFrequency.Empty();
        SeverityDistribution.Empty();
        RecentWeather.Empty();
        AverageTemperature = 20.0f;
        AverageRadiationLevel = 0.1f;
        ActiveDefenseSystems = 0;
        DefenseSystemEffectiveness = 1.0f;
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardEventStarted, const FHazardEvent&, HazardEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardEventEnded, const FHazardEvent&, HazardEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, const FWeatherData&, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardPredicted, const FHazardPrediction&, Prediction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefenseSystemActivated, int32, DefenseID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentalAlert, const FString&, AlertMessage);

// Wrapper structs for nested TMaps
USTRUCT(BlueprintType)
struct FHazardSeverityProbabilities
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EHazardSeverity, float> Probabilities;
};

USTRUCT(BlueprintType)
struct FWeatherTransitionProbabilities
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeatherCondition, float> Transitions;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UEnvironmentalHazardsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UEnvironmentalHazardsSystem();

    // Component overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Hazard management
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FHazardEvent> GetActiveHazards() const { return ActiveHazards; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FHazardEvent GetHazardEvent(int32 EventID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    int32 CreateHazardEvent(EHazardType HazardType, EHazardSeverity Severity, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool TriggerHazardEvent(int32 EventID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool EndHazardEvent(int32 EventID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FHazardEvent> GetHazardsInRadius(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool IsLocationSafe(const FVector& Location) const;

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FWeatherData GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FWeatherData GetWeatherForecast() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    void UpdateWeather();

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool PredictWeatherChange(EWeatherCondition& PredictedCondition, float& Confidence);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    void SetWeatherCondition(EWeatherCondition NewCondition);

    // Environmental zones
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FEnvironmentalZone> GetEnvironmentalZones() const { return EnvironmentalZones; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FEnvironmentalZone GetEnvironmentalZone(int32 ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FEnvironmentalZone GetZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    int32 CreateEnvironmentalZone(const FString& ZoneName, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool UpdateZoneProperties(int32 ZoneID, const FEnvironmentalZone& NewProperties);

    // Defense systems
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FHazardDefense> GetDefenseSystems() const { return DefenseSystems; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FHazardDefense GetDefenseSystem(int32 DefenseID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    int32 InstallDefenseSystem(const FString& DefenseName, EHazardType ProtectedHazard, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool ActivateDefenseSystem(int32 DefenseID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool DeactivateDefenseSystem(int32 DefenseID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool UpgradeDefenseSystem(int32 DefenseID, const FString& UpgradeType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool MaintainDefenseSystem(int32 DefenseID);

    // Hazard prediction
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FHazardPrediction> GetHazardPredictions() const { return HazardPredictions; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FHazardPrediction GetHazardPrediction(int32 PredictionID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    int32 PredictHazard(EHazardType HazardType, const FVector& Location, float TimeWindow);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool VerifyPrediction(int32 PredictionID);

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FHazardPrediction> GetPredictionsForTimeWindow(const FDateTime& StartTime, const FDateTime& EndTime);

    // Environmental analysis
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FEnvironmentalStatistics GetEnvironmentalStatistics() const { return EnvironmentalStats; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    float CalculateHazardRisk(EHazardType HazardType, const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<EHazardType> GetLikelyHazards(const FVector& Location, float TimeWindow) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    float GetEnvironmentalSafetyRating(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    TArray<FString> GetRecommendedDefenses(const FVector& Location) const;

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FString GetHazardName(EHazardType HazardType) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FString GetSeverityName(EHazardSeverity Severity) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    FString GetWeatherConditionName(EWeatherCondition Condition) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    float GetHazardDamageMultiplier(EHazardType HazardType, EHazardSeverity Severity) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Hazards")
    bool CanWithstandHazard(EHazardType HazardType, EHazardSeverity Severity, const FVector& Location) const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    float HazardUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    float WeatherUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    float PredictionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    float MaxPredictionTimeWindow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    float DefaultHazardDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    bool bEnableAutoPrediction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    bool bEnableAutoDefense;

    // Hazard settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    TMap<EHazardType, float> BaseHazardProbabilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    TMap<EHazardType, FHazardSeverityProbabilities> SeverityProbabilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards")
    TMap<EWeatherCondition, FWeatherTransitionProbabilities> WeatherTransitionProbabilities;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnHazardEventStarted OnHazardEventStarted;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnHazardEventEnded OnHazardEventEnded;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnHazardPredicted OnHazardPredicted;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnDefenseSystemActivated OnDefenseSystemActivated;

    UPROPERTY(BlueprintAssignable, Category = "Environmental Hazards")
    FOnEnvironmentalAlert OnEnvironmentalAlert;

protected:
    // Environmental data
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    TArray<FHazardEvent> ActiveHazards;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    FWeatherData CurrentWeather;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    TArray<FEnvironmentalZone> EnvironmentalZones;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    TArray<FHazardDefense> DefenseSystems;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    TArray<FHazardPrediction> HazardPredictions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    FEnvironmentalStatistics EnvironmentalStats;

    // Internal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    float LastHazardUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    float LastWeatherUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    float LastPredictionUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    int32 NextHazardEventID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    int32 NextDefenseSystemID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    int32 NextPredictionID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental Hazards")
    int32 NextZoneID;

    // Internal functions
    void InitializeEnvironmentalSystem();
    void UpdateHazardEvents(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void UpdatePredictionSystem(float DeltaTime);
    void UpdateDefenseSystems(float DeltaTime);
    void UpdateEnvironmentalStatistics();

    void ProcessHazardEvent(FHazardEvent& HazardEvent, float DeltaTime);
    void ProcessWeatherChanges();
    void ProcessPredictionAccuracy();
    void ProcessDefenseEffectiveness();

    void GenerateRandomHazard();
    void GenerateWeatherForecast();
    void GenerateHazardPredictions();

    void ApplyHazardEffects(const FHazardEvent& HazardEvent);
    void ApplyWeatherEffects(const FWeatherData& Weather);
    void ApplyDefenseProtection(FHazardEvent& HazardEvent);

    bool ValidateHazardEvent(const FHazardEvent& HazardEvent) const;
    bool ValidateDefenseSystem(const FHazardDefense& Defense) const;
    bool ValidatePrediction(const FHazardPrediction& Prediction) const;

    void CalculateEnvironmentalImpact(const FHazardEvent& HazardEvent, float& Damage, TArray<FString>& AffectedSystems);
    void CalculateWeatherImpact(const FWeatherData& Weather, float& EfficiencyModifier, TArray<FString>& AffectedSystems);

    void TriggerEnvironmentalAlert(const FString& AlertMessage, EHazardSeverity Severity);
    void BroadcastHazardWarning(const FHazardEvent& HazardEvent);

    void InitializeEnvironmentalZones();
    void InitializeWeatherPatterns();
    void InitializeHazardProbabilities();

    // Utility functions
    bool IsValidHazardType(EHazardType HazardType) const;
    bool IsValidWeatherCondition(EWeatherCondition Condition) const;
    bool IsValidSeverity(EHazardSeverity Severity) const;

    float CalculateHazardProbability(EHazardType HazardType, const FVector& Location) const;
    float CalculateWeatherTransitionProbability(EWeatherCondition From, EWeatherCondition To) const;
    float CalculatePredictionAccuracy(const FHazardPrediction& Prediction) const;

    EHazardSeverity CalculateHazardSeverity(EHazardType HazardType, const FVector& Location) const;
    EWeatherCondition CalculateNextWeatherCondition() const;

    TArray<FHazardDefense> GetActiveDefensesInRadius(const FVector& Location, float Radius) const;
    float CalculateTotalDefenseEffectiveness(const TArray<FHazardDefense>& Defenses, EHazardType HazardType) const;

private:
    // Timers
    UPROPERTY(VisibleAnywhere)
    FTimerHandle HazardUpdateTimer;

    UPROPERTY(VisibleAnywhere)
    FTimerHandle WeatherUpdateTimer;

    UPROPERTY(VisibleAnywhere)
    FTimerHandle PredictionUpdateTimer;

    // Constants
    static constexpr float DEFAULT_HAZARD_UPDATE_INTERVAL = 5.0f;
    static constexpr float DEFAULT_WEATHER_UPDATE_INTERVAL = 300.0f; // 5 minutes
    static constexpr float DEFAULT_PREDICTION_UPDATE_INTERVAL = 60.0f; // 1 minute
    static constexpr float MAX_PREDICTION_TIME_WINDOW = 86400.0f; // 24 hours
    static constexpr float DEFAULT_HAZARD_DURATION = 600.0f; // 10 minutes
    static constexpr float MAX_HAZARD_RADIUS = 5000.0f;
    static constexpr float MIN_HAZARD_RADIUS = 100.0f;
};