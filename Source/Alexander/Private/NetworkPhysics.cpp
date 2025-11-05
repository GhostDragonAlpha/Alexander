// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkPhysics.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Net/UnrealNetwork.h"  // For network physics replication

DEFINE_LOG_CATEGORY_STATIC(LogNetworkPhysics, Log, All);

UNetworkPhysics::UNetworkPhysics()
{
// Default settings
UpdateRate = 60.0f; // 60 Hz physics updates
InterpolationDelay = 0.1f; // 100ms interpolation delay
MaxExtrapolationTime = 0.2f; // Max 200ms extrapolation
PredictionErrorThreshold = 5.0f; // 5cm error threshold

bEnableLagCompensation = true;
bEnableInterpolation = true;
bEnableExtrapolation = true;
bEnableClientPrediction = true;

// Internal state
SequenceCounter = 0;
LastUpdateTime = 0.0f;
AveragePing = 0.0f;
AveragePredictionError = 0.0f;
ReconciliationCount = 0;
TotalSamples = 0;

// Initialize registries
RegisteredActors.Empty();
CurrentStates.Empty();
InterpolationData.Empty();
ClientPredictionEnabled.Empty();
HistoricalStates.Empty();
InputHistory.Empty();
}

void UNetworkPhysics::BeginDestroy()
{
// Cleanup all registries
RegisteredActors.Empty();
CurrentStates.Empty();
InterpolationData.Empty();
ClientPredictionEnabled.Empty();
HistoricalStates.Empty();
InputHistory.Empty();

Super::BeginDestroy();
}

// ============================================================================
// ACTOR REGISTRATION
// ============================================================================

void UNetworkPhysics::RegisterActor(AActor* Actor, ENetworkPhysicsMode Mode)
{
if (!Actor)
{
UE_LOG(LogNetworkPhysics, Warning, TEXT("Cannot register null actor"));
return;
}

if (RegisteredActors.Contains(Actor))
{
UE_LOG(LogNetworkPhysics, Warning, TEXT("Actor already registered: %s"), *Actor->GetName());
return;
}

RegisteredActors.Add(Actor, Mode);
CurrentStates.Add(Actor, ExtractPhysicsState(Actor));
InterpolationData.Add(Actor, FPhysicsInterpolationData());
ClientPredictionEnabled.Add(Actor, false);
HistoricalStates.Add(Actor, FHistoricalPhysicsStates());
InputHistory.Add(Actor, FInputHistoryData());

UE_LOG(LogNetworkPhysics, Log, TEXT("Registered actor: %s (Mode: %d)"), *Actor->GetName(), static_cast<int32>(Mode));
}

void UNetworkPhysics::UnregisterActor(AActor* Actor)
{
if (!Actor)
{
return;
}

RegisteredActors.Remove(Actor);
CurrentStates.Remove(Actor);
InterpolationData.Remove(Actor);
ClientPredictionEnabled.Remove(Actor);
HistoricalStates.Remove(Actor);
InputHistory.Remove(Actor);

UE_LOG(LogNetworkPhysics, Log, TEXT("Unregistered actor: %s"), *Actor->GetName());
}

bool UNetworkPhysics::IsActorRegistered(AActor* Actor) const
{
return Actor && RegisteredActors.Contains(Actor);
}

void UNetworkPhysics::SetPhysicsMode(AActor* Actor, ENetworkPhysicsMode Mode)
{
if (!IsActorRegistered(Actor))
{
UE_LOG(LogNetworkPhysics, Warning, TEXT("Actor not registered: %s"), *Actor->GetName());
return;
}

RegisteredActors[Actor] = Mode;
UE_LOG(LogNetworkPhysics, Log, TEXT("Set physics mode for %s: %d"), *Actor->GetName(), static_cast<int32>(Mode));
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

FNetworkPhysicsState UNetworkPhysics::GetPhysicsState(AActor* Actor)
{
if (!IsActorRegistered(Actor))
{
return FNetworkPhysicsState();
}

return CurrentStates[Actor];
}

void UNetworkPhysics::SetPhysicsState(AActor* Actor, const FNetworkPhysicsState& State)
{
if (!IsActorRegistered(Actor))
{
return;
}

CurrentStates[Actor] = State;
}

void UNetworkPhysics::ApplyPhysicsState(AActor* Actor, const FNetworkPhysicsState& State)
{
if (!IsActorRegistered(Actor))
{
return;
}

ApplyPhysicsStateInternal(Actor, State);
CurrentStates[Actor] = State;

UE_LOG(LogNetworkPhysics, Verbose, TEXT("Applied physics state to %s"), *Actor->GetName());
}

// ============================================================================
// CLIENT-SIDE PREDICTION
// ============================================================================

void UNetworkPhysics::EnableClientPrediction(AActor* Actor, bool bEnabled)
{
if (!IsActorRegistered(Actor))
{
return;
}

ClientPredictionEnabled[Actor] = bEnabled;
UE_LOG(LogNetworkPhysics, Log, TEXT("Client prediction %s for %s"), 
bEnabled ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
}

bool UNetworkPhysics::IsClientPredictionEnabled(AActor* Actor) const
{
if (!IsActorRegistered(Actor))
{
return false;
}

return ClientPredictionEnabled[Actor];
}

void UNetworkPhysics::StoreInputState(AActor* Actor, uint8 Input, float Timestamp)
{
if (!IsActorRegistered(Actor))
{
return;
}

// Store input with timestamp for replay during reconciliation
FInputHistoryData& History = InputHistory[Actor];
History.InputData.Add(Input);

// Keep last 100 inputs
if (History.InputData.Num() > 100)
{
History.InputData.RemoveAt(0);
}
}

void UNetworkPhysics::PredictMovement(AActor* Actor, float DeltaTime)
{
if (!IsActorRegistered(Actor) || !IsClientPredictionEnabled(Actor))
{
return;
}

// Get current state
FNetworkPhysicsState CurrentState = GetPhysicsState(Actor);

// Predict next state based on current velocity
CurrentState.Location += CurrentState.LinearVelocity * DeltaTime;
CurrentState.Rotation += FRotator(
CurrentState.AngularVelocity.Y * DeltaTime,
CurrentState.AngularVelocity.Z * DeltaTime,
CurrentState.AngularVelocity.X * DeltaTime
);

CurrentState.Timestamp += DeltaTime;
CurrentState.SequenceNumber = SequenceCounter++;

// Apply predicted state
ApplyPhysicsState(Actor, CurrentState);
StoreHistoricalState(Actor, CurrentState);
}

// ============================================================================
// SERVER RECONCILIATION
// ============================================================================

void UNetworkPhysics::ReconcileWithServer(AActor* Actor, const FNetworkPhysicsState& ServerState)
{
if (!IsActorRegistered(Actor))
{
return;
}

// Check if reconciliation is needed
if (!ShouldReconcile(Actor, ServerState))
{
return;
}

// Calculate prediction error
FNetworkPhysicsState ClientState = GetPhysicsState(Actor);
float Error = FVector::Dist(ClientState.Location, ServerState.Location);

// Update statistics
AveragePredictionError = (AveragePredictionError * TotalSamples + Error) / (TotalSamples + 1);
TotalSamples++;
ReconciliationCount++;

if (Error > PredictionErrorThreshold)
{
UE_LOG(LogNetworkPhysics, Warning, TEXT("Large prediction error for %s: %.2f cm"), 
*Actor->GetName(), Error);

// Apply correction
CorrectPredictionError(Actor, ServerState);
}
else
{
// Smooth correction
SmoothCorrection(Actor, ServerState);
}
}

void UNetworkPhysics::CorrectPredictionError(AActor* Actor, const FNetworkPhysicsState& CorrectState)
{
if (!IsActorRegistered(Actor))
{
return;
}

// Immediate correction for large errors
ApplyPhysicsState(Actor, CorrectState);

// Replay stored inputs if client prediction is enabled
if (IsClientPredictionEnabled(Actor))
{
// TODO: Implement input replay
UE_LOG(LogNetworkPhysics, Verbose, TEXT("Correcting prediction error for %s"), *Actor->GetName());
}
}

float UNetworkPhysics::GetPredictionError(AActor* Actor) const
{
if (!IsActorRegistered(Actor))
{
return 0.0f;
}

// Return distance between current and last server state
// This is a simplified implementation
return 0.0f;
}

// ============================================================================
// INTERPOLATION
// ============================================================================

void UNetworkPhysics::EnableInterpolation(AActor* Actor, bool bEnabled)
{
if (!IsActorRegistered(Actor))
{
return;
}

FPhysicsInterpolationData& InterpData = InterpolationData[Actor];
InterpData.bIsInterpolating = bEnabled;

UE_LOG(LogNetworkPhysics, Log, TEXT("Interpolation %s for %s"), 
bEnabled ? TEXT("enabled") : TEXT("disabled"), *Actor->GetName());
}

bool UNetworkPhysics::IsInterpolationEnabled(AActor* Actor) const
{
if (!IsActorRegistered(Actor))
{
return false;
}

return InterpolationData[Actor].bIsInterpolating;
}

void UNetworkPhysics::StartInterpolation(AActor* Actor, const FNetworkPhysicsState& TargetState, float Duration)
{
if (!IsActorRegistered(Actor))
{
return;
}

FPhysicsInterpolationData& InterpData = InterpolationData[Actor];
InterpData.StartState = GetPhysicsState(Actor);
InterpData.TargetState = TargetState;
InterpData.InterpolationTime = 0.0f;
InterpData.InterpolationDuration = Duration;
InterpData.bIsInterpolating = true;

UE_LOG(LogNetworkPhysics, Verbose, TEXT("Started interpolation for %s (Duration: %.3f)"), 
*Actor->GetName(), Duration);
}

void UNetworkPhysics::UpdateInterpolation(AActor* Actor, float DeltaTime)
{
if (!IsActorRegistered(Actor))
{
return;
}

FPhysicsInterpolationData& InterpData = InterpolationData[Actor];
if (!InterpData.bIsInterpolating)
{
return;
}

InterpData.InterpolationTime += DeltaTime;
float Alpha = CalculateInterpolationAlpha(
InterpData.InterpolationTime, 
0.0f, 
InterpData.InterpolationDuration
);

// Interpolate state
FNetworkPhysicsState InterpolatedState;
InterpolatedState.Location = FMath::Lerp(
InterpData.StartState.Location, 
InterpData.TargetState.Location, 
Alpha
);
InterpolatedState.Rotation = FMath::Lerp(
InterpData.StartState.Rotation, 
InterpData.TargetState.Rotation, 
Alpha
);
InterpolatedState.LinearVelocity = FMath::Lerp(
InterpData.StartState.LinearVelocity, 
InterpData.TargetState.LinearVelocity, 
Alpha
);
InterpolatedState.AngularVelocity = FMath::Lerp(
InterpData.StartState.AngularVelocity, 
InterpData.TargetState.AngularVelocity, 
Alpha
);

ApplyPhysicsStateInternal(Actor, InterpolatedState);

// Check if interpolation is complete
if (Alpha >= 1.0f)
{
StopInterpolation(Actor);
}
}

void UNetworkPhysics::StopInterpolation(AActor* Actor)
{
if (!IsActorRegistered(Actor))
{
return;
}

FPhysicsInterpolationData& InterpData = InterpolationData[Actor];
InterpData.bIsInterpolating = false;
InterpData.InterpolationTime = 0.0f;

UE_LOG(LogNetworkPhysics, Verbose, TEXT("Stopped interpolation for %s"), *Actor->GetName());
}

// ============================================================================
// EXTRAPOLATION
// ============================================================================

void UNetworkPhysics::EnableExtrapolation(AActor* Actor, bool bEnabled)
{
// Extrapolation is controlled globally via bEnableExtrapolation
bEnableExtrapolation = bEnabled;
}

bool UNetworkPhysics::IsExtrapolationEnabled(AActor* Actor) const
{
return bEnableExtrapolation;
}

FNetworkPhysicsState UNetworkPhysics::ExtrapolateState(AActor* Actor, float FutureTime)
{
if (!IsActorRegistered(Actor))
{
return FNetworkPhysicsState();
}

FNetworkPhysicsState CurrentState = GetPhysicsState(Actor);
FNetworkPhysicsState ExtrapolatedState = CurrentState;

// Clamp extrapolation time
float ClampedTime = FMath::Min(FutureTime, MaxExtrapolationTime);

// Extrapolate position and rotation
ExtrapolatedState.Location += CurrentState.LinearVelocity * ClampedTime;
ExtrapolatedState.Rotation += FRotator(
CurrentState.AngularVelocity.Y * ClampedTime,
CurrentState.AngularVelocity.Z * ClampedTime,
CurrentState.AngularVelocity.X * ClampedTime
);

return ExtrapolatedState;
}

// ============================================================================
// LAG COMPENSATION
// ============================================================================

void UNetworkPhysics::EnableLagCompensation(bool bEnabled)
{
bEnableLagCompensation = bEnabled;
UE_LOG(LogNetworkPhysics, Log, TEXT("Lag compensation %s"), 
bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UNetworkPhysics::IsLagCompensationEnabled() const
{
return bEnableLagCompensation;
}

FNetworkPhysicsState UNetworkPhysics::GetHistoricalState(AActor* Actor, float PastTime)
{
if (!IsActorRegistered(Actor))
{
return FNetworkPhysicsState();
}

FHistoricalPhysicsStates& History = HistoricalStates[Actor];
if (History.States.Num() == 0)
{
return GetPhysicsState(Actor);
}

// Find state closest to requested time
FNetworkPhysicsState BestState = History.States[0];
float BestTimeDiff = FMath::Abs(BestState.Timestamp - PastTime);

for (const FNetworkPhysicsState& State : History.States)
{
float TimeDiff = FMath::Abs(State.Timestamp - PastTime);
if (TimeDiff < BestTimeDiff)
{
BestState = State;
BestTimeDiff = TimeDiff;
}
}

return BestState;
}

void UNetworkPhysics::StoreHistoricalState(AActor* Actor, const FNetworkPhysicsState& State)
{
if (!IsActorRegistered(Actor))
{
return;
}

FHistoricalPhysicsStates& History = HistoricalStates[Actor];
History.States.Add(State);

// Keep last 100 states (about 1.6 seconds at 60Hz)
if (History.States.Num() > 100)
{
History.States.RemoveAt(0);
}
}

void UNetworkPhysics::CleanupOldStates(float MaxAge)
{
float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

for (auto& Pair : HistoricalStates)
{
FHistoricalPhysicsStates& History = Pair.Value;
History.States.RemoveAll([CurrentTime, MaxAge](const FNetworkPhysicsState& State)
{
return (CurrentTime - State.Timestamp) > MaxAge;
});
}
}

// ============================================================================
// OPTIMIZATION
// ============================================================================

void UNetworkPhysics::SetUpdateRate(float Rate)
{
UpdateRate = FMath::Max(1.0f, Rate);
UE_LOG(LogNetworkPhysics, Log, TEXT("Update rate set to %.2f Hz"), UpdateRate);
}

float UNetworkPhysics::GetUpdateRate() const
{
return UpdateRate;
}

void UNetworkPhysics::SetInterpolationDelay(float Delay)
{
InterpolationDelay = FMath::Max(0.0f, Delay);
UE_LOG(LogNetworkPhysics, Log, TEXT("Interpolation delay set to %.3f seconds"), InterpolationDelay);
}

float UNetworkPhysics::GetInterpolationDelay() const
{
return InterpolationDelay;
}

void UNetworkPhysics::OptimizePhysics()
{
// Cleanup old states
CleanupOldStates(2.0f); // Keep last 2 seconds

UE_LOG(LogNetworkPhysics, Log, TEXT("Optimized network physics (Actors: %d)"), RegisteredActors.Num());
}

// ============================================================================
// UPDATES
// ============================================================================

void UNetworkPhysics::ServerUpdate(float DeltaTime)
{
// Server authoritative update
for (auto& Pair : RegisteredActors)
{
AActor* Actor = Pair.Key;
ENetworkPhysicsMode Mode = Pair.Value;

if (Mode == ENetworkPhysicsMode::Authority)
{
// Extract and store current state
FNetworkPhysicsState State = ExtractPhysicsState(Actor);
State.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
State.SequenceNumber = SequenceCounter++;

SetPhysicsState(Actor, State);
StoreHistoricalState(Actor, State);
}
}

LastUpdateTime += DeltaTime;
}

void UNetworkPhysics::ClientUpdate(float DeltaTime)
{
// Client update with prediction/interpolation
for (auto& Pair : RegisteredActors)
{
AActor* Actor = Pair.Key;
ENetworkPhysicsMode Mode = Pair.Value;

if (Mode == ENetworkPhysicsMode::Autonomous && IsClientPredictionEnabled(Actor))
{
// Client-side prediction
PredictMovement(Actor, DeltaTime);
}
else if (Mode == ENetworkPhysicsMode::Simulated && bEnableInterpolation)
{
// Interpolate simulated proxies
UpdateInterpolation(Actor, DeltaTime);
}
}

LastUpdateTime += DeltaTime;
}

void UNetworkPhysics::UpdatePhysics(float DeltaTime)
{
UWorld* World = GetWorld();
if (!World)
{
return;
}

// Determine if server or client
if (World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer)
{
ServerUpdate(DeltaTime);
}
else
{
ClientUpdate(DeltaTime);
}

// Periodic optimization
static float OptimizationTimer = 0.0f;
OptimizationTimer += DeltaTime;
if (OptimizationTimer >= 10.0f)
{
OptimizePhysics();
OptimizationTimer = 0.0f;
}
}

// ============================================================================
// STATISTICS
// ============================================================================

float UNetworkPhysics::GetAveragePing() const
{
return AveragePing;
}

float UNetworkPhysics::GetAveragePredictionError() const
{
return AveragePredictionError;
}

int32 UNetworkPhysics::GetReconciliationCount() const
{
return ReconciliationCount;
}

void UNetworkPhysics::ResetStatistics()
{
AveragePing = 0.0f;
AveragePredictionError = 0.0f;
ReconciliationCount = 0;
TotalSamples = 0;

UE_LOG(LogNetworkPhysics, Log, TEXT("Statistics reset"));
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

FNetworkPhysicsState UNetworkPhysics::ExtractPhysicsState(AActor* Actor)
{
FNetworkPhysicsState State;

if (!Actor)
{
return State;
}

State.Location = Actor->GetActorLocation();
State.Rotation = Actor->GetActorRotation();

// Try to get velocity from primitive component
UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
if (PrimComp && PrimComp->IsSimulatingPhysics())
{
State.LinearVelocity = PrimComp->GetPhysicsLinearVelocity();
State.AngularVelocity = PrimComp->GetPhysicsAngularVelocityInDegrees();
State.bSimulatingPhysics = true;
}
else
{
State.LinearVelocity = Actor->GetVelocity();
State.AngularVelocity = FVector::ZeroVector;
State.bSimulatingPhysics = false;
}

State.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

return State;
}

void UNetworkPhysics::ApplyPhysicsStateInternal(AActor* Actor, const FNetworkPhysicsState& State)
{
if (!Actor)
{
return;
}

Actor->SetActorLocation(State.Location);
Actor->SetActorRotation(State.Rotation);

UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
if (PrimComp && State.bSimulatingPhysics)
{
PrimComp->SetPhysicsLinearVelocity(State.LinearVelocity);
PrimComp->SetPhysicsAngularVelocityInDegrees(State.AngularVelocity);
}
}

void UNetworkPhysics::SmoothCorrection(AActor* Actor, const FNetworkPhysicsState& TargetState)
{
if (!Actor || !bEnableInterpolation)
{
ApplyPhysicsState(Actor, TargetState);
return;
}

// Start smooth interpolation to target state
StartInterpolation(Actor, TargetState, InterpolationDelay);
}

float UNetworkPhysics::CalculateInterpolationAlpha(float CurrentTime, float StartTime, float Duration) const
{
if (Duration <= 0.0f)
{
return 1.0f;
}

float Alpha = (CurrentTime - StartTime) / Duration;
return FMath::Clamp(Alpha, 0.0f, 1.0f);
}

bool UNetworkPhysics::ShouldReconcile(AActor* Actor, const FNetworkPhysicsState& ServerState) const
{
if (!IsActorRegistered(Actor))
{
return false;
}

FNetworkPhysicsState ClientState = CurrentStates[Actor];
float PositionError = FVector::Dist(ClientState.Location, ServerState.Location);

return PositionError > (PredictionErrorThreshold * 0.1f); // 10% of threshold
}
