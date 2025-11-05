// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SystemValidation.generated.h"

/**
 * Validation severity levels
 */
UENUM(BlueprintType)
enum class EValidationSeverity : uint8
{
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Validation result struct
 */
USTRUCT(BlueprintType)
struct FValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	EValidationSeverity Severity;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Context;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDateTime Timestamp;

	FValidationResult()
		: bIsValid(true)
		, Severity(EValidationSeverity::Info)
		, Message(TEXT(""))
		, Context(TEXT(""))
		, Timestamp(FDateTime::Now())
	{
	}

	FValidationResult(bool bValid, EValidationSeverity InSeverity, const FString& InMessage, const FString& InContext = TEXT(""))
		: bIsValid(bValid)
		, Severity(InSeverity)
		, Message(InMessage)
		, Context(InContext)
		, Timestamp(FDateTime::Now())
	{
	}

	static FValidationResult Success(const FString& Message = TEXT("Validation passed"))
	{
		return FValidationResult(true, EValidationSeverity::Info, Message);
	}

	static FValidationResult Warning(const FString& Message, const FString& Context = TEXT(""))
	{
		return FValidationResult(true, EValidationSeverity::Warning, Message, Context);
	}

	static FValidationResult Error(const FString& Message, const FString& Context = TEXT(""))
	{
		return FValidationResult(false, EValidationSeverity::Error, Message, Context);
	}

	static FValidationResult Critical(const FString& Message, const FString& Context = TEXT(""))
	{
		return FValidationResult(false, EValidationSeverity::Critical, Message, Context);
	}
};

/**
 * System Validation - Provides validation utilities for all game systems
 * Part of Phase 6: Polish & Optimization - Bug Fix & Stability Pass
 */
UCLASS(BlueprintType)
class ALEXANDER_API USystemValidation : public UObject
{
	GENERATED_BODY()

public:
	// ============================================================================
	// NULL POINTER VALIDATION
	// ============================================================================

	/**
	 * Validate that a UObject pointer is not null
	 * @param Object Object to validate
	 * @param ObjectName Name for error message
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateNotNull(UObject* Object, const FString& ObjectName, const FString& Context = TEXT(""));

	/**
	 * Validate that an Actor pointer is not null and is valid
	 * @param Actor Actor to validate
	 * @param ActorName Name for error message
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateActor(AActor* Actor, const FString& ActorName, const FString& Context = TEXT(""));

	/**
	 * Validate that a Component pointer is not null and is registered
	 * @param Component Component to validate
	 * @param ComponentName Name for error message
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateComponent(UActorComponent* Component, const FString& ComponentName, const FString& Context = TEXT(""));

	// ============================================================================
	// RANGE VALIDATION
	// ============================================================================

	/**
	 * Validate that a float value is within range
	 * @param Value Value to validate
	 * @param Min Minimum allowed value
	 * @param Max Maximum allowed value
	 * @param ValueName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateRange(float Value, float Min, float Max, const FString& ValueName);

	/**
	 * Validate that an integer value is within range
	 * @param Value Value to validate
	 * @param Min Minimum allowed value
	 * @param Max Maximum allowed value
	 * @param ValueName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateRangeInt(int32 Value, int32 Min, int32 Max, const FString& ValueName);

	/**
	 * Validate that a value is positive (> 0)
	 * @param Value Value to validate
	 * @param ValueName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidatePositive(float Value, const FString& ValueName);

	/**
	 * Validate that a value is non-negative (>= 0)
	 * @param Value Value to validate
	 * @param ValueName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateNonNegative(float Value, const FString& ValueName);

	/**
	 * Validate that a normalized value is between 0 and 1
	 * @param Value Value to validate
	 * @param ValueName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateNormalized(float Value, const FString& ValueName);

	// ============================================================================
	// COLLECTION VALIDATION
	// ============================================================================

	/**
	 * Validate that an array is not empty
	 * @param ArraySize Size of the array
	 * @param ArrayName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateArrayNotEmpty(int32 ArraySize, const FString& ArrayName);

	/**
	 * Validate array index is within bounds
	 * @param Index Index to validate
	 * @param ArraySize Size of the array
	 * @param ArrayName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateArrayIndex(int32 Index, int32 ArraySize, const FString& ArrayName);

	// ============================================================================
	// STRING VALIDATION
	// ============================================================================

	/**
	 * Validate that a string is not empty
	 * @param String String to validate
	 * @param StringName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateStringNotEmpty(const FString& String, const FString& StringName);

	/**
	 * Validate that a name is not None
	 * @param Name Name to validate
	 * @param NameDescription Description for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateNameNotNone(const FName& Name, const FString& NameDescription);

	// ============================================================================
	// WORLD VALIDATION
	// ============================================================================

	/**
	 * Validate that a world pointer is valid
	 * @param World World to validate
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateWorld(UWorld* World, const FString& Context = TEXT(""));

	/**
	 * Validate that we're in a valid game world (not editor preview)
	 * @param World World to check
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateGameWorld(UWorld* World);

	// ============================================================================
	// PHYSICS VALIDATION
	// ============================================================================

	/**
	 * Validate that a vector is not zero
	 * @param Vector Vector to validate
	 * @param VectorName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateVectorNotZero(const FVector& Vector, const FString& VectorName);

	/**
	 * Validate that a vector is normalized
	 * @param Vector Vector to validate
	 * @param VectorName Name for error message
	 * @param Tolerance Tolerance for normalization check
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateVectorNormalized(const FVector& Vector, const FString& VectorName, float Tolerance = 0.01f);

	/**
	 * Validate that a rotation is valid (no NaN values)
	 * @param Rotation Rotation to validate
	 * @param RotationName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateRotation(const FRotator& Rotation, const FString& RotationName);

	// ============================================================================
	// ASSET VALIDATION
	// ============================================================================

	/**
	 * Validate that an asset reference is loaded
	 * @param AssetPath Soft object path to validate
	 * @param AssetName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateAssetLoaded(const FSoftObjectPath& AssetPath, const FString& AssetName);

	/**
	 * Validate that a material is not null
	 * @param Material Material to validate
	 * @param MaterialName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateMaterial(UMaterialInterface* Material, const FString& MaterialName);

	/**
	 * Validate that a mesh is not null
	 * @param Mesh Mesh to validate
	 * @param MeshName Name for error message
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateMesh(UStaticMesh* Mesh, const FString& MeshName);

	// ============================================================================
	// NETWORK VALIDATION
	// ============================================================================

	/**
	 * Validate that we have authority (server)
	 * @param Actor Actor to check authority on
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateAuthority(AActor* Actor, const FString& Context = TEXT(""));

	/**
	 * Validate network role
	 * @param Actor Actor to check
	 * @param RequiredRole Required network role
	 * @param Context Additional context for logging
	 * @return Validation result
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult ValidateNetworkRole(AActor* Actor, ENetRole RequiredRole, const FString& Context = TEXT(""));

	// ============================================================================
	// BATCH VALIDATION
	// ============================================================================

	/**
	 * Validate multiple results and combine them
	 * @param Results Array of validation results to combine
	 * @return Combined validation result (fails if any fail)
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static FValidationResult CombineResults(const TArray<FValidationResult>& Results);

	/**
	 * Log validation result
	 * @param Result Validation result to log
	 * @param bLogSuccess Whether to log successful validations
	 */
	UFUNCTION(BlueprintCallable, Category = "System Validation")
	static void LogResult(const FValidationResult& Result, bool bLogSuccess = false);

	/**
	 * Log validation result with custom category
	 * @param Result Validation result to log
	 * @param LogCategory Custom log category name
	 * @param bLogSuccess Whether to log successful validations
	 */
	static void LogResultWithCategory(const FValidationResult& Result, const FName& LogCategory, bool bLogSuccess = false);

	// ============================================================================
	// HELPER MACROS (Use in C++ code)
	// ============================================================================

	/**
	 * Quick validation check with early return
	 * Usage: if (!VALIDATE_AND_RETURN(ValidateNotNull(MyObject, "MyObject"))) return;
	 */
	static bool ValidateAndReturn(const FValidationResult& Result, bool bLog = true)
	{
		if (!Result.bIsValid && bLog)
		{
			LogResult(Result, false);
		}
		return Result.bIsValid;
	}

	/**
	 * Validate with custom failure action
	 * Usage: VALIDATE_OR_EXECUTE(ValidateNotNull(Obj, "Obj"), { return false; });
	 */
	#define VALIDATE_OR_EXECUTE(ValidationResult, FailureCode) \
		{ \
			FValidationResult __Result = ValidationResult; \
			if (!__Result.bIsValid) \
			{ \
				USystemValidation::LogResult(__Result, false); \
				FailureCode \
			} \
		}
};
