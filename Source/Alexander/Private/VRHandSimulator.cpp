// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRHandSimulator.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UVRHandSimulator::UVRHandSimulator()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.016f; // ~60fps

	CurrentHandPosition = DefaultRestPosition;
	bIsMoving = false;
	MotionElapsedTime = 0.0f;
}

void UVRHandSimulator::BeginPlay()
{
	Super::BeginPlay();

	// Initialize hand at rest position
	CurrentHandPosition = DefaultRestPosition;

	UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Initialized at rest position (%s)"),
		*DefaultRestPosition.ToString());
}

void UVRHandSimulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsMoving)
		return;

	// Update motion time
	MotionElapsedTime += DeltaTime;
	float NormalizedTime = FMath::Clamp(MotionElapsedTime / ActiveMotionProfile.Duration, 0.0f, 1.0f);

	// Calculate new hand position
	CurrentHandPosition = CalculateArcPosition(NormalizedTime, ActiveMotionProfile);

	// Draw debug visualization
	if (bDebugVisualization)
	{
		DrawDebugMotionPath();
	}

	// Check if motion complete
	if (NormalizedTime >= 1.0f)
	{
		bIsMoving = false;
		MotionElapsedTime = 0.0f;
		UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Motion complete - reached %s"), *CurrentHandPosition.ToString());
	}
}

void UVRHandSimulator::MoveHandTo(const FVector& TargetPosition, float Duration, EMotionArcType ArcType)
{
	// Clamp target to maximum reach
	FVector ClampedTarget = TargetPosition;
	float Distance = (TargetPosition - DefaultRestPosition).Size();
	if (Distance > MaxReachDistance)
	{
		FVector Direction = (TargetPosition - DefaultRestPosition).GetSafeNormal();
		ClampedTarget = DefaultRestPosition + Direction * MaxReachDistance;
		UE_LOG(LogTemp, Warning, TEXT("VRHandSimulator: Target clamped to max reach distance"));
	}

	// Setup motion profile
	ActiveMotionProfile.StartPosition = CurrentHandPosition;
	ActiveMotionProfile.TargetPosition = ClampedTarget;
	ActiveMotionProfile.Duration = Duration;
	ActiveMotionProfile.ArcType = ArcType;
	ActiveMotionProfile.ArcHeight = NaturalArcHeight;
	ActiveMotionProfile.EaseFactor = 0.5f;

	// Start motion
	bIsMoving = true;
	MotionElapsedTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Moving from %s to %s over %.2fs using %s arc"),
		*CurrentHandPosition.ToString(),
		*ClampedTarget.ToString(),
		Duration,
		*UEnum::GetValueAsString(ArcType));
}

void UVRHandSimulator::SimulateGrab(AActor* TargetObject, float ReachDuration)
{
	if (!TargetObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandSimulator: Cannot grab null object"));
		return;
	}

	HeldObject = TargetObject;
	FVector TargetPosition = TargetObject->GetActorLocation();

	// Use natural motion for grabbing
	MoveHandTo(TargetPosition, ReachDuration, EMotionArcType::Natural);

	UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Simulating grab of %s"), *TargetObject->GetName());
}

void UVRHandSimulator::SimulateRelease(const FVector& RestPosition, float ReturnDuration)
{
	if (HeldObject)
	{
		UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Releasing %s"), *HeldObject->GetName());
		HeldObject = nullptr;
	}

	// Return to rest position
	MoveHandTo(RestPosition, ReturnDuration, EMotionArcType::Natural);
}

void UVRHandSimulator::SimulateStickInput(const FVector& InputDirection, float InputMagnitude, float Duration)
{
	// Calculate stick deflection position
	FVector StickPosition = CurrentHandPosition + InputDirection.GetSafeNormal() * InputMagnitude * 10.0f;

	// Move to deflected position
	MoveHandTo(StickPosition, Duration, EMotionArcType::Linear);

	UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Applying stick input - Direction: %s, Magnitude: %.2f"),
		*InputDirection.ToString(), InputMagnitude);
}

void UVRHandSimulator::StopMotion()
{
	bIsMoving = false;
	MotionElapsedTime = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("VRHandSimulator: Motion stopped at %s"), *CurrentHandPosition.ToString());
}

FVector UVRHandSimulator::CalculateArcPosition(float NormalizedTime, const FMotionProfile& Profile)
{
	// Apply easing if enabled
	float T = bSmoothMotion ? ApplyEasing(NormalizedTime, Profile.EaseFactor) : NormalizedTime;

	// Calculate position based on arc type
	switch (Profile.ArcType)
	{
	case EMotionArcType::Linear:
		return CalculateLinearMotion(T, Profile.StartPosition, Profile.TargetPosition);

	case EMotionArcType::Bezier:
		return CalculateBezierMotion(T, Profile.StartPosition, Profile.TargetPosition, Profile.ArcHeight);

	case EMotionArcType::Parabolic:
		return CalculateParabolicMotion(T, Profile.StartPosition, Profile.TargetPosition, Profile.ArcHeight);

	case EMotionArcType::Circular:
		return CalculateCircularMotion(T, Profile.StartPosition, Profile.TargetPosition, Profile.ArcHeight);

	case EMotionArcType::Natural:
	default:
		return CalculateNaturalMotion(T, Profile.StartPosition, Profile.TargetPosition, Profile.ArcHeight);
	}
}

FVector UVRHandSimulator::CalculateVelocity(float NormalizedTime, const FMotionProfile& Profile)
{
	// Calculate velocity using numerical differentiation (derivative)
	float DeltaT = 0.001f; // Small time step
	float T1 = FMath::Clamp(NormalizedTime - DeltaT, 0.0f, 1.0f);
	float T2 = FMath::Clamp(NormalizedTime + DeltaT, 0.0f, 1.0f);

	FVector P1 = CalculateArcPosition(T1, Profile);
	FVector P2 = CalculateArcPosition(T2, Profile);

	// dP/dt = (P2 - P1) / (2 * DeltaT)
	FVector Velocity = (P2 - P1) / (2.0f * DeltaT);

	// Scale by duration (chain rule: dP/dt = dP/dT * dT/dt)
	return Velocity / Profile.Duration;
}

FVector UVRHandSimulator::CalculateAcceleration(float NormalizedTime, const FMotionProfile& Profile)
{
	// Calculate acceleration using numerical differentiation (second derivative)
	float DeltaT = 0.001f;
	float T1 = FMath::Clamp(NormalizedTime - DeltaT, 0.0f, 1.0f);
	float T2 = NormalizedTime;
	float T3 = FMath::Clamp(NormalizedTime + DeltaT, 0.0f, 1.0f);

	FVector V1 = CalculateVelocity(T1, Profile);
	FVector V2 = CalculateVelocity(T2, Profile);
	FVector V3 = CalculateVelocity(T3, Profile);

	// d²P/dt² = (V3 - V1) / (2 * DeltaT)
	return (V3 - V1) / (2.0f * DeltaT);
}

float UVRHandSimulator::ApplyEasing(float T, float EaseFactor)
{
	// Blend between linear and ease-in-out based on EaseFactor
	float EasedT = EaseInOutQuintic(T);
	return FMath::Lerp(T, EasedT, EaseFactor);
}

FVector UVRHandSimulator::CalculateLinearMotion(float T, const FVector& Start, const FVector& End)
{
	// Simple linear interpolation
	// P(t) = Start + t * (End - Start)
	return FMath::Lerp(Start, End, T);
}

FVector UVRHandSimulator::CalculateBezierMotion(float T, const FVector& Start, const FVector& End, float ArcHeight)
{
	// Cubic Bezier curve with automatic control points
	// Control points create an arc above the straight line
	FVector MidPoint = (Start + End) / 2.0f;
	FVector UpVector = FVector::UpVector;

	// Control point 1: 1/3 along path, raised by ArcHeight
	FVector C1 = FMath::Lerp(Start, MidPoint, 0.5f) + UpVector * ArcHeight;

	// Control point 2: 2/3 along path, raised by ArcHeight
	FVector C2 = FMath::Lerp(MidPoint, End, 0.5f) + UpVector * ArcHeight;

	// Cubic Bezier formula: P(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
	float OneMinusT = 1.0f - T;
	float T2 = T * T;
	float T3 = T2 * T;
	float OneMinusT2 = OneMinusT * OneMinusT;
	float OneMinusT3 = OneMinusT2 * OneMinusT;

	return OneMinusT3 * Start +
		3.0f * OneMinusT2 * T * C1 +
		3.0f * OneMinusT * T2 * C2 +
		T3 * End;
}

FVector UVRHandSimulator::CalculateParabolicMotion(float T, const FVector& Start, const FVector& End, float ArcHeight)
{
	// Parabolic arc (ballistic trajectory)
	// Horizontal: linear interpolation
	// Vertical: parabolic function with peak at t=0.5

	FVector LinearPosition = FMath::Lerp(Start, End, T);

	// Parabolic height: h(t) = 4 * ArcHeight * t * (1-t)
	// This gives maximum height at t=0.5
	float ParabolicHeight = 4.0f * ArcHeight * T * (1.0f - T);

	return LinearPosition + FVector::UpVector * ParabolicHeight;
}

FVector UVRHandSimulator::CalculateCircularMotion(float T, const FVector& Start, const FVector& End, float ArcHeight)
{
	// Circular arc motion
	// Calculate arc along a circle segment

	FVector Chord = End - Start;
	float ChordLength = Chord.Size();
	FVector ChordDirection = Chord.GetSafeNormal();

	// Calculate circle radius from chord length and arc height
	// Using: r = (h² + (c/2)²) / (2h)
	float HalfChord = ChordLength / 2.0f;
	float Radius = (ArcHeight * ArcHeight + HalfChord * HalfChord) / (2.0f * ArcHeight);

	// Center of circle
	FVector ChordMidpoint = (Start + End) / 2.0f;
	FVector UpVector = FVector::UpVector;
	FVector CenterOffset = UpVector * (Radius - ArcHeight);
	FVector CircleCenter = ChordMidpoint + CenterOffset;

	// Calculate angle range
	float MaxAngle = 2.0f * FMath::Asin(HalfChord / Radius);

	// Current angle
	float Angle = -MaxAngle / 2.0f + MaxAngle * T;

	// Position on circle
	FVector Right = FVector::CrossProduct(ChordDirection, UpVector).GetSafeNormal();
	FVector LocalUp = FVector::CrossProduct(Right, ChordDirection).GetSafeNormal();

	FVector RelativePos = ChordDirection * (HalfChord * FMath::Sin(Angle) / FMath::Sin(MaxAngle / 2.0f)) +
		LocalUp * (Radius * FMath::Cos(Angle) - CenterOffset.Size());

	return ChordMidpoint + RelativePos;
}

FVector UVRHandSimulator::CalculateNaturalMotion(float T, const FVector& Start, const FVector& End, float ArcHeight)
{
	// Natural human motion: combination of Bezier curve with slight upward bias
	// Humans tend to move hands in arcs with slight lift in the middle

	// Use Bezier as base
	FVector BezierPos = CalculateBezierMotion(T, Start, End, ArcHeight);

	// Add natural human motion characteristics:
	// 1. Slight acceleration at start
	// 2. Constant velocity in middle
	// 3. Slight deceleration at end
	float VelocityProfile = EaseInOutCubic(T);

	// Add subtle lateral movement (natural hand motion isn't perfectly 2D)
	FVector Direction = (End - Start).GetSafeNormal();
	FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

	// Slight S-curve in lateral direction
	float LateralOffset = FMath::Sin(T * PI) * ArcHeight * 0.2f;
	FVector LateralMotion = Right * LateralOffset;

	return BezierPos + LateralMotion;
}

float UVRHandSimulator::EaseInOutCubic(float T)
{
	// Cubic ease-in-out function
	// Smooth acceleration and deceleration
	if (T < 0.5f)
	{
		// Ease in: f(t) = 4t³
		return 4.0f * T * T * T;
	}
	else
	{
		// Ease out: f(t) = 1 - (-2t + 2)³ / 2
		float F = -2.0f * T + 2.0f;
		return 1.0f - F * F * F / 2.0f;
	}
}

float UVRHandSimulator::EaseInOutQuintic(float T)
{
	// Quintic ease-in-out (smoother than cubic)
	// Fifth-degree polynomial for ultra-smooth motion
	if (T < 0.5f)
	{
		// Ease in: f(t) = 16t⁵
		return 16.0f * T * T * T * T * T;
	}
	else
	{
		// Ease out: f(t) = 1 - (-2t + 2)⁵ / 2
		float F = -2.0f * T + 2.0f;
		return 1.0f - F * F * F * F * F / 2.0f;
	}
}

void UVRHandSimulator::DrawDebugMotionPath()
{
	if (!GetWorld())
		return;

	// Draw current hand position
	DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation() + CurrentHandPosition,
		5.0f, 12, FColor::Green, false, 0.0f, 0, 2.0f);

	// Draw target position
	if (bIsMoving)
	{
		DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation() + ActiveMotionProfile.TargetPosition,
			5.0f, 12, FColor::Red, false, 0.0f, 0, 2.0f);

		// Draw motion path
		const int32 PathSegments = 20;
		for (int32 i = 0; i < PathSegments; i++)
		{
			float T1 = (float)i / PathSegments;
			float T2 = (float)(i + 1) / PathSegments;

			FVector P1 = GetOwner()->GetActorLocation() + CalculateArcPosition(T1, ActiveMotionProfile);
			FVector P2 = GetOwner()->GetActorLocation() + CalculateArcPosition(T2, ActiveMotionProfile);

			DrawDebugLine(GetWorld(), P1, P2, FColor::Cyan, false, 0.0f, 0, 1.0f);
		}

		// Draw velocity vector
		FVector Velocity = CalculateVelocity(MotionElapsedTime / ActiveMotionProfile.Duration, ActiveMotionProfile);
		DrawDebugDirectionalArrow(GetWorld(),
			GetOwner()->GetActorLocation() + CurrentHandPosition,
			GetOwner()->GetActorLocation() + CurrentHandPosition + Velocity * 0.1f,
			10.0f, FColor::Yellow, false, 0.0f, 0, 2.0f);
	}

	// Draw rest position
	DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation() + DefaultRestPosition,
		3.0f, 12, FColor::Blue, false, 0.0f, 0, 1.0f);
}
