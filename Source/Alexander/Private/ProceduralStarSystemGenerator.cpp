// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralStarSystemGenerator.h"
#include "OrbitalBody.h"
#include "Sun.h"
#include "Planet.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UProceduralStarSystemGenerator::UProceduralStarSystemGenerator()
{
    SystemsGenerated = 0;
    PlanetsGenerated = 0;
    MoonsGenerated = 0;
}

FGeneratedStarSystem UProceduralStarSystemGenerator::GenerateStarSystem(const FStarSystemConfig& Config)
{
    FGeneratedStarSystem Result;
    Result.Config = Config;

    // Set random seed
    RandomStream.Initialize(Config.PlanetaryConfig.RandomSeed);

    // Generate star
    ASun* Star = GenerateStar(Config.StarConfig);
    if (!Star)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to generate star"));
        return Result;
    }

    Result.Star = Star;

    // Calculate habitable zone
    CalculateHabitableZone(Config.StarConfig.Luminosity, Result.HabitableZoneInner, Result.HabitableZoneOuter);

    // Generate planetary system
    Result.Planets = GeneratePlanetarySystem(Star, Config.PlanetaryConfig);

    // Generate asteroid belt if requested
    if (Config.PlanetaryConfig.bGenerateAsteroidBelt)
    {
        float BeltInnerRadius = Result.HabitableZoneOuter * 1.5f; // Just outside habitable zone
        float BeltOuterRadius = BeltInnerRadius * 2.0f;
        int32 AsteroidCount = RandomStream.RandRange(100, 500);
        
        GenerateAsteroidBelt(Star, BeltInnerRadius, BeltOuterRadius, AsteroidCount, Config.PlanetaryConfig.RandomSeed + 1000);
    }

    SystemsGenerated++;
    UE_LOG(LogTemp, Log, TEXT("Generated star system with %d planets"), Result.Planets.Num());

    return Result;
}

ASun* UProceduralStarSystemGenerator::GenerateStar(const FStarGenerationConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Create star actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ASun* Star = World->SpawnActor<ASun>(ASun::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (!Star)
    {
        return nullptr;
    }

    // Configure star based on spectral type
    FStarGenerationConfig MutableConfig = Config;
    if (Config.Mass <= 0.0f || Config.Radius <= 0.0f || Config.Temperature <= 0.0f)
    {
        GenerateStarFromSpectralType(MutableConfig);
    }

    // Apply star parameters
    Star->Mass = MutableConfig.Mass * 1.989e30f; // Solar masses to kg
    Star->Radius = MutableConfig.Radius * 696340.0f; // Solar radii to km
    Star->SurfaceTemperature = MutableConfig.Temperature;
    
    // Set star name based on spectral type
    FString StarName = FString::Printf(TEXT("%s-Class Star"), *UEnum::GetValueAsString(Config.SpectralType));
    Star->SetActorLabel(StarName);

    return Star;
}

TArray<APlanet*> UProceduralStarSystemGenerator::GeneratePlanetarySystem(ASun* Star, const FPlanetarySystemConfig& Config)
{
    TArray<APlanet*> Planets;

    if (!Star)
    {
        return Planets;
    }

    // Set random seed
    RandomStream.Initialize(Config.RandomSeed);

    // Calculate habitable zone
    float HabitableZoneInner, HabitableZoneOuter;
    CalculateHabitableZone(Star->SurfaceTemperature, HabitableZoneInner, HabitableZoneOuter);

    // Determine number of planets
    int32 NumPlanets = Config.NumberOfPlanets;
    if (NumPlanets <= 0)
    {
        // Random number based on system type
        switch (Config.SystemType)
        {
            case EPlanetarySystemType::SingleStar:
            case EPlanetarySystemType::Mixed:
                NumPlanets = RandomStream.RandRange(3, 8);
                break;
            case EPlanetarySystemType::GasGiantOnly:
                NumPlanets = RandomStream.RandRange(2, 5);
                break;
            case EPlanetarySystemType::TerrestrialOnly:
                NumPlanets = RandomStream.RandRange(4, 10);
                break;
            case EPlanetarySystemType::AsteroidBeltRich:
                NumPlanets = RandomStream.RandRange(1, 3);
                break;
            default:
                NumPlanets = RandomStream.RandRange(3, 8);
                break;
        }
    }

    // Generate planets
    for (int32 i = 0; i < NumPlanets; i++)
    {
        // Calculate orbital radius using modified Titius-Bode law
        float OrbitalRadius = CalculateOrbitalSpacing(i, Config.MinOrbitalRadius, Config.MaxOrbitalRadius, Config.StarConfig.SpectralType);
        
        APlanet* Planet = GeneratePlanet(Star, OrbitalRadius, i, Config.RandomSeed + i);
        if (Planet)
        {
            Planets.Add(Planet);
            
            // Generate moons if requested
            if (Config.bGenerateMoons)
            {
                int32 NumMoons = RandomStream.RandRange(0, Config.MaxMoonsPerPlanet);
                TArray<AOrbitalBody*> Moons = GenerateMoons(Planet, NumMoons, Config.RandomSeed + i * 100);
                
                for (AOrbitalBody* Moon : Moons)
                {
                    if (Moon)
                    {
                        MoonsGenerated++;
                    }
                }
            }
        }
    }

    PlanetsGenerated += Planets.Num();
    return Planets;
}

APlanet* UProceduralStarSystemGenerator::GeneratePlanet(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed)
{
    UWorld* World = GetWorld();
    if (!World || !Star)
    {
        return nullptr;
    }

    // Set random seed for this planet
    RandomStream.Initialize(RandomSeed);

    // Determine planet type based on orbital radius and system type
    float HabitableZoneInner, HabitableZoneOuter;
    CalculateHabitableZone(Star->SurfaceTemperature, HabitableZoneInner, HabitableZoneOuter);
    
    bool bIsInHabitableZone = IsInHabitableZone(OrbitalRadius, HabitableZoneInner, HabitableZoneOuter);
    EOrbitalBodyType PlanetType = GetPlanetTypeProbability(OrbitalRadius, HabitableZoneInner, HabitableZoneOuter);

    // Create appropriate planet type
    APlanet* Planet = nullptr;
    switch (PlanetType)
    {
        case EOrbitalBodyType::Planet:
            if (OrbitalRadius > HabitableZoneOuter * 2.0f)
            {
                Planet = GenerateIceGiant(Star, OrbitalRadius, PlanetIndex, RandomSeed);
            }
            else if (OrbitalRadius > HabitableZoneOuter || OrbitalRadius < HabitableZoneInner)
            {
                Planet = GenerateGasGiant(Star, OrbitalRadius, PlanetIndex, RandomSeed);
            }
            else
            {
                Planet = GenerateTerrestrialPlanet(Star, OrbitalRadius, PlanetIndex, RandomSeed);
            }
            break;
        default:
            Planet = GenerateTerrestrialPlanet(Star, OrbitalRadius, PlanetIndex, RandomSeed);
            break;
    }

    if (!Planet)
    {
        return nullptr;
    }

    // Configure orbital parameters
    float Eccentricity, Inclination, OrbitalPeriod;
    GetRealisticOrbitalParameters(OrbitalRadius, Eccentricity, Inclination, OrbitalPeriod);
    
    // Set planet properties
    Planet->OrbitTarget = Star;
    Planet->OrbitMode = EOrbitMode::Orbit;
    Planet->OrbitRadius = OrbitalRadius * 149597870.7f; // AU to km
    Planet->OrbitInclination = Inclination;
    
    // Set planet name
    FString PlanetName = GeneratePlanetName(Star->GetActorLabel(), PlanetIndex);
    Planet->SetActorLabel(PlanetName);

    // Position planet at orbital radius
    FVector StarLocation = Star->GetActorLocation();
    FVector PlanetPosition = StarLocation + FVector(Planet->OrbitRadius * 100000.0f, 0.0f, 0.0f); // km to cm
    Planet->SetActorLocation(PlanetPosition);

    return Planet;
}

TArray<AOrbitalBody*> UProceduralStarSystemGenerator::GenerateMoons(APlanet* Planet, int32 MaxMoons, int32 RandomSeed)
{
    TArray<AOrbitalBody*> Moons;
    
    if (!Planet || MaxMoons <= 0)
    {
        return Moons;
    }

    // Set random seed for moons
    RandomStream.Initialize(RandomSeed);

    // Determine actual number of moons
    int32 NumMoons = RandomStream.RandRange(0, MaxMoons);
    
    for (int32 i = 0; i < NumMoons; i++)
    {
        // Create moon (simplified as small orbital bodies)
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AOrbitalBody* Moon = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Moon)
        {
            // Configure moon properties
            Moon->Mass = RandomStream.RandRange(1e20f, 1e22f); // Moon mass range
            Moon->Radius = RandomStream.RandRange(100.0f, 2000.0f); // Moon radius range
            Moon->OrbitTarget = Planet;
            Moon->OrbitMode = EOrbitMode::Orbit;
            
            // Calculate moon orbit
            float MoonOrbitRadius = Planet->Radius * 10.0f * (i + 1); // 10-40 planet radii
            Moon->OrbitRadius = MoonOrbitRadius;
            Moon->OrbitInclination = RandomStream.RandRange(-5.0f, 5.0f); // Small inclination
            
            // Position moon
            FVector PlanetLocation = Planet->GetActorLocation();
            FVector MoonPosition = PlanetLocation + FVector(MoonOrbitRadius * 100000.0f, 0.0f, 0.0f);
            Moon->SetActorLocation(MoonPosition);
            
            // Set moon name
            FString MoonName = FString::Printf(TEXT("%s Moon %d"), *Planet->GetActorLabel(), i + 1);
            Moon->SetActorLabel(MoonName);
            
            Moons.Add(Moon);
        }
    }

    return Moons;
}

void UProceduralStarSystemGenerator::GenerateAsteroidBelt(ASun* Star, float InnerRadius, float OuterRadius, int32 AsteroidCount, int32 RandomSeed)
{
    if (!Star || AsteroidCount <= 0)
    {
        return;
    }

    // Set random seed
    RandomStream.Initialize(RandomSeed);

    for (int32 i = 0; i < AsteroidCount; i++)
    {
        // Create asteroid (simplified)
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AOrbitalBody* Asteroid = GetWorld()->SpawnActor<AOrbitalBody>(AOrbitalBody::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Asteroid)
        {
            // Random position in asteroid belt
            float Radius = RandomStream.FRandRange(InnerRadius, OuterRadius);
            float Angle = RandomStream.FRandRange(0.0f, 360.0f);
            
            // Configure asteroid
            Asteroid->Mass = RandomStream.RandRange(1e15f, 1e18f); // Asteroid mass range
            Asteroid->Radius = RandomStream.RandRange(0.1f, 50.0f); // Asteroid radius range
            Asteroid->OrbitTarget = Star;
            Asteroid->OrbitMode = EOrbitMode::Orbit;
            
            // Position asteroid
            FVector StarLocation = Star->GetActorLocation();
            FVector AsteroidPosition = StarLocation + FVector(
                FMath::Cos(Angle) * Radius * 100000.0f,
                FMath::Sin(Angle) * Radius * 100000.0f,
                0.0f
            );
            Asteroid->SetActorLocation(AsteroidPosition);
            
            // Set asteroid name
            FString AsteroidName = FString::Printf(TEXT("Asteroid %d"), i + 1);
            Asteroid->SetActorLabel(AsteroidName);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d asteroids in belt from %.2f to %.2f AU"), 
           AsteroidCount, InnerRadius, OuterRadius);
}

void UProceduralStarSystemGenerator::CalculateHabitableZone(float StellarLuminosity, float& InnerBoundary, float& OuterBoundary) const
{
    // Habitable zone calculations based on stellar luminosity
    // Inner boundary: too hot for liquid water
    // Outer boundary: too cold for liquid water
    
    // Simplified calculation: HZ = sqrt(L/Lsun) * [0.95, 1.37] AU
    float LuminosityFactor = FMath::Sqrt(StellarLuminosity);
    
    InnerBoundary = 0.95f * LuminosityFactor; // AU
    OuterBoundary = 1.37f * LuminosityFactor; // AU
}

void UProceduralStarSystemGenerator::GetRealisticOrbitalParameters(float OrbitalRadius, float& Eccentricity, float& Inclination, float& OrbitalPeriod) const
{
    // Eccentricity: typically 0-0.3 for stable orbits, higher for outer planets
    if (OrbitalRadius < 2.0f) // Inner system
    {
        Eccentricity = RandomStream.FRandRange(0.0f, 0.1f);
        Inclination = RandomStream.FRandRange(-2.0f, 2.0f);
    }
    else if (OrbitalRadius < 10.0f) // Middle system
    {
        Eccentricity = RandomStream.FRandRange(0.0f, 0.2f);
        Inclination = RandomStream.FRandRange(-5.0f, 5.0f);
    }
    else // Outer system
    {
        Eccentricity = RandomStream.FRandRange(0.0f, 0.4f);
        Inclination = RandomStream.FRandRange(-10.0f, 10.0f);
    }

    // Orbital period using Kepler's third law: P² = a³ / M
    // P in years, a in AU, M in solar masses
    OrbitalPeriod = FMath::Sqrt(FMath::Cube(OrbitalRadius)); // Simplified for 1 solar mass

    // Convert to days
    OrbitalPeriod *= 365.25f;
}

void UProceduralStarSystemGenerator::GetRealisticPlanetaryParameters(float OrbitalRadius, float& Mass, float& Radius, bool& bHasAtmosphere) const
{
    // Determine planet characteristics based on orbital radius
    if (OrbitalRadius < 0.5f) // Very close to star - rocky, no atmosphere
    {
        Mass = RandomStream.RandRange(0.1f, 0.5f) * 5.972e24f; // Earth masses to kg
        Radius = RandomStream.RandRange(0.5f, 0.8f) * 6371.0f; // Earth radii to km
        bHasAtmosphere = false;
    }
    else if (OrbitalRadius < 2.0f) // Habitable zone - Earth-like
    {
        Mass = RandomStream.RandRange(0.5f, 2.0f) * 5.972e24f;
        Radius = RandomStream.RandRange(0.8f, 1.5f) * 6371.0f;
        bHasAtmosphere = RandomStream.FRandRange(0.0f, 1.0f) > 0.3f; // 70% chance of atmosphere
    }
    else if (OrbitalRadius < 10.0f) // Outer system - gas giants possible
    {
        if (RandomStream.FRandRange(0.0f, 1.0f) > 0.6f) // 40% chance of gas giant
        {
            Mass = RandomStream.RandRange(10.0f, 300.0f) * 5.972e24f;
            Radius = RandomStream.RandRange(4.0f, 15.0f) * 6371.0f;
            bHasAtmosphere = true;
        }
        else
        {
            Mass = RandomStream.RandRange(0.5f, 5.0f) * 5.972e24f;
            Radius = RandomStream.RandRange(0.8f, 2.0f) * 6371.0f;
            bHasAtmosphere = RandomStream.FRandRange(0.0f, 1.0f) > 0.5f;
        }
    }
    else // Very outer system - ice giants
    {
        Mass = RandomStream.RandRange(5.0f, 50.0f) * 5.972e24f;
        Radius = RandomStream.RandRange(2.0f, 8.0f) * 6371.0f;
        bHasAtmosphere = true;
    }
}

bool UProceduralStarSystemGenerator::IsInHabitableZone(float OrbitalRadius, float InnerBoundary, float OuterBoundary) const
{
    return OrbitalRadius >= InnerBoundary && OrbitalRadius <= OuterBoundary;
}

FString UProceduralStarSystemGenerator::GeneratePlanetName(const FString& StarName, int32 PlanetIndex) const
{
    // Generate planet name based on star name and index
    // Format: StarName + Roman numeral for planet index
    
    static const TArray<FString> RomanNumerals = {
        TEXT("I"), TEXT("II"), TEXT("III"), TEXT("IV"), TEXT("V"),
        TEXT("VI"), TEXT("VII"), TEXT("VIII"), TEXT("IX"), TEXT("X"),
        TEXT("XI"), TEXT("XII"), TEXT("XIII"), TEXT("XIV"), TEXT("XV")
    };

    FString CleanStarName = StarName;
    CleanStarName.RemoveFromEnd(TEXT("-Class Star"));
    CleanStarName.RemoveFromEnd(TEXT(" Star"));

    FString RomanNumeral = (PlanetIndex < RomanNumerals.Num()) ? RomanNumerals[PlanetIndex] : FString::Printf(TEXT("%d"), PlanetIndex + 1);
    
    return FString::Printf(TEXT("%s %s"), *CleanStarName, *RomanNumeral);
}

void UProceduralStarSystemGenerator::GetStarParameters(EStarSpectralType SpectralType, float& Mass, float& Radius, float& Temperature, float& Luminosity) const
{
    // Based on typical values for each spectral type
    switch (SpectralType)
    {
        case EStarSpectralType::O:
            Mass = 20.0f; // Solar masses
            Radius = 10.0f; // Solar radii
            Temperature = 35000.0f; // K
            Luminosity = 100000.0f; // Solar luminosities
            break;
        case EStarSpectralType::B:
            Mass = 8.0f;
            Radius = 5.0f;
            Temperature = 15000.0f;
            Luminosity = 1000.0f;
            break;
        case EStarSpectralType::A:
            Mass = 2.0f;
            Radius = 2.0f;
            Temperature = 9000.0f;
            Luminosity = 20.0f;
            break;
        case EStarSpectralType::F:
            Mass = 1.3f;
            Radius = 1.3f;
            Temperature = 7000.0f;
            Luminosity = 3.0f;
            break;
        case EStarSpectralType::G:
            Mass = 1.0f;
            Radius = 1.0f;
            Temperature = 5778.0f;
            Luminosity = 1.0f;
            break;
        case EStarSpectralType::K:
            Mass = 0.7f;
            Radius = 0.8f;
            Temperature = 4500.0f;
            Luminosity = 0.3f;
            break;
        case EStarSpectralType::M:
            Mass = 0.3f;
            Radius = 0.5f;
            Temperature = 3000.0f;
            Luminosity = 0.05f;
            break;
        default:
            Mass = 1.0f;
            Radius = 1.0f;
            Temperature = 5778.0f;
            Luminosity = 1.0f;
            break;
    }
}

void UProceduralStarSystemGenerator::GenerateStarFromSpectralType(FStarGenerationConfig& Config) const
{
    GetStarParameters(Config.SpectralType, Config.Mass, Config.Radius, Config.Temperature, Config.Luminosity);
}

float UProceduralStarSystemGenerator::CalculateOrbitalSpacing(int32 PlanetIndex, float InnerRadius, float OuterRadius, EStarSpectralType StarType) const
{
    // Modified Titius-Bode law for planetary spacing
    // a = a₀ × rⁿ where r is spacing factor
    
    float SpacingFactor;
    switch (StarType)
    {
        case EStarSpectralType::O:
        case EStarSpectralType::B:
            SpacingFactor = 2.5f; // Larger spacing for massive stars
            break;
        case EStarSpectralType::A:
        case EStarSpectralType::F:
            SpacingFactor = 2.0f;
            break;
        case EStarSpectralType::G:
        case EStarSpectralType::K:
            SpacingFactor = 1.8f; // Solar system-like spacing
            break;
        case EStarSpectralType::M:
            SpacingFactor = 1.5f; // Closer spacing for red dwarfs
            break;
        default:
            SpacingFactor = 1.8f;
            break;
    }

    // Calculate orbital radius
    float Radius = InnerRadius * FMath::Pow(SpacingFactor, PlanetIndex);
    
    // Clamp to outer radius
    if (Radius > OuterRadius)
    {
        Radius = OuterRadius;
    }

    return Radius;
}

EOrbitalBodyType UProceduralStarSystemGenerator::GetPlanetTypeProbability(float OrbitalRadius, float HabitableZoneInner, float HabitableZoneOuter) const
{
    // Determine planet type based on orbital radius and random chance
    float RandomValue = RandomStream.FRandRange(0.0f, 1.0f);
    
    if (OrbitalRadius < HabitableZoneInner)
    {
        // Inner system - rocky planets
        return EOrbitalBodyType::Planet;
    }
    else if (OrbitalRadius > HabitableZoneOuter * 2.0f)
    {
        // Outer system - high chance of gas/ice giants
        if (RandomValue > 0.3f)
        {
            return EOrbitalBodyType::Planet; // Gas/ice giant
        }
        else
        {
            return EOrbitalBodyType::DwarfPlanet;
        }
    }
    else
    {
        // Middle system - mixed
        if (RandomValue > 0.7f)
        {
            return EOrbitalBodyType::Planet; // Gas giant
        }
        else
        {
            return EOrbitalBodyType::Planet; // Terrestrial
        }
    }
}

APlanet* UProceduralStarSystemGenerator::GenerateGasGiant(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed)
{
    // Implementation would create a gas giant planet
    // For now, use the generic planet generation
    return GeneratePlanet(Star, OrbitalRadius, PlanetIndex, RandomSeed);
}

APlanet* UProceduralStarSystemGenerator::GenerateTerrestrialPlanet(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed)
{
    // Implementation would create a terrestrial planet
    // For now, use the generic planet generation
    return GeneratePlanet(Star, OrbitalRadius, PlanetIndex, RandomSeed);
}

APlanet* UProceduralStarSystemGenerator::GenerateIceGiant(ASun* Star, float OrbitalRadius, int32 PlanetIndex, int32 RandomSeed)
{
    // Implementation would create an ice giant planet
    // For now, use the generic planet generation
    return GeneratePlanet(Star, OrbitalRadius, PlanetIndex, RandomSeed);
}