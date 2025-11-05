# Weather Integration Quick Reference

## Overview

The farming system is now fully integrated with the planet weather system, providing realistic environmental effects on crop growth and health.

## Weather Effects on Farming

### 1. Rain Watering (Automatic)

- **Function**: `ApplyRainWatering(float PrecipitationRate, float DeltaTime)`
- **Effect**: Rain automatically waters crops based on precipitation rate
- **Conversion**: 1mm/hour rain = 0.01 water level increase per hour
- **Benefits**:
  - Resets crop water needs
  - Reduces manual watering requirements
  - Tracks accumulated rainfall

### 2. Storm Damage

- **Function**: `ApplyStormDamage(float StormIntensity, float DeltaTime)`
- **Check Interval**: Every 5 seconds (configurable via `StormCheckInterval`)
- **Damage Calculation**:
  - Base damage: Up to 5% per check interval
  - Younger crops (low growth progress) are more vulnerable (2x damage)
  - Taller crops (later stages) are more susceptible to wind (1.2x per stage)
  - Random variation: 0.5x to 1.5x
- **Storm Types**:
  - Heavy Rain: 0.5 intensity
  - Thunderstorm: 0.8 intensity
  - Blizzard: 0.9 intensity
  - Sandstorm: 0.7 intensity
- **Severe Effects**: Storms > 0.8 intensity can destroy crops with health < 0.1

### 3. Temperature Effects

- **Function**: `ApplyTemperatureEffects(float Temperature, float DeltaTime)`
- **Damage Threshold**: 10°C outside optimal range (configurable via `TemperatureDamageThreshold`)
- **Damage Rate**: 1% per degree per second (configurable via `TemperatureDamageRate`)
- **Extreme Temperatures**:
  - Below -10°C or above 50°C: 10% damage per second
  - Can kill crops quickly
- **Continuous Effect**: Applied every frame for gradual impact

### 4. Seasonal Growth Variations

- **Function**: `GetSeasonalGrowthModifier(int32 DayOfYear)`
- **Season Definitions** (365-day year):
  - **Spring** (Day 0-90): 0.8x to 1.2x growth (gradual increase)
  - **Summer** (Day 91-180): 1.2x to 1.5x growth (peak season)
  - **Fall** (Day 181-270): 1.5x to 1.0x growth (gradual decrease)
  - **Winter** (Day 271-365): 1.0x to 0.5x growth (dormant season)
- **Integration**: Multiplied into total growth rate calculation

## Configuration Properties

### Storm Damage

```cpp
UPROPERTY(EditAnywhere, Category = "Weather")
float StormCheckInterval = 5.0f; // Seconds between storm damage checks
```

### Temperature Damage

```cpp
UPROPERTY(EditAnywhere, Category = "Weather")
float TemperatureDamageThreshold = 10.0f; // Degrees outside optimal before damage

UPROPERTY(EditAnywhere, Category = "Weather")
float TemperatureDamageRate = 0.01f; // Damage per degree per second
```

## Weather Component Integration

### Automatic Setup

The farm plot automatically finds and connects to the planet's weather component:

```cpp
if (!WeatherComponent && PlanetActor)
{
    APlanet* Planet = Cast<APlanet>(PlanetActor);
    if (Planet)
    {
        WeatherComponent = Planet->WeatherComponent;
    }
}
```

### Weather Queries Used

- `GetCurrentTemperature()` - For temperature effects
- `GetPrecipitationRate()` - For rain watering
- `GetCurrentWeather()` - For storm detection
- `IsStormyWeather()` - For storm intensity calculation

## Enhanced Evaporation System

Water evaporation is now weather-aware:

- **Base Rate**: 1% per second
- **During Rain**: 0.1x evaporation (90% reduction)
- **Temperature Factor**:
  - 10°C: 0.5x evaporation
  - 25°C: 1.0x evaporation
  - 40°C: 2.0x evaporation

## Growth Rate Calculation

The total growth rate now includes seasonal modifier:

```cpp
TotalGrowthModifier =
    TemperatureModifier *
    HumidityModifier *
    SunlightModifier *
    SoilQualityModifier *
    WaterModifier *
    FertilityModifier *
    HealthModifier *
    SeasonalModifier;
```

## Usage Examples

### Blueprint Usage

```cpp
// Get seasonal modifier for current day
float SeasonalMod = FarmPlot->GetSeasonalGrowthModifier(DayOfYear);

// Manually apply rain watering
FarmPlot->ApplyRainWatering(10.0f, DeltaTime); // 10mm/hr rain

// Manually apply storm damage
FarmPlot->ApplyStormDamage(0.8f, DeltaTime); // 80% intensity storm

// Manually apply temperature effects
FarmPlot->ApplyTemperatureEffects(35.0f, DeltaTime); // 35°C temperature
```

### C++ Usage

```cpp
// Weather effects are automatically updated in Tick
void AFarmPlot::Tick(float DeltaTime)
{
    // Weather effects applied first
    UpdateWeatherEffects(DeltaTime);

    // Then crop growth with weather-modified conditions
    UpdateCropGrowth(DeltaTime);
}
```

## Logging

Weather integration provides detailed logging:

- **Rain Events**: Heavy rain (>5mm/hr) logs watering
- **Storm Damage**: Logs number of crops damaged and average damage
- **Temperature Damage**: Logs when crops take temperature damage
- **Extreme Events**: Warnings when crops are destroyed

## Performance Considerations

- Storm damage checks are throttled to every 5 seconds
- Temperature effects are continuous but lightweight
- Rain watering is instant calculation
- Seasonal modifier is calculated once per growth update

## Testing

To test weather integration:

1. Create a farm plot on a planet with weather system
2. Change weather to rain - observe automatic watering
3. Change to thunderstorm - observe crop damage
4. Set extreme temperatures - observe health degradation
5. Advance time through seasons - observe growth rate changes

## Related Systems

- **Weather System**: `UPlanetWeatherComponent`
- **Growth Calculator**: `UCropGrowthCalculator`
- **Health System**: `UCropHealthSystem`
- **Day-Night Cycle**: `UDayNightCycleComponent` (for seasonal calculations)

## Future Enhancements

Potential additions:

- Frost damage for sub-zero temperatures
- Drought effects from extended dry periods
- Flood damage from excessive rainfall
- Wind damage to specific crop types
- Lightning strikes during thunderstorms
- Hail damage during severe storms
