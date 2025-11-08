// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceStationBlueprintLibrary.h"
#include "SpaceAssetManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// Initialize static member
TMap<EStationModuleType, FStationModuleMeshArray> USpaceStationBlueprintLibrary::RegisteredModules;

ASpaceStationHub* USpaceStationBlueprintLibrary::SpawnStationFromConfig(
	UObject* WorldContextObject,
	const FStationConfiguration& Config,
	const FTransform& SpawnTransform)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnStationFromConfig: Invalid WorldContextObject"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnStationFromConfig: Failed to get World"));
		return nullptr;
	}

	// Validate configuration
	FText ErrorMessage;
	if (!ValidateStationConfiguration(Config, ErrorMessage))
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnStationFromConfig: Invalid configuration - %s"), *ErrorMessage.ToString());
		return nullptr;
	}

	// Spawn station hub actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*Config.StationID.ToString());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASpaceStationHub* Station = World->SpawnActor<ASpaceStationHub>(
		ASpaceStationHub::StaticClass(),
		SpawnTransform,
		SpawnParams
	);

	if (Station)
	{
		// Configure station properties
		// Note: These would need to be added to SpaceStationHub if they don't exist
		UE_LOG(LogTemp, Log, TEXT("Spawned station '%s' at %s"), 
			*Config.StationName.ToString(), 
			*SpawnTransform.GetLocation().ToString());

		// Calculate and add docking points
		TArray<FTransform> DockingPoints = CalculateDockingPoints(Config, 1500.0f);
		UE_LOG(LogTemp, Log, TEXT("Generated %d docking points for station"), DockingPoints.Num());

		// Add modules based on configuration
		for (EStationModuleType ModuleType : Config.Modules)
		{
			TArray<UStaticMesh*> AvailableModules = GetStationModulesByType(WorldContextObject, ModuleType);
			if (AvailableModules.Num() > 0)
			{
				// Use first available module (could be randomized)
				UStaticMesh* ModuleMesh = AvailableModules[0];
				
				// Calculate relative position based on module type
				FTransform RelativeTransform = FTransform::Identity;
				// Position modules around the hub (simplified positioning)
				int32 ModuleIndex = static_cast<int32>(ModuleType);
				float Angle = (360.0f / 8.0f) * ModuleIndex;
				float RadAngle = FMath::DegreesToRadians(Angle);
				FVector Offset(FMath::Cos(RadAngle) * 500.0f, FMath::Sin(RadAngle) * 500.0f, 0.0f);
				RelativeTransform.SetLocation(Offset);

				AddModuleToStation(Station, ModuleMesh, RelativeTransform, ModuleType);
			}
		}
	}

	return Station;
}

bool USpaceStationBlueprintLibrary::RegisterStationAsset(
	UObject* WorldContextObject,
	const FStationAssetRegistration& Registration)
{
	if (!Registration.StationMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("RegisterStationAsset: Invalid mesh"));
		return false;
	}

	// Add to internal registry
	if (!RegisteredModules.Contains(Registration.ModuleType))
	{
		RegisteredModules.Add(Registration.ModuleType);
	}

	RegisteredModules[Registration.ModuleType].Meshes.AddUnique(Registration.StationMesh);

	// Also register with SpaceAssetManager if available
	USpaceAssetManager* AssetManager = GetAssetManager(WorldContextObject);
	if (AssetManager)
	{
		// SpaceAssetManager should have a method to register station assets
		UE_LOG(LogTemp, Log, TEXT("Registered station module '%s' of type %d"), 
			*Registration.StationMesh->GetName(), 
			static_cast<int32>(Registration.ModuleType));
	}

	return true;
}

TArray<UStaticMesh*> USpaceStationBlueprintLibrary::GetStationModulesByType(
	UObject* WorldContextObject,
	EStationModuleType ModuleType)
{
	if (RegisteredModules.Contains(ModuleType))
	{
		return RegisteredModules[ModuleType].Meshes;
	}

	return TArray<UStaticMesh*>();
}

bool USpaceStationBlueprintLibrary::AddModuleToStation(
	ASpaceStationHub* Station,
	UStaticMesh* ModuleMesh,
	const FTransform& RelativeTransform,
	EStationModuleType ModuleType)
{
	if (!Station || !ModuleMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("AddModuleToStation: Invalid parameters"));
		return false;
	}

	// Create a new static mesh component for the module
	UStaticMeshComponent* ModuleComponent = NewObject<UStaticMeshComponent>(Station);
	if (ModuleComponent)
	{
		ModuleComponent->SetStaticMesh(ModuleMesh);
		ModuleComponent->SetupAttachment(Station->GetRootComponent());
		ModuleComponent->SetRelativeTransform(RelativeTransform);
		ModuleComponent->RegisterComponent();

		// Set collision and physics properties
		ModuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ModuleComponent->SetCollisionResponseToAllChannels(ECR_Block);

		UE_LOG(LogTemp, Log, TEXT("Added module of type %d to station at %s"), 
			static_cast<int32>(ModuleType), 
			*RelativeTransform.GetLocation().ToString());

		return true;
	}

	return false;
}

TArray<FTransform> USpaceStationBlueprintLibrary::CalculateDockingPoints(
	const FStationConfiguration& Config,
	float Radius)
{
	TArray<FTransform> DockingPoints;

	if (Config.MaxDockingSlots <= 0)
	{
		return DockingPoints;
	}

	// Distribute docking points evenly around the station
	float AngleIncrement = 360.0f / Config.MaxDockingSlots;

	for (int32 i = 0; i < Config.MaxDockingSlots; ++i)
	{
		float Angle = AngleIncrement * i;
		float RadAngle = FMath::DegreesToRadians(Angle);

		// Calculate position on circle
		FVector Location(
			FMath::Cos(RadAngle) * Radius,
			FMath::Sin(RadAngle) * Radius,
			0.0f
		);

		// Calculate rotation facing inward
		FRotator Rotation(0.0f, Angle + 180.0f, 0.0f);

		FTransform DockingPoint;
		DockingPoint.SetLocation(Location);
		DockingPoint.SetRotation(Rotation.Quaternion());
		DockingPoint.SetScale3D(FVector::OneVector);

		DockingPoints.Add(DockingPoint);
	}

	UE_LOG(LogTemp, Log, TEXT("Calculated %d docking points at radius %.1f"), 
		DockingPoints.Num(), Radius);

	return DockingPoints;
}

bool USpaceStationBlueprintLibrary::ValidateStationConfiguration(
	const FStationConfiguration& Config,
	FText& OutErrorMessage)
{
	if (Config.StationID == NAME_None)
	{
		OutErrorMessage = FText::FromString(TEXT("Station ID cannot be empty"));
		return false;
	}

	if (Config.StationName.IsEmpty())
	{
		OutErrorMessage = FText::FromString(TEXT("Station name cannot be empty"));
		return false;
	}

	if (Config.MaxDockingSlots <= 0)
	{
		OutErrorMessage = FText::FromString(TEXT("Station must have at least one docking slot"));
		return false;
	}

	if (Config.MaxPlayerCapacity <= 0)
	{
		OutErrorMessage = FText::FromString(TEXT("Station must have positive player capacity"));
		return false;
	}

	if (Config.Modules.Num() == 0)
	{
		OutErrorMessage = FText::FromString(TEXT("Station must have at least one module"));
		return false;
	}

	// Check for required core module
	if (!Config.Modules.Contains(EStationModuleType::HubCore))
	{
		OutErrorMessage = FText::FromString(TEXT("Station must have a Hub Core module"));
		return false;
	}

	OutErrorMessage = FText::GetEmpty();
	return true;
}

FVector USpaceStationBlueprintLibrary::GetEstimatedStationBounds(const FStationConfiguration& Config)
{
	// Base size for hub core
	FVector BaseSize(2000.0f, 2000.0f, 1000.0f);

	// Add size based on module count
	float ModuleSizeMultiplier = 1.0f + (Config.Modules.Num() * 0.2f);

	// Add size based on docking slots
	float DockingSizeMultiplier = 1.0f + (Config.MaxDockingSlots * 0.05f);

	FVector EstimatedBounds = BaseSize * ModuleSizeMultiplier * DockingSizeMultiplier;

	return EstimatedBounds;
}

FStationConfiguration USpaceStationBlueprintLibrary::CreateDefaultStationConfig(const FText& StationName)
{
	FStationConfiguration Config;
	Config.StationID = FName(*StationName.ToString().Replace(TEXT(" "), TEXT("_")));
	Config.StationName = StationName;
	
	// Add default modules
	Config.Modules.Add(EStationModuleType::HubCore);
	Config.Modules.Add(EStationModuleType::DockingRing);
	Config.Modules.Add(EStationModuleType::TradingPost);
	Config.Modules.Add(EStationModuleType::SocialArea);
	Config.Modules.Add(EStationModuleType::MissionBoard);

	Config.MaxDockingSlots = 12;
	Config.MaxPlayerCapacity = 100;
	Config.bHasTrading = true;
	Config.bHasMissions = true;
	Config.bHasVoiceChat = true;

	return Config;
}

USpaceAssetManager* USpaceStationBlueprintLibrary::GetAssetManager(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Get SpaceAssetManager singleton
	USpaceAssetManager* AssetManager = Cast<USpaceAssetManager>(GEngine->AssetManager);
	
	return AssetManager;
}
