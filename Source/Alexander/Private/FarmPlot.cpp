// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmPlot.h"
#include "CropDefinition.h"
#include "CropGrowthCalculator.h"
#include "CropHealthSystem.h"
#include "Planet.h"
#include "PlanetWeatherComponent.h"
#include "DayNightCycleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProceduralMeshComponent.h"

AFarmPlot::AFarmPlot()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// Create crop instances component
	CropInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CropInstances"));
	CropInstances->SetupAttachment(RootComponent);

	// Create soil mesh component
	SoilMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SoilMesh"));
	SoilMesh->SetupAttachment(RootComponent);

	// Default values
	PlotSize = FVector2D(1000.0f, 1000.0f); // 10m x 10m
	GridResolution = 10; // 10x10 grid
	SoilQuality = 0.7f;
	WaterLevel = 0.5f;
	Fertility = 1.0f;
	PlanetActor = nullptr;

	// Soil system defaults
	NitrogenLevel = 0.7f;
	PhosphorusLevel = 0.7f;
	PotassiumLevel = 0.7f;
	OrganicMatter = 0.5f;
	SoilPH = 6.5f; // Slightly acidic, good for most crops
	Compaction = 0.2f; // Low compaction initially

	// Weather integration defaults
	WeatherComponent = nullptr;
	AccumulatedRainfall = 0.0f;
	TimeSinceLastStormCheck = 0.0f;
	StormCheckInterval = 5.0f; // Check for storm damage every 5 seconds
	TemperatureDamageThreshold = 10.0f; // 10 degrees outside optimal before damage
	TemperatureDamageRate = 0.01f; // 1% damage per degree per second

	// Harvest tracking defaults (Task 17.4)
	MaxHarvestRecords = 100; // Keep last 100 harvests
	AccumulatedSoilQuality = 0.0f;
	AccumulatedFertility = 0.0f;
	QualitySampleCount = 0;
}

void AFarmPlot::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the crop grid
	InitializeCropGrid();

	// Create initial soil mesh
	UpdateSoilVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot initialized at %s (Size: %.1f x %.1f, Grid: %dx%d)"),
		*GetActorLocation().ToString(), PlotSize.X, PlotSize.Y, GridResolution, GridResolution);
}

void AFarmPlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update weather effects first (affects water level and crop health)
	UpdateWeatherEffects(DeltaTime);

	// Update crop growth
	UpdateCropGrowth(DeltaTime);

	// Water naturally depletes over time (evaporation)
	// Reduced depletion if it's raining
	float EvaporationRate = 0.01f; // 1% per second base rate
	if (WeatherComponent)
	{
		float PrecipitationRate = WeatherComponent->GetPrecipitationRate();
		if (PrecipitationRate > 0.0f)
		{
			EvaporationRate *= 0.1f; // Much slower evaporation during rain
		}
		
		// Higher temperatures increase evaporation
		float Temperature = WeatherComponent->GetCurrentTemperature();
		float TempFactor = FMath::Lerp(0.5f, 2.0f, (Temperature - 10.0f) / 30.0f);
		EvaporationRate *= FMath::Clamp(TempFactor, 0.5f, 2.0f);
	}
	WaterLevel = FMath::Max(0.0f, WaterLevel - (DeltaTime * EvaporationRate));

	// Natural soil regeneration (very slow)
	RegenerateSoil(DeltaTime);

	// Deplete nutrients based on active crops
	for (const FCropCell& Cell : CropGrid)
	{
		if (Cell.CropType && Cell.GrowthProgress < 1.0f)
		{
			DepleteSoilNutrients(Cell, DeltaTime);
		}
	}

	// Update calculated soil quality
	SoilQuality = CalculateSoilQuality();

	// Check if crops need water
	for (FCropCell& Cell : CropGrid)
	{
		if (Cell.CropType)
		{
			Cell.bNeedsWater = (WaterLevel < 0.3f);
		}
	}

	// Accumulate soil quality and fertility for harvest tracking (Task 17.4)
	// Sample every tick to get accurate averages
	AccumulatedSoilQuality += SoilQuality;
	AccumulatedFertility += Fertility;
	QualitySampleCount++;
}

// ============================================================================
// FARMING OPERATIONS
// ============================================================================

bool AFarmPlot::PlantCrop(FIntPoint GridPosition, UCropDefinition* CropType)
{
	if (!CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Cannot plant - no crop type specified"));
		return false;
	}

	if (!IsValidGridPosition(GridPosition))
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Invalid grid position (%d, %d)"), GridPosition.X, GridPosition.Y);
		return false;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	FCropCell& Cell = CropGrid[CellIndex];

	// Check if cell is already occupied
	if (Cell.CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Cell already occupied"));
		return false;
	}

	// Check soil quality requirement
	if (SoilQuality < CropType->SoilQualityRequirement)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Soil quality too low for this crop (%.2f < %.2f)"),
			SoilQuality, CropType->SoilQualityRequirement);
		return false;
	}

	// Plant the crop
	FDateTime CurrentTime = FDateTime::Now();
	Cell.CropType = CropType;
	Cell.GrowthProgress = 0.0f;
	Cell.Health = 1.0f;
	Cell.bNeedsWater = false;
	Cell.bHasPests = false;
	Cell.PlantedTime = CurrentTime;
	Cell.CurrentGrowthStage = 0;
	Cell.LastGrowthProgress = 0.0f;
	Cell.TimeSinceLastWater = 0.0f;
	Cell.LastWateredTime = CurrentTime;

	// Update visuals
	UpdateCropVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Planted %s at (%d, %d)"), 
		*CropType->CropName, GridPosition.X, GridPosition.Y);

	return true;
}

FHarvestResult AFarmPlot::HarvestCrop(FIntPoint GridPosition)
{
	FHarvestResult Result;
	Result.bSuccess = false;

	if (!IsValidGridPosition(GridPosition))
	{
		return Result;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: No crop to harvest at (%d, %d)"), GridPosition.X, GridPosition.Y);
		return Result;
	}

	// Check if crop is mature
	if (Cell.GrowthProgress < 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Crop not ready for harvest (%.1f%% grown)"), 
			Cell.GrowthProgress * 100.0f);
		return Result;
	}

	// Calculate yield based on health and fertility
	float QualityMultiplier = Cell.Health * Fertility;
	int32 BaseYield = (Cell.CropType->YieldAmount.Min + Cell.CropType->YieldAmount.Max) / 2;
	int32 ActualYield = FMath::RoundToInt(BaseYield * QualityMultiplier);

	Result.bSuccess = true;
	Result.YieldAmount = ActualYield;
	Result.ItemName = Cell.CropType->CropName;
	Result.Quality = QualityMultiplier;

	// Record harvest in history (Task 17.4)
	RecordHarvest(Result, GridPosition, Cell);

	// Deplete fertility and nutrients after harvest
	Fertility = FMath::Max(0.3f, Fertility - 0.05f);
	NitrogenLevel = FMath::Max(0.2f, NitrogenLevel - 0.1f);
	PhosphorusLevel = FMath::Max(0.2f, PhosphorusLevel - 0.05f);
	PotassiumLevel = FMath::Max(0.2f, PotassiumLevel - 0.05f);

	// Increase compaction slightly from harvesting activity
	Compaction = FMath::Min(0.8f, Compaction + 0.02f);

	// Clear the cell
	Cell.CropType = nullptr;
	Cell.GrowthProgress = 0.0f;
	Cell.Health = 1.0f;
	Cell.CurrentGrowthStage = 0;
	Cell.LastGrowthProgress = 0.0f;

	// Update visuals
	UpdateCropVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Harvested %s - Yield: %d (Quality: %.2f)"),
		*Result.ItemName, Result.YieldAmount, Result.Quality);

	return Result;
}

void AFarmPlot::WaterPlot(float WaterAmount)
{
	WaterLevel = FMath::Clamp(WaterLevel + WaterAmount, 0.0f, 1.0f);

	// Reset water needs and timer for all crops
	FDateTime CurrentTime = FDateTime::Now();
	for (FCropCell& Cell : CropGrid)
	{
		if (Cell.CropType)
		{
			Cell.bNeedsWater = false;
			Cell.TimeSinceLastWater = 0.0f;
			Cell.LastWateredTime = CurrentTime;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Watered plot (Water level: %.2f)"), WaterLevel);
}

void AFarmPlot::FertilizePlot(float FertilizerAmount)
{
	// Generic fertilizer restores all nutrients equally
	float NutrientBoost = FertilizerAmount * 0.3f;
	NitrogenLevel = FMath::Clamp(NitrogenLevel + NutrientBoost, 0.0f, 1.0f);
	PhosphorusLevel = FMath::Clamp(PhosphorusLevel + NutrientBoost, 0.0f, 1.0f);
	PotassiumLevel = FMath::Clamp(PotassiumLevel + NutrientBoost, 0.0f, 1.0f);
	
	Fertility = FMath::Clamp(Fertility + FertilizerAmount, 0.0f, 1.0f);

	// Update soil quality
	SoilQuality = CalculateSoilQuality();
	UpdateSoilVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Fertilized plot (Fertility: %.2f, N: %.2f, P: %.2f, K: %.2f)"), 
		Fertility, NitrogenLevel, PhosphorusLevel, PotassiumLevel);
}

void AFarmPlot::ApplyFertilizer(float Nitrogen, float Phosphorus, float Potassium)
{
	NitrogenLevel = FMath::Clamp(NitrogenLevel + Nitrogen, 0.0f, 1.0f);
	PhosphorusLevel = FMath::Clamp(PhosphorusLevel + Phosphorus, 0.0f, 1.0f);
	PotassiumLevel = FMath::Clamp(PotassiumLevel + Potassium, 0.0f, 1.0f);

	// Update fertility based on nutrient levels
	Fertility = (NitrogenLevel + PhosphorusLevel + PotassiumLevel) / 3.0f;

	// Update soil quality
	SoilQuality = CalculateSoilQuality();
	UpdateSoilVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Applied NPK fertilizer (N: %.2f, P: %.2f, K: %.2f)"), 
		NitrogenLevel, PhosphorusLevel, PotassiumLevel);
}

void AFarmPlot::AddOrganicMatter(float Amount)
{
	OrganicMatter = FMath::Clamp(OrganicMatter + Amount, 0.0f, 1.0f);

	// Organic matter improves water retention and reduces compaction
	Compaction = FMath::Max(0.0f, Compaction - (Amount * 0.1f));

	// Update soil quality
	SoilQuality = CalculateSoilQuality();
	UpdateSoilVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Added organic matter (Organic: %.2f, Compaction: %.2f)"), 
		OrganicMatter, Compaction);
}

float AFarmPlot::CalculateSoilQuality() const
{
	// Soil quality is a weighted combination of multiple factors
	float NutrientScore = (NitrogenLevel + PhosphorusLevel + PotassiumLevel) / 3.0f;
	float StructureScore = (1.0f - Compaction) * OrganicMatter;
	float PHScore = 1.0f - FMath::Abs(SoilPH - 6.5f) / 2.5f; // Optimal pH is 6.5
	
	// Weighted average
	float Quality = (NutrientScore * 0.4f) + (StructureScore * 0.3f) + (PHScore * 0.2f) + (Fertility * 0.1f);
	
	return FMath::Clamp(Quality, 0.0f, 1.0f);
}

FVector AFarmPlot::GetNutrientLevels() const
{
	return FVector(NitrogenLevel, PhosphorusLevel, PotassiumLevel);
}

void AFarmPlot::UpdateCropGrowth(float DeltaTime)
{
	FFarmEnvironment Environment = GetEnvironmentalConditions();
	bool bAnyStageChanged = false;
	bool bAnyHealthChanged = false;

	for (FCropCell& Cell : CropGrid)
	{
		if (!Cell.CropType)
		{
			continue;
		}

		// Update time since last water
		Cell.TimeSinceLastWater += DeltaTime;

		// Build environmental factors for health calculation
		FEnvironmentalFactors EnvFactors;
		EnvFactors.Temperature = Environment.Temperature;
		EnvFactors.Humidity = Environment.Humidity;
		EnvFactors.Sunlight = Environment.Sunlight;
		EnvFactors.SoilQuality = SoilQuality;
		EnvFactors.WaterLevel = WaterLevel;
		EnvFactors.Fertility = Fertility;

		// Calculate comprehensive health
		FHealthCalculationResult HealthResult = UCropHealthSystem::CalculateCropHealth(
			Cell.CropType,
			EnvFactors,
			Cell.Health,
			WaterLevel,
			Fertility,
			Cell.TimeSinceLastWater,
			DeltaTime
		);

		// Update health
		float OldHealth = Cell.Health;
		Cell.Health = HealthResult.CurrentHealth;
		Cell.bNeedsWater = HealthResult.bNeedsWater;

		// Track if health changed significantly
		if (FMath::Abs(OldHealth - Cell.Health) > 0.05f)
		{
			bAnyHealthChanged = true;

			// Log significant health changes
			if (Cell.Health < 0.3f && OldHealth >= 0.3f)
			{
				UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Crop health critical! Primary issue: %s"),
					*HealthResult.PrimaryIssue);
			}
		}

		// Only grow if not fully mature
		if (Cell.GrowthProgress < 1.0f)
		{
			// Store old progress for stage change detection
			float OldProgress = Cell.GrowthProgress;

			// Calculate growth rate modifier
			float GrowthModifier = CalculateGrowthRateModifier(Cell, Environment);

			// Base growth rate (1.0 = full growth in GrowthDuration seconds)
			float BaseGrowthRate = 1.0f / Cell.CropType->GrowthDuration;

			// Apply growth (health affects growth rate)
			float HealthModifier = FMath::Lerp(0.1f, 1.0f, Cell.Health);
			float GrowthAmount = BaseGrowthRate * GrowthModifier * HealthModifier * DeltaTime;
			Cell.GrowthProgress = FMath::Clamp(Cell.GrowthProgress + GrowthAmount, 0.0f, 1.0f);

			// Check for stage changes
			int32 OldStage = Cell.CropType->GetGrowthStageIndex(OldProgress);
			int32 NewStage = Cell.CropType->GetGrowthStageIndex(Cell.GrowthProgress);

			if (OldStage != NewStage)
			{
				Cell.CurrentGrowthStage = NewStage;
				OnCropStageChanged(Cell, OldStage, NewStage);
				bAnyStageChanged = true;
			}

			Cell.LastGrowthProgress = Cell.GrowthProgress;
		}
	}

	// Update visuals periodically (not every frame for performance)
	// Or immediately if any stage or health changed significantly
	static float VisualUpdateTimer = 0.0f;
	VisualUpdateTimer += DeltaTime;
	if (VisualUpdateTimer >= 1.0f || bAnyStageChanged || bAnyHealthChanged)
	{
		UpdateCropVisuals();
		VisualUpdateTimer = 0.0f;
	}
}

FFarmEnvironment AFarmPlot::GetEnvironmentalConditions() const
{
	FFarmEnvironment Environment;

	if (!PlanetActor)
	{
		return Environment;
	}

	APlanet* Planet = Cast<APlanet>(PlanetActor);
	if (!Planet)
	{
		return Environment;
	}

	// Get temperature and humidity from weather system
	if (Planet->WeatherComponent)
	{
		Environment.Temperature = Planet->WeatherComponent->GetCurrentTemperature();
		Environment.Humidity = Planet->WeatherComponent->GetCurrentHumidity();
		Environment.Rainfall = Planet->WeatherComponent->GetPrecipitationRate();
	}

	// Get sunlight from day-night cycle
	if (Planet->DayNightCycleComponent)
	{
		Environment.Sunlight = Planet->DayNightCycleComponent->GetSunIntensity();
	}

	return Environment;
}

FGrowthCalculationResult AFarmPlot::GetCropGrowthDetails(FIntPoint GridPosition) const
{
	FGrowthCalculationResult Result;

	if (!IsValidGridPosition(GridPosition))
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Invalid grid position for growth details"));
		return Result;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	const FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: No crop at position for growth details"));
		return Result;
	}

	// Build environmental factors
	FFarmEnvironment Environment = GetEnvironmentalConditions();
	FEnvironmentalFactors EnvFactors;
	EnvFactors.Temperature = Environment.Temperature;
	EnvFactors.Humidity = Environment.Humidity;
	EnvFactors.Sunlight = Environment.Sunlight;
	EnvFactors.SoilQuality = SoilQuality;
	EnvFactors.WaterLevel = WaterLevel;
	EnvFactors.Fertility = Fertility;

	// Calculate detailed growth information
	Result = UCropGrowthCalculator::CalculateGrowthRate(
		Cell.CropType,
		EnvFactors,
		Cell.Health
	);

	return Result;
}

float AFarmPlot::GetTimeToHarvest(FIntPoint GridPosition) const
{
	if (!IsValidGridPosition(GridPosition))
	{
		return 0.0f;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	const FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		return 0.0f;
	}

	// Build environmental factors
	FFarmEnvironment Environment = GetEnvironmentalConditions();
	FEnvironmentalFactors EnvFactors;
	EnvFactors.Temperature = Environment.Temperature;
	EnvFactors.Humidity = Environment.Humidity;
	EnvFactors.Sunlight = Environment.Sunlight;
	EnvFactors.SoilQuality = SoilQuality;
	EnvFactors.WaterLevel = WaterLevel;
	EnvFactors.Fertility = Fertility;

	// Calculate time to maturity
	return UCropGrowthCalculator::CalculateTimeToMaturity(
		Cell.CropType,
		EnvFactors,
		Cell.GrowthProgress
	);
}

// ============================================================================
// SETUP FUNCTIONS
// ============================================================================

void AFarmPlot::SetPlotSize(FVector2D Size)
{
	PlotSize = Size;
	InitializeCropGrid();
	UpdateSoilVisuals();
}

void AFarmPlot::SetPlanetActor(AActor* Planet)
{
	PlanetActor = Planet;
}

void AFarmPlot::SetInitialSoilQuality(float Quality)
{
	Quality = FMath::Clamp(Quality, 0.0f, 1.0f);
	
	// Set base soil quality and derive other properties from it
	SoilQuality = Quality;
	
	// Initialize nutrients based on base quality
	NitrogenLevel = Quality * 0.9f;
	PhosphorusLevel = Quality * 0.8f;
	PotassiumLevel = Quality * 0.85f;
	OrganicMatter = Quality * 0.6f;
	Fertility = Quality;
	
	// pH varies slightly based on quality
	SoilPH = FMath::Lerp(5.5f, 7.0f, Quality);
	
	// Better soil has less compaction
	Compaction = FMath::Lerp(0.5f, 0.1f, Quality);
	
	UpdateSoilVisuals();
}

// ============================================================================
// PROTECTED HELPERS
// ============================================================================

void AFarmPlot::InitializeCropGrid()
{
	int32 TotalCells = GridResolution * GridResolution;
	CropGrid.Empty(TotalCells);
	CropGrid.SetNum(TotalCells);

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Initialized %dx%d grid (%d cells)"), 
		GridResolution, GridResolution, TotalCells);
}

void AFarmPlot::UpdateCropVisuals()
{
	if (!CropInstances)
	{
		return;
	}

	// Clear existing instances
	CropInstances->ClearInstances();

	// Calculate cell size
	float CellSizeX = PlotSize.X / GridResolution;
	float CellSizeY = PlotSize.Y / GridResolution;

	// Track which mesh we're using for instancing
	UStaticMesh* CurrentMesh = nullptr;
	TArray<FTransform> InstanceTransforms;
	TArray<FLinearColor> InstanceColors;

	// Add instances for each planted crop
	for (int32 Y = 0; Y < GridResolution; ++Y)
	{
		for (int32 X = 0; X < GridResolution; ++X)
		{
			int32 CellIndex = Y * GridResolution + X;
			const FCropCell& Cell = CropGrid[CellIndex];

			if (Cell.CropType && Cell.GrowthProgress > 0.0f)
			{
				// Get the appropriate mesh for current growth stage
				UStaticMesh* StageMesh = Cell.CropType->GetGrowthStageMesh(Cell.GrowthProgress);
				
				if (!StageMesh)
				{
					continue; // Skip if no mesh available
				}

				// If mesh changed, we need to flush current batch and start new one
				if (CurrentMesh != StageMesh)
				{
					// Flush previous batch if any
					if (CurrentMesh && InstanceTransforms.Num() > 0)
					{
						CropInstances->SetStaticMesh(CurrentMesh);
						for (int32 i = 0; i < InstanceTransforms.Num(); ++i)
						{
							CropInstances->AddInstance(InstanceTransforms[i]);
						}
					}

					// Start new batch
					CurrentMesh = StageMesh;
					InstanceTransforms.Empty();
					InstanceColors.Empty();
				}

				// Calculate world position for this cell
				FVector CellOffset(
					(X - GridResolution / 2.0f) * CellSizeX,
					(Y - GridResolution / 2.0f) * CellSizeY,
					0.0f
				);

				// Calculate health visual indicators
				FEnvironmentalFactors EnvFactors;
				FFarmEnvironment Env = GetEnvironmentalConditions();
				EnvFactors.Temperature = Env.Temperature;
				EnvFactors.Humidity = Env.Humidity;
				EnvFactors.Sunlight = Env.Sunlight;
				EnvFactors.SoilQuality = SoilQuality;
				EnvFactors.WaterLevel = WaterLevel;
				EnvFactors.Fertility = Fertility;

				FHealthDegradationFactors DegradationFactors = UCropHealthSystem::CalculateDegradationFactors(
					Cell.CropType,
					EnvFactors,
					WaterLevel,
					Fertility,
					Cell.TimeSinceLastWater
				);

				FHealthVisualIndicators VisualIndicators = UCropHealthSystem::GenerateVisualIndicators(
					Cell.Health,
					DegradationFactors
				);

				// Scale based on growth progress within stage
				// Smooth scaling between stages
				int32 NumStages = Cell.CropType->GetNumGrowthStages();
				float StageProgress = (Cell.CurrentGrowthStage + 1.0f) / FMath::Max(1.0f, (float)NumStages);
				float BaseScale = FMath::Lerp(0.1f, 1.0f, StageProgress);
				
				// Add micro-variation within stage
				float ProgressInStage = GetProgressWithinGrowthStage(FIntPoint(X, Y));
				float StageScaleBoost = ProgressInStage * 0.15f; // Up to 15% growth within stage
				
				// Apply health scale modifier
				float FinalScale = BaseScale * (1.0f + StageScaleBoost) * VisualIndicators.ScaleModifier;
				
				// Apply wilting to scale (makes plant appear to droop)
				if (VisualIndicators.bShowWilting)
				{
					FinalScale *= (1.0f - VisualIndicators.WiltingAmount * 0.2f); // Up to 20% size reduction
				}
				
				// Use health-based color from visual indicators
				FLinearColor HealthColor = VisualIndicators.LeafColor;

				// Add slight random rotation for natural look
				FRotator RandomRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(CellOffset);
				InstanceTransform.SetRotation(RandomRotation.Quaternion());
				InstanceTransform.SetScale3D(FVector(FinalScale));

				InstanceTransforms.Add(InstanceTransform);
				InstanceColors.Add(HealthColor);
			}
		}
	}

	// Flush final batch
	if (CurrentMesh && InstanceTransforms.Num() > 0)
	{
		CropInstances->SetStaticMesh(CurrentMesh);
		for (int32 i = 0; i < InstanceTransforms.Num(); ++i)
		{
			CropInstances->AddInstance(InstanceTransforms[i]);
		}
	}
}

void AFarmPlot::UpdateSoilVisuals()
{
	if (!SoilMesh)
	{
		return;
	}

	// Create a simple quad mesh for the soil
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;

	float HalfX = PlotSize.X / 2.0f;
	float HalfY = PlotSize.Y / 2.0f;

	// Create vertices
	Vertices.Add(FVector(-HalfX, -HalfY, 0.0f));
	Vertices.Add(FVector(HalfX, -HalfY, 0.0f));
	Vertices.Add(FVector(HalfX, HalfY, 0.0f));
	Vertices.Add(FVector(-HalfX, HalfY, 0.0f));

	// Create triangles
	Triangles.Add(0); Triangles.Add(1); Triangles.Add(2);
	Triangles.Add(0); Triangles.Add(2); Triangles.Add(3);

	// Create normals
	for (int32 i = 0; i < 4; ++i)
	{
		Normals.Add(FVector::UpVector);
	}

	// Create UVs
	UVs.Add(FVector2D(0.0f, 0.0f));
	UVs.Add(FVector2D(1.0f, 0.0f));
	UVs.Add(FVector2D(1.0f, 1.0f));
	UVs.Add(FVector2D(0.0f, 1.0f));

	// Calculate soil color based on quality and composition
	FLinearColor SoilColor = CalculateSoilColor();

	for (int32 i = 0; i < 4; ++i)
	{
		VertexColors.Add(SoilColor);
	}

	// Create the mesh
	SoilMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);
}

int32 AFarmPlot::GetCellIndex(FIntPoint GridPosition) const
{
	return GridPosition.Y * GridResolution + GridPosition.X;
}

bool AFarmPlot::IsValidGridPosition(FIntPoint GridPosition) const
{
	return GridPosition.X >= 0 && GridPosition.X < GridResolution &&
		   GridPosition.Y >= 0 && GridPosition.Y < GridResolution;
}

float AFarmPlot::CalculateGrowthRateModifier(const FCropCell& Cell, const FFarmEnvironment& Environment) const
{
	if (!Cell.CropType)
	{
		return 0.0f;
	}

	// Build environmental factors structure
	FEnvironmentalFactors EnvFactors;
	EnvFactors.Temperature = Environment.Temperature;
	EnvFactors.Humidity = Environment.Humidity;
	EnvFactors.Sunlight = Environment.Sunlight;
	EnvFactors.SoilQuality = SoilQuality;
	EnvFactors.WaterLevel = WaterLevel;
	EnvFactors.Fertility = Fertility;

	// Calculate seasonal modifier
	int32 DayOfYear = 180; // Default to mid-year
	if (PlanetActor)
	{
		APlanet* Planet = Cast<APlanet>(PlanetActor);
		if (Planet && Planet->DayNightCycleComponent)
		{
			float TimeOfDay = Planet->DayNightCycleComponent->GetTimeOfDay();
			DayOfYear = FMath::RoundToInt(TimeOfDay * 365.0f / 24.0f) % 365;
		}
	}
	float SeasonalModifier = GetSeasonalGrowthModifier(DayOfYear);

	// Use the comprehensive growth calculator
	FGrowthCalculationResult GrowthResult = UCropGrowthCalculator::CalculateGrowthRate(
		Cell.CropType,
		EnvFactors,
		Cell.Health,
		SeasonalModifier
	);

	// Log limiting factor if growth is significantly reduced
	if (GrowthResult.TotalGrowthModifier < 0.5f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("FarmPlot: Crop growth limited by %s (Modifier: %.2f)"),
			*GrowthResult.LimitingFactor, GrowthResult.TotalGrowthModifier);
	}

	return GrowthResult.TotalGrowthModifier;
}

void AFarmPlot::DepleteSoilNutrients(const FCropCell& Cell, float DeltaTime)
{
	if (!Cell.CropType)
	{
		return;
	}

	// Calculate depletion rate based on crop growth rate
	float DepletionRate = DeltaTime / Cell.CropType->GrowthDuration;

	// Different crops deplete different nutrients at different rates
	// Leafy crops (high nitrogen), root crops (high phosphorus), fruiting crops (high potassium)
	float NitrogenDepletion = DepletionRate * 0.15f; // 15% of growth time
	float PhosphorusDepletion = DepletionRate * 0.10f; // 10% of growth time
	float PotassiumDepletion = DepletionRate * 0.12f; // 12% of growth time

	NitrogenLevel = FMath::Max(0.0f, NitrogenLevel - NitrogenDepletion);
	PhosphorusLevel = FMath::Max(0.0f, PhosphorusLevel - PhosphorusDepletion);
	PotassiumLevel = FMath::Max(0.0f, PotassiumLevel - PotassiumDepletion);

	// Organic matter slowly depletes as it's consumed
	OrganicMatter = FMath::Max(0.0f, OrganicMatter - (DepletionRate * 0.05f));
}

void AFarmPlot::RegenerateSoil(float DeltaTime)
{
	// Very slow natural regeneration (would take hours to fully regenerate)
	float RegenerationRate = DeltaTime * 0.0001f; // 0.01% per second

	// Organic matter helps regenerate nutrients
	float OrganicBonus = OrganicMatter * 0.5f;

	NitrogenLevel = FMath::Min(0.8f, NitrogenLevel + (RegenerationRate * (1.0f + OrganicBonus)));
	PhosphorusLevel = FMath::Min(0.7f, PhosphorusLevel + (RegenerationRate * 0.5f));
	PotassiumLevel = FMath::Min(0.7f, PotassiumLevel + (RegenerationRate * 0.5f));

	// Organic matter slowly decomposes
	OrganicMatter = FMath::Max(0.1f, OrganicMatter - (RegenerationRate * 0.2f));

	// Compaction slowly reduces if soil is left fallow
	if (CropGrid.Num() > 0)
	{
		bool bHasActiveCrops = false;
		for (const FCropCell& Cell : CropGrid)
		{
			if (Cell.CropType)
			{
				bHasActiveCrops = true;
				break;
			}
		}

		if (!bHasActiveCrops)
		{
			Compaction = FMath::Max(0.1f, Compaction - (RegenerationRate * 2.0f));
		}
	}
}

FLinearColor AFarmPlot::CalculateSoilColor() const
{
	// Base color from soil quality
	FLinearColor BaseColor = FMath::Lerp(
		FLinearColor(0.45f, 0.35f, 0.25f), // Poor soil (light tan)
		FLinearColor(0.15f, 0.10f, 0.05f), // Rich soil (dark brown)
		SoilQuality
	);

	// Organic matter adds darker, richer tones
	FLinearColor OrganicColor = FLinearColor(0.08f, 0.05f, 0.02f);
	BaseColor = FMath::Lerp(BaseColor, OrganicColor, OrganicMatter * 0.5f);

	// Low nitrogen makes soil more gray/pale
	if (NitrogenLevel < 0.3f)
	{
		FLinearColor PaleColor = FLinearColor(0.5f, 0.45f, 0.4f);
		BaseColor = FMath::Lerp(BaseColor, PaleColor, (0.3f - NitrogenLevel));
	}

	// High compaction makes soil lighter and more gray
	if (Compaction > 0.5f)
	{
		FLinearColor CompactedColor = FLinearColor(0.4f, 0.38f, 0.35f);
		BaseColor = FMath::Lerp(BaseColor, CompactedColor, (Compaction - 0.5f) * 2.0f);
	}

	// Moisture affects color (darker when wet)
	if (WaterLevel > 0.5f)
	{
		BaseColor *= FMath::Lerp(1.0f, 0.7f, (WaterLevel - 0.5f) * 2.0f);
	}

	return BaseColor;
}

int32 AFarmPlot::GetCropGrowthStage(FIntPoint GridPosition) const
{
	if (!IsValidGridPosition(GridPosition))
	{
		return -1;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	const FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		return -1;
	}

	return Cell.CurrentGrowthStage;
}

FString AFarmPlot::GetCropGrowthStageName(FIntPoint GridPosition) const
{
	if (!IsValidGridPosition(GridPosition))
	{
		return TEXT("Invalid");
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	const FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		return TEXT("Empty");
	}

	return Cell.CropType->GetGrowthStageName(Cell.GrowthProgress);
}

float AFarmPlot::GetProgressWithinGrowthStage(FIntPoint GridPosition) const
{
	if (!IsValidGridPosition(GridPosition))
	{
		return 0.0f;
	}

	int32 CellIndex = GetCellIndex(GridPosition);
	const FCropCell& Cell = CropGrid[CellIndex];

	if (!Cell.CropType)
	{
		return 0.0f;
	}

	int32 NumStages = Cell.CropType->GetNumGrowthStages();
	if (NumStages <= 0)
	{
		return 0.0f;
	}

	// Calculate progress within current stage
	float StageSize = 1.0f / NumStages;
	float StageStart = Cell.CurrentGrowthStage * StageSize;
	float ProgressInStage = Cell.GrowthProgress - StageStart;

	return FMath::Clamp(ProgressInStage / StageSize, 0.0f, 1.0f);
}

void AFarmPlot::UpdateCropGrowthStage(FCropCell& Cell)
{
	if (!Cell.CropType)
	{
		return;
	}

	int32 NewStage = Cell.CropType->GetGrowthStageIndex(Cell.GrowthProgress);
	if (NewStage != Cell.CurrentGrowthStage)
	{
		int32 OldStage = Cell.CurrentGrowthStage;
		Cell.CurrentGrowthStage = NewStage;
		OnCropStageChanged(Cell, OldStage, NewStage);
	}
}

void AFarmPlot::OnCropStageChanged(FCropCell& Cell, int32 OldStage, int32 NewStage)
{
	if (!Cell.CropType)
	{
		return;
	}

	FString StageName = Cell.CropType->GetGrowthStageName(Cell.GrowthProgress);

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Crop '%s' advanced from stage %d to stage %d (%s) - Progress: %.1f%%"),
		*Cell.CropType->CropName, OldStage, NewStage, *StageName, Cell.GrowthProgress * 100.0f);

	// Could trigger visual effects, sounds, or notifications here
	// For example:
	// - Play a growth sound effect
	// - Show a particle effect
	// - Update UI notifications
	// - Trigger achievements
}

// ============================================================================
// WEATHER INTEGRATION (Task 13.4)
// ============================================================================

void AFarmPlot::ApplyRainWatering(float PrecipitationRate, float DeltaTime)
{
	if (PrecipitationRate <= 0.0f)
	{
		return;
	}

	// Convert precipitation rate (mm/hour) to water level increase
	// Assume 1mm of rain = 0.01 water level increase per hour
	float WaterIncrease = (PrecipitationRate * DeltaTime) / 3600.0f * 0.01f;
	
	// Accumulate rainfall for tracking
	AccumulatedRainfall += PrecipitationRate * (DeltaTime / 3600.0f);
	
	// Apply water to plot
	WaterLevel = FMath::Clamp(WaterLevel + WaterIncrease, 0.0f, 1.0f);
	
	// Reset water needs for all crops
	FDateTime CurrentTime = FDateTime::Now();
	for (FCropCell& Cell : CropGrid)
	{
		if (Cell.CropType)
		{
			Cell.bNeedsWater = false;
			Cell.TimeSinceLastWater = 0.0f;
			Cell.LastWateredTime = CurrentTime;
		}
	}
	
	// Log significant rainfall events
	if (PrecipitationRate > 5.0f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("FarmPlot: Heavy rain watering crops (%.1f mm/hr, Water: %.2f)"),
			PrecipitationRate, WaterLevel);
	}
}

void AFarmPlot::ApplyStormDamage(float StormIntensity, float DeltaTime)
{
	if (StormIntensity <= 0.0f)
	{
		return;
	}
	
	// Only check for storm damage periodically to avoid excessive calculations
	TimeSinceLastStormCheck += DeltaTime;
	if (TimeSinceLastStormCheck < StormCheckInterval)
	{
		return;
	}
	TimeSinceLastStormCheck = 0.0f;
	
	// Calculate damage based on storm intensity
	// Storms can damage crops through wind, hail, flooding, etc.
	float BaseDamage = StormIntensity * 0.05f; // Up to 5% damage per check interval
	
	int32 DamagedCrops = 0;
	float TotalDamage = 0.0f;
	
	for (FCropCell& Cell : CropGrid)
	{
		if (!Cell.CropType)
		{
			continue;
		}
		
		// Younger crops are more vulnerable to storm damage
		float VulnerabilityFactor = FMath::Lerp(2.0f, 0.5f, Cell.GrowthProgress);
		
		// Taller crops (later stages) are more susceptible to wind damage
		float HeightFactor = 1.0f + (Cell.CurrentGrowthStage * 0.2f);
		
		// Calculate actual damage for this crop
		float CropDamage = BaseDamage * VulnerabilityFactor * HeightFactor;
		
		// Add some randomness (some crops get hit harder than others)
		CropDamage *= FMath::FRandRange(0.5f, 1.5f);
		
		// Apply damage to health
		float OldHealth = Cell.Health;
		Cell.Health = FMath::Max(0.0f, Cell.Health - CropDamage);
		
		if (Cell.Health < OldHealth)
		{
			DamagedCrops++;
			TotalDamage += (OldHealth - Cell.Health);
		}
		
		// Severe storms can destroy very weak crops
		if (StormIntensity > 0.8f && Cell.Health < 0.1f)
		{
			// Crop is destroyed
			Cell.CropType = nullptr;
			Cell.GrowthProgress = 0.0f;
			Cell.Health = 1.0f;
			Cell.CurrentGrowthStage = 0;
			
			UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Crop destroyed by severe storm!"));
		}
	}
	
	// Log storm damage events
	if (DamagedCrops > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Storm damaged %d crops (Intensity: %.2f, Avg Damage: %.2f)"),
			DamagedCrops, StormIntensity, TotalDamage / FMath::Max(1, DamagedCrops));
	}
	
	// Update visuals to show damaged crops
	UpdateCropVisuals();
}

void AFarmPlot::ApplyTemperatureEffects(float Temperature, float DeltaTime)
{
	// Temperature effects are continuous and gradual
	for (FCropCell& Cell : CropGrid)
	{
		if (!Cell.CropType)
		{
			continue;
		}
		
		// Get optimal temperature range for this crop
		float OptimalMin = Cell.CropType->OptimalTemperature.Min;
		float OptimalMax = Cell.CropType->OptimalTemperature.Max;
		
		// Calculate how far outside optimal range we are
		float TemperatureDeviation = 0.0f;
		if (Temperature < OptimalMin)
		{
			TemperatureDeviation = OptimalMin - Temperature;
		}
		else if (Temperature > OptimalMax)
		{
			TemperatureDeviation = Temperature - OptimalMax;
		}
		
		// Apply damage if temperature is significantly outside optimal range
		if (TemperatureDeviation > TemperatureDamageThreshold)
		{
			// Damage increases with deviation
			float ExcessDeviation = TemperatureDeviation - TemperatureDamageThreshold;
			float Damage = ExcessDeviation * TemperatureDamageRate * DeltaTime;
			
			Cell.Health = FMath::Max(0.0f, Cell.Health - Damage);
			
			// Log extreme temperature damage
			if (Damage > 0.01f)
			{
				UE_LOG(LogTemp, Verbose, TEXT("FarmPlot: Crop taking temperature damage (Temp: %.1f°C, Optimal: %.1f-%.1f°C, Damage: %.3f)"),
					Temperature, OptimalMin, OptimalMax, Damage);
			}
		}
		
		// Extreme temperatures can kill crops quickly
		if (Temperature < -10.0f || Temperature > 50.0f)
		{
			// Rapid damage from extreme temperatures
			float ExtremeDamage = 0.1f * DeltaTime; // 10% per second
			Cell.Health = FMath::Max(0.0f, Cell.Health - ExtremeDamage);
			
			if (Cell.Health <= 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("FarmPlot: Crop killed by extreme temperature (%.1f°C)"), Temperature);
				
				// Crop is dead
				Cell.CropType = nullptr;
				Cell.GrowthProgress = 0.0f;
				Cell.Health = 1.0f;
				Cell.CurrentGrowthStage = 0;
			}
		}
	}
}

float AFarmPlot::GetSeasonalGrowthModifier(int32 DayOfYear) const
{
	// Create seasonal variation based on day of year
	// Assume 365 day year with seasons:
	// Spring (Day 0-90): Growing season, 1.2x growth
	// Summer (Day 91-180): Peak season, 1.5x growth
	// Fall (Day 181-270): Harvest season, 1.0x growth
	// Winter (Day 271-365): Dormant season, 0.5x growth
	
	DayOfYear = DayOfYear % 365; // Wrap to 0-365 range
	
	float SeasonalModifier = 1.0f;
	
	if (DayOfYear < 90)
	{
		// Spring - gradual increase
		SeasonalModifier = FMath::Lerp(0.8f, 1.2f, DayOfYear / 90.0f);
	}
	else if (DayOfYear < 180)
	{
		// Summer - peak growth
		float SummerProgress = (DayOfYear - 90) / 90.0f;
		SeasonalModifier = FMath::Lerp(1.2f, 1.5f, SummerProgress);
	}
	else if (DayOfYear < 270)
	{
		// Fall - gradual decrease
		float FallProgress = (DayOfYear - 180) / 90.0f;
		SeasonalModifier = FMath::Lerp(1.5f, 1.0f, FallProgress);
	}
	else
	{
		// Winter - low growth
		float WinterProgress = (DayOfYear - 270) / 95.0f;
		SeasonalModifier = FMath::Lerp(1.0f, 0.5f, WinterProgress);
	}
	
	return SeasonalModifier;
}

void AFarmPlot::UpdateWeatherEffects(float DeltaTime)
{
	// Get weather component reference if we don't have it
	if (!WeatherComponent && PlanetActor)
	{
		APlanet* Planet = Cast<APlanet>(PlanetActor);
		if (Planet)
		{
			WeatherComponent = Planet->WeatherComponent;
		}
	}
	
	if (!WeatherComponent)
	{
		return;
	}
	
	// Get current weather conditions
	float Temperature = WeatherComponent->GetCurrentTemperature();
	float PrecipitationRate = WeatherComponent->GetPrecipitationRate();
	EWeatherType CurrentWeather = WeatherComponent->GetCurrentWeather();
	
	// Apply rain watering
	if (PrecipitationRate > 0.0f)
	{
		ApplyRainWatering(PrecipitationRate, DeltaTime);
	}
	
	// Apply storm damage for stormy weather types
	bool bIsStormy = WeatherComponent->IsStormyWeather(CurrentWeather);
	if (bIsStormy)
	{
		// Calculate storm intensity based on weather type
		float StormIntensity = 0.0f;
		switch (CurrentWeather)
		{
			case EWeatherType::HeavyRain:
				StormIntensity = 0.5f;
				break;
			case EWeatherType::Thunderstorm:
				StormIntensity = 0.8f;
				break;
			case EWeatherType::Blizzard:
				StormIntensity = 0.9f;
				break;
			case EWeatherType::Sandstorm:
				StormIntensity = 0.7f;
				break;
			default:
				StormIntensity = 0.0f;
				break;
		}
		
		if (StormIntensity > 0.0f)
		{
			ApplyStormDamage(StormIntensity, DeltaTime);
		}
	}
	
	// Apply temperature effects
	ApplyTemperatureEffects(Temperature, DeltaTime);
	
	// Apply seasonal growth variations
	// Get day of year from planet's day-night cycle if available
	int32 DayOfYear = 180; // Default to mid-year
	if (PlanetActor)
	{
		APlanet* Planet = Cast<APlanet>(PlanetActor);
		if (Planet && Planet->DayNightCycleComponent)
		{
			// Calculate day of year from time of day
			// This is a simplified calculation - in a real system you'd track actual days
			float TimeOfDay = Planet->DayNightCycleComponent->GetTimeOfDay();
			DayOfYear = FMath::RoundToInt(TimeOfDay * 365.0f / 24.0f) % 365;
		}
	}
	
	float SeasonalModifier = GetSeasonalGrowthModifier(DayOfYear);
	
	// Apply seasonal modifier to growth (this is handled in the growth calculation)
	// We could store this for use in CalculateGrowthRateModifier
}

// ============================================================================
// BIOME SOIL VARIATION (Task 14.2)
// ============================================================================

void AFarmPlot::ApplyBiomeSoilVariation(const FSoilVariation& SoilVar)
{
	// Apply soil quality (clamped to 0.3-1.0 range per requirements)
	SoilQuality = FMath::Clamp(SoilVar.SoilQuality, 0.3f, 1.0f);

	// Store biome soil color for visual representation
	BiomeSoilColor = SoilVar.SoilColor;
	SoilRoughness = SoilVar.Roughness;
	SoilDrainage = SoilVar.Drainage;

	// Apply soil texture properties
	// These affect how the soil behaves and looks
	
	// Roughness affects drainage and compaction
	// Sandy soil (high roughness) = better drainage, less compaction
	// Clay soil (low roughness) = poor drainage, more compaction
	float RoughnessFactor = SoilVar.Roughness;
	Compaction = FMath::Lerp(0.6f, 0.1f, RoughnessFactor); // High roughness = low compaction
	
	// Moisture affects initial water level
	WaterLevel = FMath::Clamp(SoilVar.Moisture, 0.0f, 1.0f);
	
	// Set initial fertility based on soil quality
	Fertility = SoilQuality;
	
	// Set nutrient levels based on soil quality
	// Higher quality soil has better nutrient levels
	NitrogenLevel = FMath::Clamp(SoilQuality * 0.9f, 0.3f, 1.0f);
	PhosphorusLevel = FMath::Clamp(SoilQuality * 0.85f, 0.3f, 1.0f);
	PotassiumLevel = FMath::Clamp(SoilQuality * 0.8f, 0.3f, 1.0f);
	
	// Organic matter correlates with soil quality
	OrganicMatter = FMath::Clamp(SoilQuality * 0.7f, 0.2f, 0.9f);
	
	// Soil pH varies by biome but we'll keep it moderate for now
	// Could be extended to vary by biome type in the future
	SoilPH = 6.5f;

	// Update visual representation with new soil color
	UpdateSoilVisuals();

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Applied biome soil variation - Quality: %.2f, Moisture: %.2f, Roughness: %.2f, Biome: %s"),
		SoilVar.SoilQuality, SoilVar.Moisture, SoilVar.Roughness, *SoilVar.BiomeName);
}

// ============================================================================
// HARVEST TRACKING IMPLEMENTATION (Task 17.4)
// ============================================================================

void AFarmPlot::RecordHarvest(const FHarvestResult& HarvestResult, FIntPoint GridPosition, const FCropCell& CropCell)
{
	if (!HarvestResult.bSuccess || !CropCell.CropType)
	{
		return;
	}

	// Create harvest record
	FHarvestRecord Record;
	Record.HarvestTime = FDateTime::Now();
	Record.CropName = HarvestResult.ItemName;
	Record.CropType = CropCell.CropType;
	Record.YieldAmount = HarvestResult.YieldAmount;
	Record.Quality = HarvestResult.Quality;
	Record.GridPosition = GridPosition;

	// Calculate growth duration
	FTimespan GrowthTime = Record.HarvestTime - CropCell.PlantedTime;
	Record.GrowthDuration = GrowthTime.GetTotalSeconds();

	// Calculate average soil quality and fertility during growth
	if (QualitySampleCount > 0)
	{
		Record.AverageSoilQuality = AccumulatedSoilQuality / QualitySampleCount;
		Record.AverageFertility = AccumulatedFertility / QualitySampleCount;
	}
	else
	{
		Record.AverageSoilQuality = SoilQuality;
		Record.AverageFertility = Fertility;
	}

	// Calculate economic value
	Record.EconomicValue = CalculateEconomicValue(CropCell.CropType, HarvestResult.YieldAmount, HarvestResult.Quality);

	// Add to history
	HarvestHistory.Add(Record);

	// Trim history if it exceeds maximum
	if (HarvestHistory.Num() > MaxHarvestRecords)
	{
		TrimHarvestHistory(MaxHarvestRecords);
	}

	// Reset quality accumulators for next crop
	AccumulatedSoilQuality = 0.0f;
	AccumulatedFertility = 0.0f;
	QualitySampleCount = 0;

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Recorded harvest - %s x%d, Value: %.2f, Quality: %.2f, Duration: %.1fs"),
		*Record.CropName, Record.YieldAmount, Record.EconomicValue, Record.Quality, Record.GrowthDuration);
}

TArray<FHarvestRecord> AFarmPlot::GetHarvestHistory() const
{
	return HarvestHistory;
}

TArray<FHarvestRecord> AFarmPlot::GetHarvestHistoryForCrop(UCropDefinition* CropType) const
{
	TArray<FHarvestRecord> FilteredHistory;

	if (!CropType)
	{
		return FilteredHistory;
	}

	for (const FHarvestRecord& Record : HarvestHistory)
	{
		if (Record.CropType == CropType)
		{
			FilteredHistory.Add(Record);
		}
	}

	return FilteredHistory;
}

FHarvestStatistics AFarmPlot::GetHarvestStatistics(UCropDefinition* CropType) const
{
	FHarvestStatistics Stats;

	if (!CropType)
	{
		return Stats;
	}

	Stats.CropType = CropType;
	Stats.CropName = CropType->CropName;

	// Get all harvests for this crop
	TArray<FHarvestRecord> CropHarvests = GetHarvestHistoryForCrop(CropType);

	if (CropHarvests.Num() == 0)
	{
		return Stats;
	}

	Stats.TotalHarvests = CropHarvests.Num();

	// Calculate totals and find best values
	float TotalQuality = 0.0f;
	float TotalGrowthDuration = 0.0f;

	for (const FHarvestRecord& Record : CropHarvests)
	{
		Stats.TotalYield += Record.YieldAmount;
		Stats.TotalEconomicValue += Record.EconomicValue;
		TotalQuality += Record.Quality;
		TotalGrowthDuration += Record.GrowthDuration;

		// Track best values
		if (Record.Quality > Stats.BestQuality)
		{
			Stats.BestQuality = Record.Quality;
		}

		if (Record.YieldAmount > Stats.BestYield)
		{
			Stats.BestYield = Record.YieldAmount;
		}

		// Track first and last harvest times
		if (Stats.FirstHarvest.GetTicks() == 0 || Record.HarvestTime < Stats.FirstHarvest)
		{
			Stats.FirstHarvest = Record.HarvestTime;
		}

		if (Record.HarvestTime > Stats.LastHarvest)
		{
			Stats.LastHarvest = Record.HarvestTime;
		}
	}

	// Calculate averages
	Stats.AverageYield = (float)Stats.TotalYield / Stats.TotalHarvests;
	Stats.AverageQuality = TotalQuality / Stats.TotalHarvests;
	Stats.AverageEconomicValue = Stats.TotalEconomicValue / Stats.TotalHarvests;
	Stats.AverageGrowthDuration = TotalGrowthDuration / Stats.TotalHarvests;

	return Stats;
}

TArray<FHarvestStatistics> AFarmPlot::GetAllHarvestStatistics() const
{
	TArray<FHarvestStatistics> AllStats;

	// Get unique crop types from harvest history
	TSet<UCropDefinition*> UniqueCrops;
	for (const FHarvestRecord& Record : HarvestHistory)
	{
		if (Record.CropType)
		{
			UniqueCrops.Add(Record.CropType);
		}
	}

	// Calculate statistics for each crop type
	for (UCropDefinition* CropType : UniqueCrops)
	{
		FHarvestStatistics Stats = GetHarvestStatistics(CropType);
		if (Stats.TotalHarvests > 0)
		{
			AllStats.Add(Stats);
		}
	}

	// Sort by total economic value (descending)
	AllStats.Sort([](const FHarvestStatistics& A, const FHarvestStatistics& B)
	{
		return A.TotalEconomicValue > B.TotalEconomicValue;
	});

	return AllStats;
}

float AFarmPlot::GetTotalEconomicValue() const
{
	float TotalValue = 0.0f;

	for (const FHarvestRecord& Record : HarvestHistory)
	{
		TotalValue += Record.EconomicValue;
	}

	return TotalValue;
}

float AFarmPlot::GetEconomicValueForCrop(UCropDefinition* CropType) const
{
	if (!CropType)
	{
		return 0.0f;
	}

	float TotalValue = 0.0f;

	for (const FHarvestRecord& Record : HarvestHistory)
	{
		if (Record.CropType == CropType)
		{
			TotalValue += Record.EconomicValue;
		}
	}

	return TotalValue;
}

void AFarmPlot::TrimHarvestHistory(int32 MaxRecords)
{
	if (HarvestHistory.Num() <= MaxRecords)
	{
		return;
	}

	// Sort by harvest time (newest first)
	HarvestHistory.Sort([](const FHarvestRecord& A, const FHarvestRecord& B)
	{
		return A.HarvestTime > B.HarvestTime;
	});

	// Keep only the most recent records
	int32 RecordsToRemove = HarvestHistory.Num() - MaxRecords;
	HarvestHistory.RemoveAt(MaxRecords, RecordsToRemove);

	UE_LOG(LogTemp, Log, TEXT("FarmPlot: Trimmed harvest history - Removed %d old records, keeping %d"),
		RecordsToRemove, HarvestHistory.Num());
}

float AFarmPlot::CalculateEconomicValue(UCropDefinition* CropType, int32 YieldAmount, float Quality) const
{
	if (!CropType)
	{
		return 0.0f;
	}

	// Base value from crop definition
	float BasePrice = CropType->SellPrice;

	// Quality affects price (0.5x to 1.5x multiplier)
	float QualityMultiplier = FMath::Lerp(0.5f, 1.5f, Quality);

	// Calculate total value
	float TotalValue = BasePrice * YieldAmount * QualityMultiplier;

	return TotalValue;
}
