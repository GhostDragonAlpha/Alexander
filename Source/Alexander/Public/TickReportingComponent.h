// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickReportingComponent.generated.h"

/**
 * UE 5.6 Wrapper: TMap<UActorComponent*, float> for nested TMap compatibility
 * This wraps the inner TMap to allow TMap<FString, TMap<UActorComponent*, float>>
 */
USTRUCT(BlueprintType)
struct FComponentTickConfigMap
{
	GENERATED_BODY()

	// Map from component to tick interval
	UPROPERTY()
	TMap<UActorComponent*, float> ComponentIntervals;
};

/**
 * Component for generating tick performance reports and recommendations
 */
UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTickReportingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTickReportingComponent();

    // Reporting Functions
    UFUNCTION(BlueprintCallable, Category = "Tick|Reporting")
    FString GenerateTickReport(class UTickAnalysisComponent* AnalysisComponent) const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Reporting")
    bool ExportTickReport(const FString& Filename, class UTickAnalysisComponent* AnalysisComponent) const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Reporting")
    TArray<FString> GetOptimizationRecommendations(class UTickAnalysisComponent* AnalysisComponent) const;

    // Configuration Management
    UFUNCTION(BlueprintCallable, Category = "Tick|Configuration")
    void SaveTickConfiguration(const FString& ConfigName, class UTickAnalysisComponent* AnalysisComponent);

    UFUNCTION(BlueprintCallable, Category = "Tick|Configuration")
    void LoadTickConfiguration(const FString& ConfigName, class UTickAnalysisComponent* AnalysisComponent);

private:
    // Configuration storage (using wrapper for UE 5.6 TMap compatibility)
    UPROPERTY()
    TMap<FString, FComponentTickConfigMap> SavedConfigurations;
};