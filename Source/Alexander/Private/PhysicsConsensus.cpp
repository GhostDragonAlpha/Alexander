// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsConsensus.h"
#include "OrbitalBody.h"
#include "OrbitalMechanics.h"
#include "Engine/World.h"

UPhysicsConsensus::UPhysicsConsensus()
{
	// Initialize with default values
	BasePositionTolerance = 1.0f;
	TimeToleranceRate = 0.5f;
	BaseThrustTolerance = 1000.0f;
	ThrustTolerancePercentage = 0.05f;
	ConsensusThreshold = 0.67f;
	FlagThreshold = 3;
	KickThreshold = 5;
	KickTimeWindow = 10.0f;
	MaxHistorySize = 100;
	MaxThrustForce = 100000.0f;
	MaxSpeed = 1000.0f;
	PlayerMass = 50000.0f;
	bEnableDebugLogging = false;
}

// ============================================================================
// CORE PREDICTION AND VALIDATION
// ============================================================================

FVector UPhysicsConsensus::PredictPosition(int32 PlayerID, float DeltaTime)
{
	// Get last known state
	FVector LastPosition = GetLastKnownPosition(PlayerID);
	FVector LastVelocity = GetLastKnownVelocity(PlayerID);
	FVector LastThrust = GetLastReportedThrust(PlayerID);

	// Get player's orbital body for gravity calculations
	AOrbitalBody* OrbitalBody = GetPlayerOrbitalBody(PlayerID);
	FVector GravityAccel = FVector::ZeroVector;

	if (OrbitalBody && OrbitalBody->OrbitalMechanics)
	{
		// Calculate gravity at last known position
		// Note: CalculateGravitationalForce returns force, we need acceleration
		FVector GravityForce = OrbitalBody->CalculateGravitationalForce();
		GravityAccel = GravityForce / FMath::Max(OrbitalBody->GetMass(), 1.0f);
	}

	// Calculate thrust acceleration
	FVector ThrustAccel = LastThrust / FMath::Max(PlayerMass, 1.0f);

	// Total acceleration (gravity + thrust)
	FVector TotalAccel = GravityAccel + ThrustAccel;

	// Physics integration using kinematic equations
	// v = v0 + a*t
	FVector NewVelocity = LastVelocity + TotalAccel * DeltaTime;

	// s = s0 + v0*t + 0.5*a*t²
	FVector PredictedPosition = LastPosition +
		LastVelocity * DeltaTime +
		0.5f * TotalAccel * DeltaTime * DeltaTime;

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] PredictPosition Player %d: LastPos=%s, Predicted=%s, Gravity=%s, Thrust=%s"),
			PlayerID,
			*LastPosition.ToString(),
			*PredictedPosition.ToString(),
			*GravityAccel.ToString(),
			*ThrustAccel.ToString());
	}

	return PredictedPosition;
}

bool UPhysicsConsensus::ValidatePosition(int32 PlayerID, FVector ReportedPosition, float Timestamp)
{
	// Get last timestamp and calculate delta time
	float LastTimestamp = GetLastTimestamp(PlayerID);
	float DeltaTime = Timestamp - LastTimestamp;

	if (DeltaTime <= 0.0f)
	{
		// Invalid timestamp (out of order or same time)
		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PhysicsConsensus] ValidatePosition Player %d: Invalid DeltaTime %f"), PlayerID, DeltaTime);
		}
		return false;
	}

	// Predict where player should be
	FVector PredictedPosition = PredictPosition(PlayerID, DeltaTime);

	// Calculate distance between predicted and reported
	float Distance = FVector::Dist(PredictedPosition, ReportedPosition);

	// Calculate tolerance
	float Tolerance = CalculatePositionTolerance(DeltaTime, PlayerID);

	// Valid if within tolerance
	bool bIsValid = Distance <= Tolerance;

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] ValidatePosition Player %d: Distance=%f, Tolerance=%f, Valid=%s"),
			PlayerID, Distance, Tolerance, bIsValid ? TEXT("YES") : TEXT("NO"));
	}

	return bIsValid;
}

bool UPhysicsConsensus::ValidateThrust(int32 PlayerID, FVector ReportedThrust, FVector ReportedPosition)
{
	// Get last known state
	FVector LastPosition = GetLastKnownPosition(PlayerID);
	FVector LastVelocity = GetLastKnownVelocity(PlayerID);
	float LastTimestamp = GetLastTimestamp(PlayerID);

	// Get time elapsed (need current timestamp - we'll use the last report's sequence as proxy)
	// In real implementation, this would come from network timestamp
	float DeltaTime = 0.05f; // Assume 20 Hz update rate = 50ms

	// Calculate position if ONLY gravity acted (no thrust)
	AOrbitalBody* OrbitalBody = GetPlayerOrbitalBody(PlayerID);
	FVector GravityAccel = FVector::ZeroVector;

	if (OrbitalBody)
	{
		FVector GravityForce = OrbitalBody->CalculateGravitationalForce();
		GravityAccel = GravityForce / FMath::Max(OrbitalBody->GetMass(), 1.0f);
	}

	// Gravity-only position
	FVector GravityOnlyPosition = LastPosition +
		LastVelocity * DeltaTime +
		0.5f * GravityAccel * DeltaTime * DeltaTime;

	// Deviation from gravity trajectory
	FVector Deviation = ReportedPosition - GravityOnlyPosition;

	// Calculate thrust needed to explain deviation
	// Using s = 0.5*a*t², solve for a: a = 2*s/t²
	FVector RequiredAccel = (2.0f * Deviation) / FMath::Max(DeltaTime * DeltaTime, 0.001f);
	FVector RequiredThrust = RequiredAccel * PlayerMass;

	// Compare reported thrust to required thrust
	float ThrustError = FVector::Dist(ReportedThrust, RequiredThrust);

	// Calculate thrust tolerance
	float ThrustTolerance = CalculateThrustTolerance();

	// Valid if thrust explains the deviation
	bool bIsValid = ThrustError <= ThrustTolerance;

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] ValidateThrust Player %d: ReportedThrust=%s, RequiredThrust=%s, Error=%f, Tolerance=%f, Valid=%s"),
			PlayerID,
			*ReportedThrust.ToString(),
			*RequiredThrust.ToString(),
			ThrustError,
			ThrustTolerance,
			bIsValid ? TEXT("YES") : TEXT("NO"));
	}

	return bIsValid;
}

bool UPhysicsConsensus::ValidateDistanceOverTime(int32 PlayerID, float TimeWindow)
{
	TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (!History || History->Num() < 2)
	{
		// Not enough history to validate
		return true;
	}

	// Calculate total distance traveled
	float TotalDistance = CalculateTotalDistance(*History, TimeWindow);

	// Calculate maximum possible distance (max thrust for entire time)
	float MaxPossibleDistance = CalculateMaxPossibleDistance(*History, TimeWindow);

	// Calculate minimum possible distance (gravity only, no thrust)
	float MinPossibleDistance = CalculateMinPossibleDistance(*History, TimeWindow);

	// Valid if distance is within possible range
	bool bIsValid = (TotalDistance >= MinPossibleDistance * 0.9f) && (TotalDistance <= MaxPossibleDistance * 1.1f);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] ValidateDistanceOverTime Player %d: Distance=%f, Min=%f, Max=%f, Valid=%s"),
			PlayerID, TotalDistance, MinPossibleDistance, MaxPossibleDistance, bIsValid ? TEXT("YES") : TEXT("NO"));
	}

	return bIsValid;
}

// ============================================================================
// POSITION REPORTING
// ============================================================================

void UPhysicsConsensus::AddPositionReport(const FPositionReport& Report)
{
	TArray<FPositionReport>* History = GetOrCreatePositionHistory(Report.PlayerID);
	AddToHistory(*History, Report);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] AddPositionReport Player %d: Pos=%s, Vel=%s, Thrust=%s, Seq=%d"),
			Report.PlayerID,
			*Report.Position.ToString(),
			*Report.Velocity.ToString(),
			*Report.Thrust.ToString(),
			Report.SequenceNumber);
	}
}

FVector UPhysicsConsensus::GetLastKnownPosition(int32 PlayerID) const
{
	const TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (History && History->Num() > 0)
	{
		return History->Last().Position;
	}
	return FVector::ZeroVector;
}

FVector UPhysicsConsensus::GetLastKnownVelocity(int32 PlayerID) const
{
	const TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (History && History->Num() > 0)
	{
		return History->Last().Velocity;
	}
	return FVector::ZeroVector;
}

FVector UPhysicsConsensus::GetLastReportedThrust(int32 PlayerID) const
{
	const TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (History && History->Num() > 0)
	{
		return History->Last().Thrust;
	}
	return FVector::ZeroVector;
}

float UPhysicsConsensus::GetLastTimestamp(int32 PlayerID) const
{
	const TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (History && History->Num() > 0)
	{
		return History->Last().Timestamp;
	}
	return 0.0f;
}

// ============================================================================
// CONSENSUS VOTING
// ============================================================================

void UPhysicsConsensus::SubmitValidationVote(const FValidationVote& Vote)
{
	// Get or create vote storage for this player
	TMap<int32, TArray<FValidationVote>>* PlayerVotes = ValidationVotes.Find(Vote.TargetPlayerID);
	if (!PlayerVotes)
	{
		ValidationVotes.Add(Vote.TargetPlayerID, TMap<int32, TArray<FValidationVote>>());
		PlayerVotes = ValidationVotes.Find(Vote.TargetPlayerID);
	}

	// Get or create vote array for this sequence number
	TArray<FValidationVote>* SequenceVotes = PlayerVotes->Find(Vote.SequenceNumber);
	if (!SequenceVotes)
	{
		PlayerVotes->Add(Vote.SequenceNumber, TArray<FValidationVote>());
		SequenceVotes = PlayerVotes->Find(Vote.SequenceNumber);
	}

	// Add vote
	SequenceVotes->Add(Vote);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] SubmitValidationVote: Validator %d voted %s on Player %d Seq %d"),
			Vote.ValidatorID,
			Vote.bIsValid ? TEXT("VALID") : TEXT("INVALID"),
			Vote.TargetPlayerID,
			Vote.SequenceNumber);
	}
}

FConsensusResult UPhysicsConsensus::CalculateConsensus(int32 PlayerID, int32 SequenceNumber)
{
	FConsensusResult Result;
	Result.PlayerID = PlayerID;
	Result.bConsensusReached = false;
	Result.ValidVotes = 0;
	Result.InvalidVotes = 0;
	Result.AveragePositionError = 0.0f;

	// Get votes for this player and sequence
	TMap<int32, TArray<FValidationVote>>* PlayerVotes = ValidationVotes.Find(PlayerID);
	if (!PlayerVotes)
	{
		return Result;
	}

	TArray<FValidationVote>* SequenceVotes = PlayerVotes->Find(SequenceNumber);
	if (!SequenceVotes || SequenceVotes->Num() == 0)
	{
		return Result;
	}

	// Count votes
	float TotalPositionError = 0.0f;
	for (const FValidationVote& Vote : *SequenceVotes)
	{
		if (Vote.bIsValid)
		{
			Result.ValidVotes++;
		}
		else
		{
			Result.InvalidVotes++;
		}
		TotalPositionError += Vote.PositionError;
	}

	// Calculate average error
	int32 TotalVotes = Result.ValidVotes + Result.InvalidVotes;
	if (TotalVotes > 0)
	{
		Result.AveragePositionError = TotalPositionError / TotalVotes;
	}

	// Check if consensus reached (2/3 majority)
	float ValidPercentage = (float)Result.ValidVotes / (float)FMath::Max(TotalVotes, 1);
	Result.bConsensusReached = ValidPercentage >= ConsensusThreshold || ValidPercentage <= (1.0f - ConsensusThreshold);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] CalculateConsensus Player %d Seq %d: Valid=%d, Invalid=%d, Consensus=%s"),
			PlayerID, SequenceNumber, Result.ValidVotes, Result.InvalidVotes,
			Result.bConsensusReached ? TEXT("YES") : TEXT("NO"));
	}

	return Result;
}

bool UPhysicsConsensus::IsConsensusReached(int32 PlayerID)
{
	// Check if last sequence has consensus
	const TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (!History || History->Num() == 0)
	{
		return false;
	}

	int32 LastSequence = History->Last().SequenceNumber;
	FConsensusResult Result = CalculateConsensus(PlayerID, LastSequence);

	return Result.bConsensusReached;
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

EValidationState UPhysicsConsensus::GetValidationState(int32 PlayerID) const
{
	const EValidationState* State = ValidationStates.Find(PlayerID);
	return State ? *State : EValidationState::TRUSTED;
}

void UPhysicsConsensus::UpdateValidationState(int32 PlayerID, bool bValidationPassed)
{
	EValidationState CurrentState = GetValidationState(PlayerID);

	if (!bValidationPassed)
	{
		// Validation failed - increment failure count
		int32* FailureCount = FailureCounts.Find(PlayerID);
		if (!FailureCount)
		{
			FailureCounts.Add(PlayerID, 1);
			FirstFailureTimestamp.Add(PlayerID, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
			FailureCount = FailureCounts.Find(PlayerID);
		}
		else
		{
			(*FailureCount)++;
		}

		// Check if we need to update state
		if (*FailureCount >= KickThreshold)
		{
			ValidationStates.Add(PlayerID, EValidationState::KICKED);
			UE_LOG(LogTemp, Warning, TEXT("[PhysicsConsensus] Player %d KICKED - %d failures"), PlayerID, *FailureCount);
		}
		else if (*FailureCount >= FlagThreshold)
		{
			ValidationStates.Add(PlayerID, EValidationState::FLAGGED);
			UE_LOG(LogTemp, Warning, TEXT("[PhysicsConsensus] Player %d FLAGGED - %d failures"), PlayerID, *FailureCount);
		}
		else
		{
			ValidationStates.Add(PlayerID, EValidationState::SUSPECT);
		}
	}
	else
	{
		// Validation passed
		if (CurrentState == EValidationState::SUSPECT)
		{
			// Clear failure count and return to trusted
			FailureCounts.Remove(PlayerID);
			FirstFailureTimestamp.Remove(PlayerID);
			ValidationStates.Add(PlayerID, EValidationState::TRUSTED);
		}
	}

	// Check if failure window has expired
	float* FirstFailure = FirstFailureTimestamp.Find(PlayerID);
	if (FirstFailure)
	{
		float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		if (CurrentTime - *FirstFailure > KickTimeWindow)
		{
			// Window expired, reset
			FailureCounts.Remove(PlayerID);
			FirstFailureTimestamp.Remove(PlayerID);
			if (CurrentState == EValidationState::SUSPECT)
			{
				ValidationStates.Add(PlayerID, EValidationState::TRUSTED);
			}
		}
	}
}

bool UPhysicsConsensus::ShouldKickPlayer(int32 PlayerID) const
{
	return GetValidationState(PlayerID) == EValidationState::KICKED;
}

void UPhysicsConsensus::ResetValidationState(int32 PlayerID)
{
	ValidationStates.Remove(PlayerID);
	FailureCounts.Remove(PlayerID);
	FirstFailureTimestamp.Remove(PlayerID);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] ResetValidationState Player %d"), PlayerID);
	}
}

// ============================================================================
// ORBITAL BODY INTEGRATION
// ============================================================================

void UPhysicsConsensus::RegisterPlayer(int32 PlayerID, AOrbitalBody* OrbitalBody)
{
	if (OrbitalBody)
	{
		PlayerOrbitalBodies.Add(PlayerID, OrbitalBody);

		// Initialize validation state
		ValidationStates.Add(PlayerID, EValidationState::TRUSTED);

		if (bEnableDebugLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] RegisterPlayer %d with OrbitalBody %s"),
				PlayerID, *OrbitalBody->GetName());
		}
	}
}

void UPhysicsConsensus::UnregisterPlayer(int32 PlayerID)
{
	PlayerOrbitalBodies.Remove(PlayerID);
	PositionHistory.Remove(PlayerID);
	ValidationVotes.Remove(PlayerID);
	ValidationStates.Remove(PlayerID);
	FailureCounts.Remove(PlayerID);
	FirstFailureTimestamp.Remove(PlayerID);
	NetworkLatencies.Remove(PlayerID);

	if (bEnableDebugLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("[PhysicsConsensus] UnregisterPlayer %d"), PlayerID);
	}
}

AOrbitalBody* UPhysicsConsensus::GetPlayerOrbitalBody(int32 PlayerID) const
{
	const TWeakObjectPtr<AOrbitalBody>* Body = PlayerOrbitalBodies.Find(PlayerID);
	return (Body && Body->IsValid()) ? Body->Get() : nullptr;
}

// ============================================================================
// TOLERANCE CALCULATIONS
// ============================================================================

float UPhysicsConsensus::CalculatePositionTolerance(float DeltaTime, int32 PlayerID) const
{
	// Base tolerance
	float Tolerance = BasePositionTolerance;

	// Add time-based tolerance (prediction accumulates error over time)
	Tolerance += TimeToleranceRate * DeltaTime;

	// Add latency tolerance
	float Latency = GetNetworkLatency(PlayerID);
	Tolerance += MaxSpeed * Latency;

	return Tolerance;
}

float UPhysicsConsensus::CalculateThrustTolerance() const
{
	// Base tolerance + percentage of max thrust
	float Tolerance = BaseThrustTolerance;
	Tolerance += MaxThrustForce * ThrustTolerancePercentage;

	return Tolerance;
}

float UPhysicsConsensus::GetNetworkLatency(int32 PlayerID) const
{
	const float* Latency = NetworkLatencies.Find(PlayerID);
	return Latency ? *Latency : 0.1f; // Default 100ms
}

// ============================================================================
// HELPER METHODS
// ============================================================================

TArray<FPositionReport>* UPhysicsConsensus::GetOrCreatePositionHistory(int32 PlayerID)
{
	TArray<FPositionReport>* History = PositionHistory.Find(PlayerID);
	if (!History)
	{
		PositionHistory.Add(PlayerID, TArray<FPositionReport>());
		History = PositionHistory.Find(PlayerID);
	}
	return History;
}

void UPhysicsConsensus::AddToHistory(TArray<FPositionReport>& History, const FPositionReport& Report)
{
	// Add to history
	History.Add(Report);

	// Limit history size (circular buffer)
	if (History.Num() > MaxHistorySize)
	{
		History.RemoveAt(0);
	}
}

float UPhysicsConsensus::CalculateTotalDistance(const TArray<FPositionReport>& History, float TimeWindow) const
{
	if (History.Num() < 2)
	{
		return 0.0f;
	}

	float CurrentTime = History.Last().Timestamp;
	float StartTime = CurrentTime - TimeWindow;

	float TotalDistance = 0.0f;
	for (int32 i = 1; i < History.Num(); i++)
	{
		if (History[i].Timestamp >= StartTime)
		{
			float Distance = FVector::Dist(History[i].Position, History[i - 1].Position);
			TotalDistance += Distance;
		}
	}

	return TotalDistance;
}

float UPhysicsConsensus::CalculateMaxPossibleDistance(const TArray<FPositionReport>& History, float TimeWindow) const
{
	// Maximum distance = max thrust acceleration for entire time window
	// d = 0.5 * a * t²
	float MaxAccel = MaxThrustForce / PlayerMass;
	float MaxDistance = 0.5f * MaxAccel * TimeWindow * TimeWindow;

	// Add current velocity contribution
	if (History.Num() > 0)
	{
		float CurrentSpeed = History.Last().Velocity.Size();
		MaxDistance += CurrentSpeed * TimeWindow;
	}

	return MaxDistance;
}

float UPhysicsConsensus::CalculateMinPossibleDistance(const TArray<FPositionReport>& History, float TimeWindow) const
{
	// Minimum distance = gravity only (no thrust)
	// This is complex to calculate accurately without knowing gravity field
	// For now, return 0 (player could theoretically stay still)
	return 0.0f;
}
