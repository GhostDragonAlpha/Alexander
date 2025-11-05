// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropGrowthStageManager.h"
#include "CropDefinition.h"

UCropGrowthStageManager::UCropGrowthStageManager()
{
}

// ============================================================================
// STAGE GENERATION
// ============================================================================

TArray<FGrowthStage> UCropGrowthStageManager::GenerateDefaultStages(UCropDefinition* CropDef, int32 NumStages)
{
	TArray<FGrowthStage> Stages;

	if (!CropDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("CropGrowthStageManager: Cannot generate stages - null crop definition"));
		return Stages;
	}

	// Clamp number of stages to reasonable range
	NumStages = FMath::Clamp(NumStages, 2, 8);

	// Use provided meshes if available
	const TArray<UStaticMesh*>& ProvidedMeshes = CropDef->GrowthStageMeshes;
	if (ProvidedMeshes.Num() > 0)
	{
		NumStages = FMath::Min(NumStages, ProvidedMeshes.Num());
	}

	// Default stage names
	TArray<FString> DefaultStageNames = {
		TEXT("Seed"),
		TEXT("Sprout"),
		TEXT("Seedling"),
		TEXT("Vegetative"),
		TEXT("Budding"),
		TEXT("Flowering"),
		TEXT("Fruiting"),
		TEXT("Mature")
	};

	// Calculate progress ranges for each stage
	float ProgressPerStage = 1.0f / NumStages;

	for (int32 i = 0; i < NumStages; ++i)
	{
		FGrowthStage Stage;
		Stage.StageName = (i < DefaultStageNames.Num()) ? DefaultStageNames[i] : FString::Printf(TEXT("Stage %d"), i + 1);
		Stage.MinProgress = i * ProgressPerStage;
		Stage.MaxProgress = (i + 1) * ProgressPerStage;

		// Assign mesh if available
		if (i < ProvidedMeshes.Num())
		{
			Stage.StageMesh = ProvidedMeshes[i];
		}
		else if (CropDef->CropMesh)
		{
			Stage.StageMesh = CropDef->CropMesh;
		}

		// Scale increases with growth
		float ScaleFactor = FMath::Lerp(0.1f, 1.0f, (i + 1.0f) / NumStages);
		Stage.ScaleMultiplier = FVector(ScaleFactor);

		// Color transitions from pale to vibrant green
		if (i == 0)
		{
			// Seed stage - brown
			Stage.TintColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
		}
		else if (i < NumStages - 1)
		{
			// Growing stages - light to medium green
			float GreenIntensity = FMath::Lerp(0.5f, 0.8f, (float)i / (NumStages - 1));
			Stage.TintColor = FLinearColor(0.2f, GreenIntensity, 0.2f, 1.0f);
		}
		else
		{
			// Mature stage - vibrant green
			Stage.TintColor = FLinearColor(0.15f, 0.85f, 0.15f, 1.0f);
		}

		Stages.Add(Stage);
	}

	UE_LOG(LogTemp, Log, TEXT("CropGrowthStageManager: Generated %d growth stages for %s"), 
		NumStages, *CropDef->CropName);

	return Stages;
}

TArray<FGrowthStage> UCropGrowthStageManager::CreateCustomStages(
	const TArray<UStaticMesh*>& StageMeshes,
	const TArray<FString>& StageNames)
{
	TArray<FGrowthStage> Stages;

	int32 NumStages = StageMeshes.Num();
	if (NumStages == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CropGrowthStageManager: Cannot create custom stages - no meshes provided"));
		return Stages;
	}

	float ProgressPerStage = 1.0f / NumStages;

	for (int32 i = 0; i < NumStages; ++i)
	{
		FGrowthStage Stage;
		Stage.MinProgress = i * ProgressPerStage;
		Stage.MaxProgress = (i + 1) * ProgressPerStage;
		Stage.StageMesh = StageMeshes[i];

		// Use provided name or generate default
		if (i < StageNames.Num())
		{
			Stage.StageName = StageNames[i];
		}
		else
		{
			Stage.StageName = FString::Printf(TEXT("Stage %d"), i + 1);
		}

		// Default scale progression
		float ScaleFactor = FMath::Lerp(0.1f, 1.0f, (i + 1.0f) / NumStages);
		Stage.ScaleMultiplier = FVector(ScaleFactor);

		// Default color progression
		float GreenIntensity = FMath::Lerp(0.4f, 0.9f, (float)i / (NumStages - 1));
		Stage.TintColor = FLinearColor(0.2f, GreenIntensity, 0.2f, 1.0f);

		Stages.Add(Stage);
	}

	UE_LOG(LogTemp, Log, TEXT("CropGrowthStageManager: Created %d custom growth stages"), NumStages);

	return Stages;
}

// ============================================================================
// STAGE QUERIES
// ============================================================================

int32 UCropGrowthStageManager::GetCurrentStageIndex(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0)
	{
		return -1;
	}

	GrowthProgress = FMath::Clamp(GrowthProgress, 0.0f, 1.0f);

	for (int32 i = 0; i < Stages.Num(); ++i)
	{
		if (Stages[i].IsInStage(GrowthProgress))
		{
			return i;
		}
	}

	// If we're at exactly 1.0, return the last stage
	if (GrowthProgress >= 1.0f)
	{
		return Stages.Num() - 1;
	}

	return 0;
}

FGrowthStage UCropGrowthStageManager::GetStageByIndex(int32 StageIndex, const TArray<FGrowthStage>& Stages)
{
	if (StageIndex >= 0 && StageIndex < Stages.Num())
	{
		return Stages[StageIndex];
	}

	UE_LOG(LogTemp, Warning, TEXT("CropGrowthStageManager: Invalid stage index %d (total stages: %d)"), 
		StageIndex, Stages.Num());

	return FGrowthStage();
}

FGrowthStage UCropGrowthStageManager::GetStageByProgress(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	int32 StageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	return GetStageByIndex(StageIndex, Stages);
}

FGrowthStage UCropGrowthStageManager::GetNextStage(int32 CurrentStageIndex, const TArray<FGrowthStage>& Stages)
{
	int32 NextIndex = CurrentStageIndex + 1;
	if (NextIndex < Stages.Num())
	{
		return Stages[NextIndex];
	}

	// Return current stage if already at max
	if (CurrentStageIndex >= 0 && CurrentStageIndex < Stages.Num())
	{
		return Stages[CurrentStageIndex];
	}

	return FGrowthStage();
}

bool UCropGrowthStageManager::HasStageChanged(float OldProgress, float NewProgress, const TArray<FGrowthStage>& Stages)
{
	int32 OldStage = GetCurrentStageIndex(OldProgress, Stages);
	int32 NewStage = GetCurrentStageIndex(NewProgress, Stages);
	return OldStage != NewStage;
}

// ============================================================================
// STAGE TRANSITIONS
// ============================================================================

FStageTransition UCropGrowthStageManager::CalculateStageTransition(
	int32 FromStage,
	int32 ToStage,
	float TransitionDuration,
	float DeltaTime)
{
	FStageTransition Transition;
	Transition.FromStage = FromStage;
	Transition.ToStage = ToStage;

	if (TransitionDuration > 0.0f)
	{
		Transition.TransitionProgress = FMath::Clamp(
			Transition.TransitionProgress + (DeltaTime / TransitionDuration),
			0.0f,
			1.0f
		);
	}
	else
	{
		Transition.TransitionProgress = 1.0f;
	}

	return Transition;
}

FVector UCropGrowthStageManager::GetTransitionScale(
	const FGrowthStage& FromStage,
	const FGrowthStage& ToStage,
	float TransitionProgress)
{
	return FMath::Lerp(FromStage.ScaleMultiplier, ToStage.ScaleMultiplier, TransitionProgress);
}

FLinearColor UCropGrowthStageManager::GetTransitionColor(
	const FGrowthStage& FromStage,
	const FGrowthStage& ToStage,
	float TransitionProgress)
{
	return FMath::Lerp(FromStage.TintColor, ToStage.TintColor, TransitionProgress);
}

// ============================================================================
// VISUAL REPRESENTATION
// ============================================================================

UStaticMesh* UCropGrowthStageManager::GetStageMesh(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	FGrowthStage CurrentStage = GetStageByProgress(GrowthProgress, Stages);
	return CurrentStage.StageMesh;
}

FVector UCropGrowthStageManager::GetStageScale(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0)
	{
		return FVector::OneVector;
	}

	int32 CurrentStageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	if (CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num())
	{
		return FVector::OneVector;
	}

	const FGrowthStage& CurrentStage = Stages[CurrentStageIndex];

	// Smooth scale within stage
	float ProgressWithinStage = GetProgressWithinStage(GrowthProgress, Stages);

	// If not the last stage, interpolate towards next stage scale
	if (CurrentStageIndex < Stages.Num() - 1)
	{
		const FGrowthStage& NextStage = Stages[CurrentStageIndex + 1];
		return FMath::Lerp(CurrentStage.ScaleMultiplier, NextStage.ScaleMultiplier, ProgressWithinStage);
	}

	return CurrentStage.ScaleMultiplier;
}

FLinearColor UCropGrowthStageManager::GetStageColor(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0)
	{
		return FLinearColor::White;
	}

	int32 CurrentStageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	if (CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num())
	{
		return FLinearColor::White;
	}

	const FGrowthStage& CurrentStage = Stages[CurrentStageIndex];

	// Smooth color within stage
	float ProgressWithinStage = GetProgressWithinStage(GrowthProgress, Stages);

	// If not the last stage, interpolate towards next stage color
	if (CurrentStageIndex < Stages.Num() - 1)
	{
		const FGrowthStage& NextStage = Stages[CurrentStageIndex + 1];
		return FMath::Lerp(CurrentStage.TintColor, NextStage.TintColor, ProgressWithinStage);
	}

	return CurrentStage.TintColor;
}

FString UCropGrowthStageManager::GetStageName(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	FGrowthStage CurrentStage = GetStageByProgress(GrowthProgress, Stages);
	return CurrentStage.StageName;
}

// ============================================================================
// PROGRESS TRACKING
// ============================================================================

float UCropGrowthStageManager::GetProgressWithinStage(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0)
	{
		return 0.0f;
	}

	int32 CurrentStageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	if (CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num())
	{
		return 0.0f;
	}

	const FGrowthStage& CurrentStage = Stages[CurrentStageIndex];
	float StageRange = CurrentStage.MaxProgress - CurrentStage.MinProgress;

	if (StageRange <= 0.0f)
	{
		return 0.0f;
	}

	float ProgressInStage = GrowthProgress - CurrentStage.MinProgress;
	return FMath::Clamp(ProgressInStage / StageRange, 0.0f, 1.0f);
}

float UCropGrowthStageManager::GetPercentageToNextStage(float GrowthProgress, const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0)
	{
		return 0.0f;
	}

	int32 CurrentStageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	if (CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num() - 1)
	{
		return 100.0f; // Already at final stage
	}

	float ProgressWithinStage = GetProgressWithinStage(GrowthProgress, Stages);
	return ProgressWithinStage * 100.0f;
}

float UCropGrowthStageManager::EstimateTimeToNextStage(
	float GrowthProgress,
	float GrowthRate,
	const TArray<FGrowthStage>& Stages)
{
	if (Stages.Num() == 0 || GrowthRate <= 0.0f)
	{
		return 0.0f;
	}

	int32 CurrentStageIndex = GetCurrentStageIndex(GrowthProgress, Stages);
	if (CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num() - 1)
	{
		return 0.0f; // Already at final stage
	}

	const FGrowthStage& CurrentStage = Stages[CurrentStageIndex];
	float RemainingProgress = CurrentStage.MaxProgress - GrowthProgress;

	// Time = Progress / Rate
	return RemainingProgress / GrowthRate;
}

// ============================================================================
// VALIDATION
// ============================================================================

bool UCropGrowthStageManager::ValidateStages(const TArray<FGrowthStage>& Stages, FString& OutErrorMessage)
{
	if (Stages.Num() == 0)
	{
		OutErrorMessage = TEXT("No growth stages defined");
		return false;
	}

	// Check that stages cover the full 0-1 range
	if (Stages[0].MinProgress != 0.0f)
	{
		OutErrorMessage = FString::Printf(TEXT("First stage must start at 0.0 (starts at %.2f)"), Stages[0].MinProgress);
		return false;
	}

	if (Stages.Last().MaxProgress != 1.0f)
	{
		OutErrorMessage = FString::Printf(TEXT("Last stage must end at 1.0 (ends at %.2f)"), Stages.Last().MaxProgress);
		return false;
	}

	// Check for gaps or overlaps
	for (int32 i = 0; i < Stages.Num() - 1; ++i)
	{
		if (Stages[i].MaxProgress != Stages[i + 1].MinProgress)
		{
			OutErrorMessage = FString::Printf(
				TEXT("Gap or overlap between stage %d (%.2f-%.2f) and stage %d (%.2f-%.2f)"),
				i, Stages[i].MinProgress, Stages[i].MaxProgress,
				i + 1, Stages[i + 1].MinProgress, Stages[i + 1].MaxProgress
			);
			return false;
		}
	}

	// Check that each stage has valid range
	for (int32 i = 0; i < Stages.Num(); ++i)
	{
		if (Stages[i].MinProgress >= Stages[i].MaxProgress)
		{
			OutErrorMessage = FString::Printf(
				TEXT("Stage %d has invalid range (%.2f-%.2f)"),
				i, Stages[i].MinProgress, Stages[i].MaxProgress
			);
			return false;
		}
	}

	OutErrorMessage = TEXT("Stages are valid");
	return true;
}

TArray<FGrowthStage> UCropGrowthStageManager::FixStageRanges(const TArray<FGrowthStage>& Stages)
{
	TArray<FGrowthStage> FixedStages = Stages;

	if (FixedStages.Num() == 0)
	{
		return FixedStages;
	}

	// Recalculate ranges to be evenly distributed
	float ProgressPerStage = 1.0f / FixedStages.Num();

	for (int32 i = 0; i < FixedStages.Num(); ++i)
	{
		FixedStages[i].MinProgress = i * ProgressPerStage;
		FixedStages[i].MaxProgress = (i + 1) * ProgressPerStage;
	}

	UE_LOG(LogTemp, Log, TEXT("CropGrowthStageManager: Fixed %d stage ranges"), FixedStages.Num());

	return FixedStages;
}
