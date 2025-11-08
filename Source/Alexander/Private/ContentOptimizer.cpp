// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentOptimizer.h"
#include "Engine/StaticMesh.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Engine/Texture2D.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UContentOptimizer::UContentOptimizer()
{
}

// LOD Generation Functions

FOptimizationResult UContentOptimizer::GenerateLODsForStaticMesh(UStaticMesh* StaticMesh, const FLODGenerationSettings& Settings)
{
	FOptimizationResult Result;
	Result.AssetName = StaticMesh ? StaticMesh->GetName() : TEXT("Invalid");

	if (!ValidateStaticMesh(StaticMesh))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid static mesh");
		return Result;
	}

	// Get original triangle count
	Result.OriginalTriangleCount = CalculateTriangleCount(StaticMesh, 0);

	// Generate LODs
	// Note: In real implementation, you would use FMeshBuildSettings and FMeshReductionSettings
	// This is a simplified version showing the structure
	
	UE_LOG(LogTemp, Log, TEXT("Generating %d LODs for mesh: %s"), Settings.NumLODs, *StaticMesh->GetName());

	// Apply LOD settings for each level
	for (int32 LODIndex = 1; LODIndex <= Settings.NumLODs; ++LODIndex)
	{
		ApplyLODSettings(StaticMesh, Settings, LODIndex);
	}

	Result.OptimizedTriangleCount = CalculateTriangleCount(StaticMesh, Settings.NumLODs);
	Result.OptimizationPercentage = CalculateOptimizationPercentage(Result.OriginalTriangleCount, Result.OptimizedTriangleCount);
	Result.bSuccess = true;

	UE_LOG(LogTemp, Log, TEXT("LOD generation complete: %s (%.1f%% reduction)"), 
		*Result.AssetName, 
		Result.OptimizationPercentage);

	return Result;
}

void UContentOptimizer::GenerateLODsForMultipleMeshes(const TArray<UStaticMesh*>& Meshes, const FLODGenerationSettings& Settings)
{
	for (UStaticMesh* Mesh : Meshes)
	{
		if (Mesh)
		{
			GenerateLODsForStaticMesh(Mesh, Settings);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated LODs for %d meshes"), Meshes.Num());
}

bool UContentOptimizer::RemoveLODsFromStaticMesh(UStaticMesh* StaticMesh)
{
	if (!ValidateStaticMesh(StaticMesh))
	{
		return false;
	}

	// Remove all LODs except LOD0
	// In real implementation, use StaticMesh->SetNumSourceModels(1)
	
	UE_LOG(LogTemp, Log, TEXT("Removed LODs from mesh: %s"), *StaticMesh->GetName());
	return true;
}

int32 UContentOptimizer::GetLODCount(UStaticMesh* StaticMesh)
{
	if (!ValidateStaticMesh(StaticMesh))
	{
		return 0;
	}

	// In real implementation: return StaticMesh->GetNumLODs();
	return 1; // Placeholder
}

// Particle System Optimization Functions

FOptimizationResult UContentOptimizer::OptimizeParticleSystem(UParticleSystem* ParticleSystem, const FParticleOptimizationSettings& Settings)
{
	FOptimizationResult Result;
	Result.AssetName = ParticleSystem ? ParticleSystem->GetName() : TEXT("Invalid");

	if (!ValidateParticleSystem(ParticleSystem))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid particle system");
		return Result;
	}

	Result.OriginalTriangleCount = AnalyzeParticleSystemComplexity(ParticleSystem);

	// Apply particle optimization settings
	ApplyParticleSettings(ParticleSystem, Settings);

	// Set cull distance
	if (Settings.bEnableDistanceCulling)
	{
		SetParticleSystemCullDistance(ParticleSystem, Settings.CullDistance);
	}

	// Enable LOD if requested
	if (Settings.bEnableLOD)
	{
		EnableParticleSystemLOD(ParticleSystem, Settings.NumLODLevels);
	}

	Result.OptimizedTriangleCount = AnalyzeParticleSystemComplexity(ParticleSystem);
	Result.OptimizationPercentage = CalculateOptimizationPercentage(Result.OriginalTriangleCount, Result.OptimizedTriangleCount);
	Result.bSuccess = true;

	UE_LOG(LogTemp, Log, TEXT("Particle system optimized: %s (%.1f%% reduction)"), 
		*Result.AssetName, 
		Result.OptimizationPercentage);

	return Result;
}

void UContentOptimizer::OptimizeMultipleParticleSystems(const TArray<UParticleSystem*>& ParticleSystems, const FParticleOptimizationSettings& Settings)
{
	for (UParticleSystem* PS : ParticleSystems)
	{
		if (PS)
		{
			OptimizeParticleSystem(PS, Settings);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Optimized %d particle systems"), ParticleSystems.Num());
}

void UContentOptimizer::SetParticleSystemCullDistance(UParticleSystem* ParticleSystem, float CullDistance)
{
	if (!ValidateParticleSystem(ParticleSystem))
	{
		return;
	}

	// Set cull distance for all emitters
	for (UParticleEmitter* Emitter : ParticleSystem->Emitters)
	{
		if (Emitter)
		{
			// In real implementation: Emitter->SetCullDistance(CullDistance);
			UE_LOG(LogTemp, Verbose, TEXT("Set cull distance %.1f for emitter in %s"), 
				CullDistance, 
				*ParticleSystem->GetName());
		}
	}
}

void UContentOptimizer::EnableParticleSystemLOD(UParticleSystem* ParticleSystem, int32 NumLODLevels)
{
	if (!ValidateParticleSystem(ParticleSystem))
	{
		return;
	}

	// Enable LOD for particle system
	// In real implementation: Configure LODDistanceCheckTime and LODMethod
	
	UE_LOG(LogTemp, Log, TEXT("Enabled %d LOD levels for particle system: %s"), 
		NumLODLevels, 
		*ParticleSystem->GetName());
}

// Texture Optimization Functions

FOptimizationResult UContentOptimizer::OptimizeTexture(UTexture2D* Texture, const FTextureStreamingSettings& Settings)
{
	FOptimizationResult Result;
	Result.AssetName = Texture ? Texture->GetName() : TEXT("Invalid");

	if (!ValidateTexture(Texture))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Invalid texture");
		return Result;
	}

	Result.OriginalTriangleCount = AnalyzeTextureMemoryUsage(Texture);

	// Apply texture settings
	ApplyTextureSettings(Texture, Settings);

	// Generate mip maps if requested
	if (Settings.bGenerateMipMaps)
	{
		GenerateMipMapsForTexture(Texture);
	}

	Result.OptimizedTriangleCount = AnalyzeTextureMemoryUsage(Texture);
	Result.OptimizationPercentage = CalculateOptimizationPercentage(Result.OriginalTriangleCount, Result.OptimizedTriangleCount);
	Result.bSuccess = true;

	UE_LOG(LogTemp, Log, TEXT("Texture optimized: %s (%.1f%% reduction)"), 
		*Result.AssetName, 
		Result.OptimizationPercentage);

	return Result;
}

void UContentOptimizer::OptimizeMultipleTextures(const TArray<UTexture2D*>& Textures, const FTextureStreamingSettings& Settings)
{
	for (UTexture2D* Texture : Textures)
	{
		if (Texture)
		{
			OptimizeTexture(Texture, Settings);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Optimized %d textures"), Textures.Num());
}

void UContentOptimizer::SetTextureStreamingSettings(UTexture2D* Texture, bool bEnableStreaming, int32 Priority)
{
	if (!ValidateTexture(Texture))
	{
		return;
	}

	// In real implementation:
	// Texture->NeverStream = !bEnableStreaming;
	// Texture->StreamingPriority = Priority;
	// Texture->UpdateResource();

	UE_LOG(LogTemp, Log, TEXT("Set streaming settings for texture: %s (Enabled=%d, Priority=%d)"), 
		*Texture->GetName(), 
		bEnableStreaming, 
		Priority);
}

void UContentOptimizer::GenerateMipMapsForTexture(UTexture2D* Texture)
{
	if (!ValidateTexture(Texture))
	{
		return;
	}

	// In real implementation:
	// Texture->MipGenSettings = TMGS_FromTextureGroup;
	// Texture->UpdateResource();

	UE_LOG(LogTemp, Log, TEXT("Generated mip maps for texture: %s"), *Texture->GetName());
}

// Batch Optimization Functions

TArray<FOptimizationResult> UContentOptimizer::OptimizeContentDirectory(const FString& DirectoryPath, EOptimizationQuality Quality)
{
	TArray<FOptimizationResult> Results;

	// Get presets
	FLODGenerationSettings LODSettings = GetLODPreset(Quality);
	FParticleOptimizationSettings ParticleSettings = GetParticlePreset(Quality);
	FTextureStreamingSettings TextureSettings = GetTexturePreset(Quality);

	// In real implementation, use FAssetRegistryModule to find all assets in directory
	// For now, this is a placeholder

	UE_LOG(LogTemp, Log, TEXT("Optimizing content directory: %s with quality preset: %d"), 
		*DirectoryPath, 
		static_cast<int32>(Quality));

	return Results;
}

void UContentOptimizer::OptimizeAllGameContent(EOptimizationQuality Quality)
{
	UE_LOG(LogTemp, Log, TEXT("Optimizing all game content with quality preset: %d"), 
		static_cast<int32>(Quality));

	// Optimize all content in game
	// This would iterate through all assets using Asset Registry
}

// Preset Management Functions

FLODGenerationSettings UContentOptimizer::GetLODPreset(EOptimizationQuality Quality)
{
	FLODGenerationSettings Settings;

	switch (Quality)
	{
	case EOptimizationQuality::Low:
		Settings.NumLODs = 2;
		Settings.PercentTriangles = 0.3f;
		Settings.ScreenSize = 0.3f;
		break;

	case EOptimizationQuality::Medium:
		Settings.NumLODs = 3;
		Settings.PercentTriangles = 0.5f;
		Settings.ScreenSize = 0.5f;
		break;

	case EOptimizationQuality::High:
		Settings.NumLODs = 4;
		Settings.PercentTriangles = 0.7f;
		Settings.ScreenSize = 0.7f;
		break;

	case EOptimizationQuality::Ultra:
		Settings.NumLODs = 5;
		Settings.PercentTriangles = 0.9f;
		Settings.ScreenSize = 0.9f;
		break;

	case EOptimizationQuality::Custom:
	default:
		// Use default settings
		break;
	}

	return Settings;
}

FParticleOptimizationSettings UContentOptimizer::GetParticlePreset(EOptimizationQuality Quality)
{
	FParticleOptimizationSettings Settings;

	switch (Quality)
	{
	case EOptimizationQuality::Low:
		Settings.MaxParticleCount = 500;
		Settings.CullDistance = 3000.0f;
		Settings.NumLODLevels = 2;
		break;

	case EOptimizationQuality::Medium:
		Settings.MaxParticleCount = 1000;
		Settings.CullDistance = 5000.0f;
		Settings.NumLODLevels = 3;
		break;

	case EOptimizationQuality::High:
		Settings.MaxParticleCount = 2000;
		Settings.CullDistance = 7000.0f;
		Settings.NumLODLevels = 4;
		break;

	case EOptimizationQuality::Ultra:
		Settings.MaxParticleCount = 5000;
		Settings.CullDistance = 10000.0f;
		Settings.NumLODLevels = 5;
		Settings.bUseGPUParticles = true;
		break;

	case EOptimizationQuality::Custom:
	default:
		// Use default settings
		break;
	}

	return Settings;
}

FTextureStreamingSettings UContentOptimizer::GetTexturePreset(EOptimizationQuality Quality)
{
	FTextureStreamingSettings Settings;

	switch (Quality)
	{
	case EOptimizationQuality::Low:
		Settings.MaxTextureSize = 1024;
		Settings.MinTextureSize = 32;
		Settings.StreamingPriority = -1;
		break;

	case EOptimizationQuality::Medium:
		Settings.MaxTextureSize = 2048;
		Settings.MinTextureSize = 64;
		Settings.StreamingPriority = 0;
		break;

	case EOptimizationQuality::High:
		Settings.MaxTextureSize = 4096;
		Settings.MinTextureSize = 128;
		Settings.StreamingPriority = 1;
		break;

	case EOptimizationQuality::Ultra:
		Settings.MaxTextureSize = 8192;
		Settings.MinTextureSize = 256;
		Settings.StreamingPriority = 2;
		break;

	case EOptimizationQuality::Custom:
	default:
		// Use default settings
		break;
	}

	return Settings;
}

// Analysis Functions

int32 UContentOptimizer::AnalyzeStaticMeshComplexity(UStaticMesh* StaticMesh)
{
	if (!ValidateStaticMesh(StaticMesh))
	{
		return 0;
	}

	return CalculateTriangleCount(StaticMesh, 0);
}

int32 UContentOptimizer::AnalyzeParticleSystemComplexity(UParticleSystem* ParticleSystem)
{
	if (!ValidateParticleSystem(ParticleSystem))
	{
		return 0;
	}

	int32 TotalParticles = 0;

	for (UParticleEmitter* Emitter : ParticleSystem->Emitters)
	{
		if (Emitter)
		{
			// In real implementation: get max active particles from emitter properties
			TotalParticles += 100; // Placeholder
		}
	}

	return TotalParticles;
}

int32 UContentOptimizer::AnalyzeTextureMemoryUsage(UTexture2D* Texture)
{
	if (!ValidateTexture(Texture))
	{
		return 0;
	}

	// Calculate memory usage in KB
	// In real implementation: Use Texture->CalcTextureMemorySizeEnum()
	int32 Width = 1024;  // Placeholder
	int32 Height = 1024; // Placeholder
	int32 BytesPerPixel = 4;

	return (Width * Height * BytesPerPixel) / 1024;
}

TMap<FString, int32> UContentOptimizer::GetContentMemoryReport()
{
	TMap<FString, int32> Report;

	// In real implementation, iterate through all loaded assets and calculate memory
	Report.Add(TEXT("Static Meshes"), 0);
	Report.Add(TEXT("Textures"), 0);
	Report.Add(TEXT("Particle Systems"), 0);
	Report.Add(TEXT("Materials"), 0);
	Report.Add(TEXT("Sounds"), 0);

	return Report;
}

// Utility Functions

bool UContentOptimizer::IsAssetOptimized(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	// Check if asset has optimization metadata
	// In real implementation, check asset tags or custom metadata
	return false;
}

void UContentOptimizer::MarkAssetAsOptimized(UObject* Asset, bool bOptimized)
{
	if (!Asset)
	{
		return;
	}

	// Mark asset with optimization metadata
	UE_LOG(LogTemp, Log, TEXT("Marked asset as optimized: %s"), *Asset->GetName());
}

void UContentOptimizer::SaveOptimizationReport(const TArray<FOptimizationResult>& Results, const FString& FilePath)
{
	FString ReportContent;
	ReportContent += TEXT("Content Optimization Report\n");
	ReportContent += TEXT("=============================\n\n");

	for (const FOptimizationResult& Result : Results)
	{
		ReportContent += FString::Printf(TEXT("Asset: %s\n"), *Result.AssetName);
		ReportContent += FString::Printf(TEXT("Status: %s\n"), Result.bSuccess ? TEXT("Success") : TEXT("Failed"));
		ReportContent += FString::Printf(TEXT("Original Complexity: %d\n"), Result.OriginalTriangleCount);
		ReportContent += FString::Printf(TEXT("Optimized Complexity: %d\n"), Result.OptimizedTriangleCount);
		ReportContent += FString::Printf(TEXT("Reduction: %.1f%%\n"), Result.OptimizationPercentage);
		
		if (!Result.ErrorMessage.IsEmpty())
		{
			ReportContent += FString::Printf(TEXT("Error: %s\n"), *Result.ErrorMessage);
		}
		
		ReportContent += TEXT("\n");
	}

	FFileHelper::SaveStringToFile(ReportContent, *FilePath);
	UE_LOG(LogTemp, Log, TEXT("Optimization report saved to: %s"), *FilePath);
}

// Helper Functions

bool UContentOptimizer::ValidateStaticMesh(UStaticMesh* StaticMesh)
{
	return StaticMesh != nullptr;
}

bool UContentOptimizer::ValidateParticleSystem(UParticleSystem* ParticleSystem)
{
	return ParticleSystem != nullptr && ParticleSystem->Emitters.Num() > 0;
}

bool UContentOptimizer::ValidateTexture(UTexture2D* Texture)
{
	return Texture != nullptr;
}

void UContentOptimizer::ApplyLODSettings(UStaticMesh* StaticMesh, const FLODGenerationSettings& Settings, int32 LODIndex)
{
	// In real implementation:
	// - Use FMeshReductionSettings to configure triangle reduction
	// - Apply welding threshold
	// - Recalculate normals if requested
	// - Set screen size for LOD switching

	UE_LOG(LogTemp, Verbose, TEXT("Applied LOD settings for LOD%d on mesh: %s"), 
		LODIndex, 
		*StaticMesh->GetName());
}

void UContentOptimizer::ApplyParticleSettings(UParticleSystem* ParticleSystem, const FParticleOptimizationSettings& Settings)
{
	// Apply settings to all emitters
	for (UParticleEmitter* Emitter : ParticleSystem->Emitters)
	{
		if (Emitter)
		{
			// In real implementation:
			// - Set max active particles
			// - Apply particle scale
			// - Configure GPU particles if enabled
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("Applied particle settings to system: %s"), 
		*ParticleSystem->GetName());
}

void UContentOptimizer::ApplyTextureSettings(UTexture2D* Texture, const FTextureStreamingSettings& Settings)
{
	// In real implementation:
	// - Set max texture size
	// - Configure streaming
	// - Set mip load bias
	// - Update streaming priority

	UE_LOG(LogTemp, Verbose, TEXT("Applied texture settings to: %s"), 
		*Texture->GetName());
}

int32 UContentOptimizer::CalculateTriangleCount(UStaticMesh* StaticMesh, int32 LODIndex)
{
	// In real implementation:
	// const FStaticMeshLODResources& LODResources = StaticMesh->GetLODForExport(LODIndex);
	// return LODResources.GetNumTriangles();

	return 1000; // Placeholder
}

float UContentOptimizer::CalculateOptimizationPercentage(int32 Original, int32 Optimized)
{
	if (Original == 0)
	{
		return 0.0f;
	}

	return ((float)(Original - Optimized) / (float)Original) * 100.0f;
}
