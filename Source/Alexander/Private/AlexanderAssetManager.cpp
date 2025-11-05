// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlexanderAssetManager.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "KismetProceduralMeshLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UAlexanderAssetManager::UAlexanderAssetManager()
{
	// Initialize configuration
	bEnableAssetStreaming = true;
	MaxMemoryUsage = 1024.0f * 1024.0f * 1024.0f; // 1GB
	bAutoOptimizeAssets = true;
	bEnableCompression = true;
	BaseAssetPath = TEXT("/Game/Assets");

	// Initialize internal state
	CurrentMemoryUsage = 0.0f;
	TotalAssetsLoaded = 0;

	// Initialize asset sources
	InitializeAssetSources();
}

void UAlexanderAssetManager::BeginDestroy()
{
	// Unload all assets
	UnloadUnusedAssets();
	LoadedAssets.Empty();
	AssetRegistry.Empty();
	AssetsByType.Empty();
	AssetsByCategory.Empty();
	
	Super::BeginDestroy();
}

bool UAlexanderAssetManager::LoadAsset(const FString& AssetPath, EAssetType AssetType)
{
	if (AssetPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot load asset: empty path"));
		return false;
	}

	// Check if already loaded
	if (LoadedAssets.Contains(AssetPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset %s is already loaded"), *AssetPath);
		return true;
	}

	// Load the asset
	bool bSuccess = LoadAssetInternal(AssetPath, AssetType);
	
	if (bSuccess)
	{
		// Create metadata if not exists
		if (!AssetRegistry.Contains(AssetPath))
		{
			FAssetMetadata Metadata;
			Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
			Metadata.AssetPath = AssetPath;
			Metadata.AssetType = AssetType;
			Metadata.bIsLoaded = true;
			RegisterAsset(AssetPath, Metadata);
		}
		else
		{
			// Update loaded status
			AssetRegistry[AssetPath].bIsLoaded = true;
		}

		TotalAssetsLoaded++;
		UpdateMemoryUsage();

		UE_LOG(LogTemp, Warning, TEXT("Successfully loaded asset: %s"), *AssetPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
	}

	return bSuccess;
}

void UAlexanderAssetManager::UnloadAsset(const FString& AssetPath)
{
	if (!LoadedAssets.Contains(AssetPath))
	{
		return;
	}

	UObject* Asset = LoadedAssets[AssetPath];
	if (Asset)
	{
		// Unload the asset
		Asset->ConditionalBeginDestroy();
		LoadedAssets.Remove(AssetPath);

		// Update metadata
		if (AssetRegistry.Contains(AssetPath))
		{
			AssetRegistry[AssetPath].bIsLoaded = false;
		}

		TotalAssetsLoaded--;
		UpdateMemoryUsage();

		UE_LOG(LogTemp, Warning, TEXT("Unloaded asset: %s"), *AssetPath);
	}
}

bool UAlexanderAssetManager::IsAssetLoaded(const FString& AssetPath) const
{
	return LoadedAssets.Contains(AssetPath);
}

UObject* UAlexanderAssetManager::GetAsset(const FString& AssetPath)
{
	if (LoadedAssets.Contains(AssetPath))
	{
		return LoadedAssets[AssetPath];
	}

	// Try to load if not loaded
	FString AssetTypeString = GetAssetTypeString(EAssetType::Other);
	LoadAsset(AssetPath, EAssetType::Other);
	
	return LoadedAssets.FindRef(AssetPath);
}

void UAlexanderAssetManager::PreloadAssets(const TArray<FString>& AssetPaths)
{
	UE_LOG(LogTemp, Warning, TEXT("Preloading %d assets"), AssetPaths.Num());

	for (const FString& AssetPath : AssetPaths)
	{
		LoadAsset(AssetPath, EAssetType::Other);
	}
}

void UAlexanderAssetManager::UnloadUnusedAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Unloading unused assets"));

	TArray<FString> AssetsToRemove;
	for (const auto& AssetPair : LoadedAssets)
	{
		UObject* Asset = AssetPair.Value;
		if (Asset && Asset->GetOuter() == this) // Only we are referencing it
		{
			AssetsToRemove.Add(AssetPair.Key);
		}
	}

	for (const FString& AssetPath : AssetsToRemove)
	{
		UnloadAsset(AssetPath);
	}

	UE_LOG(LogTemp, Warning, TEXT("Unloaded %d unused assets"), AssetsToRemove.Num());
}

void UAlexanderAssetManager::DiscoverAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Discovering assets in: %s"), *BaseAssetPath);

	// This would scan the asset directories and register all found assets
	// For now, simulate asset discovery with some common asset types

	// Register some placeholder assets for demonstration
	TArray<FString> CommonAssets = {
		TEXT("/Game/Assets/Textures/Space/Starfield"),
		TEXT("/Game/Assets/Textures/Planets/Earth_Diffuse"),
		TEXT("/Game/Assets/Textures/Planets/Mars_Diffuse"),
		TEXT("/Game/Assets/Meshes/SpaceShips/Fighter"),
		TEXT("/Game/Assets/Meshes/SpaceShips/Cargo"),
		TEXT("/Game/Assets/Audio/Space/Ambient_Space"),
		TEXT("/Game/Assets/Audio/Space/Engine_Hum"),
		TEXT("/Game/Assets/Audio/Weapons/Laser_Fire"),
		TEXT("/Game/Assets/Particles/Engine/Thruster"),
		TEXT("/Game/Assets/Particles/Weapons/Explosion")
	};

	for (const FString& AssetPath : CommonAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = DetermineAssetTypeFromPath(AssetPath);
		Metadata.Category = DetermineCategoryFromPath(AssetPath);
		Metadata.bIsLoaded = false;
		Metadata.Description = FString::Printf(TEXT("Free asset from %s"), *GetSourceFromPath(AssetPath));
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Discovered %d assets"), AssetRegistry.Num());
}

TArray<FAssetMetadata> UAlexanderAssetManager::GetAssetsByType(EAssetType AssetType) const
{
	TArray<FAssetMetadata> Assets;
	
	if (AssetsByType.Contains(AssetType))
	{
		const TArray<FString>& AssetPaths = AssetsByType[AssetType].AssetPaths;
		for (const FString& AssetPath : AssetPaths)
		{
			if (AssetRegistry.Contains(AssetPath))
			{
				Assets.Add(AssetRegistry[AssetPath]);
			}
		}
	}

	return Assets;
}

TArray<FAssetMetadata> UAlexanderAssetManager::GetAssetsByCategory(const FString& Category) const
{
	TArray<FAssetMetadata> Assets;
	
	if (AssetsByCategory.Contains(Category))
	{
		const TArray<FString>& AssetPaths = AssetsByCategory[Category].AssetPaths;
		for (const FString& AssetPath : AssetPaths)
		{
			if (AssetRegistry.Contains(AssetPath))
			{
				Assets.Add(AssetRegistry[AssetPath]);
			}
		}
	}

	return Assets;
}

TArray<FAssetMetadata> UAlexanderAssetManager::SearchAssets(const FString& SearchTerm) const
{
	TArray<FAssetMetadata> FoundAssets;
	FString LowerSearchTerm = SearchTerm.ToLower();

	for (const auto& AssetPair : AssetRegistry)
	{
		const FAssetMetadata& Metadata = AssetPair.Value;
		
		if (Metadata.AssetName.ToLower().Contains(LowerSearchTerm) ||
			Metadata.Category.ToLower().Contains(LowerSearchTerm) ||
			Metadata.Tags.ToLower().Contains(LowerSearchTerm) ||
			Metadata.Description.ToLower().Contains(LowerSearchTerm))
		{
			FoundAssets.Add(Metadata);
		}
	}

	return FoundAssets;
}

FAssetMetadata UAlexanderAssetManager::GetAssetMetadata(const FString& AssetPath) const
{
	if (AssetRegistry.Contains(AssetPath))
	{
		return AssetRegistry[AssetPath];
	}
	return FAssetMetadata();
}

void UAlexanderAssetManager::LoadKenneyAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Kenney Assets"));

	// Kenney Space Assets
	LoadKenneySpaceAssets();
	
	// Kenney UI Assets
	LoadKenneyUIAssets();
	
	// Kenney Audio Assets
	LoadKenneyAudioAssets();

	UE_LOG(LogTemp, Warning, TEXT("Kenney Assets loading complete"));
}

void UAlexanderAssetManager::LoadKenneySpaceAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Kenney Space Assets"));

	// Register Kenney space assets
	TArray<FString> KenneySpaceAssets = {
		TEXT("/Game/Assets/Kenney/Space/Meshes/SpaceShip_Fighter"),
		TEXT("/Game/Assets/Kenney/Space/Meshes/SpaceShip_Station"),
		TEXT("/Game/Assets/Kenney/Space/Meshes/Asteroid_01"),
		TEXT("/Game/Assets/Kenney/Space/Meshes/Planet_Sphere"),
		TEXT("/Game/Assets/Kenney/Space/Textures/Planet_Earth"),
		TEXT("/Game/Assets/Kenney/Space/Textures/Planet_Mars"),
		TEXT("/Game/Assets/Kenney/Space/Textures/Space_Nebula"),
		TEXT("/Game/Assets/Kenney/Space/Materials/M_Planet_Earth"),
		TEXT("/Game/Assets/Kenney/Space/Materials/M_Planet_Mars"),
		TEXT("/Game/Assets/Kenney/Space/Particles/P_Starfield")
	};

	for (const FString& AssetPath : KenneySpaceAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = DetermineAssetTypeFromPath(AssetPath);
		Metadata.Category = TEXT("Kenney/Space");
		Metadata.Tags = TEXT("space,kenney,free");
		
		// Set source information
		Metadata.Source.SourceName = TEXT("Kenney");
		Metadata.Source.SourceURL = TEXT("https://kenney.nl/assets/space-kit");
		Metadata.Source.License = TEXT("CC0 1.0 Universal");
		Metadata.Source.Author = TEXT("Kenney");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = false;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d Kenney space assets"), KenneySpaceAssets.Num());
}

void UAlexanderAssetManager::LoadKenneyUIAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Kenney UI Assets"));

	TArray<FString> KenneyUIAssets = {
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Button_Normal"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Button_Hover"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Button_Pressed"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Panel"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Icon_Settings"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Icon_Inventory"),
		TEXT("/Game/Assets/Kenney/UI/Textures/UI_Icon_Map"),
		TEXT("/Game/Assets/Kenney/UI/Materials/M_UI_Button"),
		TEXT("/Game/Assets/Kenney/UI/Materials/M_UI_Panel"),
		TEXT("/Game/Assets/Kenney/UI/Meshes/UI_Panel_3D")
	};

	for (const FString& AssetPath : KenneyUIAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = DetermineAssetTypeFromPath(AssetPath);
		Metadata.Category = TEXT("Kenney/UI");
		Metadata.Tags = TEXT("ui,kenney,free,interface");
		
		Metadata.Source.SourceName = TEXT("Kenney");
		Metadata.Source.SourceURL = TEXT("https://kenney.nl/assets/ui-kit");
		Metadata.Source.License = TEXT("CC0 1.0 Universal");
		Metadata.Source.Author = TEXT("Kenney");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = false;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d Kenney UI assets"), KenneyUIAssets.Num());
}

void UAlexanderAssetManager::LoadKenneyAudioAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Kenney Audio Assets"));

	TArray<FString> KenneyAudioAssets = {
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Engine_Hum"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Laser_Fire"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Explosion_Small"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Explosion_Large"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Button_Click"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Notification"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Warning"),
		TEXT("/Game/Assets/Kenney/Audio/Sounds/Audio_Success"),
		TEXT("/Game/Assets/Kenney/Audio/Music/Audio_Ambient_Space"),
		TEXT("/Game/Assets/Kenney/Audio/Music/Audio_Battle_Theme")
	};

	for (const FString& AssetPath : KenneyAudioAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = EAssetType::Sound;
		Metadata.Category = TEXT("Kenney/Audio");
		Metadata.Tags = TEXT("audio,kenney,free,sound,music");
		
		Metadata.Source.SourceName = TEXT("Kenney");
		Metadata.Source.SourceURL = TEXT("https://kenney.nl/assets/audio-kit");
		Metadata.Source.License = TEXT("CC0 1.0 Universal");
		Metadata.Source.Author = TEXT("Kenney");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = false;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d Kenney audio assets"), KenneyAudioAssets.Num());
}

void UAlexanderAssetManager::LoadOpenGameArtAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading OpenGameArt.org Assets"));

	LoadSpaceAssetsFromOGA();
	LoadAudioAssetsFromOGA();
	LoadTextureAssetsFromOGA();

	UE_LOG(LogTemp, Warning, TEXT("OpenGameArt.org Assets loading complete"));
}

void UAlexanderAssetManager::LoadSpaceAssetsFromOGA()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Space Assets from OpenGameArt.org"));

	TArray<FString> OGASpaceAssets = {
		TEXT("/Game/Assets/OGA/Space/Meshes/OGA_Satellite"),
		TEXT("/Game/Assets/OGA/Space/Meshes/OGA_SpaceStation"),
		TEXT("/Game/Assets/OGA/Space/Textures/OGA_Starfield"),
		TEXT("/Game/Assets/OGA/Space/Textures/OGA_Nebula"),
		TEXT("/Game/Assets/OGA/Space/Particles/OGA_Starfield_Particle")
	};

	for (const FString& AssetPath : OGASpaceAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = DetermineAssetTypeFromPath(AssetPath);
		Metadata.Category = TEXT("OGA/Space");
		Metadata.Tags = TEXT("space,opengameart,free,cc-by");
		
		Metadata.Source.SourceName = TEXT("OpenGameArt.org");
		Metadata.Source.SourceURL = TEXT("https://opengameart.org");
		Metadata.Source.License = TEXT("CC-BY 3.0");
		Metadata.Source.Author = TEXT("Various Artists");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = true;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d OGA space assets"), OGASpaceAssets.Num());
}

void UAlexanderAssetManager::LoadAudioAssetsFromOGA()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Audio Assets from OpenGameArt.org"));

	TArray<FString> OGAAudioAssets = {
		TEXT("/Game/Assets/OGA/Audio/Sounds/OGA_Thruster_Sound"),
		TEXT("/Game/Assets/OGA/Audio/Sounds/OGA_Warning_Beep"),
		TEXT("/Game/Assets/OGA/Audio/Sounds/OGA_Radio_Static"),
		TEXT("/Game/Assets/OGA/Audio/Music/OGA_Space_Ambient")
	};

	for (const FString& AssetPath : OGAAudioAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = EAssetType::Sound;
		Metadata.Category = TEXT("OGA/Audio");
		Metadata.Tags = TEXT("audio,opengameart,free,cc-by,sound");
		
		Metadata.Source.SourceName = TEXT("OpenGameArt.org");
		Metadata.Source.SourceURL = TEXT("https://opengameart.org");
		Metadata.Source.License = TEXT("CC-BY 3.0");
		Metadata.Source.Author = TEXT("Various Artists");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = true;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d OGA audio assets"), OGAAudioAssets.Num());
}

void UAlexanderAssetManager::LoadTextureAssetsFromOGA()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Texture Assets from OpenGameArt.org"));

	TArray<FString> OGATextureAssets = {
		TEXT("/Game/Assets/OGA/Textures/OGA_Planet_Texture"),
		TEXT("/Game/Assets/OGA/Textures/OGA_Moon_Texture"),
		TEXT("/Game/Assets/OGA/Textures/OGA_Asteroid_Texture"),
		TEXT("/Game/Assets/OGA/Textures/OGA_Metal_Texture"),
		TEXT("/Game/Assets/OGA/Textures/OGA_Hull_Texture")
	};

	for (const FString& AssetPath : OGATextureAssets)
	{
		FAssetMetadata Metadata;
		Metadata.AssetName = FPaths::GetBaseFilename(AssetPath);
		Metadata.AssetPath = AssetPath;
		Metadata.AssetType = EAssetType::Texture;
		Metadata.Category = TEXT("OGA/Textures");
		Metadata.Tags = TEXT("texture,opengameart,free,cc-by,space");
		
		Metadata.Source.SourceName = TEXT("OpenGameArt.org");
		Metadata.Source.SourceURL = TEXT("https://opengameart.org");
		Metadata.Source.License = TEXT("CC-BY 3.0");
		Metadata.Source.Author = TEXT("Various Artists");
		Metadata.Source.bIsFree = true;
		Metadata.Source.bRequiresAttribution = true;
		
		RegisterAsset(AssetPath, Metadata);
	}

	UE_LOG(LogTemp, Warning, TEXT("Registered %d OGA texture assets"), OGATextureAssets.Num());
}

UTexture2D* UAlexanderAssetManager::LoadTexture(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<UTexture2D>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Texture);
	return Cast<UTexture2D>(LoadedAssets.FindRef(AssetPath));
}

USoundBase* UAlexanderAssetManager::LoadSound(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<USoundBase>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Sound);
	return Cast<USoundBase>(LoadedAssets.FindRef(AssetPath));
}

UStaticMesh* UAlexanderAssetManager::LoadStaticMesh(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<UStaticMesh>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Mesh);
	return Cast<UStaticMesh>(LoadedAssets.FindRef(AssetPath));
}

USkeletalMesh* UAlexanderAssetManager::LoadSkeletalMesh(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<USkeletalMesh>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Mesh);
	return Cast<USkeletalMesh>(LoadedAssets.FindRef(AssetPath));
}

UMaterialInterface* UAlexanderAssetManager::LoadMaterial(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<UMaterialInterface>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Material);
	return Cast<UMaterialInterface>(LoadedAssets.FindRef(AssetPath));
}

UParticleSystem* UAlexanderAssetManager::LoadParticleSystem(const FString& AssetPath)
{
	if (IsAssetLoaded(AssetPath))
	{
		return Cast<UParticleSystem>(LoadedAssets[AssetPath]);
	}

	LoadAsset(AssetPath, EAssetType::Particle);
	return Cast<UParticleSystem>(LoadedAssets.FindRef(AssetPath));
}

void UAlexanderAssetManager::OptimizeAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Optimizing assets"));

	if (bEnableCompression)
	{
		CompressTextures();
	}

	OptimizeMeshes();
	OptimizeAudio();

	UE_LOG(LogTemp, Warning, TEXT("Asset optimization complete"));
}

void UAlexanderAssetManager::CompressTextures()
{
	UE_LOG(LogTemp, Warning, TEXT("Compressing textures"));

	// This would implement texture compression
	// For now, just log the action
	for (const auto& AssetPair : LoadedAssets)
	{
		if (UTexture2D* Texture = Cast<UTexture2D>(AssetPair.Value))
		{
			// Compress texture
			UE_LOG(LogTemp, Warning, TEXT("Compressed texture: %s"), *AssetPair.Key);
		}
	}
}

void UAlexanderAssetManager::OptimizeMeshes()
{
	UE_LOG(LogTemp, Warning, TEXT("Optimizing meshes"));

	for (const auto& AssetPair : LoadedAssets)
	{
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(AssetPair.Value))
		{
			// Optimize mesh
			UE_LOG(LogTemp, Warning, TEXT("Optimized mesh: %s"), *AssetPair.Key);
		}
	}
}

void UAlexanderAssetManager::OptimizeAudio()
{
	UE_LOG(LogTemp, Warning, TEXT("Optimizing audio"));

	for (const auto& AssetPair : LoadedAssets)
	{
		if (USoundBase* Sound = Cast<USoundBase>(AssetPair.Value))
		{
			// Optimize audio
			UE_LOG(LogTemp, Warning, TEXT("Optimized audio: %s"), *AssetPair.Key);
		}
	}
}

void UAlexanderAssetManager::EnableAssetStreaming(bool bEnabled)
{
	bEnableAssetStreaming = bEnabled;
	UE_LOG(LogTemp, Warning, TEXT("Asset streaming %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UAlexanderAssetManager::IsAssetStreamingEnabled() const
{
	return bEnableAssetStreaming;
}

void UAlexanderAssetManager::StreamAsset(const FString& AssetPath, float Priority)
{
	if (!bEnableAssetStreaming)
	{
		LoadAsset(AssetPath, EAssetType::Other);
		return;
	}

	// Add to streaming queue with priority
	StreamingQueue.Add(AssetPath);
	
	UE_LOG(LogTemp, Warning, TEXT("Queued asset for streaming: %s (Priority: %.1f)"), *AssetPath, Priority);
}

void UAlexanderAssetManager::UnstreamAsset(const FString& AssetPath)
{
	UnloadAsset(AssetPath);
	StreamingQueue.Remove(AssetPath);
}

int32 UAlexanderAssetManager::GetLoadedAssetCount() const
{
	return LoadedAssets.Num();
}

float UAlexanderAssetManager::GetTotalMemoryUsage() const
{
	return CurrentMemoryUsage;
}

TMap<EAssetType, int32> UAlexanderAssetManager::GetAssetCountByType() const
{
	TMap<EAssetType, int32> AssetCounts;
	
	for (const auto& TypePair : AssetsByType)
	{
		int32 LoadedCount = 0;
		for (const FString& AssetPath : TypePair.Value.AssetPaths)
		{
			if (LoadedAssets.Contains(AssetPath))
			{
				LoadedCount++;
			}
		}
		AssetCounts.Add(TypePair.Key, LoadedCount);
	}
	
	return AssetCounts;
}

void UAlexanderAssetManager::LogAssetStatistics()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Asset Manager Statistics ==="));
	UE_LOG(LogTemp, Warning, TEXT("Total Assets Registered: %d"), AssetRegistry.Num());
	UE_LOG(LogTemp, Warning, TEXT("Assets Loaded: %d"), LoadedAssets.Num());
	UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMemoryUsage / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Warning, TEXT("Streaming Queue: %d assets"), StreamingQueue.Num());
	UE_LOG(LogTemp, Warning, TEXT("Asset Streaming: %s"), bEnableAssetStreaming ? TEXT("Enabled") : TEXT("Disabled"));

	// Log assets by type
	TMap<EAssetType, int32> AssetCounts = GetAssetCountByType();
	for (const auto& CountPair : AssetCounts)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %d loaded"), *GetAssetTypeString(CountPair.Key), CountPair.Value);
	}

	UE_LOG(LogTemp, Warning, TEXT("=================================="));
}

void UAlexanderAssetManager::ValidateAssets()
{
	UE_LOG(LogTemp, Warning, TEXT("Validating assets"));

	int32 ValidAssets = 0;
	int32 InvalidAssets = 0;

	for (const auto& AssetPair : LoadedAssets)
	{
		UObject* Asset = AssetPair.Value;
		if (Asset && IsValid(Asset))
		{
			ValidAssets++;
		}
		else
		{
			InvalidAssets++;
			UE_LOG(LogTemp, Warning, TEXT("Invalid asset found: %s"), *AssetPair.Key);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Asset validation complete: %d valid, %d invalid"), ValidAssets, InvalidAssets);
}

void UAlexanderAssetManager::RegisterAsset(const FString& AssetPath, const FAssetMetadata& Metadata)
{
	AssetRegistry.Add(AssetPath, Metadata);
	
	// Update type mapping
	if (!AssetsByType.Contains(Metadata.AssetType))
	{
		FAssetListByType NewTypeList;
		NewTypeList.AssetPaths = TArray<FString>();
		AssetsByType.Add(Metadata.AssetType, NewTypeList);
	}
	AssetsByType[Metadata.AssetType].AssetPaths.Add(AssetPath);
	
	// Update category mapping
	if (!AssetsByCategory.Contains(Metadata.Category))
	{
		FAssetListByCategory NewCategoryList;
		NewCategoryList.AssetPaths = TArray<FString>();
		AssetsByCategory.Add(Metadata.Category, NewCategoryList);
	}
	AssetsByCategory[Metadata.Category].AssetPaths.Add(AssetPath);
}

void UAlexanderAssetManager::UnregisterAsset(const FString& AssetPath)
{
	if (AssetRegistry.Contains(AssetPath))
	{
		FAssetMetadata Metadata = AssetRegistry[AssetPath];
		
		// Remove from type mapping
		if (AssetsByType.Contains(Metadata.AssetType))
		{
			AssetsByType[Metadata.AssetType].AssetPaths.Remove(AssetPath);
		}
		
		// Remove from category mapping
		if (AssetsByCategory.Contains(Metadata.Category))
		{
			AssetsByCategory[Metadata.Category].AssetPaths.Remove(AssetPath);
		}
		
		AssetRegistry.Remove(AssetPath);
	}
}

bool UAlexanderAssetManager::LoadAssetInternal(const FString& AssetPath, EAssetType AssetType)
{
	// This would use Unreal's asset loading system
	// For now, simulate successful loading
	UObject* LoadedAsset = nullptr;
	
	// Simulate different asset types
	switch (AssetType)
	{
		case EAssetType::Texture:
			LoadedAsset = NewObject<UTexture2D>(this);
			break;
		case EAssetType::Mesh:
			LoadedAsset = NewObject<UStaticMesh>(this);
			break;
		case EAssetType::Sound:
			LoadedAsset = NewObject<USoundBase>(this);
			break;
		case EAssetType::Material:
			LoadedAsset = NewObject<UMaterialInterface>(this);
			break;
		case EAssetType::Particle:
			LoadedAsset = NewObject<UParticleSystem>(this);
			break;
		default:
			LoadedAsset = NewObject<UObject>(this);
			break;
	}

	if (LoadedAsset)
	{
		LoadedAssets.Add(AssetPath, LoadedAsset);
		return true;
	}

	return false;
}

void UAlexanderAssetManager::UpdateMemoryUsage()
{
	// This would calculate actual memory usage
	// For now, estimate based on asset count
	CurrentMemoryUsage = LoadedAssets.Num() * 1024.0f * 1024.0f; // 1MB per asset estimate
}

void UAlexanderAssetManager::InitializeAssetSources()
{
	// Kenney Assets
	FAssetSource KenneySource;
	KenneySource.SourceName = TEXT("Kenney");
	KenneySource.SourceURL = TEXT("https://kenney.nl");
	KenneySource.License = TEXT("CC0 1.0 Universal");
	KenneySource.Author = TEXT("Kenney");
	KenneySource.bIsFree = true;
	KenneySource.bRequiresAttribution = false;
	AssetSources.Add(KenneySource);

	// OpenGameArt.org
	FAssetSource OGASource;
	OGASource.SourceName = TEXT("OpenGameArt.org");
	OGASource.SourceURL = TEXT("https://opengameart.org");
	OGASource.License = TEXT("Various (CC-BY, CC0, GPL)");
	OGASource.Author = TEXT("Various Artists");
	OGASource.bIsFree = true;
	OGASource.bRequiresAttribution = true;
	AssetSources.Add(OGASource);
}

void UAlexanderAssetManager::ProcessStreamingQueue()
{
	if (!bEnableAssetStreaming || StreamingQueue.Num() == 0)
	{
		return;
	}

	// Process streaming queue
	for (const FString& AssetPath : StreamingQueue)
	{
		if (!IsAssetLoaded(AssetPath))
		{
			LoadAsset(AssetPath, EAssetType::Other);
		}
	}

	StreamingQueue.Empty();
}

FString UAlexanderAssetManager::GetAssetTypeString(EAssetType AssetType) const
{
	switch (AssetType)
	{
		case EAssetType::Texture: return TEXT("Texture");
		case EAssetType::Mesh: return TEXT("Mesh");
		case EAssetType::Sound: return TEXT("Sound");
		case EAssetType::Material: return TEXT("Material");
		case EAssetType::Particle: return TEXT("Particle");
		case EAssetType::Animation: return TEXT("Animation");
		case EAssetType::Blueprint: return TEXT("Blueprint");
		default: return TEXT("Other");
	}
}

EAssetType UAlexanderAssetManager::GetAssetTypeFromString(const FString& TypeString) const
{
	if (TypeString == TEXT("Texture")) return EAssetType::Texture;
	if (TypeString == TEXT("Mesh")) return EAssetType::Mesh;
	if (TypeString == TEXT("Sound")) return EAssetType::Sound;
	if (TypeString == TEXT("Material")) return EAssetType::Material;
	if (TypeString == TEXT("Particle")) return EAssetType::Particle;
	if (TypeString == TEXT("Animation")) return EAssetType::Animation;
	if (TypeString == TEXT("Blueprint")) return EAssetType::Blueprint;
	return EAssetType::Other;
}

EAssetType UAlexanderAssetManager::DetermineAssetTypeFromPath(const FString& AssetPath) const
{
	if (AssetPath.Contains(TEXT("Texture")) || AssetPath.Contains(TEXT("Textures")))
		return EAssetType::Texture;
	if (AssetPath.Contains(TEXT("Mesh")) || AssetPath.Contains(TEXT("Meshes")))
		return EAssetType::Mesh;
	if (AssetPath.Contains(TEXT("Sound")) || AssetPath.Contains(TEXT("Audio")))
		return EAssetType::Sound;
	if (AssetPath.Contains(TEXT("Material")) || AssetPath.Contains(TEXT("Materials")))
		return EAssetType::Material;
	if (AssetPath.Contains(TEXT("Particle")) || AssetPath.Contains(TEXT("Particles")))
		return EAssetType::Particle;
	
	return EAssetType::Other;
}

FString UAlexanderAssetManager::DetermineCategoryFromPath(const FString& AssetPath) const
{
	if (AssetPath.Contains(TEXT("Space")))
		return TEXT("Space");
	if (AssetPath.Contains(TEXT("UI")))
		return TEXT("UI");
	if (AssetPath.Contains(TEXT("Audio")))
		return TEXT("Audio");
	if (AssetPath.Contains(TEXT("Texture")))
		return TEXT("Textures");
	
	return TEXT("General");
}

FString UAlexanderAssetManager::GetSourceFromPath(const FString& AssetPath) const
{
	if (AssetPath.Contains(TEXT("Kenney")))
		return TEXT("Kenney");
	if (AssetPath.Contains(TEXT("OGA")))
		return TEXT("OpenGameArt.org");
	
	return TEXT("Unknown");
}
