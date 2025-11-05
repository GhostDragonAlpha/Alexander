// Copyright Epic Games, Inc. All Rights Reserved.

#include "GravitySimulator.h"
#include "CelestialBodyComponent.h"
#include "CelestialBodyRegistry.h"
#include "AstronomicalConstants.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

void UGravitySimulator::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default values
	CurrentSimulationMode = EGravitySimulationMode::MultiBody;
	bGravityEnabled = true;
	MaxGForce = 50.0f; // 50 G-forces maximum
	MinGravityDistance = 100.0f; // 1 meter minimum
	GravitationalConstant = CelestialScalingConstants::SolSystem::G;
	PhysicsScaleFactor = 1.0f;
	bAutoDiscoverBodies = true;
	MaxInfluenceDistance = 1000000.0f; // 10 km
	GravityUpdateFrequency = 60.0f; // 60 Hz
	bEnableDebugVisualization = false;
	bEnableDebugLogging = false;
	DebugForceColor = FColor::Yellow;

	// Initialize statistics
	CalculationsThisFrame = 0;
	TotalCalculationTime = 0.0f;
	FrameCounter = 0;

	UE_LOG(LogTemp, Log, TEXT("GravitySimulator: Initialized with mode %d, G = %.6e"),
		static_cast<int32>(CurrentSimulationMode), GravitationalConstant);
}

void UGravitySimulator::Deinitialize()
{
	// Log final statistics
	if (bEnableDebugLogging)
	{
		int32 AvgCalcs = 0;
		float AvgTime = 0.0f;
		GetSimulationStatistics(AvgCalcs, AvgTime);
		UE_LOG(LogTemp, Log, TEXT("GravitySimulator: Final statistics - Avg calculations: %d, Avg time: %.3f ms"),
			AvgCalcs, AvgTime);
	}

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("GravitySimulator: Deinitialized"));
}

// ========== Gravitational Force Calculation ==========

FVector UGravitySimulator::CalculateTotalGravitationalForce(AActor* Target, const FVector& TargetPosition) const
{
	if (!bGravityEnabled || !Target)
	{
		return FVector::ZeroVector;
	}

	FScopeLock Lock(&SimulationLock);

	// Get target mass (default to 1000 kg if not specified)
	float TargetMass = 1000.0f;
	if (UPrimitiveComponent* PrimComp = Target->FindComponentByClass<UPrimitiveComponent>())
	{
		if (PrimComp->IsSimulatingPhysics())
		{
			TargetMass = PrimComp->GetMass();
		}
	}

	// Calculate based on simulation mode
	FVector TotalForce = FVector::ZeroVector;

	switch (CurrentSimulationMode)
	{
	case EGravitySimulationMode::SingleBody:
		TotalForce = CalculateSingleBodyGravity(TargetPosition, TargetMass);
		break;

	case EGravitySimulationMode::MultiBody:
		TotalForce = CalculateMultiBodyGravity(TargetPosition, TargetMass);
		break;

	case EGravitySimulationMode::NBody:
		TotalForce = CalculateNBodyGravity(TargetPosition, TargetMass);
		break;

	case EGravitySimulationMode::Disabled:
		return FVector::ZeroVector;
	}

	// Validate and clamp the force
	TotalForce = ValidateForce(TotalForce, TargetMass);

	CalculationsThisFrame++;

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("GravitySimulator: Total force on %s = %s N"),
			*Target->GetName(), *TotalForce.ToString());
	}

	return TotalForce;
}

FVector UGravitySimulator::CalculateGravityFromBody(UCelestialBodyComponent* Body, const FVector& TargetPosition, float TargetMass) const
{
	if (!Body || !IsValid(Body) || TargetMass <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	// Get body position and mass
	FVector BodyPosition = Body->GetOwner()->GetActorLocation();
	float BodyMass = Body->GetMass();

	if (BodyMass <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	// Calculate distance vector
	FVector DeltaPosition = BodyPosition - TargetPosition;
	float Distance = DeltaPosition.Size();

	// Prevent singularities
	if (Distance < MinGravityDistance)
	{
		Distance = MinGravityDistance;
	}

	// Calculate gravitational force: F = G * (m1 * m2) / r²
	double DistanceSquared = static_cast<double>(Distance) * static_cast<double>(Distance);
	double ForceMagnitude = (GravitationalConstant * BodyMass * TargetMass) / DistanceSquared;

	// Direction towards the body
	FVector Direction = DeltaPosition.GetSafeNormal();

	// Apply physics scale factor
	FVector Force = Direction * static_cast<float>(ForceMagnitude) * PhysicsScaleFactor;

	return Force;
}

FVector UGravitySimulator::CalculateGravitationalAcceleration(UCelestialBodyComponent* Body, const FVector& TargetPosition) const
{
	if (!Body || !IsValid(Body))
	{
		return FVector::ZeroVector;
	}

	// Get body position and mass
	FVector BodyPosition = Body->GetOwner()->GetActorLocation();
	float BodyMass = Body->GetMass();

	if (BodyMass <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	// Calculate distance vector
	FVector DeltaPosition = BodyPosition - TargetPosition;
	float Distance = DeltaPosition.Size();

	// Prevent singularities
	if (Distance < MinGravityDistance)
	{
		Distance = MinGravityDistance;
	}

	// Calculate gravitational acceleration: a = G * M / r²
	double DistanceSquared = static_cast<double>(Distance) * static_cast<double>(Distance);
	double AccelerationMagnitude = (GravitationalConstant * BodyMass) / DistanceSquared;

	// Direction towards the body
	FVector Direction = DeltaPosition.GetSafeNormal();

	return Direction * static_cast<float>(AccelerationMagnitude);
}

UCelestialBodyComponent* UGravitySimulator::GetDominantGravitationalBody(const FVector& Position) const
{
	TArray<UCelestialBodyComponent*> Bodies = GetCelestialBodies();

	if (Bodies.Num() == 0)
	{
		return nullptr;
	}

	UCelestialBodyComponent* DominantBody = nullptr;
	float MaxInfluence = 0.0f;

	for (UCelestialBodyComponent* Body : Bodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		float Influence = CalculateInfluenceStrength(Body, Position);

		if (Influence > MaxInfluence)
		{
			MaxInfluence = Influence;
			DominantBody = Body;
		}
	}

	return DominantBody;
}

// ========== Physics Integration ==========

void UGravitySimulator::ApplyGravityToActor(AActor* Target, float DeltaTime)
{
	if (!bGravityEnabled || !Target || !IsValid(Target))
	{
		return;
	}

	// Find the primitive component to apply force to
	UPrimitiveComponent* PrimComp = Target->FindComponentByClass<UPrimitiveComponent>();

	if (PrimComp && PrimComp->IsSimulatingPhysics())
	{
		ApplyGravityToComponent(PrimComp, DeltaTime);
	}
}

void UGravitySimulator::ApplyGravityToComponent(UPrimitiveComponent* Component, float DeltaTime)
{
	if (!bGravityEnabled || !Component || !IsValid(Component))
	{
		return;
	}

	if (!Component->IsSimulatingPhysics())
	{
		return;
	}

	// Calculate gravitational force
	FVector Position = Component->GetComponentLocation();
	FVector Force = CalculateTotalGravitationalForce(Component->GetOwner(), Position);

	// Convert to Unreal force units
	float Mass = Component->GetMass();
	FVector UnrealForce = ConvertNewtonsToUnrealForce(Force, Mass);

	// Clamp to max G-force
	UnrealForce = ClampGravitationalForce(UnrealForce, MaxGForce);

	// Apply force in world space
	Component->AddForce(UnrealForce, NAME_None, false);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("GravitySimulator: Applied force %s to %s"),
			*UnrealForce.ToString(), *Component->GetName());
	}
}

FVector UGravitySimulator::ConvertNewtonsToUnrealForce(const FVector& ForceInNewtons, float TargetMass) const
{
	// Unreal Engine uses kg·cm/s² for force
	// 1 Newton = 100 Unreal force units (kg·cm/s²)
	return ForceInNewtons * 100.0f;
}

FVector UGravitySimulator::ClampGravitationalForce(const FVector& Force, float MaxGForceLimit) const
{
	// Calculate G-force magnitude
	float ForceMagnitude = Force.Size();

	// 1 G = 9.80665 m/s² = 980.665 cm/s²
	float OneG = 980.665f;
	float MaxForce = MaxGForceLimit * OneG;

	if (ForceMagnitude > MaxForce)
	{
		// Clamp to maximum
		return Force.GetSafeNormal() * MaxForce;
	}

	return Force;
}

// ========== Sphere of Influence ==========

bool UGravitySimulator::IsWithinInfluenceRadius(UCelestialBodyComponent* Body, const FVector& Position, float InfluenceRadius) const
{
	if (!Body || !IsValid(Body))
	{
		return false;
	}

	// Calculate radius if not provided
	if (InfluenceRadius <= 0.0f)
	{
		InfluenceRadius = CalculateSphereOfInfluence(Body);
	}

	FVector BodyPosition = Body->GetOwner()->GetActorLocation();
	float Distance = FVector::Dist(Position, BodyPosition);

	return Distance <= InfluenceRadius;
}

float UGravitySimulator::CalculateSphereOfInfluence(UCelestialBodyComponent* Body) const
{
	if (!Body || !IsValid(Body))
	{
		return 0.0f;
	}

	// Simplified SOI calculation
	// In reality, this uses the Laplace sphere: r_SOI = a * (m_satellite / m_primary)^(2/5)
	// For now, use a simple multiple of the body radius
	float BodyRadius = Body->GetRadius();
	float BodyMass = Body->GetMass();

	// SOI scales with mass^(1/3) approximately
	float SOIMultiplier = FMath::Pow(BodyMass / 1.0e24f, 0.333f);

	return BodyRadius * 100.0f * FMath::Max(SOIMultiplier, 2.0f); // At least 2x radius
}

TArray<UCelestialBodyComponent*> UGravitySimulator::GetInfluencingBodies(const FVector& Position, int32 MaxBodies) const
{
	TArray<UCelestialBodyComponent*> AllBodies = GetCelestialBodies();
	TArray<TPair<float, UCelestialBodyComponent*>> BodiesWithInfluence;

	for (UCelestialBodyComponent* Body : AllBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		float Influence = CalculateInfluenceStrength(Body, Position);

		if (Influence > 0.0f)
		{
			BodiesWithInfluence.Add(TPair<float, UCelestialBodyComponent*>(Influence, Body));
		}
	}

	// Sort by influence strength (descending)
	BodiesWithInfluence.Sort([](const TPair<float, UCelestialBodyComponent*>& A, const TPair<float, UCelestialBodyComponent*>& B)
	{
		return A.Key > B.Key;
	});

	// Extract top N bodies
	TArray<UCelestialBodyComponent*> InfluencingBodies;
	int32 NumToReturn = FMath::Min(MaxBodies, BodiesWithInfluence.Num());

	for (int32 i = 0; i < NumToReturn; ++i)
	{
		InfluencingBodies.Add(BodiesWithInfluence[i].Value);
	}

	return InfluencingBodies;
}

// ========== Configuration ==========

void UGravitySimulator::SetSimulationMode(EGravitySimulationMode Mode)
{
	if (CurrentSimulationMode != Mode)
	{
		CurrentSimulationMode = Mode;

		UE_LOG(LogTemp, Log, TEXT("GravitySimulator: Simulation mode changed to %d"), static_cast<int32>(Mode));
	}
}

// ========== Network Prediction ==========

bool UGravitySimulator::ValidateClientPrediction(int32 ActorID, const FVector& ClientPosition, const FVector& ClientVelocity,
	FVector& OutCorrectedPosition, FVector& OutCorrectedVelocity)
{
	// Server-side validation of client's predicted physics
	// For now, simple implementation that accepts client prediction
	// In production, would compare with server simulation and correct if divergent

	OutCorrectedPosition = ClientPosition;
	OutCorrectedVelocity = ClientVelocity;

	// Threshold for position correction (10 cm)
	float CorrectionThreshold = 10.0f;

	// TODO: Implement actual server-side simulation comparison
	// For now, no correction needed
	return false;
}

// ========== Debug ==========

void UGravitySimulator::DrawGravityDebug(AActor* Target, float Duration) const
{
	if (!bEnableDebugVisualization || !Target || !GetWorld())
	{
		return;
	}

	FVector TargetPosition = Target->GetActorLocation();
	TArray<UCelestialBodyComponent*> Bodies = GetCelestialBodies();

	float Mass = 1000.0f;
	if (UPrimitiveComponent* PrimComp = Target->FindComponentByClass<UPrimitiveComponent>())
	{
		Mass = PrimComp->GetMass();
	}

	// Draw force vectors from each body
	for (UCelestialBodyComponent* Body : Bodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		FVector Force = CalculateGravityFromBody(Body, TargetPosition, Mass);
		FVector BodyPosition = Body->GetOwner()->GetActorLocation();

		// Scale force for visualization
		FVector ForceVectorEnd = TargetPosition + Force.GetSafeNormal() * FMath::Min(Force.Size() * 0.1f, 1000.0f);

		// Draw line from body to target
		DrawDebugLine(GetWorld(), BodyPosition, TargetPosition, FColor::Cyan, false, Duration, 0, 2.0f);

		// Draw force vector
		DrawDebugDirectionalArrow(GetWorld(), TargetPosition, ForceVectorEnd, 50.0f, DebugForceColor, false, Duration, 0, 3.0f);

		// Draw body name
		DrawDebugString(GetWorld(), BodyPosition, Body->GetBodyName().ToString(), nullptr, FColor::White, Duration);
	}
}

void UGravitySimulator::GetSimulationStatistics(int32& OutCalculationsPerFrame, float& OutAverageCalculationTime) const
{
	FScopeLock Lock(&SimulationLock);

	if (FrameCounter > 0)
	{
		OutCalculationsPerFrame = CalculationsThisFrame / FrameCounter;
		OutAverageCalculationTime = TotalCalculationTime / static_cast<float>(FrameCounter);
	}
	else
	{
		OutCalculationsPerFrame = 0;
		OutAverageCalculationTime = 0.0f;
	}
}

// ========== Internal Methods ==========

FVector UGravitySimulator::CalculateSingleBodyGravity(const FVector& TargetPosition, float TargetMass) const
{
	// Find the dominant body
	UCelestialBodyComponent* DominantBody = GetDominantGravitationalBody(TargetPosition);

	if (!DominantBody)
	{
		return FVector::ZeroVector;
	}

	// Calculate force from dominant body only
	return CalculateGravityFromBody(DominantBody, TargetPosition, TargetMass);
}

FVector UGravitySimulator::CalculateMultiBodyGravity(const FVector& TargetPosition, float TargetMass) const
{
	// Get the top 3 most influential bodies
	TArray<UCelestialBodyComponent*> InfluencingBodies = GetInfluencingBodies(TargetPosition, 3);

	FVector TotalForce = FVector::ZeroVector;

	for (UCelestialBodyComponent* Body : InfluencingBodies)
	{
		if (IsValid(Body))
		{
			TotalForce += CalculateGravityFromBody(Body, TargetPosition, TargetMass);
		}
	}

	return TotalForce;
}

FVector UGravitySimulator::CalculateNBodyGravity(const FVector& TargetPosition, float TargetMass) const
{
	// Calculate force from all bodies
	TArray<UCelestialBodyComponent*> AllBodies = GetCelestialBodies();

	FVector TotalForce = FVector::ZeroVector;

	for (UCelestialBodyComponent* Body : AllBodies)
	{
		if (!IsValid(Body))
		{
			continue;
		}

		// Skip bodies beyond max influence distance
		FVector BodyPosition = Body->GetOwner()->GetActorLocation();
		float Distance = FVector::Dist(TargetPosition, BodyPosition);

		if (Distance > MaxInfluenceDistance)
		{
			continue;
		}

		TotalForce += CalculateGravityFromBody(Body, TargetPosition, TargetMass);
	}

	return TotalForce;
}

TArray<UCelestialBodyComponent*> UGravitySimulator::GetCelestialBodies() const
{
	if (!bAutoDiscoverBodies)
	{
		return TArray<UCelestialBodyComponent*>();
	}

	// Get bodies from the registry
	if (UWorld* World = GetWorld())
	{
		if (UCelestialBodyRegistry* Registry = World->GetSubsystem<UCelestialBodyRegistry>())
		{
			return Registry->GetAllCelestialBodies();
		}
	}

	return TArray<UCelestialBodyComponent*>();
}

float UGravitySimulator::CalculateInfluenceStrength(UCelestialBodyComponent* Body, const FVector& Position) const
{
	if (!Body || !IsValid(Body))
	{
		return 0.0f;
	}

	FVector BodyPosition = Body->GetOwner()->GetActorLocation();
	float Distance = FVector::Dist(Position, BodyPosition);

	// Prevent division by zero
	if (Distance < MinGravityDistance)
	{
		Distance = MinGravityDistance;
	}

	// Influence strength = Mass / Distance²
	float BodyMass = Body->GetMass();
	float DistanceSquared = Distance * Distance;

	return BodyMass / DistanceSquared;
}

FVector UGravitySimulator::ValidateForce(const FVector& Force, float TargetMass) const
{
	// Check for invalid values
	if (!Force.ContainsNaN() && Force.IsZero())
	{
		return FVector::ZeroVector;
	}

	if (Force.ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("GravitySimulator: Force contains NaN!"));
		return FVector::ZeroVector;
	}

	// Clamp to max G-force
	return ClampGravitationalForce(Force, MaxGForce);
}
