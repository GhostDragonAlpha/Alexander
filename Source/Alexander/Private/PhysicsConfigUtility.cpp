// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsConfigUtility.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetData.h"

#define GRAVITY_CONSTANT 9.81f

APhysicsConfigUtility::APhysicsConfigUtility()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APhysicsConfigUtility::BeginPlay()
{
	Super::BeginPlay();
}

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

FPhysicsConfigValidation APhysicsConfigUtility::ValidatePhysicsConfig(UShipPhysicsConfig* Config)
{
	FPhysicsConfigValidation Result;

	if (!Config)
	{
		Result.bIsValid = false;
		Result.Errors.Add("Config is null");
		return Result;
	}

	// Validate individual parameters
	ValidateMass(Config, Result);
	ValidateThrust(Config, Result);
	ValidateVelocity(Config, Result);
	ValidateAngularThrust(Config, Result);
	ValidateMultipliers(Config, Result);
	ValidateBalance(Config, Result);

	// Calculate metrics
	Result.ThrustToWeightRatio = CalculateThrustToWeightRatio(Config);
	Result.MaxAcceleration = CalculateMaxAcceleration(Config);
	Result.TimeToMaxVelocity = CalculateTimeToMaxVelocity(Config);

	// Final validation
	Result.bIsValid = (Result.Errors.Num() == 0);

	return Result;
}

bool APhysicsConfigUtility::IsPhysicsConfigValid(UShipPhysicsConfig* Config)
{
	FPhysicsConfigValidation Validation = ValidatePhysicsConfig(Config);
	return Validation.bIsValid;
}

float APhysicsConfigUtility::CalculateThrustToWeightRatio(UShipPhysicsConfig* Config)
{
	if (!Config || Config->Mass <= 0.0f)
	{
		return 0.0f;
	}

	float Weight = Config->Mass * GRAVITY_CONSTANT;
	return Config->ThrustPower / Weight;
}

float APhysicsConfigUtility::CalculateMaxAcceleration(UShipPhysicsConfig* Config)
{
	if (!Config || Config->Mass <= 0.0f)
	{
		return 0.0f;
	}

	// F = ma, so a = F/m
	return Config->ThrustPower / Config->Mass;
}

float APhysicsConfigUtility::CalculateTimeToMaxVelocity(UShipPhysicsConfig* Config)
{
	if (!Config)
	{
		return 0.0f;
	}

	float Acceleration = CalculateMaxAcceleration(Config);
	if (Acceleration <= 0.0f)
	{
		return 0.0f;
	}

	// t = v / a (assuming constant acceleration)
	// Apply acceleration multiplier
	float EffectiveAcceleration = Acceleration * Config->AccelerationMultiplier;
	return Config->MaxVelocity / EffectiveAcceleration;
}

// ============================================================================
// JSON IMPORT/EXPORT
// ============================================================================

int32 APhysicsConfigUtility::LoadPhysicsConfigsFromJSON(const FString& JSONFilePath, const FString& OutputPath)
{
	// Read JSON file
	FString JSONString;
	if (!FFileHelper::LoadFileToString(JSONString, *JSONFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file: %s"), *JSONFilePath);
		return 0;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON file: %s"), *JSONFilePath);
		return 0;
	}

	// Get configs array
	const TArray<TSharedPtr<FJsonValue>>* ConfigsArray;
	if (!JsonObject->TryGetArrayField(TEXT("configs"), ConfigsArray))
	{
		UE_LOG(LogTemp, Error, TEXT("JSON file missing 'configs' array"));
		return 0;
	}

	int32 CreatedCount = 0;

	// Process each config
	for (const TSharedPtr<FJsonValue>& ConfigValue : *ConfigsArray)
	{
		TSharedPtr<FJsonObject> ConfigObj = ConfigValue->AsObject();
		if (!ConfigObj.IsValid())
		{
			continue;
		}

		// Extract data
		FString ConfigName = ConfigObj->GetStringField(TEXT("ConfigName"));
		FString Description = ConfigObj->GetStringField(TEXT("Description"));

		// Create asset name (e.g., "Light Fighter" -> "DA_LightFighter")
		FString AssetName = TEXT("DA_") + ConfigName.Replace(TEXT(" "), TEXT(""));

		// Create the asset
		UShipPhysicsConfig* NewConfig = CreatePhysicsConfigAsset(AssetName, ConfigName, Description, OutputPath);

		if (NewConfig)
		{
			// Set physics parameters
			NewConfig->Mass = ConfigObj->GetNumberField(TEXT("Mass"));
			NewConfig->ThrustPower = ConfigObj->GetNumberField(TEXT("ThrustPower"));
			NewConfig->MaxVelocity = ConfigObj->GetNumberField(TEXT("MaxVelocity"));
			NewConfig->AngularThrustPower = ConfigObj->GetNumberField(TEXT("AngularThrustPower"));
			NewConfig->FuelCapacity = ConfigObj->GetNumberField(TEXT("FuelCapacity"));
			NewConfig->FuelConsumptionRate = ConfigObj->GetNumberField(TEXT("FuelConsumptionRate"));
			NewConfig->AccelerationMultiplier = ConfigObj->GetNumberField(TEXT("AccelerationMultiplier"));
			NewConfig->RotationSpeedMultiplier = ConfigObj->GetNumberField(TEXT("RotationSpeedMultiplier"));
			NewConfig->StabilityAssistStrength = ConfigObj->GetNumberField(TEXT("StabilityAssistStrength"));

			// Mark package dirty
			NewConfig->MarkPackageDirty();

			UE_LOG(LogTemp, Log, TEXT("Created physics config: %s"), *ConfigName);
			CreatedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully created %d physics configs from JSON"), CreatedCount);
	return CreatedCount;
}

UShipPhysicsConfig* APhysicsConfigUtility::CreatePhysicsConfigAsset(
	const FString& AssetName,
	const FString& ConfigName,
	const FString& Description,
	const FString& OutputPath)
{
	// This function would need to use the asset tools to create a data asset
	// For now, return nullptr and log that this needs to be done in editor
	UE_LOG(LogTemp, Warning, TEXT("CreatePhysicsConfigAsset: Asset creation requires editor mode"));
	UE_LOG(LogTemp, Warning, TEXT("  Asset Name: %s"), *AssetName);
	UE_LOG(LogTemp, Warning, TEXT("  Config Name: %s"), *ConfigName);
	UE_LOG(LogTemp, Warning, TEXT("  Output Path: %s"), *OutputPath);

	// TODO: Implement actual asset creation using IAssetTools
	// This requires the AssetTools module which is editor-only

	return nullptr;
}

FString APhysicsConfigUtility::ExportPhysicsConfigToJSON(UShipPhysicsConfig* Config)
{
	if (!Config)
	{
		return TEXT("{}");
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("ConfigName"), Config->ConfigName.ToString());
	JsonObject->SetStringField(TEXT("Description"), Config->Description.ToString());
	JsonObject->SetNumberField(TEXT("Mass"), Config->Mass);
	JsonObject->SetNumberField(TEXT("ThrustPower"), Config->ThrustPower);
	JsonObject->SetNumberField(TEXT("MaxVelocity"), Config->MaxVelocity);
	JsonObject->SetNumberField(TEXT("AngularThrustPower"), Config->AngularThrustPower);
	JsonObject->SetNumberField(TEXT("FuelCapacity"), Config->FuelCapacity);
	JsonObject->SetNumberField(TEXT("FuelConsumptionRate"), Config->FuelConsumptionRate);
	JsonObject->SetNumberField(TEXT("AccelerationMultiplier"), Config->AccelerationMultiplier);
	JsonObject->SetNumberField(TEXT("RotationSpeedMultiplier"), Config->RotationSpeedMultiplier);
	JsonObject->SetNumberField(TEXT("StabilityAssistStrength"), Config->StabilityAssistStrength);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

int32 APhysicsConfigUtility::ExportAllPhysicsConfigsToJSON(const FString& ConfigDirectory, const FString& OutputFilePath)
{
	TArray<UShipPhysicsConfig*> AllConfigs = GetAllPhysicsConfigs();

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ConfigsArray;

	for (UShipPhysicsConfig* Config : AllConfigs)
	{
		if (!Config)
		{
			continue;
		}

		FString ConfigJSON = ExportPhysicsConfigToJSON(Config);
		TSharedPtr<FJsonObject> ConfigObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ConfigJSON);

		if (FJsonSerializer::Deserialize(Reader, ConfigObject))
		{
			ConfigsArray.Add(MakeShareable(new FJsonValueObject(ConfigObject)));
		}
	}

	RootObject->SetArrayField(TEXT("configs"), ConfigsArray);
	RootObject->SetNumberField(TEXT("total_configs"), ConfigsArray.Num());

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	if (FFileHelper::SaveStringToFile(OutputString, *OutputFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Exported %d configs to %s"), ConfigsArray.Num(), *OutputFilePath);
		return ConfigsArray.Num();
	}

	return 0;
}

// ============================================================================
// EDITOR UTILITIES
// ============================================================================

TArray<UShipPhysicsConfig*> APhysicsConfigUtility::GetAllPhysicsConfigs()
{
	TArray<UShipPhysicsConfig*> Result;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;

	AssetRegistryModule.Get().GetAssetsByClass(UShipPhysicsConfig::StaticClass()->GetClassPathName(), AssetData);

	for (const FAssetData& Asset : AssetData)
	{
		UShipPhysicsConfig* Config = Cast<UShipPhysicsConfig>(Asset.GetAsset());
		if (Config)
		{
			Result.Add(Config);
		}
	}

	return Result;
}

UShipPhysicsConfig* APhysicsConfigUtility::FindPhysicsConfigByName(const FString& ConfigName)
{
	TArray<UShipPhysicsConfig*> AllConfigs = GetAllPhysicsConfigs();

	for (UShipPhysicsConfig* Config : AllConfigs)
	{
		if (Config && Config->ConfigName.ToString().Equals(ConfigName, ESearchCase::IgnoreCase))
		{
			return Config;
		}
	}

	return nullptr;
}

FString APhysicsConfigUtility::ComparePhysicsConfigs(UShipPhysicsConfig* ConfigA, UShipPhysicsConfig* ConfigB)
{
	if (!ConfigA || !ConfigB)
	{
		return TEXT("Error: One or both configs are null");
	}

	FString Comparison;

	Comparison += FString::Printf(TEXT("Comparing: %s vs %s\n\n"), *ConfigA->ConfigName.ToString(), *ConfigB->ConfigName.ToString());

	// Mass
	float MassDiff = ConfigB->Mass - ConfigA->Mass;
	float MassPercent = (ConfigA->Mass > 0) ? (MassDiff / ConfigA->Mass * 100.0f) : 0.0f;
	Comparison += FString::Printf(TEXT("Mass: %.0f kg vs %.0f kg (%.1f%% %s)\n"),
		ConfigA->Mass, ConfigB->Mass, FMath::Abs(MassPercent), MassPercent > 0 ? TEXT("heavier") : TEXT("lighter"));

	// Thrust
	float ThrustDiff = ConfigB->ThrustPower - ConfigA->ThrustPower;
	float ThrustPercent = (ConfigA->ThrustPower > 0) ? (ThrustDiff / ConfigA->ThrustPower * 100.0f) : 0.0f;
	Comparison += FString::Printf(TEXT("Thrust: %.0f N vs %.0f N (%.1f%% %s)\n"),
		ConfigA->ThrustPower, ConfigB->ThrustPower, FMath::Abs(ThrustPercent), ThrustPercent > 0 ? TEXT("more") : TEXT("less"));

	// Max Velocity
	float VelDiff = ConfigB->MaxVelocity - ConfigA->MaxVelocity;
	float VelPercent = (ConfigA->MaxVelocity > 0) ? (VelDiff / ConfigA->MaxVelocity * 100.0f) : 0.0f;
	Comparison += FString::Printf(TEXT("Max Velocity: %.0f m/s vs %.0f m/s (%.1f%% %s)\n"),
		ConfigA->MaxVelocity, ConfigB->MaxVelocity, FMath::Abs(VelPercent), VelPercent > 0 ? TEXT("faster") : TEXT("slower"));

	// T/W Ratio
	float TwA = CalculateThrustToWeightRatio(ConfigA);
	float TwB = CalculateThrustToWeightRatio(ConfigB);
	Comparison += FString::Printf(TEXT("T/W Ratio: %.1f vs %.1f\n"), TwA, TwB);

	return Comparison;
}

FString APhysicsConfigUtility::GeneratePerformanceReport()
{
	TArray<UShipPhysicsConfig*> AllConfigs = GetAllPhysicsConfigs();

	if (AllConfigs.Num() == 0)
	{
		return TEXT("No physics configs found in project");
	}

	FString Report;
	Report += TEXT("=================================================\n");
	Report += TEXT("SHIP PHYSICS CONFIGURATION PERFORMANCE REPORT\n");
	Report += TEXT("=================================================\n\n");

	Report += FString::Printf(TEXT("Total Configurations: %d\n\n"), AllConfigs.Num());

	for (UShipPhysicsConfig* Config : AllConfigs)
	{
		if (!Config)
		{
			continue;
		}

		FPhysicsConfigValidation Validation = ValidatePhysicsConfig(Config);

		Report += FString::Printf(TEXT("--- %s ---\n"), *Config->ConfigName.ToString());
		Report += FString::Printf(TEXT("  Status: %s\n"), Validation.bIsValid ? TEXT("VALID") : TEXT("INVALID"));
		Report += FString::Printf(TEXT("  Mass: %.0f kg\n"), Config->Mass);
		Report += FString::Printf(TEXT("  Thrust: %.0f N\n"), Config->ThrustPower);
		Report += FString::Printf(TEXT("  Max Velocity: %.0f m/s\n"), Config->MaxVelocity);
		Report += FString::Printf(TEXT("  T/W Ratio: %.1f\n"), Validation.ThrustToWeightRatio);
		Report += FString::Printf(TEXT("  Max Accel: %.1f m/s²\n"), Validation.MaxAcceleration);
		Report += FString::Printf(TEXT("  Time to Max Vel: %.1f seconds\n"), Validation.TimeToMaxVelocity);

		if (Validation.Warnings.Num() > 0)
		{
			Report += TEXT("  Warnings:\n");
			for (const FString& Warning : Validation.Warnings)
			{
				Report += FString::Printf(TEXT("    - %s\n"), *Warning);
			}
		}

		if (Validation.Errors.Num() > 0)
		{
			Report += TEXT("  Errors:\n");
			for (const FString& Error : Validation.Errors)
			{
				Report += FString::Printf(TEXT("    - %s\n"), *Error);
			}
		}

		Report += TEXT("\n");
	}

	return Report;
}

// ============================================================================
// TESTING UTILITIES
// ============================================================================

bool APhysicsConfigUtility::ApplyConfigToTestActor(AActor* TestActor, UShipPhysicsConfig* Config)
{
	if (!TestActor || !Config)
	{
		return false;
	}

	// TODO: Implement actual application to physics component
	UE_LOG(LogTemp, Log, TEXT("Applied config %s to test actor %s"), *Config->ConfigName.ToString(), *TestActor->GetName());

	return true;
}

void APhysicsConfigUtility::PrintPhysicsConfigDetails(UShipPhysicsConfig* Config)
{
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot print details for null config"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Physics Config: %s"), *Config->ConfigName.ToString());
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Description: %s"), *Config->Description.ToString());
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("=== PHYSICS PARAMETERS ==="));
	UE_LOG(LogTemp, Log, TEXT("Mass: %.1f kg"), Config->Mass);
	UE_LOG(LogTemp, Log, TEXT("Thrust Power: %.1f N"), Config->ThrustPower);
	UE_LOG(LogTemp, Log, TEXT("Max Velocity: %.1f m/s"), Config->MaxVelocity);
	UE_LOG(LogTemp, Log, TEXT("Angular Thrust: %.1f N"), Config->AngularThrustPower);
	UE_LOG(LogTemp, Log, TEXT("Fuel Capacity: %.1f"), Config->FuelCapacity);
	UE_LOG(LogTemp, Log, TEXT("Fuel Consumption: %.1f /s"), Config->FuelConsumptionRate);
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("=== FLIGHT CHARACTERISTICS ==="));
	UE_LOG(LogTemp, Log, TEXT("Accel Multiplier: %.2f"), Config->AccelerationMultiplier);
	UE_LOG(LogTemp, Log, TEXT("Rotation Multiplier: %.2f"), Config->RotationSpeedMultiplier);
	UE_LOG(LogTemp, Log, TEXT("Stability Assist: %.2f"), Config->StabilityAssistStrength);
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("=== CALCULATED METRICS ==="));
	UE_LOG(LogTemp, Log, TEXT("T/W Ratio: %.2f"), CalculateThrustToWeightRatio(Config));
	UE_LOG(LogTemp, Log, TEXT("Max Acceleration: %.2f m/s²"), CalculateMaxAcceleration(Config));
	UE_LOG(LogTemp, Log, TEXT("Time to Max Vel: %.2f seconds"), CalculateTimeToMaxVelocity(Config));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

// ============================================================================
// INTERNAL VALIDATION HELPERS
// ============================================================================

void APhysicsConfigUtility::ValidateMass(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	if (Config->Mass <= 0.0f)
	{
		OutValidation.Errors.Add("Mass must be greater than 0");
	}
	else if (Config->Mass < 300.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("Mass is very low (%.0f kg). Ship may be too light."), Config->Mass));
	}
	else if (Config->Mass > 5000.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("Mass is very high (%.0f kg). Ship may be too heavy."), Config->Mass));
	}
}

void APhysicsConfigUtility::ValidateThrust(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	if (Config->ThrustPower <= 0.0f)
	{
		OutValidation.Errors.Add("Thrust power must be greater than 0");
	}
	else if (Config->ThrustPower < 50000.0f)
	{
		OutValidation.Warnings.Add("Thrust power is very low. Ship may not be able to move.");
	}
	else if (Config->ThrustPower > 500000.0f)
	{
		OutValidation.Warnings.Add("Thrust power is very high. May cause control issues.");
	}
}

void APhysicsConfigUtility::ValidateVelocity(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	if (Config->MaxVelocity <= 0.0f)
	{
		OutValidation.Errors.Add("Max velocity must be greater than 0");
	}
	else if (Config->MaxVelocity < 300.0f)
	{
		OutValidation.Warnings.Add("Max velocity is very low. Ship will be very slow.");
	}
	else if (Config->MaxVelocity > 3000.0f)
	{
		OutValidation.Warnings.Add("Max velocity is very high. May be difficult to control.");
	}
}

void APhysicsConfigUtility::ValidateAngularThrust(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	if (Config->AngularThrustPower <= 0.0f)
	{
		OutValidation.Errors.Add("Angular thrust power must be greater than 0");
	}
	else if (Config->AngularThrustPower < 30000.0f)
	{
		OutValidation.Warnings.Add("Angular thrust is very low. Ship will turn very slowly.");
	}
	else if (Config->AngularThrustPower > 200000.0f)
	{
		OutValidation.Warnings.Add("Angular thrust is very high. Ship may spin too fast.");
	}
}

void APhysicsConfigUtility::ValidateMultipliers(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	if (Config->AccelerationMultiplier <= 0.0f || Config->AccelerationMultiplier > 5.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("Acceleration multiplier (%.2f) is outside normal range (0.1 - 5.0)"), Config->AccelerationMultiplier));
	}

	if (Config->RotationSpeedMultiplier <= 0.0f || Config->RotationSpeedMultiplier > 3.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("Rotation multiplier (%.2f) is outside normal range (0.1 - 3.0)"), Config->RotationSpeedMultiplier));
	}

	if (Config->StabilityAssistStrength < 0.0f || Config->StabilityAssistStrength > 1.0f)
	{
		OutValidation.Errors.Add("Stability assist must be between 0.0 and 1.0");
	}
}

void APhysicsConfigUtility::ValidateBalance(UShipPhysicsConfig* Config, FPhysicsConfigValidation& OutValidation)
{
	float ThrustToWeight = CalculateThrustToWeightRatio(Config);

	if (ThrustToWeight < 3.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("T/W ratio (%.1f) is very low. Ship may struggle to move."), ThrustToWeight));
	}
	else if (ThrustToWeight > 100.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("T/W ratio (%.1f) is extremely high. May be overpowered."), ThrustToWeight));
	}

	// Check if ship can reach max velocity
	float TimeToMax = CalculateTimeToMaxVelocity(Config);
	if (TimeToMax > 30.0f)
	{
		OutValidation.Warnings.Add(FString::Printf(TEXT("Time to max velocity (%.1f seconds) is very long."), TimeToMax));
	}
}
