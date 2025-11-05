// Copyright Epic Games, Inc. All Rights Reserved.

#include "LODTransitionManager.h"
#include "DrawDebugHelpers.h"

ULODTransitionManager::ULODTransitionManager()
{
}

void ULODTransitionManager::Initialize(const FLODTransitionConfig& InConfig)
{
	Config = InConfig;
	ActiveTransitions.Empty();

	UE_LOG(LogTemp, Log, TEXT("LODTransitionManager initialized: Geomorphing=%s, Duration=%.2fs"),
		Config.bEnableGeomorphing ? TEXT("Enabled") : TEXT("Disabled"),
		Config.TransitionDuration);
}

void ULODTransitionManager::Shutdown()
{
	ActiveTransitions.Empty();
	UE_LOG(LogTemp, Log, TEXT("LODTransitionManager shutdown complete"));
}

bool ULODTransitionManager::StartTransition(
	FVector2D TilePosition,
	int32 OldLOD,
	int32 NewLOD,
	const FTerrainTileData& OldTileData,
	const FTerrainTileData& NewTileData,
	FVector2D ViewerPosition)
{
	// Check if we're at max concurrent transitions
	if (ActiveTransitions.Num() >= Config.MaxConcurrentTransitions)
	{
		UE_LOG(LogTemp, Warning, TEXT("Max concurrent LOD transitions reached (%d)"),
			Config.MaxConcurrentTransitions);
		return false;
	}

	// Don't transition if LOD levels are the same
	if (OldLOD == NewLOD)
	{
		return false;
	}

	// Create transition
	FLODTransition Transition;
	Transition.TilePosition = TilePosition;
	Transition.OldLODLevel = OldLOD;
	Transition.NewLODLevel = NewLOD;
	Transition.OldTileData = OldTileData;
	Transition.NewTileData = NewTileData;
	Transition.TransitionDuration = Config.TransitionDuration;
	Transition.ElapsedTime = 0.0f;
	Transition.TransitionProgress = 0.0f;
	Transition.FadeDistance = Config.FadeDistanceRange;
	Transition.CurrentDistance = FVector2D::Distance(TilePosition, ViewerPosition);

	// Set state based on configuration
	if (Config.bEnableGeomorphing)
	{
		Transition.State = ELODTransitionState::Geomorphing;

		// Pre-allocate geomorphed data arrays
		int32 VertexCount = FMath::Max(OldTileData.Vertices.Num(), NewTileData.Vertices.Num());
		Transition.GeomorphedVertices.Reserve(VertexCount);
		Transition.GeomorphedNormals.Reserve(VertexCount);
	}
	else
	{
		Transition.State = ELODTransitionState::FadingIn;
	}

	// Add to active transitions
	FString Key = GetTransitionKey(TilePosition);
	ActiveTransitions.Add(Key, Transition);

	UE_LOG(LogTemp, Verbose, TEXT("Started LOD transition at (%.1f, %.1f): LOD %d -> %d"),
		TilePosition.X, TilePosition.Y, OldLOD, NewLOD);

	return true;
}

void ULODTransitionManager::CancelTransition(FVector2D TilePosition)
{
	FString Key = GetTransitionKey(TilePosition);
	ActiveTransitions.Remove(Key);
}

bool ULODTransitionManager::IsTransitioning(FVector2D TilePosition) const
{
	FString Key = GetTransitionKey(TilePosition);
	return ActiveTransitions.Contains(Key);
}

float ULODTransitionManager::GetTransitionProgress(FVector2D TilePosition) const
{
	FString Key = GetTransitionKey(TilePosition);
	const FLODTransition* Transition = ActiveTransitions.Find(Key);

	return Transition ? Transition->TransitionProgress : -1.0f;
}

void ULODTransitionManager::UpdateTransitions(float DeltaTime, FVector2D ViewerPosition)
{
	// Collect completed transitions
	TArray<FString> CompletedKeys;

	// Update each transition
	for (auto& Pair : ActiveTransitions)
	{
		FLODTransition& Transition = Pair.Value;

		UpdateTransition(Transition, DeltaTime, ViewerPosition);

		// Check if complete
		if (Transition.IsComplete())
		{
			CompletedKeys.Add(Pair.Key);
		}
	}

	// Remove completed transitions
	for (const FString& Key : CompletedKeys)
	{
		CompleteTransition(Key);
	}
}

bool ULODTransitionManager::GetGeomorphedTileData(FVector2D TilePosition, FTerrainTileData& OutTileData)
{
	FString Key = GetTransitionKey(TilePosition);
	FLODTransition* Transition = ActiveTransitions.Find(Key);

	if (!Transition || Transition->State != ELODTransitionState::Geomorphing)
	{
		return false;
	}

	// Return new tile data with geomorphed vertices
	OutTileData = Transition->NewTileData;
	OutTileData.Vertices = Transition->GeomorphedVertices;
	OutTileData.Normals = Transition->GeomorphedNormals;

	return true;
}

void ULODTransitionManager::GeomorphVertices(FLODTransition& Transition, float Alpha)
{
	if (!Config.bEnableGeomorphing)
	{
		return;
	}

	// Get eased alpha for smoother transitions
	float EasedAlpha = Config.bUseSmoothEasing ? Transition.GetEasedAlpha() : Alpha;

	// Determine which mesh has higher resolution
	bool bOldIsHigher = Transition.OldTileData.Resolution > Transition.NewTileData.Resolution;

	if (bOldIsHigher)
	{
		// Transitioning from high to low detail
		// Interpolate from old (high res) to new (low res)
		Transition.GeomorphedVertices = InterpolateVertices(
			Transition.OldTileData.Vertices,
			Transition.NewTileData.Vertices,
			EasedAlpha
		);

		if (Config.bInterpolateNormals)
		{
			Transition.GeomorphedNormals = InterpolateNormals(
				Transition.OldTileData.Normals,
				Transition.NewTileData.Normals,
				EasedAlpha
			);
		}
	}
	else
	{
		// Transitioning from low to high detail
		// Interpolate from old (low res) to new (high res)
		Transition.GeomorphedVertices = InterpolateVertices(
			Transition.OldTileData.Vertices,
			Transition.NewTileData.Vertices,
			EasedAlpha
		);

		if (Config.bInterpolateNormals)
		{
			Transition.GeomorphedNormals = InterpolateNormals(
				Transition.OldTileData.Normals,
				Transition.NewTileData.Normals,
				EasedAlpha
			);
		}
	}
}

TArray<FVector> ULODTransitionManager::InterpolateVertices(
	const TArray<FVector>& OldVertices,
	const TArray<FVector>& NewVertices,
	float Alpha)
{
	TArray<FVector> Result;

	// If resolutions match, simple interpolation
	if (OldVertices.Num() == NewVertices.Num())
	{
		Result.Reserve(OldVertices.Num());

		for (int32 I = 0; I < OldVertices.Num(); ++I)
		{
			FVector Interpolated = FMath::Lerp(OldVertices[I], NewVertices[I], Alpha);
			Result.Add(Interpolated);
		}
	}
	else
	{
		// Different resolutions - use new vertices as base
		Result = NewVertices;

		// For now, just return new vertices
		// More sophisticated sampling could be implemented here
	}

	return Result;
}

TArray<FVector> ULODTransitionManager::InterpolateNormals(
	const TArray<FVector>& OldNormals,
	const TArray<FVector>& NewNormals,
	float Alpha)
{
	TArray<FVector> Result;

	// If counts match, interpolate and normalize
	if (OldNormals.Num() == NewNormals.Num())
	{
		Result.Reserve(OldNormals.Num());

		for (int32 I = 0; I < OldNormals.Num(); ++I)
		{
			FVector Interpolated = FMath::Lerp(OldNormals[I], NewNormals[I], Alpha);
			Interpolated.Normalize();
			Result.Add(Interpolated);
		}
	}
	else
	{
		// Different counts - use new normals
		Result = NewNormals;
	}

	return Result;
}

FVector ULODTransitionManager::SampleVertexFromMesh(
	const FTerrainTileData& HighResMesh,
	FVector2D LocalPosition)
{
	if (HighResMesh.Vertices.Num() == 0 || HighResMesh.Resolution <= 0)
	{
		return FVector::ZeroVector;
	}

	// Clamp to valid range
	float U = FMath::Clamp(LocalPosition.X, 0.0f, 1.0f);
	float V = FMath::Clamp(LocalPosition.Y, 0.0f, 1.0f);

	// Convert to grid coordinates
	int32 Resolution = HighResMesh.Resolution;
	float X = U * (Resolution - 1);
	float Y = V * (Resolution - 1);

	int32 X0 = FMath::FloorToInt(X);
	int32 Y0 = FMath::FloorToInt(Y);
	int32 X1 = FMath::Min(X0 + 1, Resolution - 1);
	int32 Y1 = FMath::Min(Y0 + 1, Resolution - 1);

	float Fx = X - X0;
	float Fy = Y - Y0;

	// Get vertices at corners
	FVector V00 = HighResMesh.Vertices[Y0 * Resolution + X0];
	FVector V10 = HighResMesh.Vertices[Y0 * Resolution + X1];
	FVector V01 = HighResMesh.Vertices[Y1 * Resolution + X0];
	FVector V11 = HighResMesh.Vertices[Y1 * Resolution + X1];

	// Bilinear interpolation
	FVector V0 = FMath::Lerp(V00, V10, Fx);
	FVector V1 = FMath::Lerp(V01, V11, Fx);

	return FMath::Lerp(V0, V1, Fy);
}

float ULODTransitionManager::CalculateDistanceFade(
	FVector2D TilePosition,
	FVector2D ViewerPosition,
	float FadeRange)
{
	float Distance = FVector2D::Distance(TilePosition, ViewerPosition);

	// Fade from 0 to 1 over fade range
	float Fade = FMath::Clamp(Distance / FadeRange, 0.0f, 1.0f);

	return Fade;
}

int32 ULODTransitionManager::GetActiveTransitionCount() const
{
	return ActiveTransitions.Num();
}

TArray<FLODTransition> ULODTransitionManager::GetActiveTransitions() const
{
	TArray<FLODTransition> Result;
	Result.Reserve(ActiveTransitions.Num());

	for (const auto& Pair : ActiveTransitions)
	{
		Result.Add(Pair.Value);
	}

	return Result;
}

void ULODTransitionManager::DrawDebugTransitions(UWorld* World, float Duration) const
{
	if (!World)
	{
		return;
	}

	for (const auto& Pair : ActiveTransitions)
	{
		const FLODTransition& Transition = Pair.Value;

		// Draw transition bounds
		FVector Center(Transition.TilePosition.X, Transition.TilePosition.Y, 0.0f);
		float HalfSize = Transition.NewTileData.TileSize * 0.5f;

		// Color based on progress
		FColor Color = FColor::MakeRedToGreenColorFromScalar(Transition.TransitionProgress);

		DrawDebugBox(World, Center, FVector(HalfSize, HalfSize, 100.0f), Color, false, Duration, 0, 3.0f);

		// Draw progress text
		FString ProgressText = FString::Printf(TEXT("LOD %d->%d: %.1f%%"),
			Transition.OldLODLevel,
			Transition.NewLODLevel,
			Transition.TransitionProgress * 100.0f);

		DrawDebugString(World, Center + FVector(0, 0, 200.0f), ProgressText, nullptr, Color, Duration, true);
	}
}

FString ULODTransitionManager::GetTransitionKey(FVector2D Position) const
{
	return FString::Printf(TEXT("%.1f_%.1f"), Position.X, Position.Y);
}

void ULODTransitionManager::UpdateTransition(FLODTransition& Transition, float DeltaTime, FVector2D ViewerPosition)
{
	// Update elapsed time
	Transition.ElapsedTime += DeltaTime;

	// Calculate progress
	if (Transition.TransitionDuration > 0.0f)
	{
		Transition.TransitionProgress = FMath::Clamp(
			Transition.ElapsedTime / Transition.TransitionDuration,
			0.0f,
			1.0f
		);
	}
	else
	{
		Transition.TransitionProgress = 1.0f;
	}

	// Update distance
	Transition.CurrentDistance = FVector2D::Distance(Transition.TilePosition, ViewerPosition);

	// Update based on state
	switch (Transition.State)
	{
	case ELODTransitionState::Geomorphing:
		// Perform vertex geomorphing
		GeomorphVertices(Transition, Transition.TransitionProgress);

		// Complete when progress reaches 1.0
		if (Transition.TransitionProgress >= 1.0f)
		{
			Transition.State = ELODTransitionState::Idle;
		}
		break;

	case ELODTransitionState::FadingIn:
	case ELODTransitionState::FadingOut:
		// Simple fade transition
		if (Transition.TransitionProgress >= 1.0f)
		{
			Transition.State = ELODTransitionState::Idle;
		}
		break;

	case ELODTransitionState::Idle:
		// Already complete
		break;
	}
}

void ULODTransitionManager::CompleteTransition(const FString& Key)
{
	const FLODTransition* Transition = ActiveTransitions.Find(Key);

	if (Transition)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Completed LOD transition at (%.1f, %.1f): LOD %d -> %d"),
			Transition->TilePosition.X,
			Transition->TilePosition.Y,
			Transition->OldLODLevel,
			Transition->NewLODLevel);
	}

	ActiveTransitions.Remove(Key);
}

FVector ULODTransitionManager::FindCorrespondingVertex(
	const FTerrainTileData& SourceMesh,
	const FTerrainTileData& TargetMesh,
	int32 SourceVertexIndex) const
{
	if (SourceMesh.Resolution <= 0 || TargetMesh.Resolution <= 0)
	{
		return FVector::ZeroVector;
	}

	// Calculate local UV coordinates in source mesh
	int32 SourceRes = SourceMesh.Resolution;
	int32 X = SourceVertexIndex % SourceRes;
	int32 Y = SourceVertexIndex / SourceRes;

	float U = static_cast<float>(X) / (SourceRes - 1);
	float V = static_cast<float>(Y) / (SourceRes - 1);

	// Sample from target mesh at same UV coordinates
	return SampleVertexFromMesh(TargetMesh, FVector2D(U, V));
}
