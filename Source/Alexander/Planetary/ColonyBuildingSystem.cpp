// Fill out your copyright notice in the Description page of Project Settings.

#include "ColonyBuildingSystem.h"
#include "../Planetary/PlanetaryResourcesComponent.h"  // For FResourceAmount definition
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "NavigationSystem.h"
#include "Algo/Reverse.h"

// Sets default values
AColonyBuildingSystem::AColonyBuildingSystem()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Set replication
    bReplicates = true;
    bAlwaysRelevant = true;
    
    // Create root component
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    
    // Create building placement component
    BuildingPlacementComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildingPlacementComponent"));
    BuildingPlacementComponent->SetupAttachment(RootComponent);
    BuildingPlacementComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Create building mesh component
    BuildingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMeshComponent"));
    BuildingMeshComponent->SetupAttachment(RootComponent);
    BuildingMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Initialize building system state
    CurrentBuildingMode = EBuildingMode::None;
    bIsBuildingValid = false;
    bIsInBuildMode = false;
    CurrentBuildingType = EBuildingType::None;
    CurrentBuildingLevel = 1;
    PlacementGridSize = 100.0f;
    MaxBuildDistance = 2000.0f;
    ConstructionProgress = 0.0f;
    bIsUnderConstruction = false;
    
    // Initialize resource requirements
    RequiredResources.Empty();
    ConstructionTime = 0.0f;
    
    // Initialize building stats
    BuildingHealth = 100.0f;
    MaxBuildingHealth = 100.0f;
    PowerConsumption = 0.0f;
    PowerGeneration = 0.0f;
    PopulationCapacity = 0;
    StorageCapacity = 0;
    DefenseRating = 0.0f;
    
    // Initialize upgrade system
    MaxBuildingLevel = 5;
    UpgradeCostMultiplier = 1.5f;
    
    // Initialize building lists
    AvailableBuildings.Empty();
    ConstructedBuildings.Empty();
    BuildingQueue.Empty();
    BuildingGrid.Empty();
    
    // Initialize building effects
    BuildingEffects.Empty();
    ActiveEffects.Empty();
    
    // Initialize building statistics
    TotalBuildingsConstructed = 0;
    TotalResourcesSpent = 0;
    TotalConstructionTime = 0.0f;
    
    // Initialize building materials
    ValidPlacementMaterial = nullptr;
    InvalidPlacementMaterial = nullptr;
    ConstructionMaterial = nullptr;
    
    // Initialize building sounds
    ConstructionStartSound = nullptr;
    ConstructionCompleteSound = nullptr;
    BuildingDestroyedSound = nullptr;
    UpgradeSound = nullptr;
    
    // Initialize building particles
    ConstructionParticles = nullptr;
    DestructionParticles = nullptr;
    UpgradeParticles = nullptr;
    
    // Initialize building UI
    BuildingUIWidget = nullptr;
    bShowBuildingUI = false;
    
    // Initialize building AI
    bAutoRepair = false;
    bAutoUpgrade = false;
    RepairThreshold = 0.3f;
    UpgradePriority.Empty();
    
    // Initialize building events
    OnBuildingConstructionStarted.Clear();
    OnBuildingConstructionCompleted.Clear();
    OnBuildingUpgraded.Clear();
    OnBuildingDestroyed.Clear();
    OnBuildingDamaged.Clear();
    OnBuildingRepaired.Clear();
}

// Called when the game starts or when spawned
void AColonyBuildingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize building system
    if (HasAuthority())
    {
        InitializeBuildingSystem();
    }
    
    // Set up building placement
    SetupBuildingPlacement();
    
    // Initialize building grid
    InitializeBuildingGrid();
    
    // Load available buildings
    LoadAvailableBuildings();
}

// Called every frame
void AColonyBuildingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle building placement
    if (bIsInBuildMode && CurrentBuildingMode != EBuildingMode::None)
    {
        UpdateBuildingPlacement();
    }
    
    // Handle construction progress
    if (bIsUnderConstruction)
    {
        UpdateConstructionProgress(DeltaTime);
    }
    
    // Handle building effects
    UpdateBuildingEffects(DeltaTime);
    
    // Handle auto repair
    if (bAutoRepair && BuildingHealth < MaxBuildingHealth * RepairThreshold)
    {
        HandleAutoRepair(DeltaTime);
    }
    
    // Handle auto upgrade
    if (bAutoUpgrade)
    {
        HandleAutoUpgrade(DeltaTime);
    }
    
    // Update building statistics
    UpdateBuildingStatistics(DeltaTime);
}

// Replicated properties
void AColonyBuildingSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AColonyBuildingSystem, CurrentBuildingMode);
    DOREPLIFETIME(AColonyBuildingSystem, bIsBuildingValid);
    DOREPLIFETIME(AColonyBuildingSystem, bIsInBuildMode);
    DOREPLIFETIME(AColonyBuildingSystem, CurrentBuildingType);
    DOREPLIFETIME(AColonyBuildingSystem, CurrentBuildingLevel);
    DOREPLIFETIME(AColonyBuildingSystem, ConstructedBuildings);
    DOREPLIFETIME(AColonyBuildingSystem, BuildingQueue);
    DOREPLIFETIME(AColonyBuildingSystem, BuildingGrid);
    DOREPLIFETIME(AColonyBuildingSystem, ConstructionProgress);
    DOREPLIFETIME(AColonyBuildingSystem, bIsUnderConstruction);
    DOREPLIFETIME(AColonyBuildingSystem, BuildingHealth);
    DOREPLIFETIME(AColonyBuildingSystem, MaxBuildingHealth);
    DOREPLIFETIME(AColonyBuildingSystem, PowerConsumption);
    DOREPLIFETIME(AColonyBuildingSystem, PowerGeneration);
    DOREPLIFETIME(AColonyBuildingSystem, PopulationCapacity);
    DOREPLIFETIME(AColonyBuildingSystem, StorageCapacity);
    DOREPLIFETIME(AColonyBuildingSystem, DefenseRating);
    DOREPLIFETIME(AColonyBuildingSystem, BuildingEffects);
    DOREPLIFETIME(AColonyBuildingSystem, ActiveEffects);
    DOREPLIFETIME(AColonyBuildingSystem, TotalBuildingsConstructed);
    DOREPLIFETIME(AColonyBuildingSystem, TotalResourcesSpent);
    DOREPLIFETIME(AColonyBuildingSystem, TotalConstructionTime);
}

// Initialize building system
void AColonyBuildingSystem::InitializeBuildingSystem()
{
    if (!HasAuthority()) return;
    
    // Initialize building templates
    InitializeBuildingTemplates();
    
    // Initialize building costs
    InitializeBuildingCosts();
    
    // Initialize building requirements
    InitializeBuildingRequirements();
    
    // Initialize building effects
    InitializeBuildingEffects();
    
    // Set up building events
    SetupBuildingEvents();
    
    UE_LOG(LogTemp, Log, TEXT("Building system initialized"));
}

// Setup building placement
void AColonyBuildingSystem::SetupBuildingPlacement()
{
    // Configure building placement component
    BuildingPlacementComponent->SetBoxExtent(FVector(PlacementGridSize));
    BuildingPlacementComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BuildingPlacementComponent->SetCollisionObjectType(ECC_WorldStatic);
    BuildingPlacementComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    BuildingPlacementComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    
    // Configure building mesh component
    BuildingMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BuildingMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
    BuildingMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    BuildingMeshComponent->SetGenerateOverlapEvents(false);
    
    // Set initial materials
    if (InvalidPlacementMaterial)
    {
        BuildingMeshComponent->SetMaterial(0, InvalidPlacementMaterial);
    }
}

// Initialize building grid
void AColonyBuildingSystem::InitializeBuildingGrid()
{
    // Clear existing grid
    BuildingGrid.Empty();
    
    // Calculate grid dimensions
    const int32 GridSize = 50; // 50x50 grid
    const float GridSpacing = PlacementGridSize;
    
    // Initialize grid cells
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FBuildingGridCell GridCell;
            GridCell.GridPosition = FIntPoint(X, Y);
            GridCell.WorldPosition = FVector(
                (X - GridSize / 2) * GridSpacing,
                (Y - GridSize / 2) * GridSpacing,
                0.0f
            );
            GridCell.bIsOccupied = false;
            GridCell.OccupyingBuilding = nullptr;
            GridCell.TerrainType = ETerrainType::Flat;
            GridCell.bIsValidForBuilding = true;
            
            BuildingGrid.Add(GridCell);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Building grid initialized with %d cells"), BuildingGrid.Num());
}

// Load available buildings
void AColonyBuildingSystem::LoadAvailableBuildings()
{
    // Clear existing buildings
    AvailableBuildings.Empty();
    
    // Load building templates from data assets or configuration
    // This would typically load from a data table or config file
    
    // Add basic building types
    FBuildingTemplate HabitatTemplate;
    HabitatTemplate.BuildingType = EBuildingType::Habitat;
    HabitatTemplate.BuildingName = TEXT("Basic Habitat");
    HabitatTemplate.Description = TEXT("Provides living space for colonists");
    HabitatTemplate.MaxLevel = 5;
    HabitatTemplate.BaseCost = FResourceAmount(100, 50, 25, 10);
    HabitatTemplate.BaseConstructionTime = 60.0f;
    HabitatTemplate.BasePowerConsumption = 10.0f;
    HabitatTemplate.BasePopulationCapacity = 4;
    HabitatTemplate.RequiredTechnologies.Empty();
    HabitatTemplate.PrerequisiteBuildings.Empty();
    HabitatTemplate.BuildingMesh = nullptr;
    HabitatTemplate.ConstructionMaterial = nullptr;
    AvailableBuildings.Add(HabitatTemplate);
    
    FBuildingTemplate PowerPlantTemplate;
    PowerPlantTemplate.BuildingType = EBuildingType::PowerPlant;
    PowerPlantTemplate.BuildingName = TEXT("Solar Power Plant");
    PowerPlantTemplate.Description = TEXT("Generates power from solar energy");
    PowerPlantTemplate.MaxLevel = 3;
    PowerPlantTemplate.BaseCost = FResourceAmount(200, 100, 50, 25);
    PowerPlantTemplate.BaseConstructionTime = 90.0f;
    PowerPlantTemplate.BasePowerGeneration = 50.0f;
    PowerPlantTemplate.BasePowerConsumption = 0.0f;
    PowerPlantTemplate.RequiredTechnologies.Empty();
    PowerPlantTemplate.PrerequisiteBuildings.Empty();
    PowerPlantTemplate.BuildingMesh = nullptr;
    PowerPlantTemplate.ConstructionMaterial = nullptr;
    AvailableBuildings.Add(PowerPlantTemplate);
    
    FBuildingTemplate MiningFacilityTemplate;
    MiningFacilityTemplate.BuildingType = EBuildingType::MiningFacility;
    MiningFacilityTemplate.BuildingName = TEXT("Mining Facility");
    MiningFacilityTemplate.Description = TEXT("Extracts resources from the environment");
    MiningFacilityTemplate.MaxLevel = 4;
    MiningFacilityTemplate.BaseCost = FResourceAmount(300, 150, 75, 50);
    MiningFacilityTemplate.BaseConstructionTime = 120.0f;
    MiningFacilityTemplate.BasePowerConsumption = 25.0f;
    MiningFacilityTemplate.BaseStorageCapacity = 500;
    MiningFacilityTemplate.RequiredTechnologies.Empty();
    MiningFacilityTemplate.PrerequisiteBuildings.Empty();
    MiningFacilityTemplate.BuildingMesh = nullptr;
    MiningFacilityTemplate.ConstructionMaterial = nullptr;
    AvailableBuildings.Add(MiningFacilityTemplate);
    
    FBuildingTemplate ResearchLabTemplate;
    ResearchLabTemplate.BuildingType = EBuildingType::ResearchLab;
    ResearchLabTemplate.BuildingName = TEXT("Research Laboratory");
    ResearchLabTemplate.Description = TEXT("Conducts scientific research");
    ResearchLabTemplate.MaxLevel = 5;
    ResearchLabTemplate.BaseCost = FResourceAmount(250, 125, 60, 30);
    ResearchLabTemplate.BaseConstructionTime = 100.0f;
    ResearchLabTemplate.BasePowerConsumption = 15.0f;
    ResearchLabTemplate.RequiredTechnologies.Empty();
    ResearchLabTemplate.PrerequisiteBuildings.Empty();
    ResearchLabTemplate.BuildingMesh = nullptr;
    ResearchLabTemplate.ConstructionMaterial = nullptr;
    AvailableBuildings.Add(ResearchLabTemplate);
    
    FBuildingTemplate DefenseTurretTemplate;
    DefenseTurretTemplate.BuildingType = EBuildingType::DefenseTurret;
    DefenseTurretTemplate.BuildingName = TEXT("Defense Turret");
    DefenseTurretTemplate.Description = TEXT("Provides defensive capabilities");
    DefenseTurretTemplate.MaxLevel = 3;
    DefenseTurretTemplate.BaseCost = FResourceAmount(150, 75, 40, 20);
    DefenseTurretTemplate.BaseConstructionTime = 45.0f;
    DefenseTurretTemplate.BasePowerConsumption = 5.0f;
    DefenseTurretTemplate.BaseDefenseRating = 25.0f;
    DefenseTurretTemplate.RequiredTechnologies.Empty();
    DefenseTurretTemplate.PrerequisiteBuildings.Empty();
    DefenseTurretTemplate.BuildingMesh = nullptr;
    DefenseTurretTemplate.ConstructionMaterial = nullptr;
    AvailableBuildings.Add(DefenseTurretTemplate);
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d available building templates"), AvailableBuildings.Num());
}

// Enter build mode
void AColonyBuildingSystem::EnterBuildMode(EBuildingType BuildingType)
{
    if (!HasAuthority()) return;
    
    // Check if building type is available
    bool bIsAvailable = false;
    for (const FBuildingTemplate& Template : AvailableBuildings)
    {
        if (Template.BuildingType == BuildingType)
        {
            bIsAvailable = true;
            break;
        }
    }
    
    if (!bIsAvailable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building type %d is not available"), (int32)BuildingType);
        return;
    }
    
    // Set building mode
    CurrentBuildingMode = EBuildingMode::Placement;
    CurrentBuildingType = BuildingType;
    CurrentBuildingLevel = 1;
    bIsInBuildMode = true;
    
    // Show building placement preview
    ShowBuildingPlacementPreview();
    
    // Notify clients
    OnBuildModeEntered(BuildingType);
    
    UE_LOG(LogTemp, Log, TEXT("Entered build mode for building type %d"), (int32)BuildingType);
}

// Exit build mode
void AColonyBuildingSystem::ExitBuildMode()
{
    if (!HasAuthority()) return;
    
    // Reset building mode
    CurrentBuildingMode = EBuildingMode::None;
    bIsInBuildMode = false;
    bIsBuildingValid = false;
    
    // Hide building placement preview
    HideBuildingPlacementPreview();
    
    // Notify clients
    OnBuildModeExited();
    
    UE_LOG(LogTemp, Log, TEXT("Exited build mode"));
}

// Update building placement
void AColonyBuildingSystem::UpdateBuildingPlacement()
{
    if (!bIsInBuildMode || CurrentBuildingMode != EBuildingMode::Placement) return;
    
    // Get player controller and viewport
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController) return;
    
    // Trace to find placement location
    FVector TraceStart, TraceEnd;
    FRotator ViewRotation;
    PlayerController->GetPlayerViewPoint(TraceStart, ViewRotation);
    TraceEnd = TraceStart + ViewRotation.Vector() * MaxBuildDistance;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Update placement location
        FVector PlacementLocation = SnapToGrid(HitResult.Location);
        SetActorLocation(PlacementLocation);
        
        // Check if placement is valid
        bIsBuildingValid = IsPlacementValid(PlacementLocation);
        
        // Update placement material
        UpdatePlacementMaterial();
    }
}

// Place building
bool AColonyBuildingSystem::PlaceBuilding(const FVector& Location)
{
    if (!HasAuthority()) return false;
    
    if (!bIsInBuildMode || CurrentBuildingMode != EBuildingMode::Placement || !bIsBuildingValid)
    {
        return false;
    }
    
    // Get building template
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(CurrentBuildingType);
    if (!BuildingTemplate)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building template not found for type %d"), (int32)CurrentBuildingType);
        return false;
    }
    
    // Check resources
    if (!HasEnoughResources(BuildingTemplate->BaseCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient resources for building construction"));
        return false;
    }
    
    // Check prerequisites
    if (!MeetsPrerequisites(*BuildingTemplate))
    {
        UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for building construction"));
        return false;
    }
    
    // Create building data
    FBuildingData BuildingData;
    BuildingData.BuildingType = CurrentBuildingType;
    BuildingData.BuildingLevel = CurrentBuildingLevel;
    BuildingData.Location = SnapToGrid(Location);
    BuildingData.Rotation = GetActorRotation();
    BuildingData.Health = BuildingTemplate->BaseHealth;
    BuildingData.MaxHealth = BuildingTemplate->BaseHealth;
    BuildingData.PowerConsumption = BuildingTemplate->BasePowerConsumption;
    BuildingData.PowerGeneration = BuildingTemplate->BasePowerGeneration;
    BuildingData.PopulationCapacity = BuildingTemplate->BasePopulationCapacity;
    BuildingData.StorageCapacity = BuildingTemplate->BaseStorageCapacity;
    BuildingData.DefenseRating = BuildingTemplate->BaseDefenseRating;
    BuildingData.ConstructionProgress = 0.0f;
    BuildingData.bIsCompleted = false;
    BuildingData.bIsOperational = false;
    BuildingData.ConstructionStartTime = GetWorld()->GetTimeSeconds();
    BuildingData.LastUpgradeTime = 0.0f;
    BuildingData.Effects.Empty();
    BuildingData.AssignedWorkers = 0;
    BuildingData.MaxWorkers = BuildingTemplate->BaseMaxWorkers;
    BuildingData.ProductionRate = BuildingTemplate->BaseProductionRate;
    BuildingData.MaintenanceCost = BuildingTemplate->BaseMaintenanceCost;
    BuildingData.BuildingID = FGuid::NewGuid();
    
    // Deduct resources
    DeductResources(BuildingTemplate->BaseCost);
    
    // Add to constructed buildings
    ConstructedBuildings.Add(BuildingData);
    
    // Update grid
    UpdateBuildingGrid(BuildingData, true);
    
    // Start construction
    StartBuildingConstruction(BuildingData);
    
    // Update statistics
    TotalBuildingsConstructed++;
    TotalResourcesSpent += CalculateTotalResourceValue(BuildingTemplate->BaseCost);
    
    // Exit build mode
    ExitBuildMode();
    
    // Notify clients
    OnBuildingPlaced(BuildingData);
    
    UE_LOG(LogTemp, Log, TEXT("Building placed at location %s"), *Location.ToString());
    return true;
}

// Start building construction
void AColonyBuildingSystem::StartBuildingConstruction(const FBuildingData& BuildingData)
{
    if (!HasAuthority()) return;
    
    // Set construction state
    bIsUnderConstruction = true;
    ConstructionProgress = 0.0f;
    
    // Get building template
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (!BuildingTemplate) return;
    
    // Set construction time
    ConstructionTime = BuildingTemplate->BaseConstructionTime;
    
    // Play construction effects
    PlayConstructionEffects();
    
    // Start construction timer
    GetWorld()->GetTimerManager().SetTimer(
        ConstructionTimer,
        this,
        &AColonyBuildingSystem::CompleteBuildingConstruction,
        ConstructionTime,
        false
    );
    
    // Notify clients
    OnBuildingConstructionStarted.Broadcast(BuildingData);
    
    UE_LOG(LogTemp, Log, TEXT("Building construction started"));
}

// Complete building construction
void AColonyBuildingSystem::CompleteBuildingConstruction()
{
    if (!HasAuthority()) return;
    
    // Set construction state
    bIsUnderConstruction = false;
    ConstructionProgress = 1.0f;
    
    // Find the building being constructed
    for (FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (!BuildingData.bIsCompleted)
        {
            BuildingData.bIsCompleted = true;
            BuildingData.bIsOperational = true;
            BuildingData.ConstructionProgress = 1.0f;
            
            // Apply building effects
            ApplyBuildingEffects(BuildingData);
            
            // Update navigation
            UpdateNavigationMesh();
            
            // Play completion effects
            PlayConstructionCompleteEffects();
            
            // Notify clients
            OnBuildingConstructionCompleted.Broadcast(BuildingData);
            
            UE_LOG(LogTemp, Log, TEXT("Building construction completed"));
            return;
        }
    }
}

// Update construction progress
void AColonyBuildingSystem::UpdateConstructionProgress(float DeltaTime)
{
    if (!bIsUnderConstruction || ConstructionTime <= 0.0f) return;
    
    // Update progress
    ConstructionProgress += DeltaTime / ConstructionTime;
    ConstructionProgress = FMath::Clamp(ConstructionProgress, 0.0f, 1.0f);
    
    // Update building data
    for (FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (!BuildingData.bIsCompleted)
        {
            BuildingData.ConstructionProgress = ConstructionProgress;
            break;
        }
    }
    
    // Update construction effects
    UpdateConstructionEffects();
}

// Upgrade building
bool AColonyBuildingSystem::UpgradeBuilding(const FGuid& BuildingID)
{
    if (!HasAuthority()) return false;
    
    // Find building to upgrade
    FBuildingData* BuildingData = FindBuildingByID(BuildingID);
    if (!BuildingData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building not found for upgrade: %s"), *BuildingID.ToString());
        return false;
    }
    
    // Check if can upgrade
    if (!CanUpgradeBuilding(*BuildingData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Building cannot be upgraded"));
        return false;
    }
    
    // Get building template
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData->BuildingType);
    if (!BuildingTemplate) return false;
    
    // Calculate upgrade cost
    FResourceAmount UpgradeCost = CalculateUpgradeCost(*BuildingData);
    
    // Check resources
    if (!HasEnoughResources(UpgradeCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient resources for building upgrade"));
        return false;
    }
    
    // Deduct resources
    DeductResources(UpgradeCost);
    
    // Start upgrade
    StartBuildingUpgrade(*BuildingData);
    
    UE_LOG(LogTemp, Log, TEXT("Building upgrade started"));
    return true;
}

// Start building upgrade
void AColonyBuildingSystem::StartBuildingUpgrade(FBuildingData& BuildingData)
{
    if (!HasAuthority()) return;
    
    // Set upgrade state
    BuildingData.bIsUnderConstruction = true;
    BuildingData.ConstructionProgress = 0.0f;
    
    // Calculate upgrade time
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (!BuildingTemplate) return;
    
    float UpgradeTime = BuildingTemplate->BaseConstructionTime * 0.5f; // Upgrades are faster
    
    // Start upgrade timer
    FTimerHandle UpgradeTimer;
    GetWorld()->GetTimerManager().SetTimer(
        UpgradeTimer,
        [this, &BuildingData]()
        {
            CompleteBuildingUpgrade(BuildingData);
        },
        UpgradeTime,
        false
    );
    
    // Play upgrade effects
    PlayUpgradeEffects();
    
    // Notify clients
    OnBuildingUpgraded.Broadcast(BuildingData);
}

// Complete building upgrade
void AColonyBuildingSystem::CompleteBuildingUpgrade(FBuildingData& BuildingData)
{
    if (!HasAuthority()) return;
    
    // Increase building level
    BuildingData.BuildingLevel++;
    
    // Update building stats
    UpdateBuildingStats(BuildingData);
    
    // Set upgrade state
    BuildingData.bIsUnderConstruction = false;
    BuildingData.ConstructionProgress = 1.0f;
    
    // Apply new effects
    ApplyBuildingEffects(BuildingData);
    
    // Play completion effects
    PlayUpgradeCompleteEffects();
    
    UE_LOG(LogTemp, Log, TEXT("Building upgrade completed to level %d"), BuildingData.BuildingLevel);
}

// Demolish building
bool AColonyBuildingSystem::DemolishBuilding(const FGuid& BuildingID)
{
    if (!HasAuthority()) return false;
    
    // Find building to demolish
    FBuildingData* BuildingData = FindBuildingByID(BuildingID);
    if (!BuildingData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building not found for demolition: %s"), *BuildingID.ToString());
        return false;
    }
    
    // Check if can demolish
    if (!CanDemolishBuilding(*BuildingData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Building cannot be demolished"));
        return false;
    }
    
    // Remove building effects
    RemoveBuildingEffects(*BuildingData);
    
    // Play demolition effects
    PlayDemolitionEffects();
    
    // Update grid
    UpdateBuildingGrid(*BuildingData, false);
    
    // Remove from constructed buildings
    ConstructedBuildings.RemoveAll([&](const FBuildingData& Data)
    {
        return Data.BuildingID == BuildingID;
    });
    
    // Notify clients
    OnBuildingDestroyed.Broadcast(*BuildingData);
    
    UE_LOG(LogTemp, Log, TEXT("Building demolished"));
    return true;
}

// Repair building
bool AColonyBuildingSystem::RepairBuilding(const FGuid& BuildingID, float RepairAmount)
{
    if (!HasAuthority()) return false;
    
    // Find building to repair
    FBuildingData* BuildingData = FindBuildingByID(BuildingID);
    if (!BuildingData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building not found for repair: %s"), *BuildingID.ToString());
        return false;
    }
    
    // Check if needs repair
    if (BuildingData->Health >= BuildingData->MaxHealth)
    {
        UE_LOG(LogTemp, Warning, TEXT("Building does not need repair"));
        return false;
    }
    
    // Calculate repair cost
    FResourceAmount RepairCost = CalculateRepairCost(*BuildingData, RepairAmount);
    
    // Check resources
    if (!HasEnoughResources(RepairCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient resources for building repair"));
        return false;
    }
    
    // Deduct resources
    DeductResources(RepairCost);
    
    // Repair building
    BuildingData->Health = FMath::Clamp(BuildingData->Health + RepairAmount, 0.0f, BuildingData->MaxHealth);
    
    // Play repair effects
    PlayRepairEffects();
    
    // Notify clients
    OnBuildingRepaired.Broadcast(*BuildingData);
    
    UE_LOG(LogTemp, Log, TEXT("Building repaired by %f"), RepairAmount);
    return true;
}

// Damage building
void AColonyBuildingSystem::DamageBuilding(const FGuid& BuildingID, float DamageAmount)
{
    if (!HasAuthority()) return;
    
    // Find building to damage
    FBuildingData* BuildingData = FindBuildingByID(BuildingID);
    if (!BuildingData) return;
    
    // Apply damage
    BuildingData->Health = FMath::Clamp(BuildingData->Health - DamageAmount, 0.0f, BuildingData->MaxHealth);
    
    // Check if destroyed
    if (BuildingData->Health <= 0.0f)
    {
        // Destroy building
        DemolishBuilding(BuildingID);
    }
    else
    {
        // Play damage effects
        PlayDamageEffects();
        
        // Notify clients
        OnBuildingDamaged.Broadcast(*BuildingData);
    }
}

// Get building template
const FBuildingTemplate* AColonyBuildingSystem::GetBuildingTemplate(EBuildingType BuildingType) const
{
    for (const FBuildingTemplate& Template : AvailableBuildings)
    {
        if (Template.BuildingType == BuildingType)
        {
            return &Template;
        }
    }
    return nullptr;
}

// Find building by ID
FBuildingData* AColonyBuildingSystem::FindBuildingByID(const FGuid& BuildingID)
{
    for (FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.BuildingID == BuildingID)
        {
            return &BuildingData;
        }
    }
    return nullptr;
}

// Snap to grid
FVector AColonyBuildingSystem::SnapToGrid(const FVector& Location) const
{
    FVector SnappedLocation;
    SnappedLocation.X = FMath::RoundToFloat(Location.X / PlacementGridSize) * PlacementGridSize;
    SnappedLocation.Y = FMath::RoundToFloat(Location.Y / PlacementGridSize) * PlacementGridSize;
    SnappedLocation.Z = Location.Z;
    return SnappedLocation;
}

// Check if placement is valid
bool AColonyBuildingSystem::IsPlacementValid(const FVector& Location) const
{
    // Check distance from other buildings
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        float Distance = FVector::Dist(Location, BuildingData.Location);
        if (Distance < PlacementGridSize * 1.5f)
        {
            return false;
        }
    }
    
    // Check terrain type
    ETerrainType TerrainType = GetTerrainType(Location);
    if (TerrainType == ETerrainType::Mountain || TerrainType == ETerrainType::Water)
    {
        return false;
    }
    
    // Check slope
    float Slope = GetTerrainSlope(Location);
    if (Slope > 30.0f) // Max 30 degree slope
    {
        return false;
    }
    
    return true;
}

// Get terrain type
ETerrainType AColonyBuildingSystem::GetTerrainType(const FVector& Location) const
{
    // This would typically check terrain data from the planet
    // For now, return flat terrain
    return ETerrainType::Flat;
}

// Get terrain slope
float AColonyBuildingSystem::GetTerrainSlope(const FVector& Location) const
{
    // This would typically calculate slope from terrain data
    // For now, return 0 (flat)
    return 0.0f;
}

// Check if has enough resources
bool AColonyBuildingSystem::HasEnoughResources(const FResourceAmount& Cost) const
{
    // This would typically check colony resources
    // For now, return true
    return true;
}

// Deduct resources
void AColonyBuildingSystem::DeductResources(const FResourceAmount& Cost)
{
    // This would typically deduct from colony resources
    TotalResourcesSpent += CalculateTotalResourceValue(Cost);
}

// Calculate total resource value
float AColonyBuildingSystem::CalculateTotalResourceValue(const FResourceAmount& Resources) const
{
    return Resources.Metals + Resources.Crystals * 2.0f + Resources.Gas * 1.5f + Resources.Biomass * 1.2f;
}

// Check if meets prerequisites
bool AColonyBuildingSystem::MeetsPrerequisites(const FBuildingTemplate& BuildingTemplate) const
{
    // Check technology prerequisites
    for (const FString& Tech : BuildingTemplate.RequiredTechnologies)
    {
        if (!HasTechnology(Tech))
        {
            return false;
        }
    }
    
    // Check building prerequisites
    for (EBuildingType PrerequisiteBuilding : BuildingTemplate.PrerequisiteBuildings)
    {
        if (!HasBuildingType(PrerequisiteBuilding))
        {
            return false;
        }
    }
    
    return true;
}

// Check if has technology
bool AColonyBuildingSystem::HasTechnology(const FString& Technology) const
{
    // This would typically check technology tree
    // For now, return true
    return true;
}

// Check if has building type
bool AColonyBuildingSystem::HasBuildingType(EBuildingType BuildingType) const
{
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.BuildingType == BuildingType && BuildingData.bIsCompleted)
        {
            return true;
        }
    }
    return false;
}

// Can upgrade building
bool AColonyBuildingSystem::CanUpgradeBuilding(const FBuildingData& BuildingData) const
{
    if (!BuildingData.bIsCompleted || BuildingData.bIsUnderConstruction)
    {
        return false;
    }
    
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (!BuildingTemplate)
    {
        return false;
    }
    
    return BuildingData.BuildingLevel < BuildingTemplate->MaxLevel;
}

// Can demolish building
bool AColonyBuildingSystem::CanDemolishBuilding(const FBuildingData& BuildingData) const
{
    // Cannot demolish buildings under construction
    if (BuildingData.bIsUnderConstruction)
    {
        return false;
    }
    
    // Cannot demolish if there are assigned workers
    if (BuildingData.AssignedWorkers > 0)
    {
        return false;
    }
    
    return true;
}

// Calculate upgrade cost
FResourceAmount AColonyBuildingSystem::CalculateUpgradeCost(const FBuildingData& BuildingData) const
{
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (!BuildingTemplate)
    {
        return FResourceAmount();
    }
    
    FResourceAmount UpgradeCost = BuildingTemplate->BaseCost;
    float CostMultiplier = FMath::Pow(UpgradeCostMultiplier, BuildingData.BuildingLevel);
    
    UpgradeCost.Metals = FMath::RoundToFloat(UpgradeCost.Metals * CostMultiplier);
    UpgradeCost.Crystals = FMath::RoundToFloat(UpgradeCost.Crystals * CostMultiplier);
    UpgradeCost.Gas = FMath::RoundToFloat(UpgradeCost.Gas * CostMultiplier);
    UpgradeCost.Biomass = FMath::RoundToFloat(UpgradeCost.Biomass * CostMultiplier);
    
    return UpgradeCost;
}

// Calculate repair cost
FResourceAmount AColonyBuildingSystem::CalculateRepairCost(const FBuildingData& BuildingData, float RepairAmount) const
{
    float RepairPercentage = RepairAmount / BuildingData.MaxHealth;
    FResourceAmount RepairCost;
    
    // Repair costs are 10% of building cost per full repair
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (BuildingTemplate)
    {
        RepairCost = BuildingTemplate->BaseCost;
        RepairCost.Metals = FMath::RoundToFloat(RepairCost.Metals * 0.1f * RepairPercentage);
        RepairCost.Crystals = FMath::RoundToFloat(RepairCost.Crystals * 0.1f * RepairPercentage);
        RepairCost.Gas = FMath::RoundToFloat(RepairCost.Gas * 0.1f * RepairPercentage);
        RepairCost.Biomass = FMath::RoundToFloat(RepairCost.Biomass * 0.1f * RepairPercentage);
    }
    
    return RepairCost;
}

// Update building stats
void AColonyBuildingSystem::UpdateBuildingStats(FBuildingData& BuildingData)
{
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingData.BuildingType);
    if (!BuildingTemplate) return;
    
    // Calculate level multipliers
    float LevelMultiplier = 1.0f + (BuildingData.BuildingLevel - 1) * 0.2f;
    
    // Update stats
    BuildingData.MaxHealth = BuildingTemplate->BaseHealth * LevelMultiplier;
    BuildingData.Health = BuildingData.MaxHealth; // Full heal on upgrade
    BuildingData.PowerConsumption = BuildingTemplate->BasePowerConsumption * LevelMultiplier;
    BuildingData.PowerGeneration = BuildingTemplate->BasePowerGeneration * LevelMultiplier;
    BuildingData.PopulationCapacity = FMath::RoundToFloat(BuildingTemplate->BasePopulationCapacity * LevelMultiplier);
    BuildingData.StorageCapacity = FMath::RoundToFloat(BuildingTemplate->BaseStorageCapacity * LevelMultiplier);
    BuildingData.DefenseRating = BuildingTemplate->BaseDefenseRating * LevelMultiplier;
    BuildingData.MaxWorkers = FMath::RoundToFloat(BuildingTemplate->BaseMaxWorkers * LevelMultiplier);
    BuildingData.ProductionRate = BuildingTemplate->BaseProductionRate * LevelMultiplier;
}

// Update building grid
void AColonyBuildingSystem::UpdateBuildingGrid(const FBuildingData& BuildingData, bool bOccupying)
{
    // Calculate grid cells occupied by building
    FIntPoint GridPosition = WorldToGrid(BuildingData.Location);
    
    // Update grid cells
    for (FBuildingGridCell& GridCell : BuildingGrid)
    {
        if (GridCell.GridPosition == GridPosition)
        {
            GridCell.bIsOccupied = bOccupying;
            GridCell.OccupyingBuilding = bOccupying ? const_cast<FBuildingData*>(&BuildingData) : nullptr;
            break;
        }
    }
}

// World to grid conversion
FIntPoint AColonyBuildingSystem::WorldToGrid(const FVector& WorldLocation) const
{
    FIntPoint GridPosition;
    GridPosition.X = FMath::RoundToInt(WorldLocation.X / PlacementGridSize);
    GridPosition.Y = FMath::RoundToInt(WorldLocation.Y / PlacementGridSize);
    return GridPosition;
}

// Apply building effects
void AColonyBuildingSystem::ApplyBuildingEffects(const FBuildingData& BuildingData)
{
    // This would apply building-specific effects to the colony
    // Effects could include resource production, population bonuses, etc.
    
    // Add to active effects
    for (const FBuildingEffect& Effect : BuildingData.Effects)
    {
        ActiveEffects.Add(Effect);
    }
}

// Remove building effects
void AColonyBuildingSystem::RemoveBuildingEffects(const FBuildingData& BuildingData)
{
    // Remove building effects from active effects
    for (const FBuildingEffect& Effect : BuildingData.Effects)
    {
        ActiveEffects.RemoveAll([&](const FBuildingEffect& ActiveEffect)
        {
            return ActiveEffect.EffectType == Effect.EffectType && 
                   ActiveEffect.SourceBuildingID == BuildingData.BuildingID;
        });
    }
}

// Update building effects
void AColonyBuildingSystem::UpdateBuildingEffects(float DeltaTime)
{
    // Update active building effects
    for (FBuildingEffect& Effect : ActiveEffects)
    {
        if (Effect.Duration > 0.0f)
        {
            Effect.Duration -= DeltaTime;
            if (Effect.Duration <= 0.0f)
            {
                // Remove expired effect
                ActiveEffects.RemoveAll([&](const FBuildingEffect& ExpiredEffect)
                {
                    return ExpiredEffect.EffectType == Effect.EffectType;
                });
            }
        }
    }
}

// Update navigation mesh
void AColonyBuildingSystem::UpdateNavigationMesh()
{
    // Update navigation system to account for new building
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (NavSystem)
    {
        NavSystem->BuildNavigationSynchronously();
    }
}

// Handle auto repair
void AColonyBuildingSystem::HandleAutoRepair(float DeltaTime)
{
    if (!bAutoRepair) return;
    
    // Check all buildings for repair needs
    for (FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.Health < BuildingData.MaxHealth * RepairThreshold)
        {
            // Calculate auto repair amount
            float RepairAmount = 10.0f * DeltaTime; // 10 HP per second
            
            // Calculate repair cost
            FResourceAmount RepairCost = CalculateRepairCost(BuildingData, RepairAmount);
            
            // Check if can afford repair
            if (HasEnoughResources(RepairCost))
            {
                // Perform auto repair
                RepairBuilding(BuildingData.BuildingID, RepairAmount);
            }
        }
    }
}

// Handle auto upgrade
void AColonyBuildingSystem::HandleAutoUpgrade(float DeltaTime)
{
    if (!bAutoUpgrade) return;
    
    // Check buildings for upgrade opportunities
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (CanUpgradeBuilding(BuildingData))
        {
            // Check upgrade priority
            bool bShouldUpgrade = false;
            for (EBuildingType PriorityType : UpgradePriority)
            {
                if (BuildingData.BuildingType == PriorityType)
                {
                    bShouldUpgrade = true;
                    break;
                }
            }
            
            if (bShouldUpgrade)
            {
                // Calculate upgrade cost
                FResourceAmount UpgradeCost = CalculateUpgradeCost(BuildingData);
                
                // Check if can afford upgrade
                if (HasEnoughResources(UpgradeCost))
                {
                    // Perform auto upgrade
                    UpgradeBuilding(BuildingData.BuildingID);
                    break; // Only upgrade one building per frame
                }
            }
        }
    }
}

// Update building statistics
void AColonyBuildingSystem::UpdateBuildingStatistics(float DeltaTime)
{
    // Update total construction time
    if (bIsUnderConstruction)
    {
        TotalConstructionTime += DeltaTime;
    }
    
    // Calculate colony-wide stats
    CalculateColonyStats();
}

// Calculate colony stats
void AColonyBuildingSystem::CalculateColonyStats()
{
    // Reset stats
    float TotalPowerConsumption = 0.0f;
    float TotalPowerGeneration = 0.0f;
    int32 TotalPopulationCapacity = 0;
    int32 TotalStorageCapacity = 0;
    float TotalDefenseRating = 0.0f;
    
    // Sum stats from all buildings
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.bIsCompleted && BuildingData.bIsOperational)
        {
            TotalPowerConsumption += BuildingData.PowerConsumption;
            TotalPowerGeneration += BuildingData.PowerGeneration;
            TotalPopulationCapacity += BuildingData.PopulationCapacity;
            TotalStorageCapacity += BuildingData.StorageCapacity;
            TotalDefenseRating += BuildingData.DefenseRating;
        }
    }
    
    // Update member variables
    PowerConsumption = TotalPowerConsumption;
    PowerGeneration = TotalPowerGeneration;
    PopulationCapacity = TotalPopulationCapacity;
    StorageCapacity = TotalStorageCapacity;
    DefenseRating = TotalDefenseRating;
}

// Show building placement preview
void AColonyBuildingSystem::ShowBuildingPlacementPreview()
{
    // Get building template
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(CurrentBuildingType);
    if (!BuildingTemplate || !BuildingTemplate->BuildingMesh) return;
    
    // Set preview mesh
    BuildingMeshComponent->SetStaticMesh(BuildingTemplate->BuildingMesh);
    BuildingMeshComponent->SetVisibility(true);
    
    // Set initial material
    if (InvalidPlacementMaterial)
    {
        BuildingMeshComponent->SetMaterial(0, InvalidPlacementMaterial);
    }
}

// Hide building placement preview
void AColonyBuildingSystem::HideBuildingPlacementPreview()
{
    BuildingMeshComponent->SetVisibility(false);
}

// Update placement material
void AColonyBuildingSystem::UpdatePlacementMaterial()
{
    UMaterialInterface* Material = bIsBuildingValid ? ValidPlacementMaterial : InvalidPlacementMaterial;
    if (Material)
    {
        BuildingMeshComponent->SetMaterial(0, Material);
    }
}

// Play construction effects
void AColonyBuildingSystem::PlayConstructionEffects()
{
    // Play construction sound
    if (ConstructionStartSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ConstructionStartSound, GetActorLocation());
    }
    
    // Spawn construction particles
    if (ConstructionParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ConstructionParticles, GetActorLocation());
    }
}

// Play construction complete effects
void AColonyBuildingSystem::PlayConstructionCompleteEffects()
{
    // Play completion sound
    if (ConstructionCompleteSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ConstructionCompleteSound, GetActorLocation());
    }
    
    // Spawn completion particles
    if (ConstructionParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ConstructionParticles, GetActorLocation());
    }
}

// Play upgrade effects
void AColonyBuildingSystem::PlayUpgradeEffects()
{
    // Play upgrade sound
    if (UpgradeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, UpgradeSound, GetActorLocation());
    }
    
    // Spawn upgrade particles
    if (UpgradeParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), UpgradeParticles, GetActorLocation());
    }
}

// Play upgrade complete effects
void AColonyBuildingSystem::PlayUpgradeCompleteEffects()
{
    // Play completion sound
    if (ConstructionCompleteSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ConstructionCompleteSound, GetActorLocation());
    }
    
    // Spawn completion particles
    if (UpgradeParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), UpgradeParticles, GetActorLocation());
    }
}

// Play demolition effects
void AColonyBuildingSystem::PlayDemolitionEffects()
{
    // Play destruction sound
    if (BuildingDestroyedSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BuildingDestroyedSound, GetActorLocation());
    }
    
    // Spawn destruction particles
    if (DestructionParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionParticles, GetActorLocation());
    }
}

// Play repair effects
void AColonyBuildingSystem::PlayRepairEffects()
{
    // Play repair sound (could use construction sound)
    if (ConstructionStartSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ConstructionStartSound, GetActorLocation());
    }
}

// Play damage effects
void AColonyBuildingSystem::PlayDamageEffects()
{
    // Play damage sound (could use a specific damage sound)
    // This would typically be a different sound for damage
}

// Update construction effects
void AColonyBuildingSystem::UpdateConstructionEffects()
{
    // Update visual effects based on construction progress
    // This could include changing materials, particle effects, etc.
}

// Initialize building templates
void AColonyBuildingSystem::InitializeBuildingTemplates()
{
    // This would typically load from data assets
    // For now, templates are initialized in LoadAvailableBuildings
}

// Initialize building costs
void AColonyBuildingSystem::InitializeBuildingCosts()
{
    // This would typically load from configuration
    // For now, costs are defined in building templates
}

// Initialize building requirements
void AColonyBuildingSystem::InitializeBuildingRequirements()
{
    // This would typically load from configuration
    // For now, requirements are defined in building templates
}

// Initialize building effects
void AColonyBuildingSystem::InitializeBuildingEffects()
{
    // This would typically load from data assets
    // For now, effects are defined in building templates
}

// Setup building events
void AColonyBuildingSystem::SetupBuildingEvents()
{
    // Setup event bindings
    // This would typically bind to other system events
}

// Client-side notifications
void AColonyBuildingSystem::OnBuildModeEntered_Implementation(EBuildingType BuildingType)
{
    // Client-side build mode entry handling
}

void AColonyBuildingSystem::OnBuildModeExited_Implementation()
{
    // Client-side build mode exit handling
}

void AColonyBuildingSystem::OnBuildingPlaced_Implementation(const FBuildingData& BuildingData)
{
    // Client-side building placement handling
}

// Get building statistics
FBuildingStatistics AColonyBuildingSystem::GetBuildingStatistics() const
{
    FBuildingStatistics Stats;
    Stats.TotalBuildings = ConstructedBuildings.Num();
    Stats.OperationalBuildings = 0;
    Stats.UnderConstruction = 0;
    Stats.TotalPowerConsumption = PowerConsumption;
    Stats.TotalPowerGeneration = PowerGeneration;
    Stats.TotalPopulationCapacity = PopulationCapacity;
    Stats.TotalStorageCapacity = StorageCapacity;
    Stats.TotalDefenseRating = DefenseRating;
    Stats.AverageBuildingLevel = 0.0f;
    
    // Calculate additional stats
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.bIsCompleted && BuildingData.bIsOperational)
        {
            Stats.OperationalBuildings++;
        }
        if (BuildingData.bIsUnderConstruction)
        {
            Stats.UnderConstruction++;
        }
        Stats.AverageBuildingLevel += BuildingData.BuildingLevel;
    }
    
    if (Stats.TotalBuildings > 0)
    {
        Stats.AverageBuildingLevel /= Stats.TotalBuildings;
    }
    
    return Stats;
}

// Get buildings by type
TArray<FBuildingData> AColonyBuildingSystem::GetBuildingsByType(EBuildingType BuildingType) const
{
    TArray<FBuildingData> Buildings;
    
    for (const FBuildingData& BuildingData : ConstructedBuildings)
    {
        if (BuildingData.BuildingType == BuildingType)
        {
            Buildings.Add(BuildingData);
        }
    }
    
    return Buildings;
}

// Get available building templates
TArray<FBuildingTemplate> AColonyBuildingSystem::GetAvailableBuildingTemplates() const
{
    return AvailableBuildings;
}

// Check if can place building at location
bool AColonyBuildingSystem::CanPlaceBuildingAtLocation(EBuildingType BuildingType, const FVector& Location) const
{
    // Check if building type is available
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingType);
    if (!BuildingTemplate)
    {
        return false;
    }
    
    // Check placement validity
    FVector SnappedLocation = SnapToGrid(Location);
    return IsPlacementValid(SnappedLocation);
}

// Get building placement cost
FResourceAmount AColonyBuildingSystem::GetBuildingPlacementCost(EBuildingType BuildingType) const
{
    const FBuildingTemplate* BuildingTemplate = GetBuildingTemplate(BuildingType);
    if (!BuildingTemplate)
    {
        return FResourceAmount();
    }
    
    return BuildingTemplate->BaseCost;
}