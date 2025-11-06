#include "Phase7SystemStartup.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Phase7IntegrationTest.h"

APhase7SystemStartup::APhase7SystemStartup()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoInitialize = true;
    bRunIntegrationTests = false;
    bEnableDebugMode = false;
    InitializationDelay = 1.0f;
    
    SystemCoordinator = nullptr;
    IntegrationTest = nullptr;
}

void APhase7SystemStartup::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Phase 7 System Startup Actor - BeginPlay"));
    
    if (bAutoInitialize)
    {
        // Delay initialization to allow all systems to be ready
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &APhase7SystemStartup::InitializeSystems);
        
        GetWorld()->GetTimerManager().SetTimer(InitializationTimer, TimerDelegate, InitializationDelay, false);
    }
}

void APhase7SystemStartup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("Phase 7 System Startup Actor - EndPlay"));
    
    // Cleanup systems
    if (SystemCoordinator)
    {
        SystemCoordinator->ShutdownSystems();
        SystemCoordinator = nullptr;
    }
    
    if (IntegrationTest)
    {
        IntegrationTest = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}

void APhase7SystemStartup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update system coordinator if it exists
    if (SystemCoordinator && SystemCoordinator->AreSystemsInitialized())
    {
        SystemCoordinator->UpdateSystems(DeltaTime);
    }
}

void APhase7SystemStartup::InitializeSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Phase 7 Systems"));
    
    // Get or create system coordinator
    SystemCoordinator = UGameSystemCoordinator::Get();
    if (!SystemCoordinator)
    {
        SystemCoordinator = NewObject<UGameSystemCoordinator>();
        SystemCoordinator->AddToRoot();
    }
    
    if (SystemCoordinator)
    {
        // Initialize all systems
        SystemCoordinator->InitializeSystems();
        
        // Setup world integration
        SystemCoordinator->OnWorldBeginPlay(GetWorld());
        
        // Setup player integration if player controller exists
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            SystemCoordinator->OnPlayerJoined(PlayerController);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Phase 7 Systems Initialized Successfully"));
        
        // Run integration tests if requested
        if (bRunIntegrationTests)
        {
            RunTests();
        }
        
        // Setup debug mode if requested
        if (bEnableDebugMode)
        {
            SetupDebugMode();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create System Coordinator"));
    }
}

void APhase7SystemStartup::RunTests()
{
    UE_LOG(LogTemp, Log, TEXT("Running Phase 7 Integration Tests"));
    
    // Create integration test instance
    IntegrationTest = NewObject<UPhase7IntegrationTest>();
    if (IntegrationTest)
    {
        IntegrationTest->RunAllTests();
    }
}

void APhase7SystemStartup::SetupDebugMode()
{
    UE_LOG(LogTemp, Log, TEXT("Setting up Debug Mode for Phase 7 Systems"));
    
    if (SystemCoordinator)
    {
        // Enable debug logging
        // This would typically be done through console variables or settings
        UE_LOG(LogTemp, Log, TEXT("Debug Mode Enabled - All systems will log detailed information"));
        
        // Display system status
        TArray<FString> SystemStatus = SystemCoordinator->GetSystemStatusList();
        for (const FString& Status : SystemStatus)
        {
            UE_LOG(LogTemp, Log, TEXT("System Status: %s"), *Status);
        }
    }
}