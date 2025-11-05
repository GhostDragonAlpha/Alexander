// Copyright Epic Games, Inc. All Rights Reserved.

#include "CelestialScalingSafetySystem.h"
#include "PlayerOriginManager.h"
#include "Spaceship.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UCelestialScalingSafetySystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize configuration with safe defaults
	bSafetySystemEnabled = true;
	bAutoRecoveryEnabled = true;
	bVerboseLogging = false;

	MinScaleFactor = 0.0001f;
	MaxScaleFactor = 10000.0f;
	MaxGForce = 50.0f;
	MaxSafeVelocityMS = 1000000.0f; // 1000 km/s
	PrecisionWarningThreshold = 1000000.0f; // 1000 km
	EmergencyRecenterThreshold = 5000000.0f; // 5000 km
	NetworkDesyncTolerance = 10.0f; // 10 meters
	MaxEventLogSize = 100;

	// Initialize statistics
	TotalEventCount = 0;
	CriticalEventCount = 0;
	RecoveryOperationCount = 0;
	DesyncEventCount = 0;
	LastEventTime = 0.0f;

	// Clear event log
	EventLog.Empty();

	UE_LOG(LogTemp, Log, TEXT("CelestialScalingSafetySystem initialized - Safety checks: %s, Auto-recovery: %s"),
		bSafetySystemEnabled ? TEXT("ENABLED") : TEXT("DISABLED"),
		bAutoRecoveryEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCelestialScalingSafetySystem::Deinitialize()
{
	// Log final statistics
	UE_LOG(LogTemp, Log, TEXT("CelestialScalingSafetySystem shutting down - Total events: %d, Critical: %d, Recoveries: %d"),
		TotalEventCount, CriticalEventCount, RecoveryOperationCount);

	Super::Deinitialize();
}

// ============================================================================
// SCALE FACTOR SAFETY
// ============================================================================

float UCelestialScalingSafetySystem::ClampScaleFactor(float Factor) const
{
	if (!bSafetySystemEnabled)
		return Factor;

	if (!FMath::IsFinite(Factor) || Factor < FLOAT_EPSILON)
	{
		UE_LOG(LogTemp, Warning, TEXT("SafetySystem: Invalid scale factor %.6f - clamping to minimum"), Factor);
		return MinScaleFactor;
	}

	return FMath::Clamp(Factor, MinScaleFactor, MaxScaleFactor);
}

bool UCelestialScalingSafetySystem::IsValidScaleFactor(float Factor) const
{
	if (!FMath::IsFinite(Factor))
		return false;

	return Factor >= MinScaleFactor && Factor <= MaxScaleFactor;
}

float UCelestialScalingSafetySystem::GetSafeScaleFactor(float Distance) const
{
	// Logarithmic scaling with safety bounds
	// Closer = larger scale, farther = smaller scale

	if (Distance <= 0.0f || !FMath::IsFinite(Distance))
		return 1.0f;

	// Reference distance: 100 km
	const float ReferenceDistance = 10000000.0f; // 100 km in cm

	// Calculate logarithmic scale factor
	float ScaleFactor = FMath::Pow(ReferenceDistance / Distance, 0.5f);

	// Clamp to safe range
	return ClampScaleFactor(ScaleFactor);
}

// ============================================================================
// GRAVITATIONAL FORCE SAFETY
// ============================================================================

FVector UCelestialScalingSafetySystem::ClampGravitationalForce(const FVector& Force, float TargetMass, float MaxGForceOverride) const
{
	if (!bSafetySystemEnabled || TargetMass < FLOAT_EPSILON)
		return Force;

	// Check for invalid force
	if (IsVectorInvalid(Force))
	{
		UE_LOG(LogTemp, Error, TEXT("SafetySystem: Invalid gravitational force detected - resetting to zero"));
		return FVector::ZeroVector;
	}

	// Calculate acceleration (F = ma, so a = F/m)
	float Acceleration = Force.Size() / FMath::Max(TargetMass, 1.0f);

	// Convert to G-force
	float GForce = Acceleration / STANDARD_GRAVITY;

	// Clamp if exceeds maximum
	float MaxG = FMath::Max(MaxGForceOverride, 0.1f);
	if (GForce > MaxG)
	{
		// Reduce force to stay within limits
		float MaxAcceleration = MaxG * STANDARD_GRAVITY;
		FVector ClampedForce = Force.GetSafeNormal() * MaxAcceleration * TargetMass;

		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Verbose, TEXT("SafetySystem: Clamped gravitational force from %.1f G to %.1f G"),
				GForce, MaxG);
		}

		return ClampedForce;
	}

	return Force;
}

bool UCelestialScalingSafetySystem::IsForceWithinSafeLimits(const FVector& Force, float TargetMass) const
{
	if (IsVectorInvalid(Force) || TargetMass < FLOAT_EPSILON)
		return false;

	float GForce = CalculateGForce(Force / FMath::Max(TargetMass, 1.0f));
	return GForce <= MaxGForce;
}

float UCelestialScalingSafetySystem::CalculateGForce(const FVector& Acceleration) const
{
	float AccelMagnitude = Acceleration.Size();
	return AccelMagnitude / STANDARD_GRAVITY;
}

float UCelestialScalingSafetySystem::GetMaxSafeVelocity(float ScaleFactor) const
{
	// Adjust maximum velocity based on scale factor
	// Smaller scales (far distances) = higher velocities allowed
	// Larger scales (close distances) = lower velocities for precision

	float BaseMaxVelocity = MaxSafeVelocityMS;

	// Scale velocity limits inversely with scale factor
	float ScaledMaxVelocity = BaseMaxVelocity / FMath::Max(ScaleFactor, 0.1f);

	return FMath::Clamp(ScaledMaxVelocity, 100.0f, MaxSafeVelocityMS);
}

// ============================================================================
// PRECISION ERROR DETECTION
// ============================================================================

bool UCelestialScalingSafetySystem::DetectPrecisionError(const FVector& Position) const
{
	// Check if position is beyond precision warning threshold
	float Distance = Position.Size() * CM_TO_METERS;
	return Distance > PrecisionWarningThreshold;
}

float UCelestialScalingSafetySystem::CalculateFloatingPointError(double OriginalValue, float ConvertedValue) const
{
	if (FMath::Abs(OriginalValue) < FLOAT_EPSILON)
		return 0.0f;

	// Calculate relative error percentage
	double Error = FMath::Abs(OriginalValue - static_cast<double>(ConvertedValue));
	return static_cast<float>((Error / FMath::Abs(OriginalValue)) * 100.0);
}

bool UCelestialScalingSafetySystem::IsPositionPrecisionSafe(const FVector& Position) const
{
	// Check if position is within safe precision range
	float Distance = Position.Size() * CM_TO_METERS;
	return Distance < PrecisionWarningThreshold;
}

float UCelestialScalingSafetySystem::GetPrecisionThreshold(float DistanceFromOrigin) const
{
	// Dynamic precision threshold based on distance
	// Closer to origin = tighter precision requirements
	// Farther from origin = looser requirements (but should trigger recenter)

	const float MinThreshold = 0.01f; // 1 cm
	const float MaxThreshold = 100.0f; // 1 meter

	// Logarithmic scaling
	float Threshold = MinThreshold * FMath::Pow(10.0f, DistanceFromOrigin / 1000000.0f);

	return FMath::Clamp(Threshold, MinThreshold, MaxThreshold);
}

// ============================================================================
// AUTOMATIC RECOVERY
// ============================================================================

void UCelestialScalingSafetySystem::TriggerEmergencyRecenter(UPlayerOriginManager* OriginManager)
{
	if (!OriginManager)
	{
		LogSafetyEvent(TEXT("Emergency recenter requested but OriginManager is null"),
			ESafetyErrorSeverity::Error);
		return;
	}

	LogSafetyEvent(TEXT("EMERGENCY RECENTER TRIGGERED"), ESafetyErrorSeverity::Critical);

	OriginManager->RecenterUniverse();

	RecoveryOperationCount++;

	UE_LOG(LogTemp, Error, TEXT("SafetySystem: Emergency universe recenter completed (total: %d)"),
		RecoveryOperationCount);
}

void UCelestialScalingSafetySystem::FallbackToWorldSpaceMode(ASpaceship* Ship, const FString& Reason)
{
	if (!Ship)
		return;

	FString Message = FString::Printf(TEXT("Fallback to world-space mode: %s"), *Reason);
	LogSafetyEvent(Message, ESafetyErrorSeverity::Critical, Ship);

	// Disable origin-centered physics on the spaceship
	Ship->bUseOriginCenteredPhysics = false;

	RecoveryOperationCount++;

	UE_LOG(LogTemp, Error, TEXT("SafetySystem: Ship '%s' reverted to world-space mode - Reason: %s"),
		*Ship->GetName(), *Reason);
}

void UCelestialScalingSafetySystem::ResetActorToSafeState(AActor* Actor)
{
	if (!Actor)
		return;

	// Reset position to origin
	Actor->SetActorLocation(FVector::ZeroVector);

	// Reset rotation to identity
	Actor->SetActorRotation(FRotator::ZeroRotator);

	// Reset velocity if it's a spaceship
	if (ASpaceship* Ship = Cast<ASpaceship>(Actor))
	{
		Ship->Velocity = FVector::ZeroVector;
	}

	LogSafetyEvent(TEXT("Actor reset to safe state"), ESafetyErrorSeverity::Warning, Actor);
	RecoveryOperationCount++;

	UE_LOG(LogTemp, Warning, TEXT("SafetySystem: Actor '%s' reset to safe state"), *Actor->GetName());
}

bool UCelestialScalingSafetySystem::ValidateAndRepairPhysicsState(AActor* Actor)
{
	if (!Actor)
		return false;

	bool bRepairsNeeded = false;

	// Check position
	FVector Position = Actor->GetActorLocation();
	if (IsVectorInvalid(Position))
	{
		Actor->SetActorLocation(FVector::ZeroVector);
		bRepairsNeeded = true;
		LogSafetyEvent(TEXT("Invalid position repaired"), ESafetyErrorSeverity::Error, Actor);
	}

	// Check rotation
	FRotator Rotation = Actor->GetActorRotation();
	if (IsValueInvalid(Rotation.Pitch) || IsValueInvalid(Rotation.Yaw) || IsValueInvalid(Rotation.Roll))
	{
		Actor->SetActorRotation(FRotator::ZeroRotator);
		bRepairsNeeded = true;
		LogSafetyEvent(TEXT("Invalid rotation repaired"), ESafetyErrorSeverity::Error, Actor);
	}

	// Check velocity for spaceships
	if (ASpaceship* Ship = Cast<ASpaceship>(Actor))
	{
		if (IsVectorInvalid(Ship->Velocity))
		{
			Ship->Velocity = FVector::ZeroVector;
			bRepairsNeeded = true;
			LogSafetyEvent(TEXT("Invalid velocity repaired"), ESafetyErrorSeverity::Error, Ship);
		}
	}

	if (bRepairsNeeded)
	{
		RecoveryOperationCount++;
		UE_LOG(LogTemp, Warning, TEXT("SafetySystem: Physics state repairs applied to '%s'"),
			*Actor->GetName());
	}

	return bRepairsNeeded;
}

// ============================================================================
// NETWORK SAFETY
// ============================================================================

void UCelestialScalingSafetySystem::LogNetworkDesync(AActor* Actor, const FVector& ClientPosition, const FVector& ServerPosition)
{
	float ErrorDistance = FVector::Dist(ClientPosition, ServerPosition) * CM_TO_METERS;

	ESafetyErrorSeverity Severity = ESafetyErrorSeverity::Info;
	if (ErrorDistance > 100.0f)
		Severity = ESafetyErrorSeverity::Critical;
	else if (ErrorDistance > 50.0f)
		Severity = ESafetyErrorSeverity::Error;
	else if (ErrorDistance > NetworkDesyncTolerance)
		Severity = ESafetyErrorSeverity::Warning;

	FString Message = FString::Printf(TEXT("Network desync: %.2f m error"), ErrorDistance);
	LogSafetyEvent(Message, Severity, Actor);

	DesyncEventCount++;

	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SafetySystem: Network desync on '%s' - Client: %s, Server: %s, Error: %.2f m"),
			Actor ? *Actor->GetName() : TEXT("Unknown"),
			*ClientPosition.ToString(),
			*ServerPosition.ToString(),
			ErrorDistance);
	}
}

bool UCelestialScalingSafetySystem::IsDesyncAcceptable(float ErrorDistance) const
{
	return ErrorDistance <= NetworkDesyncTolerance;
}

float UCelestialScalingSafetySystem::GetNetworkCorrectionStrength(float ErrorDistance) const
{
	// Smooth correction strength based on error magnitude
	// Small errors: gentle correction
	// Large errors: immediate correction

	if (ErrorDistance <= NetworkDesyncTolerance)
		return 0.0f; // No correction needed

	if (ErrorDistance >= 100.0f)
		return 1.0f; // Immediate correction

	// Linear interpolation between tolerance and 100m
	return (ErrorDistance - NetworkDesyncTolerance) / (100.0f - NetworkDesyncTolerance);
}

// ============================================================================
// LOGGING AND MONITORING
// ============================================================================

void UCelestialScalingSafetySystem::LogSafetyEvent(const FString& Message, ESafetyErrorSeverity Severity, AActor* Actor)
{
	if (!bSafetySystemEnabled && Severity < ESafetyErrorSeverity::Critical)
		return;

	// Get current time
	float CurrentTime = 0.0f;
	if (UWorld* World = GetWorld())
	{
		CurrentTime = World->GetTimeSeconds();
	}

	// Create event
	FString ActorName = Actor ? Actor->GetName() : TEXT("System");
	FSafetyEventLog Event(Severity, Message, ActorName, CurrentTime);

	// Add to log
	AddEventToLog(Event);

	// Update statistics
	TotalEventCount++;
	if (Severity >= ESafetyErrorSeverity::Critical)
	{
		CriticalEventCount++;
	}

	// Log to console based on severity
	switch (Severity)
	{
	case ESafetyErrorSeverity::Info:
		if (bVerboseLogging)
			UE_LOG(LogTemp, Log, TEXT("SafetySystem [INFO]: %s"), *FormatSafetyMessage(Message, Actor));
		break;

	case ESafetyErrorSeverity::Warning:
		UE_LOG(LogTemp, Warning, TEXT("SafetySystem [WARNING]: %s"), *FormatSafetyMessage(Message, Actor));
		break;

	case ESafetyErrorSeverity::Error:
		UE_LOG(LogTemp, Error, TEXT("SafetySystem [ERROR]: %s"), *FormatSafetyMessage(Message, Actor));
		break;

	case ESafetyErrorSeverity::Critical:
		UE_LOG(LogTemp, Error, TEXT("SafetySystem [CRITICAL]: %s"), *FormatSafetyMessage(Message, Actor));
		break;

	case ESafetyErrorSeverity::Fatal:
		UE_LOG(LogTemp, Fatal, TEXT("SafetySystem [FATAL]: %s"), *FormatSafetyMessage(Message, Actor));
		break;
	}

	LastEventTime = CurrentTime;
}

TArray<FSafetyEventLog> UCelestialScalingSafetySystem::GetRecentSafetyEvents(int32 MaxEvents) const
{
	int32 NumEvents = FMath::Min(MaxEvents, EventLog.Num());
	TArray<FSafetyEventLog> RecentEvents;

	// Return most recent events
	for (int32 i = EventLog.Num() - NumEvents; i < EventLog.Num(); ++i)
	{
		RecentEvents.Add(EventLog[i]);
	}

	return RecentEvents;
}

void UCelestialScalingSafetySystem::ClearEventLog()
{
	EventLog.Empty();
	UE_LOG(LogTemp, Log, TEXT("SafetySystem: Event log cleared"));
}

void UCelestialScalingSafetySystem::GetSafetyStatistics(int32& OutTotalEvents, int32& OutCriticalEvents, int32& OutRecoveryOperations) const
{
	OutTotalEvents = TotalEventCount;
	OutCriticalEvents = CriticalEventCount;
	OutRecoveryOperations = RecoveryOperationCount;
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

void UCelestialScalingSafetySystem::AddEventToLog(const FSafetyEventLog& Event)
{
	EventLog.Add(Event);

	// Prune old events if log is too large
	while (EventLog.Num() > MaxEventLogSize)
	{
		EventLog.RemoveAt(0);
	}
}

bool UCelestialScalingSafetySystem::IsValueInvalid(float Value) const
{
	return !FMath::IsFinite(Value);
}

bool UCelestialScalingSafetySystem::IsVectorInvalid(const FVector& Vector) const
{
	return IsValueInvalid(Vector.X) || IsValueInvalid(Vector.Y) || IsValueInvalid(Vector.Z);
}

FString UCelestialScalingSafetySystem::FormatSafetyMessage(const FString& Message, AActor* Actor) const
{
	if (Actor)
	{
		return FString::Printf(TEXT("[%s] %s"), *Actor->GetName(), *Message);
	}
	return Message;
}
