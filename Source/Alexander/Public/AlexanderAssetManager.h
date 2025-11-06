// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/AssetManager.h"

// Forward declarations
class UTexture2D;
class USoundBase;
class UStaticMesh;
class USkeletalMesh;
class UMaterialInterface;
class UParticleSystem;

#include "AlexanderAssetManager.generated.h"  // MUST BE LAST include before UENUM/USTRUCT/UCLASS

/**
 * Asset types for categorization
 */
UENUM(BlueprintType)
enum class EAssetType : uint8
{
	Texture UMETA(DisplayName = "Texture"),
	Mesh UMETA(DisplayName = "Mesh"),
	Sound UMETA(DisplayName = "Sound"),
	Material UMETA(DisplayName = "Material"),
	Particle UMETA(DisplayName = "Particle"),
	Animation UMETA(DisplayName = "Animation"),
	Blueprint UMETA(DisplayName = "Blueprint"),
	Other UMETA(DisplayName = "Other")
};

/**
 * Asset source information
 */
USTRUCT(BlueprintType)
struct FAssetSource
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SourceName;

	UPROPERTY(BlueprintReadOnly)
	FString SourceURL;

	UPROPERTY(BlueprintReadOnly)
	FString License;

	UPROPERTY(BlueprintReadOnly)
	FString Author;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFree;

	UPROPERTY(BlueprintReadOnly)
	bool bRequiresAttribution;

	FAssetSource()
	{
		SourceName = TEXT("");
		SourceURL = TEXT("");
		License = TEXT("");
		Author = TEXT("");
		bIsFree = true;
		bRequiresAttribution = false;
	}
};

/**
 * Asset metadata
 */
USTRUCT(BlueprintType)
struct FAssetMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString AssetName;

	UPROPERTY(BlueprintReadOnly)
	FString AssetPath;

	UPROPERTY(BlueprintReadOnly)
	EAssetType AssetType;

	UPROPERTY(BlueprintReadOnly)
	FString Category;

	UPROPERTY(BlueprintReadOnly)
	FString Tags;

	UPROPERTY(BlueprintReadOnly)
	FAssetSource Source;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLoaded;

	UPROPERTY(BlueprintReadOnly)
	float FileSize;

	UPROPERTY(BlueprintReadOnly)
	FString Description;

	FAssetMetadata()
	{
		AssetName = TEXT("");
		AssetPath = TEXT("");
		AssetType = EAssetType::Other;
		Category = TEXT("");
		Tags = TEXT("");
		bIsLoaded = false;
		FileSize = 0.0f;
		Description = TEXT("");
	}
};

/**
 * Asset Manager - Handles integration of free assets from various sources
 * Manages downloading, loading, and organization of game assets
 */

/**
 * Wrapper for asset lists by type
 */
USTRUCT(BlueprintType)
struct FAssetListByType
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> AssetPaths;
};

/**
 * Wrapper for asset lists by category
 */
USTRUCT(BlueprintType)
struct FAssetListByCategory
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> AssetPaths;
};

UCLASS()
class ALEXANDER_API UAlexanderAssetManager : public UObject
{
    GENERATED_BODY()

public:
	UAlexanderAssetManager();

protected:
	virtual void BeginDestroy() override;

public:
	// Asset loading and management
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	bool LoadAsset(const FString& AssetPath, EAssetType AssetType);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void UnloadAsset(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	bool IsAssetLoaded(const FString& AssetPath) const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	UObject* GetAsset(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void PreloadAssets(const TArray<FString>& AssetPaths);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void UnloadUnusedAssets();

	// Asset discovery and cataloging
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void DiscoverAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	TArray<FAssetMetadata> GetAssetsByType(EAssetType AssetType) const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	TArray<FAssetMetadata> GetAssetsByCategory(const FString& Category) const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	TArray<FAssetMetadata> SearchAssets(const FString& SearchTerm) const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	FAssetMetadata GetAssetMetadata(const FString& AssetPath) const;

	// Kenney Assets integration
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadKenneyAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadKenneySpaceAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadKenneyUIAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadKenneyAudioAssets();

	// OpenGameArt.org integration
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadOpenGameArtAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadSpaceAssetsFromOGA();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadAudioAssetsFromOGA();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LoadTextureAssetsFromOGA();

	// Asset utilities
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	UTexture2D* LoadTexture(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	USoundBase* LoadSound(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	UStaticMesh* LoadStaticMesh(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	USkeletalMesh* LoadSkeletalMesh(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	UMaterialInterface* LoadMaterial(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	UParticleSystem* LoadParticleSystem(const FString& AssetPath);

	// Asset optimization
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void OptimizeAssets();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void CompressTextures();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void OptimizeMeshes();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void OptimizeAudio();

	// Asset streaming
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void EnableAssetStreaming(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	bool IsAssetStreamingEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void StreamAsset(const FString& AssetPath, float Priority);

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void UnstreamAsset(const FString& AssetPath);

	// Statistics and debugging
	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	int32 GetLoadedAssetCount() const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	float GetTotalMemoryUsage() const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	TMap<EAssetType, int32> GetAssetCountByType() const;

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void LogAssetStatistics();

	UFUNCTION(BlueprintCallable, Category = "Asset Manager")
	void ValidateAssets();

protected:
	// Asset registry
	UPROPERTY()
	TMap<FString, FAssetMetadata> AssetRegistry;

	UPROPERTY()
	TMap<FString, UObject*> LoadedAssets;

	UPROPERTY()
	TMap<EAssetType, FAssetListByType> AssetsByType;

	UPROPERTY()
	TMap<FString, FAssetListByCategory> AssetsByCategory;

	// Asset sources
	UPROPERTY()
	TArray<FAssetSource> AssetSources;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	bool bEnableAssetStreaming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	float MaxMemoryUsage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	bool bAutoOptimizeAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	bool bEnableCompression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Manager")
	FString BaseAssetPath;

	// Internal state
	UPROPERTY()
	float CurrentMemoryUsage;

	UPROPERTY()
	int32 TotalAssetsLoaded;

	UPROPERTY()
	TArray<FString> StreamingQueue;

private:
	void RegisterAsset(const FString& AssetPath, const FAssetMetadata& Metadata);
	void UnregisterAsset(const FString& AssetPath);
	bool LoadAssetInternal(const FString& AssetPath, EAssetType AssetType);
	void UpdateMemoryUsage();
	void InitializeAssetSources();
	void ProcessStreamingQueue();
	FString GetAssetTypeString(EAssetType AssetType) const;
	EAssetType GetAssetTypeFromString(const FString& TypeString) const;
	EAssetType DetermineAssetTypeFromPath(const FString& AssetPath) const;
	FString DetermineCategoryFromPath(const FString& AssetPath) const;
	FString GetSourceFromPath(const FString& AssetPath) const;
};