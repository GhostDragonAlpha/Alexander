// Copyright Epic Games, Inc. All Rights Reserved.

#include "StarSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "PerformanceProfilerSubsystem.h"
#include "SpatialPartitioningComponent.h"
#include "AsyncLoadingComponent.h"

UStarSystemManager::UStarSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update orbital positions 10 times per second

    bEnableOrbitalMechanics = true;
    OrbitalSpeedMultiplier = 100.0f; // Speed up orbits for gameplay
    SystemScale = 0.001f; // 1 km = 0.001 UE units (1 meter)
    MaxLoadedSystems = 10; // Increased for better performance
    bAutoGenerateConnections = true;
    MaxConnectionDistance = 50.0f; // Light years
    SystemUnloadDistance = 100.0f; // Distance threshold for unloading
    SpatialPartitioningDepth = 5; // Octree depth
    bEnableAsyncLoading = true; // Enable async loading by default
    AsyncLoadingThreadPoolSize = 4; // Default thread pool size
    TotalGameTime = 0.0f;
    TotalSystemsGenerated = 0;
    TotalLoadTime = 0.0;
}

void UStarSystemManager::BeginPlay()
{
    Super::BeginPlay();

    RandomStream.Initialize(FDateTime::Now().GetTicks());
    
    // Initialize spatial partitioning
    InitializeSpatialPartitioning();
    
    // Initialize async loading
    InitializeAsyncLoading();
    
    UE_LOG(LogTemp, Log, TEXT("StarSystemManager initialized with optimizations: MaxLoadedSystems=%d, AsyncLoading=%s"),
        MaxLoadedSystems, bEnableAsyncLoading ? TEXT("Enabled") : TEXT("Disabled"));
}

void UStarSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Cleanup all systems before destruction
    UnloadAllSystems();
    
    Super::EndPlay(EndPlayReason);
}

void UStarSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Profile star system manager
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
    PROFILE_SCOPE(Profiler, FName("StarSystemManager"));

    TotalGameTime += DeltaTime;

    if (bEnableOrbitalMechanics)
    {
        UpdateOrbitalPositions(DeltaTime);
    }
}

FString UStarSystemManager::CreateStarSystem(const FString& SystemName, const FVector& GalacticPosition, EStarClass StarClass)
{
    FStarSystemData NewSystem;
    NewSystem.SystemID = GenerateUniqueID(TEXT("SYS"));
    NewSystem.SystemName = SystemName.IsEmpty() ? GenerateSystemName() : SystemName;
    NewSystem.GalacticPosition = GalacticPosition;
    NewSystem.StarClass = StarClass;
    NewSystem.bIsDiscovered = false;

    // Create central star
    FCelestialBody Star;
    Star.BodyID = GenerateUniqueID(TEXT("STAR"));
    Star.BodyName = NewSystem.SystemName + TEXT(" Primary");
    Star.BodyType = ECelestialBodyType::Star;
    Star.Radius = 696000.0f; // Sun-like default
    Star.Temperature = GetStarTemperature(StarClass);
    Star.Mass = 1.0f;
    
    NewSystem.CelestialBodies.Add(Star);
    CelestialBodies.Add(Star.BodyID, Star);

    StarSystems.Add(NewSystem.SystemID, NewSystem);

    if (bAutoGenerateConnections)
    {
        ConnectNearbySystem(NewSystem.SystemID);
    }

    OnSystemGenerated.Broadcast(NewSystem.SystemID);
    
    UE_LOG(LogTemp, Log, TEXT("Created star system: %s (%s)"), *NewSystem.SystemName, *NewSystem.SystemID);
    
    return NewSystem.SystemID;
}

FStarSystemData UStarSystemManager::GenerateProceduralSystem(const FString& SystemName, const FProceduralSystemParams& Params)
{
    // Initialize random stream with seed
    FRandomStream LocalRandom;
    LocalRandom.Initialize(Params.RandomSeed != 0 ? Params.RandomSeed : FDateTime::Now().GetTicks());

    // Create base system
    EStarClass StarClass = static_cast<EStarClass>(LocalRandom.RandRange(0, 6));
    FVector GalacticPos(LocalRandom.FRandRange(-10000, 10000), LocalRandom.FRandRange(-10000, 10000), LocalRandom.FRandRange(-1000, 1000));
    
    FString SystemID = CreateStarSystem(SystemName, GalacticPos, StarClass);
    FStarSystemData* System = StarSystems.Find(SystemID);
    
    if (!System)
    {
        return FStarSystemData();
    }

    // Generate planets
    int32 NumPlanets = LocalRandom.RandRange(Params.MinPlanets, Params.MaxPlanets);
    float HabitableZone = CalculateHabitableZone(StarClass);

    for (int32 i = 0; i < NumPlanets; ++i)
    {
        FCelestialBody Planet = GenerateRandomPlanet(SystemID, i);
        
        // Determine planet type based on distance
        float DistanceAU = Planet.OrbitalData.SemiMajorAxis;
        Planet.BodyType = ECelestialBodyType::Planet;
        
        // Generate moons
        int32 NumMoons = LocalRandom.RandRange(Params.MinMoons, Params.MaxMoons);
        for (int32 j = 0; j < NumMoons; ++j)
        {
            FCelestialBody Moon = GenerateRandomMoon(Planet.BodyID, j);
            Planet.ChildBodyIDs.Add(Moon.BodyID);
            CelestialBodies.Add(Moon.BodyID, Moon);
            System->CelestialBodies.Add(Moon);
        }

        System->CelestialBodies.Add(Planet);
        CelestialBodies.Add(Planet.BodyID, Planet);
        System->NumPlanets++;
    }

    // Generate asteroid belt
    if (LocalRandom.FRand() < Params.AsteroidBeltChance)
    {
        int32 BeltOrbit = LocalRandom.RandRange(2, NumPlanets - 2);
        GenerateAsteroidBelt(SystemID, BeltOrbit);
        System->bHasAsteroidBelt = true;
    }

    // Generate space stations
    int32 NumStations = LocalRandom.RandRange(Params.MinStations, Params.MaxStations);
    for (int32 i = 0; i < NumStations; ++i)
    {
        FSpaceStationData Station = GenerateRandomStation(SystemID);
        CreateSpaceStation(SystemID, Station);
    }

    UE_LOG(LogTemp, Log, TEXT("Generated procedural system: %s with %d planets, %d stations"), 
        *System->SystemName, System->NumPlanets, System->NumStations);

    return *System;
}

bool UStarSystemManager::LoadStarSystem(const FString& SystemID)
{
    FScopeLock Lock(&SystemDataLock);

    if (!StarSystems.Contains(SystemID))
    {
        UE_LOG(LogTemp, Warning, TEXT("System not found: %s"), *SystemID);
        return false;
    }

    if (LoadedSystems.Contains(SystemID))
    {
        UE_LOG(LogTemp, Log, TEXT("System already loaded: %s"), *SystemID);
        return true;
    }

    // Unload distant systems if at capacity
    if (LoadedSystems.Num() >= MaxLoadedSystems)
    {
        UnloadDistantSystems(SystemUnloadDistance);
    }

    double LoadStartTime = FPlatformTime::Seconds();

    LoadedSystems.Add(SystemID);
    
    // Spawn visual representations
    FStarSystemData* System = StarSystems.Find(SystemID);
    if (System)
    {
        for (FCelestialBody& Body : System->CelestialBodies)
        {
            FVector SpawnLocation = CalculateOrbitalPosition(Body.OrbitalData, TotalGameTime);
            Body.SpawnedActor = SpawnCelestialBody(Body, SpawnLocation);
        }
    }

    double LoadEndTime = FPlatformTime::Seconds();
    double LoadDuration = LoadEndTime - LoadStartTime;

    // Update performance tracking
    TotalSystemsGenerated++;
    TotalLoadTime += LoadDuration;
    SystemLoadTimes.Add(SystemID, LoadDuration);

    UE_LOG(LogTemp, Log, TEXT("Loaded system: %s in %.2f seconds"), *SystemID, LoadDuration);
    return true;
}

void UStarSystemManager::UnloadStarSystem(const FString& SystemID)
{
    if (!LoadedSystems.Contains(SystemID))
    {
        return;
    }

    DespawnSystemActors(SystemID);
    LoadedSystems.Remove(SystemID);
    
    UE_LOG(LogTemp, Log, TEXT("Unloaded system: %s"), *SystemID);
}

FStarSystemData UStarSystemManager::GetSystemData(const FString& SystemID, bool& bSuccess) const
{
    const FStarSystemData* System = StarSystems.Find(SystemID);
    bSuccess = (System != nullptr);
    return bSuccess ? *System : FStarSystemData();
}

TArray<FString> UStarSystemManager::GetAllSystemIDs() const
{
    TArray<FString> SystemIDs;
    StarSystems.GetKeys(SystemIDs);
    return SystemIDs;
}

FString UStarSystemManager::GetCurrentSystemID() const
{
    return CurrentSystemID;
}

bool UStarSystemManager::SetCurrentSystem(const FString& SystemID)
{
    if (!StarSystems.Contains(SystemID))
    {
        return false;
    }

    FString OldSystemID = CurrentSystemID;
    CurrentSystemID = SystemID;

    LoadStarSystem(SystemID);
    OnSystemChanged.Broadcast(OldSystemID, CurrentSystemID);

    UE_LOG(LogTemp, Log, TEXT("Changed current system from %s to %s"), *OldSystemID, *CurrentSystemID);
    return true;
}

FString UStarSystemManager::AddCelestialBody(const FString& SystemID, const FCelestialBody& Body)
{
    FStarSystemData* System = StarSystems.Find(SystemID);
    if (!System)
    {
        return FString();
    }

    FCelestialBody NewBody = Body;
    if (NewBody.BodyID.IsEmpty())
    {
        NewBody.BodyID = GenerateUniqueID(TEXT("BODY"));
    }

    System->CelestialBodies.Add(NewBody);
    CelestialBodies.Add(NewBody.BodyID, NewBody);

    return NewBody.BodyID;
}

FCelestialBody UStarSystemManager::GetCelestialBody(const FString& BodyID, bool& bSuccess) const
{
    const FCelestialBody* Body = CelestialBodies.Find(BodyID);
    bSuccess = (Body != nullptr);
    return bSuccess ? *Body : FCelestialBody();
}

TArray<FCelestialBody> UStarSystemManager::GetBodiesInSystem(const FString& SystemID) const
{
    const FStarSystemData* System = StarSystems.Find(SystemID);
    return System ? System->CelestialBodies : TArray<FCelestialBody>();
}

TArray<FCelestialBody> UStarSystemManager::GetPlanetsInSystem(const FString& SystemID) const
{
    TArray<FCelestialBody> Planets;
    const FStarSystemData* System = StarSystems.Find(SystemID);
    
    if (System)
    {
        for (const FCelestialBody& Body : System->CelestialBodies)
        {
            if (Body.BodyType == ECelestialBodyType::Planet)
            {
                Planets.Add(Body);
            }
        }
    }

    return Planets;
}

void UStarSystemManager::UpdateOrbitalPositions(float DeltaTime)
{
    for (auto& Pair : CelestialBodies)
    {
        FCelestialBody& Body = Pair.Value;
        
        if (Body.SpawnedActor && !Body.ParentBodyID.IsEmpty())
        {
            // Update orbital position
            float DeltaAngle = (360.0f / (Body.OrbitalData.OrbitalPeriod * 86400.0f)) * DeltaTime * OrbitalSpeedMultiplier;
            Body.OrbitalData.CurrentAnomaly = FMath::Fmod(Body.OrbitalData.CurrentAnomaly + DeltaAngle, 360.0f);

            FVector NewPosition = CalculateOrbitalPosition(Body.OrbitalData, 0.0f);
            Body.SpawnedActor->SetActorLocation(NewPosition);

            // Update rotation
            float RotationDelta = (360.0f / (Body.OrbitalData.RotationPeriod * 3600.0f)) * DeltaTime * OrbitalSpeedMultiplier;
            FRotator CurrentRotation = Body.SpawnedActor->GetActorRotation();
            CurrentRotation.Yaw += RotationDelta;
            Body.SpawnedActor->SetActorRotation(CurrentRotation);
        }
    }
}

FVector UStarSystemManager::CalculateOrbitalPosition(const FOrbitalData& OrbitalData, float TimeOffset) const
{
    float Angle = FMath::DegreesToRadians(OrbitalData.CurrentAnomaly);
    float Distance = OrbitalData.SemiMajorAxis * 149597870.7f * SystemScale; // AU to km to UE units

    // Simple circular orbit (can be extended for elliptical)
    float X = Distance * FMath::Cos(Angle);
    float Y = Distance * FMath::Sin(Angle);
    float Z = Distance * FMath::Sin(FMath::DegreesToRadians(OrbitalData.Inclination));

    return FVector(X, Y, Z);
}

FString UStarSystemManager::CreateSpaceStation(const FString& SystemID, const FSpaceStationData& StationData)
{
    FStarSystemData* System = StarSystems.Find(SystemID);
    if (!System)
    {
        return FString();
    }

    FSpaceStationData NewStation = StationData;
    if (NewStation.StationID.IsEmpty())
    {
        NewStation.StationID = GenerateUniqueID(TEXT("STATION"));
    }
    NewStation.SystemID = SystemID;

    SpaceStations.Add(NewStation.StationID, NewStation);
    System->NumStations++;

    UE_LOG(LogTemp, Log, TEXT("Created space station: %s in system %s"), *NewStation.StationName, *SystemID);
    return NewStation.StationID;
}

FSpaceStationData UStarSystemManager::GetStationData(const FString& StationID, bool& bSuccess) const
{
    const FSpaceStationData* Station = SpaceStations.Find(StationID);
    bSuccess = (Station != nullptr);
    return bSuccess ? *Station : FSpaceStationData();
}

TArray<FSpaceStationData> UStarSystemManager::GetStationsInSystem(const FString& SystemID) const
{
    TArray<FSpaceStationData> Stations;
    
    for (const auto& Pair : SpaceStations)
    {
        if (Pair.Value.SystemID == SystemID)
        {
            Stations.Add(Pair.Value);
        }
    }

    return Stations;
}

FSpaceStationData UStarSystemManager::GetNearestStation(const FVector& Location, float MaxDistance) const
{
    FSpaceStationData NearestStation;
    float MinDistance = MaxDistance;

    for (const auto& Pair : SpaceStations)
    {
        if (Pair.Value.SpawnedActor)
        {
            float Distance = FVector::Dist(Location, Pair.Value.LocalPosition);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestStation = Pair.Value;
            }
        }
    }

    return NearestStation;
}

bool UStarSystemManager::CreateJumpGate(const FString& SourceSystemID, const FString& DestinationSystemID, float JumpCost)
{
    if (!StarSystems.Contains(SourceSystemID) || !StarSystems.Contains(DestinationSystemID))
    {
        return false;
    }

    FJumpGateConnection Gate;
    Gate.GateID = GenerateUniqueID(TEXT("GATE"));
    Gate.SourceSystemID = SourceSystemID;
    Gate.DestinationSystemID = DestinationSystemID;
    Gate.JumpCost = JumpCost;
    Gate.JumpTime = CalculateJumpDistance(SourceSystemID, DestinationSystemID) * 2.0f; // 2 seconds per LY

    JumpGates.Add(Gate);

    UE_LOG(LogTemp, Log, TEXT("Created jump gate from %s to %s"), *SourceSystemID, *DestinationSystemID);
    return true;
}

TArray<FJumpGateConnection> UStarSystemManager::GetJumpGates(const FString& SystemID) const
{
    TArray<FJumpGateConnection> Gates;
    
    for (const FJumpGateConnection& Gate : JumpGates)
    {
        if (Gate.SourceSystemID == SystemID)
        {
            Gates.Add(Gate);
        }
    }

    return Gates;
}

bool UStarSystemManager::CanJumpToSystem(const FString& SourceSystemID, const FString& DestinationSystemID) const
{
    for (const FJumpGateConnection& Gate : JumpGates)
    {
        if (Gate.SourceSystemID == SourceSystemID && Gate.DestinationSystemID == DestinationSystemID)
        {
            return true;
        }
    }

    return false;
}

float UStarSystemManager::CalculateJumpDistance(const FString& SystemID1, const FString& SystemID2) const
{
    const FStarSystemData* System1 = StarSystems.Find(SystemID1);
    const FStarSystemData* System2 = StarSystems.Find(SystemID2);

    if (!System1 || !System2)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(System1->GalacticPosition, System2->GalacticPosition);
    return Distance / 9461000000000.0f; // Convert to light years
}

TArray<FString> UStarSystemManager::FindRoute(const FString& StartSystemID, const FString& EndSystemID) const
{
    TArray<FString> Route;
    
    // Simple breadth-first search for route
    TMap<FString, FString> CameFrom;
    TArray<FString> Queue;
    TSet<FString> Visited;

    Queue.Add(StartSystemID);
    Visited.Add(StartSystemID);

    while (Queue.Num() > 0)
    {
        FString Current = Queue[0];
        Queue.RemoveAt(0);

        if (Current == EndSystemID)
        {
            // Reconstruct path
            FString Step = EndSystemID;
            while (Step != StartSystemID)
            {
                Route.Insert(Step, 0);
                Step = CameFrom[Step];
            }
            Route.Insert(StartSystemID, 0);
            break;
        }

        // Check all connected systems
        TArray<FJumpGateConnection> Gates = GetJumpGates(Current);
        for (const FJumpGateConnection& Gate : Gates)
        {
            if (!Visited.Contains(Gate.DestinationSystemID))
            {
                Queue.Add(Gate.DestinationSystemID);
                Visited.Add(Gate.DestinationSystemID);
                CameFrom.Add(Gate.DestinationSystemID, Current);
            }
        }
    }

    return Route;
}

void UStarSystemManager::DiscoverBody(const FString& BodyID)
{
    if (!DiscoveredBodies.Contains(BodyID))
    {
        DiscoveredBodies.Add(BodyID);
        OnBodyDiscovered.Broadcast(BodyID);
        
        UE_LOG(LogTemp, Log, TEXT("Discovered celestial body: %s"), *BodyID);
    }
}

void UStarSystemManager::DiscoverSystem(const FString& SystemID)
{
    FStarSystemData* System = StarSystems.Find(SystemID);
    if (System && !System->bIsDiscovered)
    {
        System->bIsDiscovered = true;
        DiscoveredSystems.Add(SystemID);
        
        UE_LOG(LogTemp, Log, TEXT("Discovered star system: %s"), *System->SystemName);
    }
}

bool UStarSystemManager::IsBodyDiscovered(const FString& BodyID) const
{
    return DiscoveredBodies.Contains(BodyID);
}

bool UStarSystemManager::IsSystemDiscovered(const FString& SystemID) const
{
    return DiscoveredSystems.Contains(SystemID);
}

AActor* UStarSystemManager::SpawnCelestialBody(const FCelestialBody& Body, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn placeholder actor (in production, use proper mesh/material)
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*Body.BodyName);
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (SpawnedActor)
    {
        // Scale based on body radius
        float Scale = Body.Radius * SystemScale;
        SpawnedActor->SetActorScale3D(FVector(Scale));
    }

    return SpawnedActor;
}

AActor* UStarSystemManager::SpawnSpaceStation(const FSpaceStationData& Station, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*Station.StationName);
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    return SpawnedActor;
}

void UStarSystemManager::DespawnSystemActors(const FString& SystemID)
{
    FStarSystemData* System = StarSystems.Find(SystemID);
    if (!System)
    {
        return;
    }

    for (FCelestialBody& Body : System->CelestialBodies)
    {
        if (Body.SpawnedActor)
        {
            Body.SpawnedActor->Destroy();
            Body.SpawnedActor = nullptr;
        }
    }

    // Despawn stations
    for (auto& Pair : SpaceStations)
    {
        if (Pair.Value.SystemID == SystemID && Pair.Value.SpawnedActor)
        {
            Pair.Value.SpawnedActor->Destroy();
            Pair.Value.SpawnedActor = nullptr;
        }
    }
}

FLinearColor UStarSystemManager::GetStarColor(EStarClass StarClass) const
{
    switch (StarClass)
    {
        case EStarClass::O_Blue:
            return FLinearColor(0.6f, 0.7f, 1.0f);
        case EStarClass::B_BlueWhite:
            return FLinearColor(0.7f, 0.8f, 1.0f);
        case EStarClass::A_White:
            return FLinearColor(0.9f, 0.9f, 1.0f);
        case EStarClass::F_YellowWhite:
            return FLinearColor(1.0f, 0.95f, 0.8f);
        case EStarClass::G_Yellow:
            return FLinearColor(1.0f, 0.9f, 0.6f);
        case EStarClass::K_Orange:
            return FLinearColor(1.0f, 0.7f, 0.4f);
        case EStarClass::M_Red:
            return FLinearColor(1.0f, 0.5f, 0.3f);
        case EStarClass::RedGiant:
            return FLinearColor(1.0f, 0.3f, 0.2f);
        case EStarClass::WhiteDwarf:
            return FLinearColor(0.95f, 0.95f, 1.0f);
        case EStarClass::Neutron:
            return FLinearColor(0.8f, 0.8f, 0.95f);
        case EStarClass::Pulsar:
            return FLinearColor(0.7f, 0.9f, 1.0f);
        default:
            return FLinearColor::White;
    }
}

float UStarSystemManager::GetStarTemperature(EStarClass StarClass) const
{
    switch (StarClass)
    {
        case EStarClass::O_Blue:
            return 30000.0f;
        case EStarClass::B_BlueWhite:
            return 15000.0f;
        case EStarClass::A_White:
            return 9000.0f;
        case EStarClass::F_YellowWhite:
            return 7000.0f;
        case EStarClass::G_Yellow:
            return 5800.0f;
        case EStarClass::K_Orange:
            return 4500.0f;
        case EStarClass::M_Red:
            return 3000.0f;
        case EStarClass::RedGiant:
            return 4000.0f;
        case EStarClass::WhiteDwarf:
            return 8000.0f;
        case EStarClass::Neutron:
            return 600000.0f;
        case EStarClass::Pulsar:
            return 1000000.0f;
        default:
            return 5800.0f;
    }
}

FString UStarSystemManager::GenerateSystemName() const
{
    TArray<FString> Prefixes = {TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"), TEXT("Epsilon"), 
                                 TEXT("Zeta"), TEXT("Nova"), TEXT("Proxima"), TEXT("Rigel"), TEXT("Vega")};
    TArray<FString> Suffixes = {TEXT("Prime"), TEXT("Secundus"), TEXT("Major"), TEXT("Minor"), TEXT("Centauri"),
                                 TEXT("Draconis"), TEXT("Aquilae"), TEXT("Orionis"), TEXT("Cygni"), TEXT("Lyrae")};

    int32 PrefixIdx = RandomStream.RandRange(0, Prefixes.Num() - 1);
    int32 SuffixIdx = RandomStream.RandRange(0, Suffixes.Num() - 1);

    return Prefixes[PrefixIdx] + TEXT(" ") + Suffixes[SuffixIdx];
}

FString UStarSystemManager::GenerateBodyName(ECelestialBodyType BodyType) const
{
    FString TypeName;
    switch (BodyType)
    {
        case ECelestialBodyType::Planet:
            TypeName = TEXT("Planet");
            break;
        case ECelestialBodyType::Moon:
            TypeName = TEXT("Moon");
            break;
        case ECelestialBodyType::Asteroid:
            TypeName = TEXT("Asteroid");
            break;
        default:
            TypeName = TEXT("Body");
            break;
    }

    return TypeName + TEXT(" ") + FString::FromInt(RandomStream.RandRange(1000, 9999));
}

FCelestialBody UStarSystemManager::GenerateRandomPlanet(const FString& SystemID, int32 OrbitIndex)
{
    FCelestialBody Planet;
    Planet.BodyID = GenerateUniqueID(TEXT("PLANET"));
    Planet.BodyName = GenerateBodyName(ECelestialBodyType::Planet);
    Planet.BodyType = ECelestialBodyType::Planet;

    // Orbital parameters
    Planet.OrbitalData.SemiMajorAxis = 0.4f + (OrbitIndex * 0.7f); // AU
    Planet.OrbitalData.Eccentricity = RandomStream.FRandRange(0.0f, 0.2f);
    Planet.OrbitalData.Inclination = RandomStream.FRandRange(-5.0f, 5.0f);
    Planet.OrbitalData.OrbitalPeriod = FMath::Sqrt(FMath::Pow(Planet.OrbitalData.SemiMajorAxis, 3)) * 365.0f;
    Planet.OrbitalData.RotationPeriod = RandomStream.FRandRange(10.0f, 120.0f);
    Planet.OrbitalData.AxialTilt = RandomStream.FRandRange(0.0f, 45.0f);
    Planet.OrbitalData.CurrentAnomaly = RandomStream.FRandRange(0.0f, 360.0f);

    // Physical parameters
    Planet.Radius = RandomStream.FRandRange(2000.0f, 70000.0f);
    Planet.Mass = RandomStream.FRandRange(0.1f, 318.0f);
    Planet.Temperature = RandomStream.FRandRange(150.0f, 700.0f);
    Planet.bHasAtmosphere = RandomStream.FRand() > 0.3f;
    Planet.bHasRings = RandomStream.FRand() > 0.9f;
    Planet.bIsHabitable = (Planet.Temperature > 270.0f && Planet.Temperature < 310.0f && Planet.bHasAtmosphere);

    return Planet;
}

FCelestialBody UStarSystemManager::GenerateRandomMoon(const FString& PlanetID, int32 MoonIndex)
{
    FCelestialBody Moon;
    Moon.BodyID = GenerateUniqueID(TEXT("MOON"));
    Moon.BodyName = GenerateBodyName(ECelestialBodyType::Moon);
    Moon.BodyType = ECelestialBodyType::Moon;
    Moon.ParentBodyID = PlanetID;

    Moon.OrbitalData.SemiMajorAxis = 0.001f + (MoonIndex * 0.002f); // Much closer orbit
    Moon.OrbitalData.OrbitalPeriod = RandomStream.FRandRange(1.0f, 30.0f);
    Moon.OrbitalData.RotationPeriod = Moon.OrbitalData.OrbitalPeriod; // Tidally locked
    Moon.OrbitalData.CurrentAnomaly = RandomStream.FRandRange(0.0f, 360.0f);

    Moon.Radius = RandomStream.FRandRange(500.0f, 3000.0f);
    Moon.Mass = RandomStream.FRandRange(0.001f, 0.1f);
    Moon.Temperature = RandomStream.FRandRange(100.0f, 300.0f);

    return Moon;
}

FSpaceStationData UStarSystemManager::GenerateRandomStation(const FString& SystemID)
{
    FSpaceStationData Station;
    Station.StationID = GenerateUniqueID(TEXT("STATION"));
    Station.SystemID = SystemID;
    
    EStationType Type = static_cast<EStationType>(RandomStream.RandRange(0, 7));
    Station.StationType = Type;
    
    TArray<FString> TypeNames = {TEXT("Trading Hub"), TEXT("Military Base"), TEXT("Research Station"), 
                                  TEXT("Mining Outpost"), TEXT("Refinery"), TEXT("Shipyard"), 
                                  TEXT("Colony"), TEXT("Starport")};
    Station.StationName = TypeNames[static_cast<int32>(Type)] + TEXT(" ") + FString::FromInt(RandomStream.RandRange(100, 999));

    Station.Population = RandomStream.RandRange(100, 50000);
    Station.DockingBays = RandomStream.RandRange(2, 20);
    Station.TradingTax = RandomStream.FRandRange(0.01f, 0.15f);

    Station.AvailableServices.Add(TEXT("Docking"));
    if (Type == EStationType::Trading || Type == EStationType::Starport)
    {
        Station.AvailableServices.Add(TEXT("Trading"));
        Station.AvailableServices.Add(TEXT("Missions"));
    }
    if (Type == EStationType::Military || Type == EStationType::Shipyard)
    {
        Station.AvailableServices.Add(TEXT("Repair"));
        Station.AvailableServices.Add(TEXT("Outfitting"));
    }

    return Station;
}

void UStarSystemManager::GenerateAsteroidBelt(const FString& SystemID, int32 OrbitIndex)
{
    // Generate multiple asteroid markers
    for (int32 i = 0; i < 20; ++i)
    {
        FCelestialBody Asteroid;
        Asteroid.BodyID = GenerateUniqueID(TEXT("ASTEROID"));
        Asteroid.BodyName = TEXT("Asteroid ") + FString::FromInt(i);
        Asteroid.BodyType = ECelestialBodyType::Asteroid;
        
        Asteroid.OrbitalData.SemiMajorAxis = 2.0f + RandomStream.FRandRange(-0.3f, 0.3f);
        Asteroid.OrbitalData.OrbitalPeriod = FMath::Sqrt(FMath::Pow(Asteroid.OrbitalData.SemiMajorAxis, 3)) * 365.0f;
        Asteroid.OrbitalData.CurrentAnomaly = RandomStream.FRandRange(0.0f, 360.0f);
        
        Asteroid.Radius = RandomStream.FRandRange(10.0f, 500.0f);
        Asteroid.Mass = 0.001f;
        
        CelestialBodies.Add(Asteroid.BodyID, Asteroid);
    }
}

void UStarSystemManager::ConnectNearbySystem(const FString& SystemID)
{
    const FStarSystemData* System = StarSystems.Find(SystemID);
    if (!System)
    {
        return;
    }

    for (const auto& Pair : StarSystems)
    {
        if (Pair.Key != SystemID)
        {
            float Distance = CalculateJumpDistance(SystemID, Pair.Key);
            if (Distance <= MaxConnectionDistance)
            {
                CreateJumpGate(SystemID, Pair.Key, Distance * 50.0f);
            }
        }
    }
}

float UStarSystemManager::CalculateHabitableZone(EStarClass StarClass) const
{
    float Luminosity = 1.0f;
    
    switch (StarClass)
    {
        case EStarClass::O_Blue:
            Luminosity = 100000.0f;
            break;
        case EStarClass::B_BlueWhite:
            Luminosity = 10000.0f;
            break;
        case EStarClass::A_White:
            Luminosity = 50.0f;
            break;
        case EStarClass::F_YellowWhite:
            Luminosity = 5.0f;
            break;
        case EStarClass::G_Yellow:
            Luminosity = 1.0f;
            break;
        case EStarClass::K_Orange:
            Luminosity = 0.3f;
            break;
        case EStarClass::M_Red:
            Luminosity = 0.05f;
            break;
        default:
            Luminosity = 1.0f;
            break;
    }

    return FMath::Sqrt(Luminosity); // AU
}

EStarSystemPlanetType UStarSystemManager::DeterminePlanetType(float DistanceFromStar, EStarClass StarClass)
{
    float HabZone = CalculateHabitableZone(StarClass);
    
    if (DistanceFromStar < HabZone * 0.5f)
    {
        	return RandomStream.FRand() > 0.5f ? EStarSystemPlanetType::Lava : EStarSystemPlanetType::Desert;
    }
    else if (DistanceFromStar < HabZone * 0.8f)
    {
        	return EStarSystemPlanetType::Terrestrial;
    }
    else if (DistanceFromStar < HabZone * 1.2f)
    {
        	return EStarSystemPlanetType::Earthlike;
    }
    else if (DistanceFromStar < HabZone * 2.0f)
    {
        	return EStarSystemPlanetType::Frozen;
    }
    else if (DistanceFromStar < HabZone * 5.0f)
    {
        	return EStarSystemPlanetType::GasGiant;
    }
    else
    {
        	return EStarSystemPlanetType::IceGiant;
    }
}

void UStarSystemManager::UnloadDistantSystems()
{
    if (LoadedSystems.Num() == 0 || CurrentSystemID.IsEmpty())
    {
        return;
    }

    // Find furthest system from current
    FString FurthestSystem;
    float MaxDistance = 0.0f;

    for (const FString& SystemID : LoadedSystems)
    {
        if (SystemID != CurrentSystemID)
        {
            float Distance = CalculateJumpDistance(CurrentSystemID, SystemID);
            if (Distance > MaxDistance)
            {
                MaxDistance = Distance;
                FurthestSystem = SystemID;
            }
        }
    }

    if (!FurthestSystem.IsEmpty())
    {
        UnloadStarSystem(FurthestSystem);
    }
}

FString UStarSystemManager::GenerateUniqueID(const FString& Prefix) const
{
    return Prefix + TEXT("_") + FString::FromInt(FDateTime::Now().GetTicks()) + TEXT("_") + FString::FromInt(RandomStream.RandRange(1000, 9999));
}

// Optimization Functions Implementation

void UStarSystemManager::LoadSystemAsync(const FString& SystemID)
{
    if (!bEnableAsyncLoading || !AsyncLoader)
    {
        // Fallback to synchronous loading
        LoadStarSystem(SystemID);
        return;
    }

    if (SystemLoadStates.Contains(SystemID) &&
        (SystemLoadStates[SystemID] == ESystemLoadState::Loading ||
         SystemLoadStates[SystemID] == ESystemLoadState::Loaded))
    {
        return; // Already loading or loaded
    }

    SystemLoadStates.Add(SystemID, ESystemLoadState::Loading);

    // Queue async loading task
    AsyncLoader->QueueLoadingTask(
        SystemID,
        [this, SystemID]() {
            // Load system data (this runs on background thread)
            LoadStarSystem(SystemID);
        },
        [this, SystemID]() {
            // Completion callback (runs on game thread)
            SystemLoadStates.Add(SystemID, ESystemLoadState::Loaded);
            UpdateMemoryTracking(SystemID);
        }
    );

    UE_LOG(LogTemp, Log, TEXT("Queued async load for system: %s"), *SystemID);
}

ESystemLoadState UStarSystemManager::GetSystemLoadState(const FString& SystemID) const
{
    const ESystemLoadState* State = SystemLoadStates.Find(SystemID);
    return State ? *State : ESystemLoadState::Unloaded;
}

void UStarSystemManager::UnloadSystemAsync(const FString& SystemID)
{
    if (!bEnableAsyncLoading || !AsyncLoader)
    {
        // Fallback to synchronous unloading
        UnloadStarSystem(SystemID);
        return;
    }

    if (SystemLoadStates.Contains(SystemID) && SystemLoadStates[SystemID] == ESystemLoadState::Unloaded)
    {
        return; // Already unloaded
    }

    SystemLoadStates.Add(SystemID, ESystemLoadState::Unloading);

    // Queue async unloading task
    AsyncLoader->QueueUnloadingTask(
        SystemID,
        [this, SystemID]() {
            // Unload system data (this runs on background thread)
            DespawnSystemActors(SystemID);
        },
        [this, SystemID]() {
            // Completion callback (runs on game thread)
            LoadedSystems.Remove(SystemID);
            SystemLoadStates.Add(SystemID, ESystemLoadState::Unloaded);
            RemoveFromSpatialIndex(SystemID);
        }
    );

    UE_LOG(LogTemp, Log, TEXT("Queued async unload for system: %s"), *SystemID);
}

TArray<FSpatialQueryResult> UStarSystemManager::FindSystemsInRadius(const FVector& Position, float Radius) const
{
    TArray<FSpatialQueryResult> Results;

    if (SpatialPartitioning)
    {
        TArray<FString> SystemIDs = SpatialPartitioning->FindInRadius(Position, Radius);
        
        for (const FString& SystemID : SystemIDs)
        {
            const FStarSystemData* System = StarSystems.Find(SystemID);
            if (System)
            {
                FSpatialQueryResult Result;
                Result.SystemID = SystemID;
                Result.Position = System->GalacticPosition;
                Result.Distance = FVector::Dist(Position, System->GalacticPosition);
                Results.Add(Result);
            }
        }
    }
    else
    {
        // Fallback to linear search
        for (const auto& Pair : StarSystems)
        {
            float Distance = FVector::Dist(Position, Pair.Value.GalacticPosition);
            if (Distance <= Radius)
            {
                FSpatialQueryResult Result;
                Result.SystemID = Pair.Key;
                Result.Position = Pair.Value.GalacticPosition;
                Result.Distance = Distance;
                Results.Add(Result);
            }
        }
    }

    // Sort by distance
    Results.Sort([](const FSpatialQueryResult& A, const FSpatialQueryResult& B) {
        return A.Distance < B.Distance;
    });

    return Results;
}

TArray<FSpatialQueryResult> UStarSystemManager::FindNearestSystems(const FVector& Position, int32 MaxCount) const
{
    TArray<FSpatialQueryResult> Results;

    if (SpatialPartitioning && MaxCount > 0)
    {
        TArray<FString> SystemIDs = SpatialPartitioning->FindNearest(Position, MaxCount);
        
        for (const FString& SystemID : SystemIDs)
        {
            const FStarSystemData* System = StarSystems.Find(SystemID);
            if (System)
            {
                FSpatialQueryResult Result;
                Result.SystemID = SystemID;
                Result.Position = System->GalacticPosition;
                Result.Distance = FVector::Dist(Position, System->GalacticPosition);
                Results.Add(Result);
            }
        }
    }
    else
    {
        // Fallback to linear search with sorting
        for (const auto& Pair : StarSystems)
        {
            FSpatialQueryResult Result;
            Result.SystemID = Pair.Key;
            Result.Position = Pair.Value.GalacticPosition;
            Result.Distance = FVector::Dist(Position, Pair.Value.GalacticPosition);
            Results.Add(Result);
        }

        // Sort and limit results
        Results.Sort([](const FSpatialQueryResult& A, const FSpatialQueryResult& B) {
            return A.Distance < B.Distance;
        });

        if (MaxCount > 0 && Results.Num() > MaxCount)
        {
            Results.SetNum(MaxCount);
        }
    }

    return Results;
}

TArray<FSpatialQueryResult> UStarSystemManager::FindSystemsInBox(const FVector& Center, const FVector& Extent) const
{
    TArray<FSpatialQueryResult> Results;

    if (SpatialPartitioning)
    {
        TArray<FString> SystemIDs = SpatialPartitioning->FindInBox(Center, Extent);
        
        for (const FString& SystemID : SystemIDs)
        {
            const FStarSystemData* System = StarSystems.Find(SystemID);
            if (System)
            {
                FSpatialQueryResult Result;
                Result.SystemID = SystemID;
                Result.Position = System->GalacticPosition;
                Result.Distance = FVector::Dist(Center, System->GalacticPosition);
                Results.Add(Result);
            }
        }
    }
    else
    {
        // Fallback to linear search
        for (const auto& Pair : StarSystems)
        {
            FVector Delta = Pair.Value.GalacticPosition - Center;
            if (FMath::Abs(Delta.X) <= Extent.X &&
                FMath::Abs(Delta.Y) <= Extent.Y &&
                FMath::Abs(Delta.Z) <= Extent.Z)
            {
                FSpatialQueryResult Result;
                Result.SystemID = Pair.Key;
                Result.Position = Pair.Value.GalacticPosition;
                Result.Distance = Delta.Size();
                Results.Add(Result);
            }
        }
    }

    return Results;
}

FSystemMemoryInfo UStarSystemManager::GetSystemMemoryInfo(const FString& SystemID) const
{
    const FSystemMemoryInfo* Info = SystemMemoryTracker.Find(SystemID);
    return Info ? *Info : FSystemMemoryInfo();
}

int64 UStarSystemManager::GetTotalMemoryUsage() const
{
    int64 TotalMemory = 0;
    for (const auto& Pair : SystemMemoryTracker)
    {
        TotalMemory += Pair.Value.TotalSize;
    }
    return TotalMemory;
}

void UStarSystemManager::UnloadDistantSystems(float MaxDistance)
{
    if (CurrentSystemID.IsEmpty())
    {
        return;
    }

    const FStarSystemData* CurrentSystem = StarSystems.Find(CurrentSystemID);
    if (!CurrentSystem)
    {
        return;
    }

    TArray<FString> SystemsToUnload;
    
    for (const FString& SystemID : LoadedSystems)
    {
        if (SystemID == CurrentSystemID)
        {
            continue;
        }

        float Distance = CalculateJumpDistance(CurrentSystemID, SystemID);
        if (Distance > MaxDistance)
        {
            SystemsToUnload.Add(SystemID);
        }
    }

    for (const FString& SystemID : SystemsToUnload)
    {
        if (bEnableAsyncLoading)
        {
            UnloadSystemAsync(SystemID);
        }
        else
        {
            UnloadStarSystem(SystemID);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Unloaded %d distant systems"), SystemsToUnload.Num());
}

void UStarSystemManager::SetMaxLoadedSystems(int32 MaxSystems)
{
    MaxLoadedSystems = FMath::Max(1, MaxSystems);
    
    // Unload excess systems if needed
    while (LoadedSystems.Num() > MaxLoadedSystems)
    {
        UnloadDistantSystems(SystemUnloadDistance);
    }
}

void UStarSystemManager::PreloadSystemsInRadius(const FVector& Position, float Radius)
{
    TArray<FSpatialQueryResult> NearbySystems = FindSystemsInRadius(Position, Radius);
    
    for (const FSpatialQueryResult& Result : NearbySystems)
    {
        if (!LoadedSystems.Contains(Result.SystemID))
        {
            if (bEnableAsyncLoading)
            {
                LoadSystemAsync(Result.SystemID);
            }
            else
            {
                LoadStarSystem(Result.SystemID);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Preloading %d systems in radius %.2f"), NearbySystems.Num(), Radius);
}

void UStarSystemManager::UnloadAllSystems()
{
    TArray<FString> SystemsToUnload = LoadedSystems;
    
    for (const FString& SystemID : SystemsToUnload)
    {
        if (bEnableAsyncLoading)
        {
            UnloadSystemAsync(SystemID);
        }
        else
        {
            UnloadStarSystem(SystemID);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Unloaded all %d systems"), SystemsToUnload.Num());
}

int32 UStarSystemManager::GetTotalSystemCount() const
{
    return StarSystems.Num();
}

int32 UStarSystemManager::GetLoadedSystemCount() const
{
    return LoadedSystems.Num();
}

int32 UStarSystemManager::GetDiscoveredSystemCount() const
{
    return DiscoveredSystems.Num();
}

float UStarSystemManager::GetAverageSystemLoadTime() const
{
    if (TotalSystemsGenerated == 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(TotalLoadTime / TotalSystemsGenerated);
}

// Helper Functions for Optimization

void UStarSystemManager::UpdateSpatialIndex(const FString& SystemID, const FVector& Position)
{
    FScopeLock Lock(&SpatialIndexLock);
    
    // Remove from any existing position
    RemoveFromSpatialIndex(SystemID);
    
    // Add to new position with coarse grid for fast lookup
    FVector GridCell(FMath::RoundToInt(Position.X / 1000.0f) * 1000.0f,
                     FMath::RoundToInt(Position.Y / 1000.0f) * 1000.0f,
                     FMath::RoundToInt(Position.Z / 1000.0f) * 1000.0f);
    
    TArray<FString>& SystemsInCell = SpatialIndex.FindOrAdd(GridCell);
    SystemsInCell.AddUnique(SystemID);
}

void UStarSystemManager::RemoveFromSpatialIndex(const FString& SystemID)
{
    FScopeLock Lock(&SpatialIndexLock);
    
    for (auto& Pair : SpatialIndex)
    {
        Pair.Value.Remove(SystemID);
    }
}

void UStarSystemManager::InitializeSpatialPartitioning()
{
    if (!SpatialPartitioning)
    {
        SpatialPartitioning = NewObject<USpatialPartitioningComponent>(this, USpatialPartitioningComponent::StaticClass(), TEXT("SpatialPartitioning"));
        SpatialPartitioning->RegisterComponent();
    }
    
    // Initialize with galaxy-sized bounds
    SpatialPartitioning->Initialize(FVector::ZeroVector, 200000.0f, SpatialPartitioningDepth);
}

void UStarSystemManager::InitializeAsyncLoading()
{
    if (!AsyncLoader && bEnableAsyncLoading)
    {
        AsyncLoader = NewObject<UAsyncLoadingComponent>(this, UAsyncLoadingComponent::StaticClass(), TEXT("AsyncLoader"));
        AsyncLoader->RegisterComponent();
        AsyncLoader->Initialize(AsyncLoadingThreadPoolSize);
    }
}

bool UStarSystemManager::ShouldUnloadSystem(const FString& SystemID) const
{
    if (SystemID == CurrentSystemID)
    {
        return false; // Never unload current system
    }

    // Check reference count
    const int32* RefCount = SystemReferenceCount.Find(SystemID);
    if (RefCount && *RefCount > 0)
    {
        return false; // System is still referenced
    }

    // Check distance from current system
    float Distance = CalculateJumpDistance(CurrentSystemID, SystemID);
    return Distance > SystemUnloadDistance;
}

void UStarSystemManager::UpdateMemoryTracking(const FString& SystemID)
{
    FSystemMemoryInfo MemoryInfo;
    
    const FStarSystemData* System = StarSystems.Find(SystemID);
    if (System)
    {
        // Estimate memory usage (these are rough estimates)
        MemoryInfo.SystemDataSize = sizeof(FStarSystemData);
        MemoryInfo.CelestialBodiesSize = System->CelestialBodies.Num() * sizeof(FCelestialBody);
        
        // Count stations in this system
        int32 StationCount = 0;
        for (const auto& StationPair : SpaceStations)
        {
            if (StationPair.Value.SystemID == SystemID)
            {
                StationCount++;
            }
        }
        MemoryInfo.StationsSize = StationCount * sizeof(FSpaceStationData);
        
        MemoryInfo.TotalSize = MemoryInfo.SystemDataSize +
                               MemoryInfo.CelestialBodiesSize +
                               MemoryInfo.StationsSize;
    }
    
    SystemMemoryTracker.Add(SystemID, MemoryInfo);
}
