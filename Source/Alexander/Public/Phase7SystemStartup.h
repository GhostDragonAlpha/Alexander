#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameSystemCoordinator.h"
#include "Phase7SystemStartup.generated.h"

// Forward declarations
class UPhase7IntegrationTest;

/**
 * Phase 7 System Startup Actor
 * Automatically initializes all Phase 7 systems when placed in a level
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=SystemIntegration)
class ALEXANDER_API APhase7SystemStartup : public AActor
{
    GENERATED_BODY()

public:
    APhase7SystemStartup();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Startup")
    bool bAutoInitialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Startup")
    bool bRunIntegrationTests = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Startup")
    bool bEnableDebugMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Startup")
    float InitializationDelay = 1.0f;

private:
    UPROPERTY()
    UGameSystemCoordinator* SystemCoordinator;

    UPROPERTY()
    UPhase7IntegrationTest* IntegrationTest;

    FTimerHandle InitializationTimer;

    void InitializeSystems();
    void RunTests();
    void SetupDebugMode();
};