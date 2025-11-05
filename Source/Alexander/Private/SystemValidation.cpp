// Copyright Epic Games, Inc. All Rights Reserved.

#include "SystemValidation.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"  // For ENetRole, GetLocalRole(), network validation

DEFINE_LOG_CATEGORY_STATIC(LogSystemValidation, Log, All);

// ============================================================================
// NULL POINTER VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateNotNull(UObject* Object, const FString& ObjectName, const FString& Context)
{
	if (Object == nullptr)
	{
		FString Message = FString::Printf(TEXT("%s is null"), *ObjectName);
		return FValidationResult::Error(Message, Context);
	}
	
	if (!IsValid(Object))
	{
		FString Message = FString::Printf(TEXT("%s is invalid (pending kill or destroyed)"), *ObjectName);
		return FValidationResult::Error(Message, Context);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateActor(AActor* Actor, const FString& ActorName, const FString& Context)
{
	if (Actor == nullptr)
	{
		FString Message = FString::Printf(TEXT("Actor %s is null"), *ActorName);
		return FValidationResult::Error(Message, Context);
	}
	
	if (!IsValid(Actor))
	{
		FString Message = FString::Printf(TEXT("Actor %s is invalid (pending kill)"), *ActorName);
		return FValidationResult::Error(Message, Context);
	}
	
	    if (Actor->IsPendingKillPending())
	{
		FString Message = FString::Printf(TEXT("Actor %s is pending kill"), *ActorName);
		return FValidationResult::Error(Message, Context);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateComponent(UActorComponent* Component, const FString& ComponentName, const FString& Context)
{
	if (Component == nullptr)
	{
		FString Message = FString::Printf(TEXT("Component %s is null"), *ComponentName);
		return FValidationResult::Error(Message, Context);
	}
	
	if (!IsValid(Component))
	{
		FString Message = FString::Printf(TEXT("Component %s is invalid"), *ComponentName);
		return FValidationResult::Error(Message, Context);
	}
	
	if (!Component->IsRegistered())
	{
		FString Message = FString::Printf(TEXT("Component %s is not registered"), *ComponentName);
		return FValidationResult::Warning(Message, Context);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// RANGE VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateRange(float Value, float Min, float Max, const FString& ValueName)
{
	if (FMath::IsNaN(Value))
	{
		FString Message = FString::Printf(TEXT("%s is NaN"), *ValueName);
		return FValidationResult::Critical(Message);
	}
	
	if (!FMath::IsFinite(Value))
	{
		FString Message = FString::Printf(TEXT("%s is infinite"), *ValueName);
		return FValidationResult::Critical(Message);
	}
	
	if (Value < Min || Value > Max)
	{
		FString Message = FString::Printf(TEXT("%s (%.2f) is out of range [%.2f, %.2f]"), 
			*ValueName, Value, Min, Max);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateRangeInt(int32 Value, int32 Min, int32 Max, const FString& ValueName)
{
	if (Value < Min || Value > Max)
	{
		FString Message = FString::Printf(TEXT("%s (%d) is out of range [%d, %d]"), 
			*ValueName, Value, Min, Max);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidatePositive(float Value, const FString& ValueName)
{
	if (FMath::IsNaN(Value))
	{
		FString Message = FString::Printf(TEXT("%s is NaN"), *ValueName);
		return FValidationResult::Critical(Message);
	}
	
	if (Value <= 0.0f)
	{
		FString Message = FString::Printf(TEXT("%s (%.2f) must be positive (> 0)"), *ValueName, Value);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateNonNegative(float Value, const FString& ValueName)
{
	if (FMath::IsNaN(Value))
	{
		FString Message = FString::Printf(TEXT("%s is NaN"), *ValueName);
		return FValidationResult::Critical(Message);
	}
	
	if (Value < 0.0f)
	{
		FString Message = FString::Printf(TEXT("%s (%.2f) must be non-negative (>= 0)"), *ValueName, Value);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateNormalized(float Value, const FString& ValueName)
{
	return ValidateRange(Value, 0.0f, 1.0f, ValueName);
}

// ============================================================================
// COLLECTION VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateArrayNotEmpty(int32 ArraySize, const FString& ArrayName)
{
	if (ArraySize <= 0)
	{
		FString Message = FString::Printf(TEXT("Array %s is empty"), *ArrayName);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateArrayIndex(int32 Index, int32 ArraySize, const FString& ArrayName)
{
	if (Index < 0)
	{
		FString Message = FString::Printf(TEXT("Array %s index %d is negative"), *ArrayName, Index);
		return FValidationResult::Error(Message);
	}
	
	if (Index >= ArraySize)
	{
		FString Message = FString::Printf(TEXT("Array %s index %d is out of bounds (size: %d)"), 
			*ArrayName, Index, ArraySize);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// STRING VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateStringNotEmpty(const FString& String, const FString& StringName)
{
	if (String.IsEmpty())
	{
		FString Message = FString::Printf(TEXT("String %s is empty"), *StringName);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateNameNotNone(const FName& Name, const FString& NameDescription)
{
	if (Name.IsNone())
	{
		FString Message = FString::Printf(TEXT("Name %s is None"), *NameDescription);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// WORLD VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateWorld(UWorld* World, const FString& Context)
{
	if (World == nullptr)
	{
		FString Message = TEXT("World is null");
		return FValidationResult::Critical(Message, Context);
	}
	
	if (!IsValid(World))
	{
		FString Message = TEXT("World is invalid");
		return FValidationResult::Critical(Message, Context);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateGameWorld(UWorld* World)
{
	FValidationResult WorldCheck = ValidateWorld(World);
	if (!WorldCheck.bIsValid)
	{
		return WorldCheck;
	}
	
	if (World->WorldType == EWorldType::Editor || 
		World->WorldType == EWorldType::EditorPreview)
	{
		FString Message = TEXT("Not in a game world (editor or preview)");
		return FValidationResult::Warning(Message);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// PHYSICS VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateVectorNotZero(const FVector& Vector, const FString& VectorName)
{
	if (Vector.IsNearlyZero())
	{
		FString Message = FString::Printf(TEXT("Vector %s is zero"), *VectorName);
		return FValidationResult::Error(Message);
	}
	
	if (Vector.ContainsNaN())
	{
		FString Message = FString::Printf(TEXT("Vector %s contains NaN values"), *VectorName);
		return FValidationResult::Critical(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateVectorNormalized(const FVector& Vector, const FString& VectorName, float Tolerance)
{
	if (Vector.ContainsNaN())
	{
		FString Message = FString::Printf(TEXT("Vector %s contains NaN values"), *VectorName);
		return FValidationResult::Critical(Message);
	}
	
	float Length = Vector.Size();
	if (!FMath::IsNearlyEqual(Length, 1.0f, Tolerance))
	{
		FString Message = FString::Printf(TEXT("Vector %s is not normalized (length: %.4f)"), 
			*VectorName, Length);
		return FValidationResult::Error(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateRotation(const FRotator& Rotation, const FString& RotationName)
{
	if (Rotation.ContainsNaN())
	{
		FString Message = FString::Printf(TEXT("Rotation %s contains NaN values"), *RotationName);
		return FValidationResult::Critical(Message);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// ASSET VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateAssetLoaded(const FSoftObjectPath& AssetPath, const FString& AssetName)
{
	if (!AssetPath.IsValid())
	{
		FString Message = FString::Printf(TEXT("Asset path for %s is invalid"), *AssetName);
		return FValidationResult::Error(Message);
	}
	
	UObject* LoadedAsset = AssetPath.ResolveObject();
	if (LoadedAsset == nullptr)
	{
		FString Message = FString::Printf(TEXT("Asset %s is not loaded (path: %s)"), 
			*AssetName, *AssetPath.ToString());
		return FValidationResult::Warning(Message);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateMaterial(UMaterialInterface* Material, const FString& MaterialName)
{
	return ValidateNotNull(Material, MaterialName);
}

FValidationResult USystemValidation::ValidateMesh(UStaticMesh* Mesh, const FString& MeshName)
{
	return ValidateNotNull(Mesh, MeshName);
}

// ============================================================================
// NETWORK VALIDATION
// ============================================================================

FValidationResult USystemValidation::ValidateAuthority(AActor* Actor, const FString& Context)
{
	FValidationResult ActorCheck = ValidateActor(Actor, TEXT("Actor"));
	if (!ActorCheck.bIsValid)
	{
		return ActorCheck;
	}
	
	if (!Actor->HasAuthority())
	{
		FString Message = TEXT("No authority (not server)");
		return FValidationResult::Error(Message, Context);
	}
	
	return FValidationResult::Success();
}

FValidationResult USystemValidation::ValidateNetworkRole(AActor* Actor, ENetRole RequiredRole, const FString& Context)
{
	FValidationResult ActorCheck = ValidateActor(Actor, TEXT("Actor"));
	if (!ActorCheck.bIsValid)
	{
		return ActorCheck;
	}
	
	ENetRole CurrentRole = Actor->GetLocalRole();
	if (CurrentRole != RequiredRole)
	{
		FString Message = FString::Printf(TEXT("Invalid network role (current: %d, required: %d)"), 
			static_cast<int32>(CurrentRole), static_cast<int32>(RequiredRole));
		return FValidationResult::Error(Message, Context);
	}
	
	return FValidationResult::Success();
}

// ============================================================================
// BATCH VALIDATION
// ============================================================================

FValidationResult USystemValidation::CombineResults(const TArray<FValidationResult>& Results)
{
	if (Results.Num() == 0)
	{
		return FValidationResult::Success(TEXT("No validations to check"));
	}
	
	// Find the worst result
	bool bAllValid = true;
	EValidationSeverity WorstSeverity = EValidationSeverity::Info;
	TArray<FString> ErrorMessages;
	
	for (const FValidationResult& Result : Results)
	{
		if (!Result.bIsValid)
		{
			bAllValid = false;
			ErrorMessages.Add(Result.Message);
		}
		
		// Track worst severity
		if (static_cast<int32>(Result.Severity) > static_cast<int32>(WorstSeverity))
		{
			WorstSeverity = Result.Severity;
		}
	}
	
	if (!bAllValid)
	{
		FString CombinedMessage = FString::Printf(TEXT("Multiple validation failures (%d):"), ErrorMessages.Num());
		for (const FString& Msg : ErrorMessages)
		{
			CombinedMessage += TEXT("\n  - ") + Msg;
		}
		
		return FValidationResult(false, WorstSeverity, CombinedMessage);
	}
	
	return FValidationResult::Success(TEXT("All validations passed"));
}

void USystemValidation::LogResult(const FValidationResult& Result, bool bLogSuccess)
{
	LogResultWithCategory(Result, TEXT("SystemValidation"), bLogSuccess);
}

void USystemValidation::LogResultWithCategory(const FValidationResult& Result, const FName& LogCategory, bool bLogSuccess)
{
	if (Result.bIsValid && !bLogSuccess)
	{
		return; // Don't log successful validations unless requested
	}
	
	FString ContextStr = Result.Context.IsEmpty() ? TEXT("") : FString::Printf(TEXT(" [%s]"), *Result.Context);
	FString LogMessage = FString::Printf(TEXT("%s%s"), *Result.Message, *ContextStr);
	
	switch (Result.Severity)
	{
		case EValidationSeverity::Info:
			UE_LOG(LogSystemValidation, Log, TEXT("%s"), *LogMessage);
			break;
			
		case EValidationSeverity::Warning:
			UE_LOG(LogSystemValidation, Warning, TEXT("%s"), *LogMessage);
			break;
			
		case EValidationSeverity::Error:
			UE_LOG(LogSystemValidation, Error, TEXT("%s"), *LogMessage);
			break;
			
		case EValidationSeverity::Critical:
			UE_LOG(LogSystemValidation, Fatal, TEXT("%s"), *LogMessage);
			break;
	}
}
