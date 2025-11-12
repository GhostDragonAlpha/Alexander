// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TickAnalysisComponent.generated.h"

/**
 * Tick priority enumeration
 */
UENUM(BlueprintType)
enum class ETickPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical - Every Frame"),
    High        UMETA(DisplayName = "High - 60 FPS"),
    Medium      UMETA(DisplayName = "Medium - 30 FPS"),
    Low         UMETA(DisplayName = "Low - 10 FPS"),
    VeryLow     UMETA(DisplayName = "Very Low - 1 FPS"),
    Dormant     UMETA(DisplayName = "Dormant - No Tick"),
    Disabled    UMETA(DisplayName = "Disabled")
};



/**
 * Component tick information
 */
USTRUCT(BlueprintType)
struct FTickComponentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    FString ComponentName;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    float CurrentTickInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    float OptimalTickInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    float AverageTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    bool bCanEverTick;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    ETickPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Tick")
    int32 TickCount;

    FTickComponentInfo()
        : ComponentName("")
        , CurrentTickInterval(0.0f)
        , OptimalTickInterval(0.0f)
        , AverageTickTime(0.0f)
        , bCanEverTick(false)
        , Priority(ETickPriority::Medium)
        , TickCount(0)
    {}
};



/**
 * Component for analyzing tick performance across all components
 */
UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTickAnalysisComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTickAnalysisComponent();

    virtual void BeginPlay() override;

    // Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    void AnalyzeAllComponents();

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    TArray<FTickComponentInfo> GetTickingComponents() const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    FTickComponentInfo GetComponentTickInfo(UActorComponent* Component) const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    TArray<UActorComponent*> GetInefficientComponents() const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    float GetTotalTickTime() const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    int32 GetTickingComponentCount() const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Utility")
    ETickPriority DetermineTickPriority(UActorComponent* Component) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Utility")
    float GetOptimalTickInterval(ETickPriority Priority) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tick|Analysis")
    const TArray<UActorComponent*>& GetAnalyzedComponents() const;

    UFUNCTION(BlueprintCallable, Category = "Tick|Analysis")
    void GatherTickingComponents();

private:
    // Analysis data
    TMap<UActorComponent*, FTickComponentInfo> ComponentTickInfo;
    TArray<UActorComponent*> AnalyzedComponents;
    float LastAnalysisTime;

    // Internal functions
    void AnalyzeComponent(UActorComponent* Component);
    float CalculateOptimalInterval(UActorComponent* Component) const;
    FString GetComponentCategory(UActorComponent* Component) const;
    bool IsPlayerNearby(AActor* Actor) const;
};