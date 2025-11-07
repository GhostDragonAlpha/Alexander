// Copyright Epic Games, Inc. All Rights Reserved.

#include "TriangulationValidator.h"
#include "DrawDebugHelpers.h"

UTriangulationValidator::UTriangulationValidator()
{
	// Default configuration values set in header
}

// ============================================================================
// CORE VALIDATION METHODS
// ============================================================================

FGeometricValidationResult UTriangulationValidator::ValidatePosition(const TArray<FObserverMeasurement>& Measurements)
{
	FGeometricValidationResult Result;
	Result.ObserverCount = Measurements.Num();

	if (Measurements.Num() < 2)
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Need at least 2 observers for validation"));
		}
		Result.bIsValid = false;
		Result.ValidationMethod = TEXT("Insufficient observers");
		return Result;
	}

	// Auto-select validation method based on observer count
	if (Measurements.Num() == 2)
	{
		// 2 observers: Can only check distance consistency
		float observerDistance = GetDistanceBetweenObservers(Measurements[0], Measurements[1]);
		float distanceA = Measurements[0].Distance;
		float distanceB = Measurements[1].Distance;

		// Triangle inequality: distA + distB > observerDistance
		Result.bIsValid = CheckTriangleInequality(distanceA, distanceB, observerDistance);
		Result.ValidationMethod = TEXT("2-Observer Distance Check");
		Result.Confidence = 0.5f; // Low confidence with only 2 observers
		Result.GeometricError = FMath::Abs((distanceA + distanceB) - observerDistance);
	}
	else if (Measurements.Num() == 3)
	{
		// 3 observers: Triangle validation (2D planar)
		Result.bIsValid = ValidateTriangle(Measurements[0], Measurements[1], Measurements[2]);
		Result.ValidationMethod = TEXT("3-Observer Triangle");
		Result.Confidence = CalculateConfidence(3);
		Result.GeometricError = CalculateGeometricError(Measurements);

		if (Result.bIsValid)
		{
			Result.TriangulatedPosition = TriangulateFrom3Observers(Measurements);
		}
	}
	else if (Measurements.Num() == 4)
	{
		// 4 observers: Tetrahedron validation (full 3D)
		Result.bIsValid = ValidateTetrahedron(Measurements);
		Result.ValidationMethod = TEXT("4-Observer Tetrahedron");
		Result.Confidence = CalculateConfidence(4);
		Result.GeometricError = CalculateGeometricError(Measurements);

		if (Result.bIsValid)
		{
			Result.TriangulatedPosition = TriangulateFrom4Observers(Measurements);
		}
	}
	else
	{
		// 5+ observers: Overdetermined system (best-fit)
		Result = ValidateOverdetermined(Measurements);
		Result.ValidationMethod = TEXT("Overdetermined Least-Squares");
	}

	if (bEnableDebugVisualization && Measurements.Num() > 0)
	{
		DrawDebugGeometry(Measurements, Result.TriangulatedPosition, Result.bIsValid);
	}

	return Result;
}

bool UTriangulationValidator::ValidateTriangle(const FObserverMeasurement& ObsA, const FObserverMeasurement& ObsB, const FObserverMeasurement& ObsC)
{
	// Get distances between observers
	float DistAB = GetDistanceBetweenObservers(ObsA, ObsB);
	float DistBC = GetDistanceBetweenObservers(ObsB, ObsC);
	float DistCA = GetDistanceBetweenObservers(ObsC, ObsA);

	// Get distances to target
	float DistAT = ObsA.Distance;
	float DistBT = ObsB.Distance;
	float DistCT = ObsC.Distance;

	// Calculate tolerance based on distances and scale factors
	float ToleranceA = CalculateTolerance(DistAT, ObsA.ScaleFactor, 3);
	float ToleranceB = CalculateTolerance(DistBT, ObsB.ScaleFactor, 3);
	float ToleranceC = CalculateTolerance(DistCT, ObsC.ScaleFactor, 3);
	float MaxTolerance = FMath::Max3(ToleranceA, ToleranceB, ToleranceC);

	// Validate triangle inequalities for all three triangles formed:
	// Triangle 1: Observers A-B and target T
	if (!CheckTriangleInequality(DistAT, DistBT, DistAB))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Triangle inequality failed for observers A-B"));
		}
		return false;
	}

	// Triangle 2: Observers B-C and target T
	if (!CheckTriangleInequality(DistBT, DistCT, DistBC))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Triangle inequality failed for observers B-C"));
		}
		return false;
	}

	// Triangle 3: Observers C-A and target T
	if (!CheckTriangleInequality(DistCT, DistAT, DistCA))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Triangle inequality failed for observers C-A"));
		}
		return false;
	}

	// Use Law of Cosines to verify geometric consistency
	// For triangle A-B-T: Calculate angle at A using Law of Cosines
	float AngleA = CalculateAngle(DistAT, DistAB, DistBT);

	// Calculate what the angle should be from direction vectors
	FVector DirAB = (ObsB.ObserverPosition - ObsA.ObserverPosition).GetSafeNormal();
	FVector DirAT = ObsA.Direction;
	float ExpectedAngleA = GetAngleBetweenDirections(DirAB, DirAT);

	// Check if calculated angle matches expected angle within tolerance
	float AngleError = FMath::Abs(AngleA - ExpectedAngleA);
	float AngleTolerance = FMath::DegreesToRadians(5.0f); // 5 degrees tolerance

	if (AngleError > AngleTolerance)
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Angle error %.2f deg exceeds tolerance"),
				FMath::RadiansToDegrees(AngleError));
		}
		return false;
	}

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("TriangulationValidator: Triangle validation PASSED (angle error: %.2f deg)"),
			FMath::RadiansToDegrees(AngleError));
	}

	return true;
}

bool UTriangulationValidator::ValidateTetrahedron(const TArray<FObserverMeasurement>& Measurements)
{
	if (Measurements.Num() != 4)
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron validation requires exactly 4 observers"));
		}
		return false;
	}

	// Validate all 4 faces of the tetrahedron
	// Face 1: Observers 0, 1, 2
	if (!ValidateTriangle(Measurements[0], Measurements[1], Measurements[2]))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron face 0-1-2 invalid"));
		}
		return false;
	}

	// Face 2: Observers 0, 1, 3
	if (!ValidateTriangle(Measurements[0], Measurements[1], Measurements[3]))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron face 0-1-3 invalid"));
		}
		return false;
	}

	// Face 3: Observers 0, 2, 3
	if (!ValidateTriangle(Measurements[0], Measurements[2], Measurements[3]))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron face 0-2-3 invalid"));
		}
		return false;
	}

	// Face 4: Observers 1, 2, 3
	if (!ValidateTriangle(Measurements[1], Measurements[2], Measurements[3]))
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron face 1-2-3 invalid"));
		}
		return false;
	}

	// Calculate tetrahedron volume to ensure non-degenerate (not all points coplanar)
	TArray<FVector> Vertices;
	Vertices.Add(Measurements[0].ObserverPosition);
	Vertices.Add(Measurements[1].ObserverPosition);
	Vertices.Add(Measurements[2].ObserverPosition);
	Vertices.Add(Measurements[3].ObserverPosition);

	float Volume = CalculateTetrahedronVolume(Vertices);

	if (Volume < MinimumTetrahedronVolume)
	{
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriangulationValidator: Tetrahedron volume %.2f below minimum %.2f (degenerate/coplanar)"),
				Volume, MinimumTetrahedronVolume);
		}
		return false;
	}

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("TriangulationValidator: Tetrahedron validation PASSED (volume: %.2f)"), Volume);
	}

	return true;
}

FGeometricValidationResult UTriangulationValidator::ValidateOverdetermined(const TArray<FObserverMeasurement>& Measurements)
{
	FGeometricValidationResult Result;
	Result.ObserverCount = Measurements.Num();
	Result.ValidationMethod = TEXT("Overdetermined Least-Squares");

	if (Measurements.Num() < 5)
	{
		Result.bIsValid = false;
		Result.Confidence = 0.0f;
		return Result;
	}

	// Use least-squares fitting to find best position
	Result = TriangulateOverdetermined(Measurements);

	// Calculate geometric error (RMS of residuals)
	float TotalSquaredError = 0.0f;
	int32 ValidMeasurements = 0;

	for (const FObserverMeasurement& Obs : Measurements)
	{
		FVector ToTarget = Result.TriangulatedPosition - Obs.ObserverPosition;
		float CalculatedDistance = ToTarget.Size();
		float Error = FMath::Abs(CalculatedDistance - Obs.Distance);

		TotalSquaredError += Error * Error;
		ValidMeasurements++;
	}

	Result.GeometricError = FMath::Sqrt(TotalSquaredError / ValidMeasurements);

	// Validate against maximum allowed error
	Result.bIsValid = (Result.GeometricError <= MaximumGeometricError);

	// Calculate confidence based on observer count (anti-fragile property)
	Result.Confidence = CalculateConfidence(Measurements.Num());

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("TriangulationValidator: Overdetermined validation - %d observers, RMS error: %.2f, Confidence: %.2f%%"),
			Measurements.Num(), Result.GeometricError, Result.Confidence * 100.0f);
	}

	return Result;
}

// ============================================================================
// GEOMETRIC CALCULATIONS
// ============================================================================

float UTriangulationValidator::CalculateThirdSide(float DistanceA, float DistanceB, float AngleRadians)
{
	// Law of Cosines: c² = a² + b² - 2ab·cos(θ)
	float CosTerm = 2.0f * DistanceA * DistanceB * FMath::Cos(AngleRadians);
	float CSqr = (DistanceA * DistanceA) + (DistanceB * DistanceB) - CosTerm;

	// Handle floating-point precision issues
	if (CSqr < 0.0f)
	{
		CSqr = 0.0f;
	}

	return FMath::Sqrt(CSqr);
}

float UTriangulationValidator::CalculateAngle(float SideA, float SideB, float SideC)
{
	// Law of Cosines (inverse): cos(θ) = (a² + b² - c²) / (2ab)
	float Numerator = (SideA * SideA) + (SideB * SideB) - (SideC * SideC);
	float Denominator = 2.0f * SideA * SideB;

	if (FMath::IsNearlyZero(Denominator))
	{
		return 0.0f;
	}

	float CosTheta = Numerator / Denominator;

	// Clamp to valid range [-1, 1] to handle floating-point precision
	CosTheta = FMath::Clamp(CosTheta, -1.0f, 1.0f);

	return FMath::Acos(CosTheta);
}

bool UTriangulationValidator::CheckTriangleInequality(float SideA, float SideB, float SideC)
{
	// Triangle inequality: Sum of any two sides must be greater than the third
	bool Check1 = (SideA + SideB) > SideC;
	bool Check2 = (SideB + SideC) > SideA;
	bool Check3 = (SideC + SideA) > SideB;

	return Check1 && Check2 && Check3;
}

float UTriangulationValidator::CalculateTetrahedronVolume(const TArray<FVector>& Vertices)
{
	if (Vertices.Num() != 4)
	{
		return 0.0f;
	}

	// Volume = |det(v1-v0, v2-v0, v3-v0)| / 6
	// Using scalar triple product: V = |(a · (b × c))| / 6
	FVector V0 = Vertices[0];
	FVector V1 = Vertices[1];
	FVector V2 = Vertices[2];
	FVector V3 = Vertices[3];

	FVector A = V1 - V0;
	FVector B = V2 - V0;
	FVector C = V3 - V0;

	// Scalar triple product
	float ScalarTriple = FVector::DotProduct(A, FVector::CrossProduct(B, C));

	// Volume is 1/6 of absolute value
	return FMath::Abs(ScalarTriple) / 6.0f;
}

// ============================================================================
// TRIANGULATION (POSITION CALCULATION)
// ============================================================================

FVector UTriangulationValidator::TriangulateFrom3Observers(const TArray<FObserverMeasurement>& Measurements)
{
	if (Measurements.Num() != 3)
	{
		return FVector::ZeroVector;
	}

	// Trilateration in 2D/3D
	// Place first observer at origin, second on X-axis
	FVector P1 = Measurements[0].ObserverPosition;
	FVector P2 = Measurements[1].ObserverPosition;
	FVector P3 = Measurements[2].ObserverPosition;

	float R1 = Measurements[0].Distance;
	float R2 = Measurements[1].Distance;
	float R3 = Measurements[2].Distance;

	// Transform to local coordinate system
	FVector Ex = (P2 - P1).GetSafeNormal();
	float D = (P2 - P1).Size();

	FVector Diff = P3 - P1;
	float I = FVector::DotProduct(Ex, Diff);

	FVector Ey = (Diff - (Ex * I)).GetSafeNormal();
	FVector Ez = FVector::CrossProduct(Ex, Ey);

	float J = FVector::DotProduct(Ey, P3 - P1);

	// Calculate target position in local coordinates
	float X = (R1*R1 - R2*R2 + D*D) / (2.0f * D);
	float Y = (R1*R1 - R3*R3 + I*I + J*J) / (2.0f * J) - (I * X / J);

	float ZSqr = R1*R1 - X*X - Y*Y;
	float Z = (ZSqr > 0.0f) ? FMath::Sqrt(ZSqr) : 0.0f;

	// Transform back to world coordinates
	FVector Result = P1 + (Ex * X) + (Ey * Y) + (Ez * Z);

	return Result;
}

FVector UTriangulationValidator::TriangulateFrom4Observers(const TArray<FObserverMeasurement>& Measurements)
{
	if (Measurements.Num() != 4)
	{
		return FVector::ZeroVector;
	}

	// Multilateration using least-squares (4 observers gives unique solution)
	// Use first 3 for initial estimate, then refine with 4th
	TArray<FObserverMeasurement> First3;
	First3.Add(Measurements[0]);
	First3.Add(Measurements[1]);
	First3.Add(Measurements[2]);

	FVector InitialEstimate = TriangulateFrom3Observers(First3);

	// Refine using all 4 measurements with gradient descent
	FVector Position = InitialEstimate;
	const int32 MaxIterations = 10;
	const float LearningRate = 0.1f;

	for (int32 Iter = 0; Iter < MaxIterations; Iter++)
	{
		FVector Gradient = FVector::ZeroVector;

		for (int32 i = 0; i < 4; i++)
		{
			FVector ToTarget = Position - Measurements[i].ObserverPosition;
			float CalculatedDist = ToTarget.Size();
			float Error = CalculatedDist - Measurements[i].Distance;

			if (!FMath::IsNearlyZero(CalculatedDist))
			{
				FVector Direction = ToTarget / CalculatedDist;
				Gradient += Direction * Error;
			}
		}

		Position -= Gradient * LearningRate;
	}

	return Position;
}

FGeometricValidationResult UTriangulationValidator::TriangulateOverdetermined(const TArray<FObserverMeasurement>& Measurements)
{
	FGeometricValidationResult Result;
	Result.ObserverCount = Measurements.Num();

	if (Measurements.Num() < 5)
	{
		Result.bIsValid = false;
		return Result;
	}

	// Least-squares multilateration
	// Initial estimate: Average of observer positions weighted by inverse distance
	FVector WeightedSum = FVector::ZeroVector;
	float TotalWeight = 0.0f;

	for (const FObserverMeasurement& Obs : Measurements)
	{
		float Weight = 1.0f / FMath::Max(Obs.Distance, 1.0f);
		WeightedSum += Obs.ObserverPosition * Weight;
		TotalWeight += Weight;
	}

	FVector Position = WeightedSum / TotalWeight;

	// Iterative refinement using Gauss-Newton method
	const int32 MaxIterations = 20;
	const float ConvergenceThreshold = 0.01f;

	for (int32 Iter = 0; Iter < MaxIterations; Iter++)
	{
		FVector Gradient = FVector::ZeroVector;
		float TotalError = 0.0f;

		for (const FObserverMeasurement& Obs : Measurements)
		{
			FVector ToTarget = Position - Obs.ObserverPosition;
			float CalculatedDist = ToTarget.Size();
			float Error = CalculatedDist - Obs.Distance;
			TotalError += Error * Error;

			if (!FMath::IsNearlyZero(CalculatedDist))
			{
				FVector Direction = ToTarget / CalculatedDist;
				Gradient += Direction * Error;
			}
		}

		// Update position
		FVector Delta = Gradient * 0.1f; // Learning rate
		Position -= Delta;

		// Check convergence
		if (Delta.Size() < ConvergenceThreshold)
		{
			break;
		}
	}

	Result.TriangulatedPosition = Position;
	Result.Confidence = CalculateConfidence(Measurements.Num());
	Result.bIsValid = true;

	return Result;
}

// ============================================================================
// TOLERANCE AND ERROR CALCULATIONS
// ============================================================================

float UTriangulationValidator::CalculateTolerance(float Distance, float ScaleFactor, int32 ObserverCount)
{
	// Base tolerance
	float Tolerance = BaseGeometricTolerance;

	// Add distance-based tolerance (larger distances = more tolerance)
	float DistanceKM = Distance / 100000.0f; // Convert cm to km
	Tolerance += DistanceKM * TolerancePerKM;

	// Scale factor affects precision (larger scale factor = more compression = more tolerance)
	Tolerance *= FMath::Max(1.0f, ScaleFactor);

	// More observers = tighter tolerance (anti-fragile property)
	float ObserverFactor = 1.0f / FMath::Sqrt(static_cast<float>(FMath::Max(1, ObserverCount)));
	Tolerance *= ObserverFactor;

	return Tolerance;
}

float UTriangulationValidator::CalculateConfidence(int32 ObserverCount)
{
	// Anti-fragile confidence scaling
	// More observers = exponentially higher confidence
	// Formula: Confidence = 1 - (1 / ObserverCount^1.5)

	if (ObserverCount < 2)
	{
		return 0.0f;
	}

	float Power = FMath::Pow(static_cast<float>(ObserverCount), 1.5f);
	float Confidence = 1.0f - (1.0f / Power);

	// Clamp to [0, 1] range
	return FMath::Clamp(Confidence, 0.0f, 1.0f);
}

float UTriangulationValidator::CalculateGeometricError(const TArray<FObserverMeasurement>& Measurements)
{
	if (Measurements.Num() < 3)
	{
		return 0.0f;
	}

	// Calculate centroid as approximate position
	FVector Centroid = FVector::ZeroVector;
	for (const FObserverMeasurement& Obs : Measurements)
	{
		Centroid += Obs.ObserverPosition + (Obs.Direction * Obs.Distance);
	}
	Centroid /= static_cast<float>(Measurements.Num());

	// Calculate RMS error
	float TotalSquaredError = 0.0f;
	for (const FObserverMeasurement& Obs : Measurements)
	{
		float CalculatedDistance = (Centroid - Obs.ObserverPosition).Size();
		float Error = FMath::Abs(CalculatedDistance - Obs.Distance);
		TotalSquaredError += Error * Error;
	}

	return FMath::Sqrt(TotalSquaredError / static_cast<float>(Measurements.Num()));
}

// ============================================================================
// HELPER METHODS
// ============================================================================

float UTriangulationValidator::GetDistanceBetweenObservers(const FObserverMeasurement& ObsA, const FObserverMeasurement& ObsB) const
{
	return (ObsB.ObserverPosition - ObsA.ObserverPosition).Size();
}

float UTriangulationValidator::GetAngleBetweenDirections(const FVector& DirA, const FVector& DirB) const
{
	float DotProduct = FVector::DotProduct(DirA.GetSafeNormal(), DirB.GetSafeNormal());
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);
	return FMath::Acos(DotProduct);
}

bool UTriangulationValidator::AreConsistentWithinTolerance(float Measured, float Expected, float Tolerance) const
{
	return FMath::Abs(Measured - Expected) <= Tolerance;
}

void UTriangulationValidator::DrawDebugGeometry(const TArray<FObserverMeasurement>& Measurements, const FVector& TargetPosition, bool bIsValid)
{
	if (!bEnableDebugVisualization)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
	if (!World)
	{
		return;
	}

	FColor ValidationColor = bIsValid ? FColor::Green : FColor::Red;
	float LifeTime = 5.0f;

	// Draw lines from each observer to target
	for (const FObserverMeasurement& Obs : Measurements)
	{
		// Observer position sphere
		DrawDebugSphere(World, Obs.ObserverPosition, 50.0f, 8, FColor::Blue, false, LifeTime);

		// Line to target
		DrawDebugLine(World, Obs.ObserverPosition, TargetPosition, ValidationColor, false, LifeTime);

		// Distance sphere around observer
		DrawDebugSphere(World, Obs.ObserverPosition, Obs.Distance, 16, FColor::Cyan, false, LifeTime);
	}

	// Target position
	DrawDebugSphere(World, TargetPosition, 100.0f, 12, ValidationColor, false, LifeTime, 0, 5.0f);
}
