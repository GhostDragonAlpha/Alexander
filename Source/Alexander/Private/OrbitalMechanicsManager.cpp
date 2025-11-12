// Copyright Epic Games, Inc. All Rights Reserved.

#include "OrbitalMechanicsManager.h"
#include "OrbitalBody.h"
#include "VrBubbleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "SystemSelfTestInterface.h"

UOrbitalMechanicsManager::UOrbitalMechanicsManager()
{
    bEnableAutomaticSOITransitions = true;
    bEnableDebugVisualization = false;
    MaxTimeStep = 0.1f; // 100ms max timestep for stability
    GravityCalculationMode = EGravityCalculationMode::PatchedConics;
    LastUpdateTime = 0.0f;
    UpdateCount = 0;
}

void UOrbitalMechanicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create sub-components
    HohmannCalculator = NewObject<UHohmannTransferCalculator>(this);
    SOIManager = NewObject<USphereOfInfluenceManager>(this);
    SystemGenerator = NewObject<UProceduralStarSystemGenerator>(this);
    SpatialPartitioner = NewObject<USpatialPartitioningOctree>(this);

    // Initialize spatial partitioner with large bounds (1 billion km)
    FBox WorldBounds(
        FVector(-1e11f, -1e11f, -1e11f), // -100 million km
        FVector(1e11f, 1e11f, 1e11f)     // +100 million km
    );
    SpatialPartitioner->Initialize(WorldBounds);

    UE_LOG(LogTemp, Log, TEXT("Orbital Mechanics Manager initialized"));
}

void UOrbitalMechanicsManager::Deinitialize()
{
    // Clean up resources
    RegisteredBodies.Empty();
    BodyRegistry.Empty();

    UE_LOG(LogTemp, Log, TEXT("Orbital Mechanics Manager deinitialized"));
    Super::Deinitialize();
}

//~ Begin ISystemSelfTestInterface interface
bool UOrbitalMechanicsManager::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
    OutResult.SystemName = GetSystemName_Implementation();
    OutResult.WarningMessages.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("=== Testing Orbital Mechanics Manager ==="));
    
    // Test 1: Check if manager is properly initialized
    if (!HohmannCalculator || !SOIManager || !SystemGenerator || !SpatialPartitioner)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Orbital Mechanics Manager not properly initialized - missing subsystems");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Manager not properly initialized"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Subsystem initialization test PASSED"));
    
    // Test 2: Test star system generation
    FStarSystemConfig TestConfig;
    TestConfig.StarClass = EStarClass::G;
    TestConfig.StarMass = 1.0f;
    TestConfig.StarRadius = 696340.0f; // Sun radius in km
    TestConfig.MinPlanets = 2;
    TestConfig.MaxPlanets = 5;
    
    AOrbitalBody* GeneratedStar = GenerateStarSystem(TestConfig);
    if (!GeneratedStar)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Failed to generate test star system");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Star system generation failed"));
        return false;
    }
 87
 88     // Verify generated bodies were registered
 89     if (RegisteredBodies.Num() < 3) // At least star + 2 planets
 90     {
 91         OutResult.bPassed = false;
 92         OutResult.ErrorMessage = TEXT("Generated star system but bodies were not properly registered");
 93         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Body registration failed"));
 94         return false;
 95     }
 96     UE_LOG(LogTemp, Log, TEXT("✓ Star system generation test PASSED - Generated %d bodies"), RegisteredBodies.Num());
 97
 98     // Test 3: Test orbital calculations
 99     if (RegisteredBodies.Num() >= 2)
100     {
101         AOrbitalBody* TestBody1 = RegisteredBodies[0].Get();
102         AOrbitalBody* TestBody2 = RegisteredBodies[1].Get();
103
104         if (TestBody1 && TestBody2)
105         {
106             // Test orbital position calculation
107             FVector InitialPosition = TestBody1->GetActorLocation();
108             FVector NewPosition = CalculateOrbitalPosition(TestBody1, 1.0f);
109
110             if (NewPosition.Equals(InitialPosition, 1.0f)) // Positions should be different after 1 second
111             {
112                 OutResult.bPassed = false;
113                 OutResult.ErrorMessage = TEXT("Orbital position calculation failed - position did not change");
114                 UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Orbital position calculation"));
115                 return false;
116             }
117             UE_LOG(LogTemp, Log, TEXT("✓ Orbital position calculation test PASSED"));
118
119             // Test gravitational force calculation
120             FVector GravitationalForce = CalculateGravitationalForce(TestBody1, TestBody2);
121             if (GravitationalForce.IsNearlyZero())
122             {
123                 OutResult.bPassed = false;
124                 OutResult.ErrorMessage = TEXT("Gravitational force calculation failed - zero force returned");
125                 UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Gravitational force calculation"));
126                 return false;
127             }
128             UE_LOG(LogTemp, Log, TEXT("✓ Gravitational force calculation test PASSED"));
129
130             // Test Hohmann transfer calculation
131             float DeltaV1, DeltaV2, TransferTime;
132             if (!CalculateHohmannTransfer(TestBody1, TestBody2, DeltaV1, DeltaV2, TransferTime))
133             {
134                 OutResult.bPassed = false;
135                 OutResult.ErrorMessage = TEXT("Hohmann transfer calculation failed");
136                 UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Hohmann transfer calculation"));
137                 return false;
138             }
139             UE_LOG(LogTemp, Log, TEXT("✓ Hohmann transfer calculation test PASSED"));
140         }
141     }
142
143     // Test 4: Test spatial partitioning
144     FVector TestPosition = FVector::ZeroVector;
145     float TestRadius = 1000000000.0f; // 10,000 km in cm
146     TArray<AOrbitalBody*> FoundBodies = FindBodiesInRadius(TestPosition, TestRadius);
147
148     if (FoundBodies.Num() != RegisteredBodies.Num())
149     {
150         OutResult.WarningMessages.Add(TEXT("Spatial partitioning may not be working optimally - found different number of bodies than registered"));
151         UE_LOG(LogTemp, Warning, TEXT("WARNING: Spatial partitioning test found %d bodies, expected %d"),
152             FoundBodies.Num(), RegisteredBodies.Num());
153     }
154     else
155     {
156         UE_LOG(LogTemp, Log, TEXT("✓ Spatial partitioning test PASSED"));
157     }
158
159     // Test 5: Test sphere of influence calculations
160     if (SOIManager && RegisteredBodies.Num() > 0)
161     {
162         AOrbitalBody* TestBody = RegisteredBodies[0].Get();
163         if (TestBody)
164         {
165             FVector BodyPosition = TestBody->GetActorLocation();
166             bool bInSOI = IsInSphereOfInfluence(BodyPosition, TestBody);
167
168             if (!bInSOI)
169             {
170                 OutResult.WarningMessages.Add(TEXT("Sphere of influence test returned unexpected result"));
171                 UE_LOG(LogTemp, Warning, TEXT("WARNING: SOI test - body not in its own SOI"));
172             }
173             else
174             {
175                 UE_LOG(LogTemp, Log, TEXT("✓ Sphere of influence test PASSED"));
176             }
177         }
178     }
179
180     // Test 6: Performance metrics
181     float TestDuration = 5.0f; // 5 second test
182     float StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
183     int32 UpdatesDuringTest = 0;
184
185     while (GetWorld() && (GetWorld()->GetTimeSeconds() - StartTime) < TestDuration)
186     {
187         Tick(0.016f); // Simulate 60 FPS
188         UpdatesDuringTest++;
189     }
190
191     float ActualDuration = GetWorld() ? (GetWorld()->GetTimeSeconds() - StartTime) : TestDuration;
192     float AverageUpdateTime = ActualDuration / UpdatesDuringTest;
193
194     if (AverageUpdateTime > 0.1f) // Should update faster than 10 FPS
195     {
196         OutResult.WarningMessages.Add(FString::Printf(TEXT("Performance warning - average update time: %.2f ms"),
197             AverageUpdateTime * 1000.0f));
198         UE_LOG(LogTemp, Warning, TEXT("WARNING: Performance test - average update time: %.2f ms"),
199             AverageUpdateTime * 1000.0f);
200     }
201     else
202     {
203         UE_LOG(LogTemp, Log, TEXT("✓ Performance test PASSED - average update time: %.2f ms"),
204             AverageUpdateTime * 1000.0f);
205     }
206
207     OutResult.bPassed = true;
208     OutResult.PerformanceMetrics.Add(TEXT("AverageUpdateTime"), AverageUpdateTime);
209     OutResult.PerformanceMetrics.Add(TEXT("BodiesGenerated"), RegisteredBodies.Num());
210     OutResult.PerformanceMetrics.Add(TEXT("UpdatesDuringTest"), UpdatesDuringTest);
211
212     UE_LOG(LogTemp, Log, TEXT("=== Orbital Mechanics Manager test PASSED ==="));
213     return true;
214 }
215
216 FString UOrbitalMechanicsManager::GetSystemName_Implementation() const
217 {
218     return TEXT("OrbitalMechanicsManager");
219 }
220
220 FString UOrbitalMechanicsManager::GetTestDescription_Implementation() const
221 {
222     return TEXT("Tests orbital mechanics calculations, star system generation, and spatial partitioning");
223 }
224
225 bool UOrbitalMechanicsManager::IsReadyForTesting_Implementation() const
226 {
227     return HohmannCalculator != nullptr && SOIManager != nullptr &&
228            SystemGenerator != nullptr && SpatialPartitioner != nullptr;
229 }
230 //~ End ISystemSelfTestInterface interface
231
232 void UOrbitalMechanicsManager::Tick(float DeltaTime)
233 {
234     Super::Tick(DeltaTime);

    // Clamp delta time for stability
    float ClampedDeltaTime = FMath::Min(DeltaTime, MaxTimeStep);

    // Update orbital bodies
    UpdateOrbitalBodies(ClampedDeltaTime);

    // Update sphere of influence
    if (SOIManager && bEnableAutomaticSOITransitions)
    {
        SOIManager->UpdateSOI(ClampedDeltaTime);
    }

    // Apply gravity
    ApplyGravity(ClampedDeltaTime);

    // Check collisions
    CheckCollisions();

    // Draw debug info
    if (bEnableDebugVisualization)
    {
        DrawDebugInfo();
    }

    UpdateCount++;
    LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

TStatId UOrbitalMechanicsManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOrbitalMechanicsManager, STATGROUP_Tickables);
}

void UOrbitalMechanicsManager::InitializeUniverse(AOrbitalBody* RootBody)
{
    if (!RootBody)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize universe with null root body"));
        return;
    }

    this->RootBody = RootBody;
    RegisterOrbitalBody(RootBody);

    // Initialize SOI manager with root
    if (SOIManager)
    {
        SOIManager->Initialize(RootBody);
    }

    UE_LOG(LogTemp, Log, TEXT("Universe initialized with root: %s"), *RootBody->GetName());
}

void UOrbitalMechanicsManager::RegisterOrbitalBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Check if already registered
    FGuid BodyId = Body->GetUniqueID();
    if (BodyRegistry.Contains(BodyId))
    {
        return;
    }

    // Add to arrays
    RegisteredBodies.Add(Body);
    BodyRegistry.Add(BodyId, Body);

    // Add to spatial partitioner
    if (SpatialPartitioner)
    {
        SpatialPartitioner->AddBody(Body);
    }

    // Register with SOI manager
    if (SOIManager)
    {
        SOIManager->RegisterBody(Body);
    }

    UE_LOG(LogTemp, Verbose, TEXT("Registered orbital body: %s"), *Body->GetName());
}

void UOrbitalMechanicsManager::UnregisterOrbitalBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    FGuid BodyId = Body->GetUniqueID();
    
    // Remove from registry
    BodyRegistry.Remove(BodyId);
    RegisteredBodies.Remove(Body);

    // Remove from spatial partitioner
    if (SpatialPartitioner)
    {
        SpatialPartitioner->RemoveBody(Body);
    }

    // Unregister from SOI manager
    if (SOIManager)
    {
        SOIManager->UnregisterBody(Body);
    }

    UE_LOG(LogTemp, Verbose, TEXT("Unregistered orbital body: %s"), *Body->GetName());
}

TArray<AOrbitalBody*> UOrbitalMechanicsManager::GetAllOrbitalBodies() const
{
    TArray<AOrbitalBody*> Bodies;
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            Bodies.Add(Body);
        }
    }
    return Bodies;
}

TArray<AOrbitalBody*> UOrbitalMechanicsManager::FindBodiesInRadius(const FVector& Position, float Radius) const
{
    if (!SpatialPartitioner)
    {
        return TArray<AOrbitalBody*>();
    }

    FSpatialQueryResult QueryResult = SpatialPartitioner->QueryBodiesInRadius(Position, Radius);
    TArray<AOrbitalBody*> Bodies;
    
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : QueryResult.FoundBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            Bodies.Add(Body);
        }
    }
    
    return Bodies;
}

FVector UOrbitalMechanicsManager::CalculateOrbitalPosition(AOrbitalBody* Body, float DeltaTime) const
{
    if (!Body || !Body->OrbitTarget.IsValid() || !Body->OrbitalMechanics)
    {
        return Body ? Body->GetActorLocation() : FVector::ZeroVector;
    }

    // Use the orbital mechanics component to calculate new position
    const FOrbitalElements& Elements = Body->OrbitalMechanics->GetCurrentOrbitalElements();
    
    // Update time since periapsis
    Body->TimeSincePeriapsis += DeltaTime;
    
    // Calculate mean anomaly
    double CurrentOrbitalPeriod = UOrbitalMechanics::CalculateOrbitalPeriod(Elements.SemiMajorAxis, Elements.StandardGravitationalParameter);
    double MeanAnomaly = (2.0 * PI * Body->TimeSincePeriapsis) / CurrentOrbitalPeriod;
    MeanAnomaly = FMath::Fmod(MeanAnomaly, 2.0 * PI);
    
    // Solve Kepler's equation for eccentric anomaly
    double EccentricAnomaly = MeanAnomaly;
    double Eccentricity = Elements.Eccentricity;
    
    for (int32 i = 0; i < 10; ++i)
    {
        double DeltaE = (MeanAnomaly - (EccentricAnomaly - Eccentricity * FMath::Sin(EccentricAnomaly))) /
                       (1.0 - Eccentricity * FMath::Cos(EccentricAnomaly));
        
        EccentricAnomaly += DeltaE;
        
        if (FMath::Abs(DeltaE) < 1e-6)
        {
            break;
        }
    }
    
    // Calculate true anomaly
    double CosE = FMath::Cos(EccentricAnomaly);
    double SinE = FMath::Sin(EccentricAnomaly);
    double TrueAnomalyRad = FMath::Atan2(
        FMath::Sqrt(1.0 - Eccentricity * Eccentricity) * SinE,
        CosE - Eccentricity
    );
    
    // Calculate distance from focus
    double Distance = Elements.SemiMajorAxis * (1.0 - Eccentricity * CosE);
    
    // Calculate position in orbital plane
    double X = Distance * FMath::Cos(TrueAnomalyRad);
    double Y = Distance * FMath::Sin(TrueAnomalyRad);
    
    // Convert to 3D space using orbital elements
    double InclinationRad = FMath::DegreesToRadians(Elements.Inclination);
    double LongitudeAscendingNodeRad = FMath::DegreesToRadians(Elements.LongitudeAscendingNode);
    double ArgumentOfPeriapsisRad = FMath::DegreesToRadians(Elements.ArgumentOfPeriapsis);
    
    // Apply rotations (3-1-3 Euler angles)
    double CosLAN = FMath::Cos(LongitudeAscendingNodeRad);
    double SinLAN = FMath::Sin(LongitudeAscendingNodeRad);
    double CosI = FMath::Cos(InclinationRad);
    double SinI = FMath::Sin(InclinationRad);
    double CosAOP = FMath::Cos(ArgumentOfPeriapsisRad);
    double SinAOP = FMath::Sin(ArgumentOfPeriapsisRad);
    
    // Rotation matrix (3-1-3)
    double R11 = CosLAN * CosAOP - SinLAN * SinAOP * CosI;
    double R12 = -CosLAN * SinAOP - SinLAN * CosAOP * CosI;
    double R13 = SinLAN * SinI;
    
    double R21 = SinLAN * CosAOP + CosLAN * SinAOP * CosI;
    double R22 = -SinLAN * SinAOP + CosLAN * CosAOP * CosI;
    double R23 = -CosLAN * SinI;
    
    double R31 = SinAOP * SinI;
    double R32 = CosAOP * SinI;
    double R33 = CosI;
    
    // Apply rotation to position
    FVector Position3D(
        R11 * X + R12 * Y,
        R21 * X + R22 * Y,
        R31 * X + R32 * Y
    );
    
    // Convert from km to cm and add primary position
    FVector PrimaryPosition = Body->OrbitTarget->GetActorLocation();
    return PrimaryPosition + (Position3D * 100000.0f); // km to cm
}

FVector UOrbitalMechanicsManager::CalculateGravitationalForce(AOrbitalBody* Body1, AOrbitalBody* Body2) const
{
    if (!Body1 || !Body2)
    {
        return FVector::ZeroVector;
    }

    // Get positions
    FVector Position1 = Body1->GetActorLocation();
    FVector Position2 = Body2->GetActorLocation();
    
    // Calculate direction and distance
    FVector Direction = Position2 - Position1;
    float Distance = Direction.Size();
    
    if (Distance < KINDA_SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }
    
    // Normalize direction
    Direction.Normalize();
    
    // Calculate gravitational force magnitude (F = G × m1 × m2 / r²)
    const float GravitationalConstant = 6.67430e-11f; // m³/(kg·s²)
    float DistanceM = Distance * 0.01f; // cm to m
    float ForceMagnitude = GravitationalConstant * (Body1->Mass * Body2->Mass) / (DistanceM * DistanceM);
    
    // Convert to kN for Unreal physics
    return Direction * (ForceMagnitude * 0.001f);
}

bool UOrbitalMechanicsManager::WillCollide(AOrbitalBody* Body1, AOrbitalBody* Body2, float TimeStep) const
{
    if (!Body1 || !Body2)
    {
        return false;
    }

    // Get current positions and velocities
    FVector Pos1 = Body1->GetActorLocation();
    FVector Pos2 = Body2->GetActorLocation();
    FVector Vel1 = Body1->Velocity * 100000.0f; // km/s to cm/s
    FVector Vel2 = Body2->Velocity * 100000.0f; // km/s to cm/s
    
    // Predict positions after timestep
    FVector FuturePos1 = Pos1 + Vel1 * TimeStep;
    FVector FuturePos2 = Pos2 + Vel2 * TimeStep;
    
    // Get body radii
    float Radius1 = Body1->Radius * 100000.0f; // km to cm
    float Radius2 = Body2->Radius * 100000.0f; // km to cm
    
    // Check if distance will be less than sum of radii
    float CurrentDistance = FVector::Dist(Pos1, Pos2);
    float FutureDistance = FVector::Dist(FuturePos1, FuturePos2);
    float MinDistance = FMath::Min(CurrentDistance, FutureDistance);
    float CollisionDistance = Radius1 + Radius2;
    
    return MinDistance <= CollisionDistance;
}

bool UOrbitalMechanicsManager::CalculateHohmannTransfer(AOrbitalBody* FromBody, AOrbitalBody* ToBody, float& OutDeltaV1, float& OutDeltaV2, float& OutTransferTime)
{
    if (!FromBody || !ToBody || !FromBody->OrbitTarget.IsValid() || !HohmannCalculator)
    {
        return false;
    }

    // Get orbital radii
    FVector FromPos = FromBody->GetActorLocation();
    FVector ToPos = ToBody->GetActorLocation();
    FVector PrimaryPos = FromBody->OrbitTarget->GetActorLocation();
    
    float R1 = FVector::Dist(FromPos, PrimaryPos) * 0.00001f; // cm to km
    float R2 = FVector::Dist(ToPos, PrimaryPos) * 0.00001f; // cm to km
    
    float Mu = FromBody->OrbitTarget->GetStandardGravitationalParameter();
    
    return HohmannCalculator->CalculateTransfer(R1, R2, Mu, OutDeltaV1, OutDeltaV2, OutTransferTime);
}

float UOrbitalMechanicsManager::CalculateTransferWindow(AOrbitalBody* FromBody, AOrbitalBody* ToBody) const
{
    if (!FromBody || !ToBody || !FromBody->OrbitTarget.IsValid() || !ToBody->OrbitTarget.IsValid())
    {
        return -1.0f;
    }

    // Check if both bodies orbit the same primary
    if (FromBody->OrbitTarget.Get() != ToBody->OrbitTarget.Get())
    {
        return -1.0f;
    }

    // Get orbital parameters
    FVector FromPos = FromBody->GetActorLocation();
    FVector ToPos = ToBody->GetActorLocation();
    FVector PrimaryPos = FromBody->OrbitTarget->GetActorLocation();
    
    float R1 = FVector::Dist(FromPos, PrimaryPos) * 0.00001f; // cm to km
    float R2 = FVector::Dist(ToPos, PrimaryPos) * 0.00001f; // cm to km
    
    float Mu = FromBody->OrbitTarget->GetStandardGravitationalParameter();
    
    // Calculate current phase angle
    FVector FromDir = (FromPos - PrimaryPos).GetSafeNormal();
    FVector ToDir = (ToPos - PrimaryPos).GetSafeNormal();
    float PhaseAngle = FMath::Acos(FVector::DotProduct(FromDir, ToDir));
    PhaseAngle = FMath::RadiansToDegrees(PhaseAngle);
    
    if (HohmannCalculator)
    {
        return HohmannCalculator->CalculateTransferWindow(R1, R2, Mu, PhaseAngle);
    }
    
    return -1.0f;
}

bool UOrbitalMechanicsManager::IsInSphereOfInfluence(const FVector& Position, AOrbitalBody* Body) const
{
    if (!Body || !SOIManager)
    {
        return false;
    }

    return SOIManager->IsInSphereOfInfluence(Position, Body);
}

AOrbitalBody* UOrbitalMechanicsManager::GetDominantBody(const FVector& Position) const
{
    if (!SOIManager)
    {
        return nullptr;
    }

    return SOIManager->GetDominantBody(Position);
}

void UOrbitalMechanicsManager::HandleSOITransition(AOrbitalBody* Body)
{
    if (!Body || !SOIManager)
    {
        return;
    }

    SOIManager->HandleSOITransition(Body);
}

AOrbitalBody* UOrbitalMechanicsManager::GenerateStarSystem(const FStarSystemConfig& Config)
{
    if (!SystemGenerator)
    {
        return nullptr;
    }

    FGeneratedStarSystem GeneratedSystem = SystemGenerator->GenerateStarSystem(Config);
    AOrbitalBody* Star = GeneratedSystem.Star.Get();
    
    if (Star)
    {
        RegisterOrbitalBody(Star);
        
        // Register all planets and moons
        for (APlanet* Planet : GeneratedSystem.Planets)
        {
            if (Planet)
            {
                RegisterOrbitalBody(Planet);
            }
        }
        
        for (AOrbitalBody* Moon : GeneratedSystem.Moons)
        {
            if (Moon)
            {
                RegisterOrbitalBody(Moon);
            }
        }
    }

    return Star;
}

TArray<AOrbitalBody*> UOrbitalMechanicsManager::GeneratePlanetarySystem(AOrbitalBody* Star, const FPlanetarySystemConfig& Config)
{
    TArray<AOrbitalBody*> Bodies;
    
    if (!Star || !SystemGenerator)
    {
        return Bodies;
    }

    TArray<APlanet*> Planets = SystemGenerator->GeneratePlanetarySystem(Cast<ASun>(Star), Config);
    
    for (APlanet* Planet : Planets)
    {
        if (Planet)
        {
            RegisterOrbitalBody(Planet);
            Bodies.Add(Planet);
        }
    }

    return Bodies;
}

void UOrbitalMechanicsManager::AddBodyToSpatialPartition(AOrbitalBody* Body)
{
    if (!Body || !SpatialPartitioner)
    {
        return;
    }

    SpatialPartitioner->AddBody(Body);
}

void UOrbitalMechanicsManager::RemoveBodyFromSpatialPartition(AOrbitalBody* Body)
{
    if (!Body || !SpatialPartitioner)
    {
        return;
    }

    SpatialPartitioner->RemoveBody(Body);
}

TArray<AOrbitalBody*> UOrbitalMechanicsManager::QueryNearbyBodies(const FVector& Position, float Radius) const
{
    return FindBodiesInRadius(Position, Radius);
}

void UOrbitalMechanicsManager::SetDebugVisualization(bool bEnable)
{
    bEnableDebugVisualization = bEnable;
}

void UOrbitalMechanicsManager::DrawOrbitalPaths() const
{
    // Implementation would draw orbital paths for all bodies
    // This is a placeholder for the actual visualization
}

void UOrbitalMechanicsManager::DrawSpheresOfInfluence() const
{
    if (!SOIManager)
    {
        return;
    }

    // Implementation would draw SOI for all bodies
    // This is a placeholder for the actual visualization
}

void UOrbitalMechanicsManager::SaveOrbitalState()
{
    // Implementation would save orbital state to persistence system
    // This is a placeholder for the actual persistence logic
}

void UOrbitalMechanicsManager::LoadOrbitalState()
{
    // Implementation would load orbital state from persistence system
    // This is a placeholder for the actual persistence logic
}

void UOrbitalMechanicsManager::UpdateOrbitalBodies(float DeltaTime)
{
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        AOrbitalBody* Body = BodyPtr.Get();
        if (!Body || Body->OrbitMode != EOrbitMode::Orbit)
        {
            continue;
        }

        // Update orbital position
        FVector NewPosition = CalculateOrbitalPosition(Body, DeltaTime);
        Body->SetActorLocation(NewPosition, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void UOrbitalMechanicsManager::UpdateSphereOfInfluenceTransitions()
{
    // Handled by SOI manager in Tick()
}

void UOrbitalMechanicsManager::ApplyGravity(float DeltaTime)
{
    if (GravityCalculationMode == EGravityCalculationMode::PrimaryOnly)
    {
        // Only apply gravity from primary bodies
        for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
        {
            AOrbitalBody* Body = BodyPtr.Get();
            if (!Body || !Body->OrbitTarget.IsValid() || !Body->bEnableGravity)
            {
                continue;
            }

            // Simple gravitational acceleration
            FVector ToPrimary = Body->OrbitTarget->GetActorLocation() - Body->GetActorLocation();
            float Distance = ToPrimary.Size() * 0.01f; // cm to m
            ToPrimary.Normalize();

            const float GravitationalConstant = 6.67430e-11f;
            float Acceleration = GravitationalConstant * Body->OrbitTarget->Mass / (Distance * Distance);
            
            FVector Force = ToPrimary * (Acceleration * Body->Mass * 0.001f); // Convert to kN

            if (UPrimitiveComponent* PhysicsComponent = Cast<UPrimitiveComponent>(Body->GetRootComponent()))
            {
                PhysicsComponent->AddForce(Force, NAME_None, true);
            }
        }
    }
    else if (GravityCalculationMode == EGravityCalculationMode::PatchedConics)
    {
        // Use patched conics approximation
        for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
        {
            AOrbitalBody* Body = BodyPtr.Get();
            if (!Body || !Body->bEnableGravity)
            {
                continue;
            }

            // Get dominant body and apply gravity from it
            AOrbitalBody* DominantBody = GetDominantBody(Body->GetActorLocation());
            if (DominantBody && DominantBody != Body)
            {
                FVector Force = CalculateGravitationalForce(Body, DominantBody);
                
                if (UPrimitiveComponent* PhysicsComponent = Cast<UPrimitiveComponent>(Body->GetRootComponent()))
                {
                    PhysicsComponent->AddForce(Force, NAME_None, true);
                }
            }
        }
    }
}

void UOrbitalMechanicsManager::CheckCollisions()
{
    // Simple collision detection between nearby bodies
    for (int32 i = 0; i < RegisteredBodies.Num(); i++)
    {
        AOrbitalBody* Body1 = RegisteredBodies[i].Get();
        if (!Body1 || !Body1->bEnableCollision)
        {
            continue;
        }

        for (int32 j = i + 1; j < RegisteredBodies.Num(); j++)
        {
            AOrbitalBody* Body2 = RegisteredBodies[j].Get();
            if (!Body2 || !Body2->bEnableCollision)
            {
                continue;
            }

            // Check if bodies are close enough to collide
            float Distance = FVector::Dist(Body1->GetActorLocation(), Body2->GetActorLocation());
            float MinDistance = (Body1->Radius + Body2->Radius) * 100000.0f; // km to cm

            if (Distance <= MinDistance)
            {
                // Handle collision
                UE_LOG(LogTemp, Warning, TEXT("Collision detected between %s and %s"), 
                       *Body1->GetName(), *Body2->GetName());
                
                // Could trigger collision events, damage, etc.
            }
        }
    }
}

void UOrbitalMechanicsManager::DrawDebugInfo() const
{
    if (!GetWorld())
    {
        return;
    }

    // Draw debug information for all registered bodies
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        if (const AOrbitalBody* Body = BodyPtr.Get())
        {
            FVector Location = Body->GetActorLocation();
            
            // Draw body as sphere
            float DrawRadius = FMath::Max(Body->Radius * 1000.0f, 100.0f); // At least 1m radius
            DrawDebugSphere(GetWorld(), Location, DrawRadius, 16, FColor::Green, false, -1.0f, 0, 1.0f);
            
            // Draw velocity vector
            if (!Body->Velocity.IsNearlyZero())
            {
                FVector VelocityVector = Body->Velocity * 100000.0f; // km/s to cm/s
                DrawDebugDirectionalArrow(GetWorld(), Location, Location + VelocityVector, 100.0f, FColor::Red, false, -1.0f, 0, 2.0f);
            }
            
            // Draw orbit target line
            if (Body->OrbitTarget.IsValid())
            {
                DrawDebugLine(GetWorld(), Location, Body->OrbitTarget->GetActorLocation(), FColor::Blue, false, -1.0f, 0, 1.0f);
            }
        }
    }
}