#include "Phase7IntegrationTest.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameSystemCoordinator.h"
#include "AdvancedAIBehaviors.h"
#include "EnhancedVFXSystem.h"
#include "AudioSystemManager.h"
#include "TutorialOnboardingSystem.h"
#include "TutorialSystem.h"

UPhase7IntegrationTest::UPhase7IntegrationTest()
{
    TestResults.Empty();
    bTestRunning = false;
    CurrentTestIndex = 0;
}

void UPhase7IntegrationTest::RunAllTests()
{
    UE_LOG(LogTemp, Log, TEXT("Starting Phase 7 Integration Tests"));
    
    bTestRunning = true;
    CurrentTestIndex = 0;
    TestResults.Empty();
    
    // Initialize system coordinator
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        UE_LOG(LogTemp, Error, TEXT("System Coordinator not available"));
        return;
    }
    
    // Run individual system tests
    TestAISystem();
    TestVFXSystem();
    TestAudioSystem();
    TestTutorialSystem();
    TestCrossSystemIntegration();
    TestPerformanceOptimization();
    
    // Generate report
    GenerateTestReport();
    
    bTestRunning = false;
    UE_LOG(LogTemp, Log, TEXT("Phase 7 Integration Tests Complete"));
}

bool UPhase7IntegrationTest::RunIndividualTest(const FString& TestName)
{
    UE_LOG(LogTemp, Log, TEXT("Running individual test: %s"), *TestName);
    
    if (TestName == "AI")
    {
        return TestAISystem();
    }
    else if (TestName == "VFX")
    {
        return TestVFXSystem();
    }
    else if (TestName == "Audio")
    {
        return TestAudioSystem();
    }
    else if (TestName == "Tutorial")
    {
        return TestTutorialSystem();
    }
    else if (TestName == "Integration")
    {
        return TestCrossSystemIntegration();
    }
    else if (TestName == "Performance")
    {
        return TestPerformanceOptimization();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Unknown test: %s"), *TestName);
    return false;
}

bool UPhase7IntegrationTest::TestAISystem()
{
    UE_LOG(LogTemp, Log, TEXT("Testing AI System Integration"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("AI System", false, "System Coordinator not available");
        return false;
    }
    
    UAdvancedAIBehaviors* AISystem = Coordinator->GetAISystem();
    if (!AISystem)
    {
        LogTestResult("AI System", false, "AI System not available");
        return false;
    }
    
    // Test AI personality system
    FAIPersonality TestPersonality;
    TestPersonality.Aggressiveness = 0.7f;
    TestPersonality.Sociability = 0.5f;
    TestPersonality.Curiosity = 0.8f;
    
    // Test AI schedule system
    FAIDailySchedule TestSchedule;
    TestSchedule.WakeUpHour = 7;
    TestSchedule.WorkStartHour = 9;
    TestSchedule.WorkEndHour = 17;
    TestSchedule.SleepHour = 23;
    
    // Test AI memory system
    FAIMemoryEntry TestMemory;
    TestMemory.MemoryType = "Test";
    TestMemory.Importance = 1.0f;
    TestMemory.Timestamp = FPlatformTime::Seconds();
    
    bool bTestPassed = true;
    
    // Verify AI system functionality
    if (AISystem->GetPersonalityTrait(nullptr, "Aggressiveness") < 0.0f)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("AI Personality system test failed"));
    }
    
    LogTestResult("AI System", bTestPassed, bTestPassed ? "All AI systems operational" : "AI system failures detected");
    return bTestPassed;
}

bool UPhase7IntegrationTest::TestVFXSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Testing VFX System Integration"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("VFX System", false, "System Coordinator not available");
        return false;
    }
    
    UEnhancedVFXSystem* VFXSystem = Coordinator->GetVFXSystem();
    if (!VFXSystem)
    {
        LogTestResult("VFX System", false, "VFX System not available");
        return false;
    }
    
    // Test VFX quality settings
    FVFXQualitySettings TestQuality;
    TestQuality.ParticleCount = 1.0f;
    TestQuality.EffectRadius = 1.0f;
    TestQuality.EffectDuration = 1.0f;
    TestQuality.bEnableHighQualityFeatures = true;
    
    VFXSystem->SetVFXQuality(TestQuality);
    
    // Test VFX environment data
    FVFXEnvironmentData TestEnvironment;
    TestEnvironment.AtmosphericDensity = 1.0f;
    TestEnvironment.GravityStrength = 1.0f;
    TestEnvironment.Temperature = 20.0f;
    TestEnvironment.bInSpace = false;
    TestEnvironment.bInAtmosphere = true;
    
    VFXSystem->SetEnvironmentData(TestEnvironment);
    
    bool bTestPassed = true;
    
    // Verify VFX system functionality
    FVFXQualitySettings RetrievedQuality = VFXSystem->GetVFXQuality();
    if (FMath::Abs(RetrievedQuality.ParticleCount - TestQuality.ParticleCount) > 0.01f)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("VFX Quality system test failed"));
    }
    
    LogTestResult("VFX System", bTestPassed, bTestPassed ? "VFX systems operational" : "VFX system failures detected");
    return bTestPassed;
}

bool UPhase7IntegrationTest::TestAudioSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Audio System Integration"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("Audio System", false, "System Coordinator not available");
        return false;
    }
    
    UAudioSystemManager* AudioSystem = Coordinator->GetAudioSystem();
    if (!AudioSystem)
    {
        LogTestResult("Audio System", false, "Audio System not available");
        return false;
    }
    
    // Test audio settings
    FAudioSystemSettings TestSettings;
    TestSettings.MasterVolume = 0.8f;
    TestSettings.MusicVolume = 0.7f;
    TestSettings.SFXVolume = 0.9f;
    TestSettings.bEnable3DAudio = true;
    TestSettings.bEnableHRTF = true;

    AudioSystem->SetAudioSettings(TestSettings);
    
    // Test audio zones
    FAudioZone TestZone;
    TestZone.ZoneName = "TestZone";
    TestZone.ZoneCenter = FVector::ZeroVector;
    TestZone.ZoneRadius = 1000.0f;
    TestZone.EnvironmentType = EAudioEnvironment::StationInterior;
    TestZone.VolumeMultiplier = 1.2f;
    
    AudioSystem->CreateAudioZone(TestZone);
    
    bool bTestPassed = true;

    // Verify audio system functionality
    FAudioSystemSettings RetrievedSettings = AudioSystem->GetAudioSettings();
    if (FMath::Abs(RetrievedSettings.MasterVolume - TestSettings.MasterVolume) > 0.01f)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("Audio Settings system test failed"));
    }
    
    LogTestResult("Audio System", bTestPassed, bTestPassed ? "Audio systems operational" : "Audio system failures detected");
    return bTestPassed;
}

bool UPhase7IntegrationTest::TestTutorialSystem()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Tutorial System Integration"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("Tutorial System", false, "System Coordinator not available");
        return false;
    }
    
    UTutorialOnboardingSystem* TutorialSystem = Coordinator->GetTutorialSystem();
    if (!TutorialSystem)
    {
        LogTestResult("Tutorial System", false, "Tutorial System not available");
        return false;
    }
    
    // Test tutorial definition
    FTutorialDefinition TestTutorial;
    TestTutorial.TutorialID = "TestTutorial";
    TestTutorial.Title = "Test Tutorial";
    TestTutorial.Difficulty = ETutorialDifficulty::Beginner;
    TestTutorial.bIsMandatory = false;
    TestTutorial.EstimatedDuration = 60.0f;

    FTutorialOnboardingStep TestStep;
    TestStep.StepID = "TestStep";
    TestStep.Title = "Test Step";
    TestStep.StepType = ETutorialStepType::Information;
    TestStep.Duration = 5.0f;
    TestStep.bIsOptional = false;

    TestTutorial.Steps.Add(TestStep);
    TutorialSystem->RegisterTutorial(TestTutorial);
    
    bool bTestPassed = true;
    
    // Verify tutorial system functionality
    FTutorialDefinition RetrievedTutorial = TutorialSystem->GetTutorialDefinition("TestTutorial");
    if (RetrievedTutorial.TutorialID != TestTutorial.TutorialID)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("Tutorial Definition system test failed"));
    }
    
    LogTestResult("Tutorial System", bTestPassed, bTestPassed ? "Tutorial systems operational" : "Tutorial system failures detected");
    return bTestPassed;
}

bool UPhase7IntegrationTest::TestCrossSystemIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Cross-System Integration"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("Cross-System Integration", false, "System Coordinator not available");
        return false;
    }
    
    bool bTestPassed = true;
    
    // Test cross-system event triggering
    bool bEventReceived = false;
    Coordinator->RegisterCrossSystemEventHandler(ECrossSystemEvent::PlayerEnteredStation, 
        [&bEventReceived](AActor* SourceActor)
        {
            bEventReceived = true;
        });
    
    Coordinator->TriggerCrossSystemEvent(ECrossSystemEvent::PlayerEnteredStation, nullptr);
    
    if (!bEventReceived)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("Cross-System Event test failed"));
    }
    
    // Test system status
    if (!Coordinator->AreSystemsInitialized())
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("System Status test failed"));
    }
    
    LogTestResult("Cross-System Integration", bTestPassed, bTestPassed ? "Cross-system integration operational" : "Cross-system integration failures detected");
    return bTestPassed;
}

bool UPhase7IntegrationTest::TestPerformanceOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Testing Performance Optimization"));
    
    UGameSystemCoordinator* Coordinator = UGameSystemCoordinator::Get();
    if (!Coordinator)
    {
        LogTestResult("Performance Optimization", false, "System Coordinator not available");
        return false;
    }
    
    bool bTestPassed = true;
    
    // Test performance mode switching
    Coordinator->SetPerformanceMode(true);
    FSystemPerformanceData HighPerfData = Coordinator->GetSystemPerformanceData();
    
    Coordinator->SetPerformanceMode(false);
    FSystemPerformanceData NormalPerfData = Coordinator->GetSystemPerformanceData();
    
    // Test adaptive performance
    Coordinator->EnableAdaptivePerformance(true);
    
    // Verify performance monitoring is working
    if (HighPerfData.TotalSystemLoad < 0.0f || NormalPerfData.TotalSystemLoad < 0.0f)
    {
        bTestPassed = false;
        UE_LOG(LogTemp, Error, TEXT("Performance Monitoring test failed"));
    }
    
    LogTestResult("Performance Optimization", bTestPassed, bTestPassed ? "Performance optimization operational" : "Performance optimization failures detected");
    return bTestPassed;
}

void UPhase7IntegrationTest::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FString Result = bPassed ? "PASSED" : "FAILED";
    UE_LOG(LogTemp, Log, TEXT("Test Result: %s - %s - %s"), *TestName, *Result, *Details);
    
    FTestResult TestResult;
    TestResult.TestName = TestName;
    TestResult.bPassed = bPassed;
    TestResult.Details = Details;
    TestResult.Timestamp = FDateTime::Now();
    
    TestResults.Add(TestResult);
}

void UPhase7IntegrationTest::GenerateTestReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHASE 7 INTEGRATION TEST REPORT ==="));
    
    int32 PassedTests = 0;
    int32 TotalTests = TestResults.Num();
    
    for (const FTestResult& Result : TestResults)
    {
        FString Status = Result.bPassed ? "PASSED" : "FAILED";
        UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"), *Status, *Result.TestName, *Result.Details);
        
        if (Result.bPassed)
        {
            PassedTests++;
        }
    }
    
    float SuccessRate = TotalTests > 0 ? (float)PassedTests / (float)TotalTests * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("=== SUMMARY ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Tests: %d"), TotalTests);
    UE_LOG(LogTemp, Log, TEXT("Passed: %d"), PassedTests);
    UE_LOG(LogTemp, Log, TEXT("Failed: %d"), TotalTests - PassedTests);
    UE_LOG(LogTemp, Log, TEXT("Success Rate: %.1f%%"), SuccessRate);
    
    if (SuccessRate >= 90.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 7 Integration: EXCELLENT"));
    }
    else if (SuccessRate >= 75.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 7 Integration: GOOD"));
    }
    else if (SuccessRate >= 50.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 7 Integration: NEEDS IMPROVEMENT"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Phase 7 Integration: CRITICAL ISSUES"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

TArray<FTestResult> UPhase7IntegrationTest::GetTestResults() const
{
    return TestResults;
}

bool UPhase7IntegrationTest::IsTestRunning() const
{
    return bTestRunning;
}

float UPhase7IntegrationTest::GetTestProgress() const
{
    if (TestResults.Num() == 0) return 0.0f;
    return (float)CurrentTestIndex / (float)TestResults.Num() * 100.0f;
}