// Copyright Epic Games, Inc. All Rights Reserved.

#include "RelativitySystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

URelativitySystem::URelativitySystem()
{
	UniversalTime = 0.0;
	CurrentTimeWarpFactor = 1.0f;
	bEnableRelativisticEffects = true;
	bEnableTimeDebt = true;
	RelativityScaleFactor = 1.0f;
	TimeWarpWarningThreshold = 10.0f;
	AchievementTimeDebtThreshold = 168.0f;
}

void URelativitySystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UniversalTime = 0.0;
	CurrentTimeWarpFactor = 1.0f;
	PlayerTimeData.Empty();

	UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Initialized"));
}

void URelativitySystem::Deinitialize()
{
	PlayerTimeData.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Deinitialized"));
}

void URelativitySystem::Tick(float DeltaTime)
{
	if (!bEnableRelativisticEffects)
	{
		// Still advance universal time even if relativity is disabled
		AdvanceUniversalTime(DeltaTime);
		return;
	}

	// Advance universal time
	AdvanceUniversalTime(DeltaTime);

	// Update each player's biological time based on their time dilation
	for (auto& Pair : PlayerTimeData)
	{
		if (Pair.Key && Pair.Key->GetPawn())
		{
			UpdatePlayerAge(Pair.Key, DeltaTime);
		}
	}
}

// ============================================================================
// TIME DILATION CALCULATIONS
// ============================================================================

float URelativitySystem::CalculateVelocityTimeDilation(float VelocityMetersPerSec) const
{
	if (!bEnableRelativisticEffects || VelocityMetersPerSec <= 0.0f)
	{
		return 1.0f;
	}

	// Special Relativity: t' = t * sqrt(1 - v²/c²)
	// This is the Lorentz factor (gamma)

	double v = static_cast<double>(VelocityMetersPerSec);
	double c = SPEED_OF_LIGHT;

	// Calculate v²/c²
	double vSquaredOverCSquared = (v * v) / (c * c);

	// Clamp to prevent going faster than light (numerical stability)
	vSquaredOverCSquared = FMath::Min(vSquaredOverCSquared, 0.9999);

	// Calculate sqrt(1 - v²/c²)
	double dilationFactor = FMath::Sqrt(1.0 - vSquaredOverCSquared);

	// Apply scale factor for gameplay tuning
	// Interpolate between 1.0 (no dilation) and calculated value based on scale
	double scaledDilation = FMath::Lerp(1.0, dilationFactor, static_cast<double>(RelativityScaleFactor));

	return static_cast<float>(scaledDilation);
}

float URelativitySystem::CalculateGravitationalTimeDilation(float Altitude, float BodyMass, float BodyRadius) const
{
	if (!bEnableRelativisticEffects || Altitude <= 0.0f || BodyMass <= 0.0f)
	{
		return 1.0f;
	}

	// General Relativity: t' = t * sqrt(1 - (2GM)/(rc²))
	// Where r is distance from center of mass

	double G = GRAV_CONSTANT;
	double M = static_cast<double>(BodyMass);
	double r = static_cast<double>(Altitude);
	double c = SPEED_OF_LIGHT;

	// Calculate Schwarzschild radius: rs = 2GM/c²
	double schwarzschildRadius = (2.0 * G * M) / (c * c);

	// Calculate (2GM)/(rc²)
	double gravityFactor = schwarzschildRadius / r;

	// Clamp to prevent going inside event horizon (numerical stability)
	gravityFactor = FMath::Min(gravityFactor, 0.9999);

	// Calculate sqrt(1 - (2GM)/(rc²))
	double dilationFactor = FMath::Sqrt(1.0 - gravityFactor);

	// Apply scale factor for gameplay tuning
	double scaledDilation = FMath::Lerp(1.0, dilationFactor, static_cast<double>(RelativityScaleFactor));

	return static_cast<float>(scaledDilation);
}

float URelativitySystem::GetCombinedTimeDilation(AActor* Actor) const
{
	if (!Actor || !bEnableRelativisticEffects)
	{
		return 1.0f;
	}

	// Get velocity-based dilation
	FVector Velocity = Actor->GetVelocity();
	float Speed = Velocity.Size();
	float VelocityDilation = CalculateVelocityTimeDilation(Speed);

	// Get gravitational dilation
	float BodyMass, BodyRadius, Distance;
	FindNearestMassiveBody(Actor, BodyMass, BodyRadius, Distance);
	float GravityDilation = CalculateGravitationalTimeDilation(Distance, BodyMass, BodyRadius);

	// Combined dilation: multiply the factors
	// (Both effects slow time, so we multiply)
	float CombinedDilation = VelocityDilation * GravityDilation;

	return CombinedDilation;
}

float URelativitySystem::GetEffectiveTimeDilation(AActor* Actor, float TimeWarpFactor) const
{
	if (!Actor)
	{
		return TimeWarpFactor;
	}

	// Get natural relativistic dilation
	float RelativisticDilation = GetCombinedTimeDilation(Actor);

	// When time warp is active:
	// - Universe advances at TimeWarpFactor rate
	// - Player is "protected" by relativistic effects
	// - Effective player time = TimeWarpFactor * RelativisticDilation

	// This creates the core mechanic:
	// Higher time warp = more universe time passes
	// But relativistic effects partially protect the player

	float EffectiveDilation = TimeWarpFactor * RelativisticDilation;

	return EffectiveDilation;
}

// ============================================================================
// PLAYER TIME TRACKING
// ============================================================================

void URelativitySystem::RegisterPlayer(APlayerController* Player, float StartingAge)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelativitySystem: Cannot register null player"));
		return;
	}

	FPlayerTimeData NewData;
	NewData.BiologicalAge = StartingAge;
	NewData.TotalRealTimeSeconds = 0.0;
	NewData.TotalWorldTimeSeconds = 0.0;
	NewData.TimeDebt = 0.0;
	NewData.CumulativeVelocityDilation = 0.0;
	NewData.CumulativeGravityDilation = 0.0;
	NewData.SessionStartTime = UniversalTime;

	PlayerTimeData.Add(Player, NewData);

	UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Registered player %s with starting age %.1f"),
		*Player->GetName(), StartingAge);
}

void URelativitySystem::UnregisterPlayer(APlayerController* Player)
{
	if (!Player)
	{
		return;
	}

	PlayerTimeData.Remove(Player);

	UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Unregistered player %s"), *Player->GetName());
}

void URelativitySystem::UpdatePlayerAge(APlayerController* Player, float RealDeltaTime)
{
	if (!Player || !bEnableRelativisticEffects)
	{
		return;
	}

	FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelativitySystem: Player %s not registered for time tracking"),
			*Player->GetName());
		return;
	}

	APawn* Pawn = Player->GetPawn();
	if (!Pawn)
	{
		return;
	}

	// Get current time dilation for this player
	float TimeDilation = GetCombinedTimeDilation(Pawn);

	// Calculate how much biological time passes for the player
	// Player experiences time slower if dilation < 1.0
	double PlayerTimeDelta = static_cast<double>(RealDeltaTime) * static_cast<double>(TimeDilation);

	// Universe time advances at time warp rate
	double UniverseTimeDelta = static_cast<double>(RealDeltaTime) * static_cast<double>(CurrentTimeWarpFactor);

	// Update tracking
	Data->TotalRealTimeSeconds += static_cast<double>(RealDeltaTime);
	Data->TotalWorldTimeSeconds += PlayerTimeDelta;

	// Update biological age (convert seconds to years: 1 year = 31,536,000 seconds)
	const double SecondsPerYear = 31536000.0;
	Data->BiologicalAge += static_cast<float>(PlayerTimeDelta / SecondsPerYear);

	// Update velocity and dilation tracking
	FVector Velocity = Pawn->GetVelocity();
	Data->CurrentVelocity = Velocity.Size();
	Data->CumulativeVelocityDilation += static_cast<double>(CalculateVelocityTimeDilation(Data->CurrentVelocity)) * RealDeltaTime;

	// Update altitude and gravity dilation tracking
	float BodyMass, BodyRadius, Distance;
	FindNearestMassiveBody(Pawn, BodyMass, BodyRadius, Distance);
	Data->CurrentAltitude = Distance;
	Data->CumulativeGravityDilation += static_cast<double>(CalculateGravitationalTimeDilation(Distance, BodyMass, BodyRadius)) * RealDeltaTime;

	// Update time debt if enabled
	if (bEnableTimeDebt)
	{
		UpdateTimeDebt(Player, UniverseTimeDelta, PlayerTimeDelta);
	}

	// Check for extreme dilation and broadcast events
	if (TimeDilation < 0.5f)
	{
		OnExtremeDilationEntered.Broadcast(Player, TimeDilation, TEXT("Extreme velocity or gravitational field"));
	}
}

float URelativitySystem::GetPlayerBiologicalAge(APlayerController* Player) const
{
	if (!Player)
	{
		return 0.0f;
	}

	const FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	return Data ? Data->BiologicalAge : 25.0f;
}

double URelativitySystem::GetTimeDebt(APlayerController* Player) const
{
	if (!Player)
	{
		return 0.0;
	}

	const FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	return Data ? Data->TimeDebt : 0.0;
}

FString URelativitySystem::GetFormattedTimeDebt(APlayerController* Player) const
{
	double TimeDebt = GetTimeDebt(Player);
	return FormatTimeDuration(TimeDebt);
}

FPlayerTimeData URelativitySystem::GetPlayerTimeData(APlayerController* Player) const
{
	if (!Player)
	{
		return FPlayerTimeData();
	}

	const FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	return Data ? *Data : FPlayerTimeData();
}

// ============================================================================
// UNIVERSAL TIME
// ============================================================================

FString URelativitySystem::GetFormattedUniversalTime() const
{
	// Convert seconds to years, days, hours, minutes, seconds
	const double SecondsPerYear = 31536000.0; // 365 days
	const double SecondsPerDay = 86400.0;
	const double SecondsPerHour = 3600.0;
	const double SecondsPerMinute = 60.0;

	int32 Years = FMath::FloorToInt(UniversalTime / SecondsPerYear);
	double Remainder = FMath::Fmod(UniversalTime, SecondsPerYear);

	int32 Days = FMath::FloorToInt(Remainder / SecondsPerDay);
	Remainder = FMath::Fmod(Remainder, SecondsPerDay);

	int32 Hours = FMath::FloorToInt(Remainder / SecondsPerHour);
	Remainder = FMath::Fmod(Remainder, SecondsPerHour);

	int32 Minutes = FMath::FloorToInt(Remainder / SecondsPerMinute);
	int32 Seconds = FMath::FloorToInt(FMath::Fmod(Remainder, SecondsPerMinute));

	return FString::Printf(TEXT("Year %d, Day %d, %02d:%02d:%02d"),
		Years + 1, Days + 1, Hours, Minutes, Seconds);
}

void URelativitySystem::AdvanceUniversalTime(float DeltaSeconds)
{
	UniversalTime += static_cast<double>(DeltaSeconds) * static_cast<double>(CurrentTimeWarpFactor);
}

// ============================================================================
// TIME WARP INTERACTION
// ============================================================================

void URelativitySystem::ApplyTimeWarpFactor(float Factor)
{
	float OldFactor = CurrentTimeWarpFactor;
	CurrentTimeWarpFactor = FMath::Max(1.0f, Factor);

	if (OldFactor < 1.1f && CurrentTimeWarpFactor >= 1.1f)
	{
		// Activating time warp
		OnTimeWarpActivated.Broadcast(CurrentTimeWarpFactor);
		UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Time warp activated at %.1fx"), CurrentTimeWarpFactor);
	}
	else if (OldFactor >= 1.1f && CurrentTimeWarpFactor < 1.1f)
	{
		// Deactivating time warp
		OnTimeWarpDeactivated.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Time warp deactivated"));
	}
}

FTimeWarpPrediction URelativitySystem::PredictTimeWarpConsequences(float WarpFactor, float DurationSeconds, APlayerController* Player) const
{
	FTimeWarpPrediction Prediction;
	Prediction.TimeWarpFactor = WarpFactor;
	Prediction.RealDurationSeconds = DurationSeconds;

	if (!Player || !bEnableRelativisticEffects)
	{
		Prediction.UniverseTimeAdvancement = static_cast<double>(DurationSeconds);
		Prediction.PlayerBiologicalTimeAdvancement = static_cast<double>(DurationSeconds);
		Prediction.TimeDebtIncrease = 0.0;
		Prediction.WarningLevel = 0;
		Prediction.FormattedTimeDebt = TEXT("0 seconds");
		return Prediction;
	}

	// Get player's current time dilation
	APawn* Pawn = Player->GetPawn();
	float CurrentDilation = Pawn ? GetCombinedTimeDilation(Pawn) : 1.0f;

	// Calculate universe time advancement
	Prediction.UniverseTimeAdvancement = static_cast<double>(DurationSeconds) * static_cast<double>(WarpFactor);

	// Calculate player biological time advancement
	// Player experiences time slower due to relativistic protection
	Prediction.PlayerBiologicalTimeAdvancement = static_cast<double>(DurationSeconds) * static_cast<double>(CurrentDilation);

	// Calculate time debt increase
	Prediction.TimeDebtIncrease = Prediction.UniverseTimeAdvancement - Prediction.PlayerBiologicalTimeAdvancement;

	// Format time debt
	Prediction.FormattedTimeDebt = FormatTimeDuration(Prediction.TimeDebtIncrease);

	// Calculate warning level
	Prediction.WarningLevel = CalculateWarningLevel(Prediction.TimeDebtIncrease);

	return Prediction;
}

bool URelativitySystem::ShouldWarnAboutTimeWarp(float WarpFactor, float DurationSeconds) const
{
	if (WarpFactor < TimeWarpWarningThreshold)
	{
		return false;
	}

	// Calculate potential time debt
	double UniverseAdvancement = static_cast<double>(DurationSeconds) * static_cast<double>(WarpFactor);

	// Warn if universe will advance more than 1 hour
	return UniverseAdvancement > 3600.0;
}

// ============================================================================
// GAMEPLAY CONSEQUENCES
// ============================================================================

bool URelativitySystem::HasSignificantTimeDebt(APlayerController* Player, float ThresholdHours) const
{
	if (!Player || !bEnableTimeDebt)
	{
		return false;
	}

	double TimeDebt = GetTimeDebt(Player);
	double ThresholdSeconds = static_cast<double>(ThresholdHours) * 3600.0;

	return TimeDebt >= ThresholdSeconds;
}

bool URelativitySystem::IsExperiencingExtremeDilation(APlayerController* Player, float Threshold) const
{
	if (!Player || !bEnableRelativisticEffects)
	{
		return false;
	}

	APawn* Pawn = Player->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	float Dilation = GetCombinedTimeDilation(Pawn);
	return Dilation < Threshold;
}

FLinearColor URelativitySystem::GetTimeDilationColor(float DilationFactor) const
{
	// Green = normal (1.0)
	// Yellow = moderate (0.7-0.9)
	// Orange = high (0.5-0.7)
	// Red = extreme (< 0.5)

	if (DilationFactor >= 0.9f)
	{
		return FLinearColor::Green;
	}
	else if (DilationFactor >= 0.7f)
	{
		return FLinearColor::Yellow;
	}
	else if (DilationFactor >= 0.5f)
	{
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	}
	else
	{
		return FLinearColor::Red;
	}
}

int32 URelativitySystem::GetTimeDebtSeverity(APlayerController* Player) const
{
	if (!Player || !bEnableTimeDebt)
	{
		return 0;
	}

	double TimeDebt = GetTimeDebt(Player);
	double Hours = TimeDebt / 3600.0;

	if (Hours < 1.0)
	{
		return 0; // None
	}
	else if (Hours < 24.0)
	{
		return 1; // Low (< 1 day)
	}
	else if (Hours < 168.0)
	{
		return 2; // Moderate (< 1 week)
	}
	else
	{
		return 3; // High (>= 1 week)
	}
}

// ============================================================================
// NETWORK REPLICATION
// ============================================================================

void URelativitySystem::SyncPlayerOnJoin(APlayerController* Player)
{
	if (!Player)
	{
		return;
	}

	// Register player with current universal time
	RegisterPlayer(Player);

	// Set their session start time to now
	FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	if (Data)
	{
		Data->SessionStartTime = UniversalTime;
	}

	UE_LOG(LogTemp, Log, TEXT("RelativitySystem: Synced player %s at universal time %.2f"),
		*Player->GetName(), UniversalTime);
}

TArray<APlayerController*> URelativitySystem::GetTrackedPlayers() const
{
	TArray<APlayerController*> Players;
	PlayerTimeData.GetKeys(Players);
	return Players;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void URelativitySystem::FindNearestMassiveBody(AActor* Actor, float& OutMass, float& OutRadius, float& OutDistance) const
{
	// Default values (Earth-like body at reasonable distance)
	OutMass = 5.972e24f; // Earth mass in kg
	OutRadius = 6371000.0f; // Earth radius in meters
	OutDistance = 7000000.0f; // ~700km altitude

	if (!Actor)
	{
		return;
	}

	// TODO: In a full implementation, this would:
	// 1. Query all planetary bodies in the scene
	// 2. Find the nearest one to the actor
	// 3. Return its mass, radius, and distance
	// For now, we use reasonable defaults

	// Simple approximation: distance from world origin
	FVector ActorLocation = Actor->GetActorLocation();
	OutDistance = FMath::Max(ActorLocation.Size(), OutRadius + 100000.0f); // At least 100km above surface
}

FString URelativitySystem::FormatTimeDuration(double Seconds) const
{
	if (Seconds < 0.001)
	{
		return TEXT("0 seconds");
	}

	const double SecondsPerYear = 31536000.0;
	const double SecondsPerDay = 86400.0;
	const double SecondsPerHour = 3600.0;
	const double SecondsPerMinute = 60.0;

	TArray<FString> Parts;

	// Years
	if (Seconds >= SecondsPerYear)
	{
		int32 Years = FMath::FloorToInt(Seconds / SecondsPerYear);
		Parts.Add(FString::Printf(TEXT("%d year%s"), Years, Years > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerYear);
	}

	// Days
	if (Seconds >= SecondsPerDay)
	{
		int32 Days = FMath::FloorToInt(Seconds / SecondsPerDay);
		Parts.Add(FString::Printf(TEXT("%d day%s"), Days, Days > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerDay);
	}

	// Hours
	if (Seconds >= SecondsPerHour)
	{
		int32 Hours = FMath::FloorToInt(Seconds / SecondsPerHour);
		Parts.Add(FString::Printf(TEXT("%d hour%s"), Hours, Hours > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerHour);
	}

	// Minutes
	if (Seconds >= SecondsPerMinute)
	{
		int32 Minutes = FMath::FloorToInt(Seconds / SecondsPerMinute);
		Parts.Add(FString::Printf(TEXT("%d minute%s"), Minutes, Minutes > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerMinute);
	}

	// Seconds
	if (Seconds >= 1.0 || Parts.Num() == 0)
	{
		int32 Secs = FMath::FloorToInt(Seconds);
		Parts.Add(FString::Printf(TEXT("%d second%s"), Secs, Secs != 1 ? TEXT("s") : TEXT("")));
	}

	// Join parts (take first 2-3 for readability)
	int32 MaxParts = FMath::Min(3, Parts.Num());
	FString Result;
	for (int32 i = 0; i < MaxParts; ++i)
	{
		if (i > 0)
		{
			if (i == MaxParts - 1)
			{
				Result += TEXT(" and ");
			}
			else
			{
				Result += TEXT(", ");
			}
		}
		Result += Parts[i];
	}

	return Result;
}

int32 URelativitySystem::CalculateWarningLevel(double TimeDebtIncrease) const
{
	// 0 = safe (< 1 hour)
	// 1 = moderate (1-24 hours)
	// 2 = high (1-7 days)
	// 3 = extreme (> 7 days)

	const double SecondsPerHour = 3600.0;
	const double SecondsPerDay = 86400.0;
	const double SecondsPerWeek = 604800.0;

	if (TimeDebtIncrease < SecondsPerHour)
	{
		return 0;
	}
	else if (TimeDebtIncrease < SecondsPerDay)
	{
		return 1;
	}
	else if (TimeDebtIncrease < SecondsPerWeek)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

void URelativitySystem::UpdateTimeDebt(APlayerController* Player, double UniverseTimeDelta, double PlayerTimeDelta)
{
	FPlayerTimeData* Data = PlayerTimeData.Find(Player);
	if (!Data)
	{
		return;
	}

	double OldTimeDebt = Data->TimeDebt;

	// Time debt = how much more time passed in universe vs for player
	double TimeDebtIncrease = UniverseTimeDelta - PlayerTimeDelta;
	Data->TimeDebt += TimeDebtIncrease;

	// Broadcast event if time debt changed significantly (> 1 minute)
	if (FMath::Abs(Data->TimeDebt - OldTimeDebt) > 60.0)
	{
		OnTimeDebtChanged.Broadcast(Player, Data->TimeDebt);
	}
}
