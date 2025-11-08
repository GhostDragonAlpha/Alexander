// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "Particles/ParticleSystem.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "ContentOptimizer.generated.h"

/**
 * LOD Generation Settings
 */
USTRUCT(BlueprintType)
struct FLODGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	int32 NumLODs = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float PercentTriangles = 0.5f; // Percentage of triangles per LOD

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float ScreenSize = 0.5f; // Screen size threshold for LOD switching

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bAutoComputeLODScreenSize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bRecalculateNormals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float WeldingThreshold = 0.1f;
};

/**
 * Particle System Optimization Settings
 */
USTRUCT(BlueprintType)
struct FParticleOptimizationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	int32 MaxParticleCount = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	float ParticleScaleFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	bool bEnableDistanceCulling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	float CullDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	bool bEnableLOD = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	int32 NumLODLevels = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	bool bUseGPUParticles = false;
};

/**
 * Texture Streaming Settings
 */
USTRUCT(BlueprintType)
struct FTextureStreamingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	bool bEnableStreaming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	int32 MaxTextureSize = 2048;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	int32 MinTextureSize = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	bool bGenerateMipMaps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	float MipLoadBias = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	int32 StreamingPriority = 0;
};

/**
 * Optimization Quality Preset
 */
UENUM(BlueprintType)
enum class EOptimizationQuality : uint8
{
	Low UMETA(DisplayName = "Low Quality"),
	Medium UMETA(DisplayName = "Medium Quality"),
	High UMETA(DisplayName = "High Quality"),
	Ultra UMETA(DisplayName = "Ultra Quality"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Optimization Result
 */
USTRUCT(BlueprintType)
struct FOptimizationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	FString AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	int32 OriginalTriangleCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	int32 OptimizedTriangleCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	float OptimizationPercentage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Optimization")
	FString ErrorMessage;
};

/**
 * Content Optimizer - Handles automated asset optimization
 */
UCLASS(BlueprintType)
class ALEXANDER_API UContentOptimizer : public UObject
{
	GENERATED_BODY()

public:
	UContentOptimizer();

	// LOD Generation
	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | LOD")
	static FOptimizationResult GenerateLODsForStaticMesh(UStaticMesh* StaticMesh, const FLODGenerationSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | LOD")
	static void GenerateLODsForMultipleMeshes(const TArray<UStaticMesh*>& Meshes, const FLODGenerationSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | LOD")
	static bool RemoveLODsFromStaticMesh(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | LOD")
	static int32 GetLODCount(UStaticMesh* StaticMesh);

	// Particle System Optimization
	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Particles")
	static FOptimizationResult OptimizeParticleSystem(UParticleSystem* ParticleSystem, const FParticleOptimizationSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Particles")
	static void OptimizeMultipleParticleSystems(const TArray<UParticleSystem*>& ParticleSystems, const FParticleOptimizationSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Particles")
	static void SetParticleSystemCullDistance(UParticleSystem* ParticleSystem, float CullDistance);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Particles")
	static void EnableParticleSystemLOD(UParticleSystem* ParticleSystem, int32 NumLODLevels);

	// Texture Optimization
	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Textures")
	static FOptimizationResult OptimizeTexture(UTexture2D* Texture, const FTextureStreamingSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Textures")
	static void OptimizeMultipleTextures(const TArray<UTexture2D*>& Textures, const FTextureStreamingSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Textures")
	static void SetTextureStreamingSettings(UTexture2D* Texture, bool bEnableStreaming, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Textures")
	static void GenerateMipMapsForTexture(UTexture2D* Texture);

	// Batch Optimization
	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Batch")
	static TArray<FOptimizationResult> OptimizeContentDirectory(const FString& DirectoryPath, EOptimizationQuality Quality);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Batch")
	static void OptimizeAllGameContent(EOptimizationQuality Quality);

	// Preset Management
	UFUNCTION(BlueprintPure, Category = "Content Optimizer | Presets")
	static FLODGenerationSettings GetLODPreset(EOptimizationQuality Quality);

	UFUNCTION(BlueprintPure, Category = "Content Optimizer | Presets")
	static FParticleOptimizationSettings GetParticlePreset(EOptimizationQuality Quality);

	UFUNCTION(BlueprintPure, Category = "Content Optimizer | Presets")
	static FTextureStreamingSettings GetTexturePreset(EOptimizationQuality Quality);

	// Analysis Functions
	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Analysis")
	static int32 AnalyzeStaticMeshComplexity(UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Analysis")
	static int32 AnalyzeParticleSystemComplexity(UParticleSystem* ParticleSystem);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Analysis")
	static int32 AnalyzeTextureMemoryUsage(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Analysis")
	static TMap<FString, int32> GetContentMemoryReport();

	// Utility Functions
	UFUNCTION(BlueprintPure, Category = "Content Optimizer | Utility")
	static bool IsAssetOptimized(UObject* Asset);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Utility")
	static void MarkAssetAsOptimized(UObject* Asset, bool bOptimized);

	UFUNCTION(BlueprintCallable, Category = "Content Optimizer | Utility")
	static void SaveOptimizationReport(const TArray<FOptimizationResult>& Results, const FString& FilePath);

private:
	// Helper functions
	static bool ValidateStaticMesh(UStaticMesh* StaticMesh);
	static bool ValidateParticleSystem(UParticleSystem* ParticleSystem);
	static bool ValidateTexture(UTexture2D* Texture);

	static void ApplyLODSettings(UStaticMesh* StaticMesh, const FLODGenerationSettings& Settings, int32 LODIndex);
	static void ApplyParticleSettings(UParticleSystem* ParticleSystem, const FParticleOptimizationSettings& Settings);
	static void ApplyTextureSettings(UTexture2D* Texture, const FTextureStreamingSettings& Settings);

	static int32 CalculateTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex);
	static float CalculateOptimizationPercentage(int32 Original, int32 Optimized);
};
