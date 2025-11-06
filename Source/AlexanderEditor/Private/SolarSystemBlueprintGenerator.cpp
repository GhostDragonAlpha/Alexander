// Copyright Epic Games, Inc. All Rights Reserved.

#include "SolarSystemBlueprintGenerator.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "EdGraphSchema_K2.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PackageTools.h"

// Include celestial scaling components
#include "CelestialScaling/Public/CelestialBodyComponent.h"
#include "Public/VRSpaceshipPawn.h"

USolarSystemBlueprintGenerator::USolarSystemBlueprintGenerator()
{
	BaseBlueprintPath = TEXT("/Game/Blueprints/CelestialBodies");
	BaseMaterialPath = TEXT("/Game/Materials");
	BaseWidgetPath = TEXT("/Game/UI/Widgets");
	TotalSteps = 0;
	CurrentStep = 0;
}

bool USolarSystemBlueprintGenerator::GenerateAllBlueprints()
{
	LogProgress(TEXT("=== Starting Solar System Blueprint Generation ==="));

	TotalSteps = 13; // 5 celestial + 1 game mode + 3 widgets + 4 materials
	CurrentStep = 0;

	bool bSuccess = true;

	// Phase 1: Generate celestial body Blueprints
	LogProgress(TEXT("\n[Phase 1/4] Generating Celestial Body Blueprints..."));
	bSuccess &= GenerateCelestialBodyBlueprints();

	// Phase 2: Generate game mode
	LogProgress(TEXT("\n[Phase 2/4] Generating Game Mode Blueprint..."));
	bSuccess &= GenerateGameModeBlueprint();

	// Phase 3: Generate UI widgets
	LogProgress(TEXT("\n[Phase 3/4] Generating UI Widget Blueprints..."));
	bSuccess &= GenerateUIWidgetBlueprints();

	// Phase 4: Generate materials
	LogProgress(TEXT("\n[Phase 4/4] Generating Materials..."));
	bSuccess &= GenerateMaterials();

	// Summary
	LogProgress(TEXT("\n=== Generation Complete ==="));
	LogProgress(FString::Printf(TEXT("Created Assets: %d"), CreatedAssets.Num()));
	LogProgress(FString::Printf(TEXT("Failed Assets: %d"), FailedAssets.Num()));

	if (FailedAssets.Num() > 0)
	{
		LogError(TEXT("Failed to create the following assets:"));
		for (const FString& Failed : FailedAssets)
		{
			LogError(FString::Printf(TEXT("  - %s"), *Failed));
		}
	}

	return bSuccess && (FailedAssets.Num() == 0);
}

bool USolarSystemBlueprintGenerator::GenerateCelestialBodyBlueprints()
{
	bool bSuccess = true;

	// Create BP_CelestialBodyBase first (parent class)
	UBlueprint* BaseBlueprint = CreateBP_CelestialBodyBase();
	bSuccess &= (BaseBlueprint != nullptr);

	if (!bSuccess)
	{
		LogError(TEXT("Failed to create BP_CelestialBodyBase - cannot continue with child Blueprints"));
		return false;
	}

	// Create child Blueprints
	UBlueprint* SunBlueprint = CreateBP_Sun();
	bSuccess &= (SunBlueprint != nullptr);

	UBlueprint* EarthBlueprint = CreateBP_Earth();
	bSuccess &= (EarthBlueprint != nullptr);

	UBlueprint* MoonBlueprint = CreateBP_Moon();
	bSuccess &= (MoonBlueprint != nullptr);

	UBlueprint* MarsBlueprint = CreateBP_Mars();
	bSuccess &= (MarsBlueprint != nullptr);

	return bSuccess;
}

bool USolarSystemBlueprintGenerator::GenerateGameModeBlueprints()
{
	UBlueprint* GameModeBlueprint = CreateBP_SolarSystemGameMode();
	return (GameModeBlueprint != nullptr);
}

bool USolarSystemBlueprintGenerator::GenerateUIWidgetBlueprints()
{
	bool bSuccess = true;

	UBlueprint* ChronometerWidget = CreateWBP_Chronometer();
	bSuccess &= (ChronometerWidget != nullptr);

	UBlueprint* DebugOverlayWidget = CreateWBP_DebugOverlay();
	bSuccess &= (DebugOverlayWidget != nullptr);

	UBlueprint* TimeWarpWarningWidget = CreateWBP_TimeWarpWarning();
	bSuccess &= (TimeWarpWarningWidget != nullptr);

	return bSuccess;
}

bool USolarSystemBlueprintGenerator::GenerateMaterials()
{
	bool bSuccess = true;

	UMaterial* SunMaterial = CreateM_Sun_Emissive();
	bSuccess &= (SunMaterial != nullptr);

	UMaterial* EarthMaterial = CreateM_Earth_Simple();
	bSuccess &= (EarthMaterial != nullptr);

	UMaterial* MoonMaterial = CreateM_Moon_Simple();
	bSuccess &= (MoonMaterial != nullptr);

	UMaterial* MarsMaterial = CreateM_Mars_Simple();
	bSuccess &= (MarsMaterial != nullptr);

	return bSuccess;
}

// ============================================================================
// Blueprint Creation Core Functions
// ============================================================================

UBlueprint* USolarSystemBlueprintGenerator::CreateBlueprintAsset(
	const FString& AssetName,
	const FString& PackagePath,
	UClass* ParentClass)
{
	LogProgress(FString::Printf(TEXT("Creating Blueprint: %s"), *AssetName));
	CurrentStep++;

	if (!ParentClass)
	{
		LogError(FString::Printf(TEXT("Parent class is null for %s"), *AssetName));
		FailedAssets.Add(AssetName);
		return nullptr;
	}

	// Create package
	FString FullPackagePath = FString::Printf(TEXT("%s/%s"), *PackagePath, *AssetName);
	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
	{
		LogError(FString::Printf(TEXT("Failed to create package for %s"), *AssetName));
		FailedAssets.Add(AssetName);
		return nullptr;
	}

	// Create Blueprint factory
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClass;

	// Create the Blueprint
	UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
		UBlueprint::StaticClass(),
		Package,
		*AssetName,
		RF_Standalone | RF_Public,
		nullptr,
		GWarn
	));

	if (!NewBlueprint)
	{
		LogError(FString::Printf(TEXT("Failed to create Blueprint %s"), *AssetName));
		FailedAssets.Add(AssetName);
		return nullptr;
	}

	// Mark package as dirty and save
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewBlueprint);

	CreatedAssets.Add(AssetName);
	LogProgress(FString::Printf(TEXT("✓ Created %s"), *AssetName));

	return NewBlueprint;
}

bool USolarSystemBlueprintGenerator::AddComponentToBlueprint(
	UBlueprint* Blueprint,
	UClass* ComponentClass,
	const FName& ComponentName,
	USceneComponent* AttachTo)
{
	if (!Blueprint || !ComponentClass)
	{
		return false;
	}

	// Get the Simple Construction Script
	USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
	if (!SCS)
	{
		LogError(FString::Printf(TEXT("No SCS found for Blueprint %s"), *Blueprint->GetName()));
		return false;
	}

	// Create new SCS node
	USCS_Node* NewNode = SCS->CreateNode(ComponentClass, ComponentName);
	if (!NewNode)
	{
		LogError(FString::Printf(TEXT("Failed to create SCS node for component %s"), *ComponentName.ToString()));
		return false;
	}

	// Attach to parent or set as root
	if (AttachTo)
	{
		// Find the parent node
		for (USCS_Node* Node : SCS->GetAllNodes())
		{
			if (Node->ComponentTemplate == AttachTo)
			{
				Node->AddChildNode(NewNode);
				break;
			}
		}
	}
	else
	{
		// Set as root component
		SCS->AddNode(NewNode);
		if (SCS->GetDefaultSceneRootNode() == nullptr)
		{
			SCS->SetDefaultSceneRootNode(NewNode);
		}
	}

	return true;
}

bool USolarSystemBlueprintGenerator::CompileBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}

	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	if (Blueprint->Status == BS_Error)
	{
		LogError(FString::Printf(TEXT("Blueprint %s failed to compile"), *Blueprint->GetName()));
		return false;
	}

	return true;
}

// ============================================================================
// Celestial Body Blueprint Creation
// ============================================================================

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_CelestialBodyBase()
{
	// Create Blueprint with AActor as parent
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_CelestialBodyBase"),
		BaseBlueprintPath,
		AActor::StaticClass()
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// Add CelestialBodyComponent
	AddComponentToBlueprint(
		Blueprint,
		UCelestialBodyComponent::StaticClass(),
		TEXT("CelestialBodyComponent"),
		nullptr
	);

	// Add VisualMesh (StaticMeshComponent)
	AddComponentToBlueprint(
		Blueprint,
		UStaticMeshComponent::StaticClass(),
		TEXT("VisualMesh"),
		nullptr
	);

	// Add GravityZone (SphereComponent)
	AddComponentToBlueprint(
		Blueprint,
		USphereComponent::StaticClass(),
		TEXT("GravityZone"),
		nullptr
	);

	// TODO: Add Construction Script logic (requires Blueprint graph manipulation)
	// This is complex and requires creating K2Nodes for the graph

	// Compile Blueprint
	if (!CompileBlueprint(Blueprint))
	{
		LogError(TEXT("BP_CelestialBodyBase failed to compile"));
		return nullptr;
	}

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_Sun()
{
	// First, load BP_CelestialBodyBase as parent
	FString BaseClassPath = FString::Printf(TEXT("%s/BP_CelestialBodyBase"), *BaseBlueprintPath);
	UBlueprint* ParentBlueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BaseClassPath));

	if (!ParentBlueprint)
	{
		LogError(TEXT("Could not load BP_CelestialBodyBase for BP_Sun"));
		FailedAssets.Add(TEXT("BP_Sun"));
		return nullptr;
	}

	// Create child Blueprint
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_Sun"),
		BaseBlueprintPath,
		ParentBlueprint->GeneratedClass
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Set property defaults (Mass, Radius, etc.)
	// TODO: Add PointLight component
	// TODO: Set material reference

	// Compile
	CompileBlueprint(Blueprint);

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_Earth()
{
	FString BaseClassPath = FString::Printf(TEXT("%s/BP_CelestialBodyBase"), *BaseBlueprintPath);
	UBlueprint* ParentBlueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BaseClassPath));

	if (!ParentBlueprint)
	{
		LogError(TEXT("Could not load BP_CelestialBodyBase for BP_Earth"));
		FailedAssets.Add(TEXT("BP_Earth"));
		return nullptr;
	}

	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_Earth"),
		BaseBlueprintPath,
		ParentBlueprint->GeneratedClass
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Set Earth-specific properties
	CompileBlueprint(Blueprint);

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_Moon()
{
	FString BaseClassPath = FString::Printf(TEXT("%s/BP_CelestialBodyBase"), *BaseBlueprintPath);
	UBlueprint* ParentBlueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BaseClassPath));

	if (!ParentBlueprint)
	{
		LogError(TEXT("Could not load BP_CelestialBodyBase for BP_Moon"));
		FailedAssets.Add(TEXT("BP_Moon"));
		return nullptr;
	}

	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_Moon"),
		BaseBlueprintPath,
		ParentBlueprint->GeneratedClass
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Set Moon-specific properties
	CompileBlueprint(Blueprint);

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_Mars()
{
	FString BaseClassPath = FString::Printf(TEXT("%s/BP_CelestialBodyBase"), *BaseBlueprintPath);
	UBlueprint* ParentBlueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BaseClassPath));

	if (!ParentBlueprint)
	{
		LogError(TEXT("Could not load BP_CelestialBodyBase for BP_Mars"));
		FailedAssets.Add(TEXT("BP_Mars"));
		return nullptr;
	}

	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_Mars"),
		BaseBlueprintPath,
		ParentBlueprint->GeneratedClass
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Set Mars-specific properties
	CompileBlueprint(Blueprint);

	return Blueprint;
}

// ============================================================================
// Game Mode Blueprint Creation
// ============================================================================

UBlueprint* USolarSystemBlueprintGenerator::CreateBP_SolarSystemGameMode()
{
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("BP_SolarSystemGameMode"),
		TEXT("/Game/Blueprints/GameModes"),
		AGameModeBase::StaticClass()
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Add BeginPlay event graph logic
	// TODO: Set default pawn class
	// TODO: Add subsystem initialization logic

	CompileBlueprint(Blueprint);

	return Blueprint;
}

// ============================================================================
// UI Widget Blueprint Creation
// ============================================================================

UBlueprint* USolarSystemBlueprintGenerator::CreateWBP_Chronometer()
{
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("WBP_Chronometer"),
		BaseWidgetPath,
		UUserWidget::StaticClass()
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	// TODO: Add widget components (text blocks, progress bar)
	// TODO: Add binding logic

	CompileBlueprint(Blueprint);

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateWBP_DebugOverlay()
{
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("WBP_DebugOverlay"),
		BaseWidgetPath,
		UUserWidget::StaticClass()
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	CompileBlueprint(Blueprint);

	return Blueprint;
}

UBlueprint* USolarSystemBlueprintGenerator::CreateWBP_TimeWarpWarning()
{
	UBlueprint* Blueprint = CreateBlueprintAsset(
		TEXT("WBP_TimeWarpWarning"),
		BaseWidgetPath,
		UUserWidget::StaticClass()
	);

	if (!Blueprint)
	{
		return nullptr;
	}

	CompileBlueprint(Blueprint);

	return Blueprint;
}

// ============================================================================
// Material Creation
// ============================================================================

UMaterial* USolarSystemBlueprintGenerator::CreateM_Sun_Emissive()
{
	LogProgress(TEXT("Creating Material: M_Sun_Emissive"));
	CurrentStep++;

	FString PackagePath = FString::Printf(TEXT("%s/M_Sun_Emissive"), *BaseMaterialPath);
	UPackage* Package = CreatePackage(*PackagePath);

	if (!Package)
	{
		LogError(TEXT("Failed to create package for M_Sun_Emissive"));
		FailedAssets.Add(TEXT("M_Sun_Emissive"));
		return nullptr;
	}

	// Create material
	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(),
		Package,
		TEXT("M_Sun_Emissive"),
		RF_Standalone | RF_Public,
		nullptr,
		GWarn
	));

	if (!Material)
	{
		LogError(TEXT("Failed to create M_Sun_Emissive"));
		FailedAssets.Add(TEXT("M_Sun_Emissive"));
		return nullptr;
	}

	// Set shading model to Unlit
	Material->SetShadingModel(MSM_Unlit);

	// Create color constant (warm yellow)
	UMaterialExpressionConstant3Vector* ColorExpression = NewObject<UMaterialExpressionConstant3Vector>(Material);
	ColorExpression->Constant = FLinearColor(1.0f, 0.9f, 0.6f); // Warm yellow
	Material->GetExpressionCollection().AddExpression(ColorExpression);

	// Create emissive multiplier
	UMaterialExpressionConstant* MultiplierExpression = NewObject<UMaterialExpressionConstant>(Material);
	MultiplierExpression->R = 100.0f; // Emissive strength
	Material->GetExpressionCollection().AddExpression(MultiplierExpression);

	// Create multiply node
	UMaterialExpressionMultiply* MultiplyExpression = NewObject<UMaterialExpressionMultiply>(Material);
	Material->GetExpressionCollection().AddExpression(MultiplyExpression);

	// Connect nodes
	MultiplyExpression->A.Expression = ColorExpression;
	MultiplyExpression->B.Expression = MultiplierExpression;

	// Connect to material outputs
	Material->GetEditorOnlyData()->BaseColor.Expression = ColorExpression;
	Material->GetEditorOnlyData()->EmissiveColor.Expression = MultiplyExpression;

	// Compile material
	Material->PreEditChange(nullptr);
	Material->PostEditChange();

	// Save
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Material);

	CreatedAssets.Add(TEXT("M_Sun_Emissive"));
	LogProgress(TEXT("✓ Created M_Sun_Emissive"));

	return Material;
}

UMaterial* USolarSystemBlueprintGenerator::CreateM_Earth_Simple()
{
	// Similar to Sun material but with different colors and no emissive
	LogProgress(TEXT("Creating Material: M_Earth_Simple"));
	CurrentStep++;

	FString PackagePath = FString::Printf(TEXT("%s/M_Earth_Simple"), *BaseMaterialPath);
	UPackage* Package = CreatePackage(*PackagePath);

	if (!Package)
	{
		LogError(TEXT("Failed to create package for M_Earth_Simple"));
		FailedAssets.Add(TEXT("M_Earth_Simple"));
		return nullptr;
	}

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(),
		Package,
		TEXT("M_Earth_Simple"),
		RF_Standalone | RF_Public,
		nullptr,
		GWarn
	));

	if (!Material)
	{
		LogError(TEXT("Failed to create M_Earth_Simple"));
		FailedAssets.Add(TEXT("M_Earth_Simple"));
		return nullptr;
	}

	// Set to default lit
	Material->SetShadingModel(MSM_DefaultLit);

	// Create blue color
	UMaterialExpressionConstant3Vector* ColorExpression = NewObject<UMaterialExpressionConstant3Vector>(Material);
	ColorExpression->Constant = FLinearColor(0.1f, 0.3f, 0.8f); // Blue
	Material->GetExpressionCollection().AddExpression(ColorExpression);

	// Create roughness
	UMaterialExpressionConstant* RoughnessExpression = NewObject<UMaterialExpressionConstant>(Material);
	RoughnessExpression->R = 0.4f;
	Material->GetExpressionCollection().AddExpression(RoughnessExpression);

	// Connect to outputs
	Material->GetEditorOnlyData()->BaseColor.Expression = ColorExpression;
	Material->GetEditorOnlyData()->Roughness.Expression = RoughnessExpression;

	// Compile
	Material->PreEditChange(nullptr);
	Material->PostEditChange();

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Material);

	CreatedAssets.Add(TEXT("M_Earth_Simple"));
	LogProgress(TEXT("✓ Created M_Earth_Simple"));

	return Material;
}

UMaterial* USolarSystemBlueprintGenerator::CreateM_Moon_Simple()
{
	LogProgress(TEXT("Creating Material: M_Moon_Simple"));
	CurrentStep++;

	FString PackagePath = FString::Printf(TEXT("%s/M_Moon_Simple"), *BaseMaterialPath);
	UPackage* Package = CreatePackage(*PackagePath);

	if (!Package)
	{
		LogError(TEXT("Failed to create package for M_Moon_Simple"));
		FailedAssets.Add(TEXT("M_Moon_Simple"));
		return nullptr;
	}

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(),
		Package,
		TEXT("M_Moon_Simple"),
		RF_Standalone | RF_Public,
		nullptr,
		GWarn
	));

	if (!Material)
	{
		LogError(TEXT("Failed to create M_Moon_Simple"));
		FailedAssets.Add(TEXT("M_Moon_Simple"));
		return nullptr;
	}

	Material->SetShadingModel(MSM_DefaultLit);

	// Gray color
	UMaterialExpressionConstant3Vector* ColorExpression = NewObject<UMaterialExpressionConstant3Vector>(Material);
	ColorExpression->Constant = FLinearColor(0.4f, 0.4f, 0.4f); // Gray
	Material->GetExpressionCollection().AddExpression(ColorExpression);

	// High roughness (dusty)
	UMaterialExpressionConstant* RoughnessExpression = NewObject<UMaterialExpressionConstant>(Material);
	RoughnessExpression->R = 0.9f;
	Material->GetExpressionCollection().AddExpression(RoughnessExpression);

	Material->GetEditorOnlyData()->BaseColor.Expression = ColorExpression;
	Material->GetEditorOnlyData()->Roughness.Expression = RoughnessExpression;

	Material->PreEditChange(nullptr);
	Material->PostEditChange();

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Material);

	CreatedAssets.Add(TEXT("M_Moon_Simple"));
	LogProgress(TEXT("✓ Created M_Moon_Simple"));

	return Material;
}

UMaterial* USolarSystemBlueprintGenerator::CreateM_Mars_Simple()
{
	LogProgress(TEXT("Creating Material: M_Mars_Simple"));
	CurrentStep++;

	FString PackagePath = FString::Printf(TEXT("%s/M_Mars_Simple"), *BaseMaterialPath);
	UPackage* Package = CreatePackage(*PackagePath);

	if (!Package)
	{
		LogError(TEXT("Failed to create package for M_Mars_Simple"));
		FailedAssets.Add(TEXT("M_Mars_Simple"));
		return nullptr;
	}

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(
		UMaterial::StaticClass(),
		Package,
		TEXT("M_Mars_Simple"),
		RF_Standalone | RF_Public,
		nullptr,
		GWarn
	));

	if (!Material)
	{
		LogError(TEXT("Failed to create M_Mars_Simple"));
		FailedAssets.Add(TEXT("M_Mars_Simple"));
		return nullptr;
	}

	Material->SetShadingModel(MSM_DefaultLit);

	// Red-orange color
	UMaterialExpressionConstant3Vector* ColorExpression = NewObject<UMaterialExpressionConstant3Vector>(Material);
	ColorExpression->Constant = FLinearColor(0.8f, 0.4f, 0.2f); // Red-orange
	Material->GetExpressionCollection().AddExpression(ColorExpression);

	// Medium-high roughness
	UMaterialExpressionConstant* RoughnessExpression = NewObject<UMaterialExpressionConstant>(Material);
	RoughnessExpression->R = 0.85f;
	Material->GetExpressionCollection().AddExpression(RoughnessExpression);

	Material->GetEditorOnlyData()->BaseColor.Expression = ColorExpression;
	Material->GetEditorOnlyData()->Roughness.Expression = RoughnessExpression;

	Material->PreEditChange(nullptr);
	Material->PostEditChange();

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Material);

	CreatedAssets.Add(TEXT("M_Mars_Simple"));
	LogProgress(TEXT("✓ Created M_Mars_Simple"));

	return Material;
}

// ============================================================================
// Utility Functions
// ============================================================================

FString USolarSystemBlueprintGenerator::GetContentPath(const FString& SubPath) const
{
	return FString::Printf(TEXT("/Game/%s"), *SubPath);
}

void USolarSystemBlueprintGenerator::LogProgress(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Blueprint Generator] %s"), *Message);
}

void USolarSystemBlueprintGenerator::LogError(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("[Blueprint Generator] ERROR: %s"), *Message);
}
