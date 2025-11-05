// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "StarSystemManager.generated.h"

// Forward declarations
class AActor;
class UStaticMeshComponent;

/**
 * Celestial body types
 */
UENUM(BlueprintType)
enum class ECelestialBodyType : uint8
{
    Star            UMETA(DisplayName = "Star"),
    Planet          UMETA(DisplayName = "Planet"),
    Moon            UMETA(DisplayName = "Moon"),
    Asteroid        UMETA(DisplayName = "Asteroid"),
    Comet           UMETA(DisplayName = "Comet"),
    SpaceStation    UMETA(DisplayName = "Space Station"),
    Stargate        UMETA(DisplayName = "Stargate"),
    BlackHole       UMETA(DisplayName = "Black Hole")
};

/**
 * Star classes
 */
UENUM(BlueprintType)
enum class EStarClass : uint8
{
    O_Blue          UMETA(DisplayName = "O-Class (Blue)"),
    B_BlueWhite     UMETA(DisplayName = "B-Class (Blue-White)"),
    A_White         UMETA(DisplayName = "A-Class (White)"),
    F_YellowWhite   UMETA(DisplayName = "F-Class (Yellow-White)"),
    G_Yellow        UMETA(DisplayName = "G-Class (Yellow)"),
    K_Orange        UMETA(DisplayName = "K-Class (Orange)"),
    M_Red           UMETA(DisplayName = "M-Class (Red)"),
    RedGiant        UMETA(DisplayName = "Red Giant"),
    WhiteDwarf      UMETA(DisplayName = "White Dwarf"),
    Neutron         UMETA(DisplayName = "Neutron Star"),
    Pulsar          UMETA(DisplayName = "Pulsar")
};

/**
 * Star system planet classification types
 */
UENUM(BlueprintType)
enum class EStarSystemPlanetType : uint8
{
    Terrestrial     UMETA(DisplayName = "Terrestrial"),
    GasGiant        UMETA(DisplayName = "Gas Giant"),
    IceGiant        UMETA(DisplayName = "Ice Giant"),
    Desert          UMETA(DisplayName = "Desert"),
    Ocean           UMETA(DisplayName = "Ocean"),
    Lava            UMETA(DisplayName = "Lava"),
    Frozen          UMETA(DisplayName = "Frozen"),
    Toxic           UMETA(DisplayName = "Toxic"),
    Barren          UMETA(DisplayName = "Barren"),
    Earthlike       UMETA(DisplayName = "Earth-like")
};

/**
 * Space station types
 */
UENUM(BlueprintType)
enum class EStationType : uint8
{
    Trading         UMETA(DisplayName = "Trading Hub"),
    Military        UMETA(DisplayName = "Military Base"),
    Research        UMETA(DisplayName = "Research Station"),
    Mining          UMETA(DisplayName = "Mining Outpost"),
    Refinery        UMETA(DisplayName = "Refinery"),
    Shipyard        UMETA(DisplayName = "Shipyard"),
    Colony          UMETA(DisplayName = "Colony"),
    Starport        UMETA(DisplayName = "Starport")
};

/**
 * Orbital mechanics data
 */
USTRUCT(BlueprintType)
struct FOrbitalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float SemiMajorAxis; // AU or km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float Eccentricity; // 0 = circular, 0.9 = highly elliptical

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float Inclination; // Degrees from ecliptic plane

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float OrbitalPeriod; // Days

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float RotationPeriod; // Hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float AxialTilt; // Degrees

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbital")
    float CurrentAnomaly; // Current position in orbit (0-360 degrees)

    FOrbitalData()
        : SemiMajorAxis(1.0f)
        , Eccentricity(0.0f)
        , Inclination(0.0f)
        , OrbitalPeriod(365.0f)
        , RotationPeriod(24.0f)
        , AxialTilt(0.0f)
        , CurrentAnomaly(0.0f)
    {}
};

/**
 * Celestial body data
 */
USTRUCT(BlueprintType)
struct FCelestialBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    FString BodyID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    FString BodyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    ECelestialBodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    float Radius; // km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    float Mass; // Earth masses

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    float Temperature; // Kelvin

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    FOrbitalData OrbitalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    FString ParentBodyID; // What it orbits

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    TArray<FString> ChildBodyIDs; // Moons, stations, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    bool bHasAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    bool bHasRings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    bool bIsHabitable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial")
    AActor* SpawnedActor;

    FCelestialBody()
        : BodyType(ECelestialBodyType::Planet)
        , Radius(6371.0f)
        , Mass(1.0f)
        , Temperature(288.0f)
        , bHasAtmosphere(false)
        , bHasRings(false)
        , bIsHabitable(false)
        , SpawnedActor(nullptr)
    {}
};

/**
 * Star system data
 */
USTRUCT(BlueprintType)
struct FStarSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    FString SystemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    FVector GalacticPosition; // Position in galaxy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    EStarClass StarClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    TArray<FCelestialBody> CelestialBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    int32 NumPlanets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    int32 NumStations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    bool bHasAsteroidBelt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    bool bIsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    FString ControllingFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    float SecurityLevel; // 0.0 = lawless, 1.0 = maximum security

    FStarSystemData()
        : StarClass(EStarClass::G_Yellow)
        , NumPlanets(0)
        , NumStations(0)
        , bHasAsteroidBelt(false)
        , bIsDiscovered(false)
        , SecurityLevel(0.5f)
    {}
};

/**
 * Space station data
 */
USTRUCT(BlueprintType)
struct FSpaceStationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    FString StationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    FString StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    EStationType StationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    FString SystemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    int32 Population;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    int32 DockingBays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    TArray<FString> AvailableServices; // Trading, Repair, Missions, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    FString OwningFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    float TradingTax; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
    AActor* SpawnedActor;

    FSpaceStationData()
        : StationType(EStationType::Trading)
        , LocalPosition(FVector::ZeroVector)
        , Population(1000)
        , DockingBays(4)
        , TradingTax(0.05f)
        , SpawnedActor(nullptr)
    {}
};

/**
 * Jump gate connection
 */
USTRUCT(BlueprintType)
struct FJumpGateConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    FString GateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    FString SourceSystemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    FString DestinationSystemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    float JumpCost; // Fuel or credits

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    float JumpTime; // Seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpGate")
    bool bRequiresPermission;

    FJumpGateConnection()
        : JumpCost(100.0f)
        , JumpTime(5.0f)
        , bRequiresPermission(false)
    {}
};

/**
 * Procedural generation parameters
 */
USTRUCT(BlueprintType)
struct FProceduralSystemParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MinPlanets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MaxPlanets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MinMoons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MaxMoons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    float AsteroidBeltChance; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MinStations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 MaxStations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 RandomSeed;

    FProceduralSystemParams()
        : MinPlanets(3)
        , MaxPlanets(12)
        , MinMoons(0)
        , MaxMoons(3)
        , AsteroidBeltChance(0.3f)
        , MinStations(1)
        , MaxStations(5)
        , RandomSeed(0)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemGenerated, const FString&, SystemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemChanged, const FString&, OldSystemID, const FString&, NewSystemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyDiscovered, const FString&, BodyID);

/**
 * Star System Manager Component
 * Manages multiple star systems, procedural generation, and inter-system travel
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UStarSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UStarSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString CreateStarSystem(const FString& SystemName, const FVector& GalacticPosition, EStarClass StarClass);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FStarSystemData GenerateProceduralSystem(const FString& SystemName, const FProceduralSystemParams& Params);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool LoadStarSystem(const FString& SystemID);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    void UnloadStarSystem(const FString& SystemID);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FStarSystemData GetSystemData(const FString& SystemID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FString> GetAllSystemIDs() const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString GetCurrentSystemID() const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool SetCurrentSystem(const FString& SystemID);

    // Celestial Bodies
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString AddCelestialBody(const FString& SystemID, const FCelestialBody& Body);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FCelestialBody GetCelestialBody(const FString& BodyID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FCelestialBody> GetBodiesInSystem(const FString& SystemID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FCelestialBody> GetPlanetsInSystem(const FString& SystemID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    void UpdateOrbitalPositions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FVector CalculateOrbitalPosition(const FOrbitalData& OrbitalData, float TimeOffset) const;

    // Space Stations
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString CreateSpaceStation(const FString& SystemID, const FSpaceStationData& StationData);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FSpaceStationData GetStationData(const FString& StationID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FSpaceStationData> GetStationsInSystem(const FString& SystemID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FSpaceStationData GetNearestStation(const FVector& Location, float MaxDistance) const;

    // Inter-System Travel
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool CreateJumpGate(const FString& SourceSystemID, const FString& DestinationSystemID, float JumpCost);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FJumpGateConnection> GetJumpGates(const FString& SystemID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool CanJumpToSystem(const FString& SourceSystemID, const FString& DestinationSystemID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    float CalculateJumpDistance(const FString& SystemID1, const FString& SystemID2) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    TArray<FString> FindRoute(const FString& StartSystemID, const FString& EndSystemID) const;

    // Discovery
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    void DiscoverBody(const FString& BodyID);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    void DiscoverSystem(const FString& SystemID);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool IsBodyDiscovered(const FString& BodyID) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    bool IsSystemDiscovered(const FString& SystemID) const;

    // Visual Spawning
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    AActor* SpawnCelestialBody(const FCelestialBody& Body, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    AActor* SpawnSpaceStation(const FSpaceStationData& Station, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    void DespawnSystemActors(const FString& SystemID);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FLinearColor GetStarColor(EStarClass StarClass) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    float GetStarTemperature(EStarClass StarClass) const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString GenerateSystemName() const;

    UFUNCTION(BlueprintCallable, Category = "StarSystem")
    FString GenerateBodyName(ECelestialBodyType BodyType) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "StarSystem")
    FOnSystemGenerated OnSystemGenerated;

    UPROPERTY(BlueprintAssignable, Category = "StarSystem")
    FOnSystemChanged OnSystemChanged;

    UPROPERTY(BlueprintAssignable, Category = "StarSystem")
    FOnBodyDiscovered OnBodyDiscovered;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    bool bEnableOrbitalMechanics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    float OrbitalSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    float SystemScale; // km to UE units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    int32 MaxLoadedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    bool bAutoGenerateConnections;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StarSystem")
    float MaxConnectionDistance; // Light years

private:
    UPROPERTY()
    TMap<FString, FStarSystemData> StarSystems;

    UPROPERTY()
    TMap<FString, FCelestialBody> CelestialBodies;

    UPROPERTY()
    TMap<FString, FSpaceStationData> SpaceStations;

    UPROPERTY()
    TArray<FJumpGateConnection> JumpGates;

    UPROPERTY()
    TSet<FString> DiscoveredBodies;

    UPROPERTY()
    TSet<FString> DiscoveredSystems;

    UPROPERTY()
    TArray<FString> LoadedSystems;

    FString CurrentSystemID;
    float TotalGameTime;
    FRandomStream RandomStream;

    // Helper functions
    FCelestialBody GenerateRandomPlanet(const FString& SystemID, int32 OrbitIndex);
    FCelestialBody GenerateRandomMoon(const FString& PlanetID, int32 MoonIndex);
    FSpaceStationData GenerateRandomStation(const FString& SystemID);
    void GenerateAsteroidBelt(const FString& SystemID, int32 OrbitIndex);
    void ConnectNearbySystem(const FString& SystemID);
    float CalculateHabitableZone(EStarClass StarClass) const;
    EStarSystemPlanetType DeterminePlanetType(float DistanceFromStar, EStarClass StarClass);
    void UnloadDistantSystems();
    FString GenerateUniqueID(const FString& Prefix) const;
};
