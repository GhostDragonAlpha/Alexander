// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickReportingComponent.h"
#include "TickAnalysisComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UTickReportingComponent::UTickReportingComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Reporting is on-demand
}

FString UTickReportingComponent::GenerateTickReport(class UTickAnalysisComponent* AnalysisComponent) const
{
    if (!AnalysisComponent)
    {
        return TEXT("Error: No analysis component provided");
    }

    FString Report = TEXT("====================================\n");
    Report += TEXT("   TICK OPTIMIZATION REPORT\n");
    Report += TEXT("====================================\n\n");

    Report += FString::Printf(TEXT("Components Analyzed: %d\n"), AnalysisComponent->GetTickingComponentCount());
    Report += FString::Printf(TEXT("Total Tick Time: %.2fms\n"), AnalysisComponent->GetTotalTickTime());
    Report += TEXT("\n");

    Report += TEXT("--- Component Breakdown ---\n");
    TArray<FTickComponentInfo> Components = AnalysisComponent->GetTickingComponents();
    for (const FTickComponentInfo& Info : Components)
    {
        Report += FString::Printf(TEXT("%s: %.3fms (Interval: %.2fs, Priority: %s)\n"),
            *Info.ComponentName, Info.AverageTickTime, Info.CurrentTickInterval,
            *UEnum::GetValueAsString(Info.Priority));
    }

    Report += TEXT("\n--- Recommendations ---\n");
    TArray<FString> Recommendations = GetOptimizationRecommendations(AnalysisComponent);
    for (const FString& Rec : Recommendations)
    {
        Report += TEXT("- ") + Rec + TEXT("\n");
    }

    Report += TEXT("\n====================================\n");
    return Report;
}

bool UTickReportingComponent::ExportTickReport(const FString& Filename, class UTickAnalysisComponent* AnalysisComponent) const
{
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("Profiling/") + Filename + TEXT(".txt");
    FString Report = GenerateTickReport(AnalysisComponent);

    return FFileHelper::SaveStringToFile(Report, *FilePath);
}

TArray<FString> UTickReportingComponent::GetOptimizationRecommendations(class UTickAnalysisComponent* AnalysisComponent) const
{
    TArray<FString> Recommendations;

    if (!AnalysisComponent)
    {
        return Recommendations;
    }

    TArray<UActorComponent*> Inefficient = AnalysisComponent->GetInefficientComponents();
    if (Inefficient.Num() > 0)
    {
        Recommendations.Add(FString::Printf(TEXT("%d components could benefit from reduced tick rates"), Inefficient.Num()));
    }

    if (AnalysisComponent->GetTotalTickTime() > 5.0f) // 5ms budget
    {
        Recommendations.Add(TEXT("Total tick time exceeds budget - consider disabling some components"));
    }

    if (AnalysisComponent->GetTickingComponentCount() > 500)
    {
        Recommendations.Add(TEXT("High component count - consider object pooling or culling"));
    }

    return Recommendations;
}

void UTickReportingComponent::SaveTickConfiguration(const FString& ConfigName, class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    TMap<UActorComponent*, float> Config;
    TArray<UActorComponent*> Components = AnalysisComponent->GetAnalyzedComponents();

    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            Config.Add(Component, Component->PrimaryComponentTick.TickInterval);
        }
    }

    SavedConfigurations.Add(ConfigName, Config);
}

void UTickReportingComponent::LoadTickConfiguration(const FString& ConfigName, class UTickAnalysisComponent* AnalysisComponent)
{
    if (!AnalysisComponent)
    {
        return;
    }

    if (const TMap<UActorComponent*, float>* Config = SavedConfigurations.Find(ConfigName))
    {
        for (const auto& Pair : *Config)
        {
            if (Pair.Key.IsValid())
            {
                Pair.Key->PrimaryComponentTick.TickInterval = Pair.Value;
            }
        }
    }
}