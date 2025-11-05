// Copyright Epic Games, Inc. All Rights Reserved.

#include "TickAnalysisComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

UTickAnalysisComponent::UTickAnalysisComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Analysis is on-demand
}

void UTickAnalysisComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initial analysis
    AnalyzeAllComponents();

    UE_LOG(LogTemp, Log, TEXT("Tick Analysis Component: Initialized"));
}

void UTickAnalysisComponent::AnalyzeAllComponents()
{
    UE_LOG(LogTemp, Log, TEXT("Tick Analysis: Analyzing all components"));

    GatherTickingComponents();

    for (UActorComponent* Component : AnalyzedComponents)
    {
        if (Component)
        {
            AnalyzeComponent(Component);
        }
    }

    LastAnalysisTime = GetWorld()->GetTimeSeconds();
    UE_LOG(LogTemp, Log, TEXT("Tick Analysis: Analyzed %d components"), AnalyzedComponents.Num());
}

TArray<FTickComponentInfo> UTickAnalysisComponent::GetTickingComponents() const
{
    TArray<FTickComponentInfo> Components;
    ComponentTickInfo.GenerateValueArray(Components);
    return Components;
}

FTickComponentInfo UTickAnalysisComponent::GetComponentTickInfo(UActorComponent* Component) const
{
    if (const FTickComponentInfo* Info = ComponentTickInfo.Find(Component))
    {
        return *Info;
    }
    return FTickComponentInfo();
}

TArray<UActorComponent*> UTickAnalysisComponent::GetInefficientComponents() const
{
    TArray<UActorComponent*> Inefficient;

    for (const auto& Pair : ComponentTickInfo)
    {
        const FTickComponentInfo& Info = Pair.Value;
        // Component is inefficient if it ticks every frame but could tick less frequently
        if (Info.CurrentTickInterval == 0.0f && Info.OptimalTickInterval > 0.0f)
        {
            Inefficient.Add(Pair.Key);
        }
    }

    return Inefficient;
}

float UTickAnalysisComponent::GetTotalTickTime() const
{
    float Total = 0.0f;
    for (const auto& Pair : ComponentTickInfo)
    {
        Total += Pair.Value.AverageTickTime;
    }
    return Total;
}

int32 UTickAnalysisComponent::GetTickingComponentCount() const
{
    return ComponentTickInfo.Num();
}

const TArray<UActorComponent*>& UTickAnalysisComponent::GetAnalyzedComponents() const
{
    return AnalyzedComponents;
}

ETickPriority UTickAnalysisComponent::DetermineTickPriority(UActorComponent* Component) const
{
    if (!Component)
    {
        return ETickPriority::Disabled;
    }

    FString Category = GetComponentCategory(Component);

    // Critical: Player input, camera, physics
    if (Category.Contains(TEXT("Input")) || Category.Contains(TEXT("Camera")) || Category.Contains(TEXT("Physics")))
    {
        return ETickPriority::Critical;
    }

    // High: Combat, movement, nearby actors
    if (Category.Contains(TEXT("Weapon")) || Category.Contains(TEXT("Movement")) || IsPlayerNearby(Component->GetOwner()))
    {
        return ETickPriority::High;
    }

    // Medium: AI, gameplay systems
    if (Category.Contains(TEXT("AI")) || Category.Contains(TEXT("Quest")) || Category.Contains(TEXT("Economy")))
    {
        return ETickPriority::Medium;
    }

    // Low: Background systems
    return ETickPriority::Low;
}

float UTickAnalysisComponent::GetOptimalTickInterval(ETickPriority Priority) const
{
    switch (Priority)
    {
        case ETickPriority::Critical:
            return 0.0f; // Every frame
        case ETickPriority::High:
            return 0.0166f; // 60 FPS
        case ETickPriority::Medium:
            return 0.0333f; // 30 FPS
        case ETickPriority::Low:
            return 0.1f; // 10 FPS
        case ETickPriority::VeryLow:
            return 1.0f; // 1 FPS
        case ETickPriority::Disabled:
            return -1.0f; // Disabled
        default:
            return 0.0333f;
    }
}

void UTickAnalysisComponent::GatherTickingComponents()
{
    AnalyzedComponents.Empty();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor)
        {
            TArray<UActorComponent*> Components;
            Actor->GetComponents(Components);

            for (UActorComponent* Component : Components)
            {
                if (Component && Component->PrimaryComponentTick.bCanEverTick)
                {
                    AnalyzedComponents.Add(Component);
                }
            }
        }
    }
}

void UTickAnalysisComponent::AnalyzeComponent(UActorComponent* Component)
{
    if (!Component)
    {
        return;
    }

    FTickComponentInfo Info;
    Info.ComponentName = Component->GetName();
    Info.CurrentTickInterval = Component->PrimaryComponentTick.TickInterval;
    Info.bCanEverTick = Component->PrimaryComponentTick.bCanEverTick;
    Info.Priority = DetermineTickPriority(Component);
    Info.OptimalTickInterval = CalculateOptimalInterval(Component);

    ComponentTickInfo.Add(Component, Info);
}

float UTickAnalysisComponent::CalculateOptimalInterval(UActorComponent* Component) const
{
    if (!Component)
    {
        return 1.0f;
    }

    ETickPriority Priority = DetermineTickPriority(Component);
    return GetOptimalTickInterval(Priority);
}

FString UTickAnalysisComponent::GetComponentCategory(UActorComponent* Component) const
{
    if (!Component)
    {
        return TEXT("Unknown");
    }

    FString ClassName = Component->GetClass()->GetName();

    if (ClassName.Contains(TEXT("Weapon"))) return TEXT("Weapon");
    if (ClassName.Contains(TEXT("AI"))) return TEXT("AI");
    if (ClassName.Contains(TEXT("Movement"))) return TEXT("Movement");
    if (ClassName.Contains(TEXT("Camera"))) return TEXT("Camera");
    if (ClassName.Contains(TEXT("Input"))) return TEXT("Input");
    if (ClassName.Contains(TEXT("Physics"))) return TEXT("Physics");

    return TEXT("General");
}

bool UTickAnalysisComponent::IsPlayerNearby(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
    return Distance < 5000.0f; // 50 meters
}