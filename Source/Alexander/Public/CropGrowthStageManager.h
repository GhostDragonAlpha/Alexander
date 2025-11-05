// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropGrowthStageManager.generated.h"

// Forward declarations
class UCropDefinition;
class UStaticMesh;

/**
 * Growth stage information
 */
USTRUCT(BlueprintType)
struct FGrowthStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	FString StageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	float MinProgress; // 0-1, when this stage starts

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	float MaxProgress; // 0-1, when this stage ends

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	UStaticMesh* StageMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	FVector ScaleMultiplier; // Scale adjustment for this stage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stage")
	FLinearColor TintColor; // Color tint for this stage

	FGrowthStage()
		: StageName(TEXT("Unknown"))
		, MinProgress(0.0f)
		, MaxProgress(1.0f)
		, StageMesh(nullptr)
		, ScaleMultiplier(FVector::OneVector)
		, TintColor(FLinearColor::White)
	{
	}

	FGrowthStage(const FString& InName, float InMin, float InMax)
		: StageName(InName)
		, MinProgress(InMin)
		, MaxProgress(InMax)
		, StageMesh(nullptr)
		, ScaleMultiplier(FVector::OneVector)
		, TintColor(FLinearColor::White)
	{
	}

	bool IsInStage(float Progress) const
	{
		return Progress >= MinProgress && Progress < MaxProgress;
	}
};

/**
 * Stage transition information
 */
USTRUCT(BlueprintType)
struct FStageTransition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Transition")
	int32 FromStage;

	UPROPERTY(BlueprintReadWrite, Category = "Transition")
	int32 ToStage;

	UPROPERTY(BlueprintReadWrite, Category = "Transition")
	float TransitionProgress; // 0-1, for smooth transitions

	UPROPERTY(BlueprintReadWrite, Category = "Transition")
	FDateTime TransitionStartTime;

	FStageTransition()
		: FromStage(-1)
		, ToStage(-1)
		, TransitionProgress(0.0f)
	{
	}
};

/**
 * Crop growth stage manager
 * Handles growth stage progression, transitions, and visual representation
 */
UCLASS(BlueprintType)
class ALEXANDER_API UCropGrowthStageManager : public UObject
{
	GENERATED_BODY()

public:
	UCropGrowthStageManager();

	// ============================================================================
	// STAGE GENERATION
	// ============================================================================

	/**
	 * Generate default growth stages for a crop
	 * Creates 4-6 stages based on crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static TArray<FGrowthStage> GenerateDefaultStages(UCropDefinition* CropDef, int32 NumStages = 4);

	/**
	 * Create custom growth stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static TArray<FGrowthStage> CreateCustomStages(
		const TArray<UStaticMesh*>& StageMeshes,
		const TArray<FString>& StageNames
	);

	// ============================================================================
	// STAGE QUERIES
	// ============================================================================

	/**
	 * Get current growth stage index based on progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static int32 GetCurrentStageIndex(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get growth stage by index
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FGrowthStage GetStageByIndex(int32 StageIndex, const TArray<FGrowthStage>& Stages);

	/**
	 * Get growth stage by progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FGrowthStage GetStageByProgress(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get next growth stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FGrowthStage GetNextStage(int32 CurrentStageIndex, const TArray<FGrowthStage>& Stages);

	/**
	 * Check if crop has reached a new stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static bool HasStageChanged(float OldProgress, float NewProgress, const TArray<FGrowthStage>& Stages);

	// ============================================================================
	// STAGE TRANSITIONS
	// ============================================================================

	/**
	 * Calculate smooth transition between stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FStageTransition CalculateStageTransition(
		int32 FromStage,
		int32 ToStage,
		float TransitionDuration,
		float DeltaTime
	);

	/**
	 * Get interpolated scale during transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FVector GetTransitionScale(
		const FGrowthStage& FromStage,
		const FGrowthStage& ToStage,
		float TransitionProgress
	);

	/**
	 * Get interpolated color during transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FLinearColor GetTransitionColor(
		const FGrowthStage& FromStage,
		const FGrowthStage& ToStage,
		float TransitionProgress
	);

	// ============================================================================
	// VISUAL REPRESENTATION
	// ============================================================================

	/**
	 * Get mesh for current growth stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static UStaticMesh* GetStageMesh(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get scale for current growth stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FVector GetStageScale(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get color for current growth stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FLinearColor GetStageColor(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get stage name for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static FString GetStageName(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	// ============================================================================
	// PROGRESS TRACKING
	// ============================================================================

	/**
	 * Calculate progress within current stage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static float GetProgressWithinStage(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Get percentage to next stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static float GetPercentageToNextStage(float GrowthProgress, const TArray<FGrowthStage>& Stages);

	/**
	 * Estimate time to next stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static float EstimateTimeToNextStage(
		float GrowthProgress,
		float GrowthRate,
		const TArray<FGrowthStage>& Stages
	);

	// ============================================================================
	// VALIDATION
	// ============================================================================

	/**
	 * Validate growth stages configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static bool ValidateStages(const TArray<FGrowthStage>& Stages, FString& OutErrorMessage);

	/**
	 * Fix overlapping or invalid stage ranges
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth Stages")
	static TArray<FGrowthStage> FixStageRanges(const TArray<FGrowthStage>& Stages);
};
