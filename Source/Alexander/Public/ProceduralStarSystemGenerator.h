// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProceduralStarSystemGenerator.generated.h"

// Forward declarations
class AOrbitalBody;
class APlanet;
class ASun;

/**
 * Star spectral types (O, B, A, F, G, K, M)
 */
UENUM(BlueprintType)
enum class EStarSpectralType : uint8
{
    O,  // Blue giants, >30,000K
    B,  // Blue-white, 10,000-30,000K
    A,  // White, 7,500-10,000K
    F,  // Yellow-white, 6,000-7,500K
    G,  // Yellow, 5,200-6,000K (like our Sun)
    K,  // Orange, 3,700-5,200K
    M   // Red, 2,400-3,700K
};

/**
 * Planetary system type
 */
UENUM(BlueprintType)
enum class EPlanetarySystemType : uint8
{
    // Single star system
    SingleStar,
    
    // Binary star system
    BinaryStar,
    
    // Multiple star system
    MultipleStar,
    
    // System with gas giants only
    GasGiantOnly,
    
    // System with terrestrial planets only
    TerrestrialOnly,
    
    // Mixed system with both types
    Mixed,
    
    // System with asteroid belt but few planets
    AsteroidBeltRich
};

/**
 * Configuration for star generation
 */
USTRUCT(BlueprintType)
struct FStarGenerationConfig
{
    GENERATED_BODY()

    // Spectral type of the star
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    EStarSpectralType SpectralType;

    // Star mass in solar masses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    float Mass;

    // Star radius in solar radii
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    float Radius;

    // Star temperature in Kelvin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    float Temperature;

    // Star luminosity in solar luminosities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    float Luminosity;

    // Random seed for generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star")
    int32 RandomSeed;

    FStarGenerationConfig()
        : SpectralType(EStarSpectralType::G)
        , Mass(1.0f)
        , Radius(1.0f)
        , Temperature(5778.0f)
        , Luminosity(1.0f)
        , RandomSeed(12345)
    {
    }
};

/**
 * Configuration for planetary system generation
 */
USTRUCT(BlueprintType)
struct FPlanetarySystemConfig
{
    GENERATED_BODY()

    // Type of planetary system to generate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EPlanetarySystemType SystemType;

    // Number of planets to generate (0 = random)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 NumberOfPlanets;

    // Minimum orbital radius (AU)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float MinOrbitalRadius;

    // Maximum orbital radius (AU)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float MaxOrbitalRadius;

    // Enable asteroid belt generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bGenerateAsteroidBelt;

    // Enable moon generation for planets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bGenerateMoons;

    // Maximum number of moons per planet
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 MaxMoonsPerPlanet;

    // Random seed for generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 RandomSeed;

    // Star spectral type for orbital calculations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EStarSpectralType StarSpectralType;

    // Habitable zone inner boundary (AU)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitable Zone")
    float HabitableZoneInner;

    // Habitable zone outer boundary (AU)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Habitable Zone")
    float HabitableZoneOuter;

    FPlanetarySystemConfig()
        : SystemType(EPlanetarySystemType::Mixed)
        , NumberOfPlanets(0)
        , MinOrbitalRadius(0.3f)
        , MaxOrbitalRadius(30.0f)
        , bGenerateAsteroidBelt(true)
        , bGenerateMoons(true)
        , MaxMoonsPerPlanet(5)
        , RandomSeed(12345)
        , StarSpectralType(EStarSpectralType::G)
        , HabitableZoneInner(0.95f)
        , HabitableZoneOuter(1.37f)
    {
    }
};

/**
 * Configuration for complete star system generation
 */
USTRUCT(BlueprintType)
struct FStarSystemConfig
{
    GENERATED_BODY()

    // Star configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
    FStarGenerationConfig StarConfig;

    // Planetary system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
    FPlanetarySystemConfig PlanetaryConfig;

    // System name (empty = auto-generate)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
    FString SystemName;

    // Position in galaxy (light years from galactic center)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
    FVector GalacticPosition;

    FStarSystemConfig()
        : SystemName(TEXT(""))
        , GalacticPosition(FVector::ZeroVector)
    {
    }
};

/**
 * Generated star system data
 */
USTRUCT(BlueprintType)
struct FGeneratedStarSystem
{
    GENERATED_BODY()

    // Generated star
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    TWeakObjectPtr<ASun> Star;

    // Generated planets
    UPROPERTY(VisibleAnywhere, Category = "System")
    TArray<TWeakObjectPtr<APlanet>> Planets;

    // Generated moons
    UPROPERTY(VisibleAnywhere, Category = "System")
    TArray<TWeakObjectPtr<AOrbitalBody>> Moons;

    // System configuration used for generation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    FStarSystemConfig Config;

    // Habitable zone boundaries (AU)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    float HabitableZoneInner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System")
    float HabitableZoneOuter;

    FGeneratedStarSystem()
        : HabitableZoneInner(0.0f)
        , HabitableZoneOuter(0.0f)
    {
    }
};

/**
 * Procedural star system generator using scientific principles
 * Generates realistic star systems based on astrophysical models
 */
UCLASS(Blueprintable)
class ALEXANDER_API UProceduralStarSystemGenerator : public UObject
{
    GENERATED_BODY()

public:
    UProceduralStarSystemGenerator();

    // Generate a complete star system
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    FGeneratedStarSystem GenerateStarSystem(const FStarSystemConfig& Config);

    // Generate a star based on spectral type
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    ASun* GenerateStar(const FStarGenerationConfig& Config);

    // Generate a planetary system around a star
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    TArray<APlanet*> GeneratePlanetarySystem(ASun* Star, const FPlanetarySystemConfig& Config);

    // Generate a planet with realistic parameters
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    APlanet* GeneratePlanet(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed);

    // Generate moons for a planet
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    TArray<AOrbitalBody*> GenerateMoons(APlanet* Planet, int32 MaxMoons, int32 RandomSeed);

    // Generate asteroid belt
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    void GenerateAsteroidBelt(ASun* Star, float InnerRadius, float OuterRadius, int32 AsteroidCount, int32 RandomSeed);

    // Calculate habitable zone for a star
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    void CalculateHabitableZone(float StellarLuminosity, float& InnerBoundary, float& OuterBoundary) const;

    // Get realistic orbital parameters for a planet
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    void GetRealisticOrbitalParameters(float OrbitalRadius, float& Eccentricity, float& Inclination, float& OrbitalPeriod) const;

    // Get realistic planetary parameters based on orbital radius
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    void GetRealisticPlanetaryParameters(float OrbitalRadius, float& Mass, float& Radius, bool& bHasAtmosphere) const;

    // Check if a planet is in the habitable zone
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    bool IsInHabitableZone(float OrbitalRadius, float InnerBoundary, float OuterBoundary) const;

    // Generate realistic planet name
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    FString GeneratePlanetName(const FString& StarName, int32 PlanetIndex) const;

    // Get star parameters based on spectral type
    UFUNCTION(BlueprintCallable, Category = "Orbital Mechanics|Generation")
    void GetStarParameters(EStarSpectralType SpectralType, float& Mass, float& Radius, float& Temperature, float& Luminosity) const;

protected:
    // Generate star parameters from spectral type
    void GenerateStarFromSpectralType(FStarGenerationConfig& Config) const;

    // Generate planetary system based on system type
    TArray<APlanet*> GenerateSystemByType(ASun* Star, const FPlanetarySystemConfig& Config);

    // Generate single star system
    TArray<APlanet*> GenerateSingleStarSystem(ASun* Star, const FPlanetarySystemConfig& Config);

    // Generate binary star system
    TArray<APlanet*> GenerateBinaryStarSystem(ASun* Star, const FPlanetarySystemConfig& Config);

    // Generate gas giant planet
    APlanet* GenerateGasGiant(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed);

    // Generate terrestrial planet
    APlanet* GenerateTerrestrialPlanet(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed);

    // Generate ice giant
    APlanet* GenerateIceGiant(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed);

    // Calculate orbital spacing using Titius-Bode law approximation
    float CalculateOrbitalSpacing(int32 PlanetIndex, float InnerRadius, float OuterRadius, EStarSpectralType StarType) const;

    // Get planet type probability based on orbital radius
    EOrbitalBodyType GetPlanetTypeProbability(float OrbitalRadius, float HabitableZoneInner, float HabitableZoneOuter) const;

    // Random number generator
    mutable FRandomStream RandomStream;

    // Generation statistics
    int32 SystemsGenerated;
    int32 PlanetsGenerated;
    int32 MoonsGenerated;
};