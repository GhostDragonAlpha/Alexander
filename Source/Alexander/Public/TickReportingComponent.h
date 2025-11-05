// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickReportingComponent.generated.h"

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
    // Configuration storage
    TMap<FString, TMap<UActorComponent*, float>> SavedConfigurations;
};