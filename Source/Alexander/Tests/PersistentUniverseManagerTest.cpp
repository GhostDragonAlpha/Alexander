// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/AutomationTest.h"
#include "PersistentUniverseManager.h"
#include "OriginCenteredPhysicsManager.h"
#include "PlayerProgression.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPersistentUniverseManagerBasicTest, "Alexander.Persistence.PersistentUniverseManager.BasicOperations", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPersistentUniverseManagerBasicTest::RunTest(const FString& Parameters)
{
    // Create a test world
    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Create test actor with PersistentUniverseManager
    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor"));
        return false;
    }

    UPersistentUniverseManager* PersistenceManager = NewObject<UPersistentUniverseManager>(TestActor);
    if (!PersistenceManager)
    {
        AddError(TEXT("Failed to create PersistentUniverseManager"));
        return false;
    }

    PersistenceManager->RegisterComponent();
    PersistenceManager->BeginPlay();

    // Test basic save operations
    FString TestPlayerID = TEXT("TestPlayer001");
    FPlayerPersistentData TestPlayerData;
    TestPlayerData.PlayerID = TestPlayerID;
    TestPlayerData.PlayerName = TEXT("Test Player");
    TestPlayerData.Level = 5;
    TestPlayerData.Experience = 1250;
    TestPlayerData.Credits = 50000;
    TestPlayerData.LastLocation = FVector(1000.0f, 2000.0f, 3000.0f);
    TestPlayerData.CurrentSystemID = TEXT("SolSystem");

    // Test player data save
    FSaveOperationResult SaveResult = PersistenceManager->SavePlayerData(TestPlayerID, TestPlayerData);
    if (!SaveResult.bSuccess)
    {
        AddError(FString::Printf(TEXT("Failed to save player data: %s"), *SaveResult.ErrorMessage));
        return false;
    }

    TestEqual("Save operation should be successful", SaveResult.bSuccess, true);
    TestTrue("Save duration should be positive", SaveResult.SaveDuration > 0.0f);
    TestTrue("Bytes saved should be positive", SaveResult.BytesSaved > 0);

    // Test player data load
    bool bLoadSuccess = false;
    FPlayerPersistentData LoadedData = PersistenceManager->LoadPlayerData(TestPlayerID, bLoadSuccess);
    if (!bLoadSuccess)
    {
        AddError(TEXT("Failed to load player data"));
        return false;
    }

    TestEqual("Loaded player ID should match", LoadedData.PlayerID, TestPlayerData.PlayerID);
    TestEqual("Loaded player name should match", LoadedData.PlayerName, TestPlayerData.PlayerName);
    TestEqual("Loaded level should match", LoadedData.Level, TestPlayerData.Level);
    TestEqual("Loaded experience should match", LoadedData.Experience, TestPlayerData.Experience);
    TestEqual("Loaded credits should match", LoadedData.Credits, TestPlayerData.Credits);
    TestEqual("Loaded location should match", LoadedData.LastLocation, TestPlayerData.LastLocation);
    TestEqual("Loaded system ID should match", LoadedData.CurrentSystemID, TestPlayerData.CurrentSystemID);

    // Test world state save/load
    FString TestWorldID = TEXT("TestWorld001");
    FWorldStateData TestWorldData;
    TestWorldData.WorldID = TestWorldID;
    TestWorldData.DynamicActorLocations.Add(TEXT("Actor001"), FVector(100.0f, 200.0f, 300.0f));
    TestWorldData.DynamicActorLocations.Add(TEXT("Actor002"), FVector(400.0f, 500.0f, 600.0f));
    TestWorldData.WorldFlags.Add(TEXT("MissionCompleted"), true);
    TestWorldData.WorldFlags.Add(TEXT("BossDefeated"), false);
    TestWorldData.CompletedEvents.Add(TEXT("FirstContact"));
    TestWorldData.CompletedEvents.Add(TEXT("ResourceGathering"));

    FSaveOperationResult WorldSaveResult = PersistenceManager->SaveWorldState(TestWorldID, TestWorldData);
    TestEqual("World save should be successful", WorldSaveResult.bSuccess, true);

    bool bWorldLoadSuccess = false;
    FWorldStateData LoadedWorldData = PersistenceManager->LoadWorldState(TestWorldID, bWorldLoadSuccess);
    TestEqual("World load should be successful", bWorldLoadSuccess, true);
    TestEqual("Loaded world ID should match", LoadedWorldData.WorldID, TestWorldData.WorldID);
    TestEqual("Loaded actor locations count should match", LoadedWorldData.DynamicActorLocations.Num(), TestWorldData.DynamicActorLocations.Num());
    TestEqual("Loaded world flags count should match", LoadedWorldData.WorldFlags.Num(), TestWorldData.WorldFlags.Num());
    TestEqual("Loaded completed events count should match", LoadedWorldData.CompletedEvents.Num(), TestWorldData.CompletedEvents.Num());

    // Test statistics
    FPersistenceStatistics Stats = PersistenceManager->GetStatistics();
    TestEqual("Total saves should be 2", Stats.TotalSaves, 2);
    TestEqual("Total loads should be 2", Stats.TotalLoads, 2);
    TestEqual("Failed operations should be 0", Stats.FailedOperations, 0);
    TestTrue("Average save time should be positive", Stats.AverageSaveTime > 0.0f);
    TestTrue("Average load time should be positive", Stats.AverageLoadTime > 0.0f);

    // Cleanup
    PersistenceManager->DeletePlayerData(TestPlayerID);
    PersistenceManager->DeleteWorldState(TestWorldID);

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPersistentUniverseManagerAutoSaveTest, "Alexander.Persistence.PersistentUniverseManager.AutoSave", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPersistentUniverseManagerAutoSaveTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor"));
        return false;
    }

    UPersistentUniverseManager* PersistenceManager = NewObject<UPersistentUniverseManager>(TestActor);
    if (!PersistenceManager)
    {
        AddError(TEXT("Failed to create PersistentUniverseManager"));
        return false;
    }

    PersistenceManager->RegisterComponent();
    PersistenceManager->BeginPlay();

    // Configure auto-save
    PersistenceManager->bEnableAutoSave = true;
    PersistenceManager->AutoSaveInterval = 0.1f; // 0.1 seconds for testing
    PersistenceManager->EnableAutoSave(true);

    // Wait for auto-save to trigger
    float ElapsedTime = 0.0f;
    bool bAutoSaveTriggered = false;
    
    while (ElapsedTime < 0.2f)
    {
        ElapsedTime += 0.01f;
        PersistenceManager->TickComponent(0.01f, ELevelTick::LEVELTICK_All, nullptr);
        
        FPersistenceStatistics Stats = PersistenceManager->GetStatistics();
        if (Stats.TotalSaves > 0)
        {
            bAutoSaveTriggered = true;
            break;
        }
    }

    TestEqual("Auto-save should have triggered", bAutoSaveTriggered, true);

    // Test disabling auto-save
    PersistenceManager->EnableAutoSave(false);
    int32 SavesBeforeDisable = PersistenceManager->GetStatistics().TotalSaves;
    
    // Wait and verify no additional saves
    ElapsedTime = 0.0f;
    while (ElapsedTime < 0.2f)
    {
        ElapsedTime += 0.01f;
        PersistenceManager->TickComponent(0.01f, ELevelTick::LEVELTICK_All, nullptr);
    }

    int32 SavesAfterDisable = PersistenceManager->GetStatistics().TotalSaves;
    TestEqual("No additional saves after disabling auto-save", SavesAfterDisable, SavesBeforeDisable);

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPersistentUniverseManagerBackupTest, "Alexander.Persistence.PersistentUniverseManager.BackupManagement", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPersistentUniverseManagerBackupTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor"));
        return false;
    }

    UPersistentUniverseManager* PersistenceManager = NewObject<UPersistentUniverseManager>(TestActor);
    if (!PersistenceManager)
    {
        AddError(TEXT("Failed to create PersistentUniverseManager"));
        return false;
    }

    PersistenceManager->RegisterComponent();
    PersistenceManager->BeginPlay();

    // Create test data
    FString TestPlayerID = TEXT("BackupTestPlayer");
    FPlayerPersistentData TestData;
    TestData.PlayerID = TestPlayerID;
    TestData.PlayerName = TEXT("Backup Test Player");
    TestData.Level = 10;
    TestData.Credits = 100000;

    PersistenceManager->SavePlayerData(TestPlayerID, TestData);

    // Create multiple backups
    TArray<FString> BackupNames;
    for (int32 i = 0; i < 3; i++)
    {
        FString BackupName = FString::Printf(TEXT("TestBackup_%d"), i);
        bool bBackupCreated = PersistenceManager->CreateBackup(BackupName);
        TestEqual(FString::Printf(TEXT("Backup %d should be created"), i), bBackupCreated, true);
        BackupNames.Add(BackupName);
    }

    // Verify backups exist
    TArray<FString> AvailableBackups = PersistenceManager->GetAvailableBackups();
    for (const FString& BackupName : BackupNames)
    {
        TestTrue(FString::Printf(TEXT("Backup %s should be available"), *BackupName), AvailableBackups.Contains(BackupName));
    }

    // Modify data and restore from backup
    TestData.Level = 20;
    TestData.Credits = 200000;
    PersistenceManager->SavePlayerData(TestPlayerID, TestData);

    // Restore from first backup
    bool bRestoreSuccess = PersistenceManager->RestoreBackup(BackupNames[0]);
    TestEqual("Restore from backup should be successful", bRestoreSuccess, true);

    // Verify restored data
    bool bLoadSuccess = false;
    FPlayerPersistentData RestoredData = PersistenceManager->LoadPlayerData(TestPlayerID, bLoadSuccess);
    TestEqual("Restored level should match original", RestoredData.Level, 10);
    TestEqual("Restored credits should match original", RestoredData.Credits, 100000);

    // Test backup deletion
    bool bDeleteSuccess = PersistenceManager->DeleteBackup(BackupNames[1]);
    TestEqual("Backup deletion should be successful", bDeleteSuccess, true);

    AvailableBackups = PersistenceManager->GetAvailableBackups();
    TestFalse("Deleted backup should not be available", AvailableBackups.Contains(BackupNames[1]));

    // Cleanup
    for (const FString& BackupName : BackupNames)
    {
        PersistenceManager->DeleteBackup(BackupName);
    }
    PersistenceManager->DeletePlayerData(TestPlayerID);

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOriginCenteredPhysicsManagerTest, "Alexander.Physics.OriginCenteredPhysicsManager.BasicOperations", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FOriginCenteredPhysicsManagerTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Create physics manager
    AActor* ManagerActor = TestWorld->SpawnActor<AActor>();
    if (!ManagerActor)
    {
        AddError(TEXT("Failed to spawn manager actor"));
        return false;
    }

    UOriginCenteredPhysicsManager* PhysicsManager = NewObject<UOriginCenteredPhysicsManager>(ManagerActor);
    if (!PhysicsManager)
    {
        AddError(TEXT("Failed to create OriginCenteredPhysicsManager"));
        return false;
    }

    PhysicsManager->RegisterComponent();
    PhysicsManager->BeginPlay();

    // Configure sector bounds for testing
    PhysicsManager->SectorBounds.SectorSize = 1000.0f; // 1km sectors for easier testing

    // Create test actors
    AActor* TestActor1 = TestWorld->SpawnActor<AActor>();
    AActor* TestActor2 = TestWorld->SpawnActor<AActor>();
    
    if (!TestActor1 || !TestActor2)
    {
        AddError(TEXT("Failed to spawn test actors"));
        return false;
    }

    // Set initial positions
    TestActor1->SetActorLocation(FVector(500.0f, 500.0f, 500.0f));
    TestActor2->SetActorLocation(FVector(1500.0f, 1500.0f, 1500.0f));

    // Register actors
    PhysicsManager->RegisterActor(TestActor1);
    PhysicsManager->RegisterActor(TestActor2);

    TestEqual("Actor 1 should be registered", PhysicsManager->IsActorRegistered(TestActor1), true);
    TestEqual("Actor 2 should be registered", PhysicsManager->IsActorRegistered(TestActor2), true);

    // Test sector calculations
    FSectorCoordinate Sector1 = PhysicsManager->GetActorSector(TestActor1);
    FSectorCoordinate Sector2 = PhysicsManager->GetActorSector(TestActor2);

    TestEqual("Actor 1 should be in sector (0,0,0)", Sector1, FSectorCoordinate(0, 0, 0));
    TestEqual("Actor 2 should be in sector (1,1,1)", Sector2, FSectorCoordinate(1, 1, 1));

    // Test world position retrieval
    FVector RetrievedPos1 = PhysicsManager->GetWorldPosition(TestActor1);
    FVector RetrievedPos2 = PhysicsManager->GetWorldPosition(TestActor2);

    TestEqual("Retrieved position 1 should match", RetrievedPos1, TestActor1->GetActorLocation());
    TestEqual("Retrieved position 2 should match", RetrievedPos2, TestActor2->GetActorLocation());

    // Test distance calculations
    float Distance = PhysicsManager->GetDistanceBetweenActors(TestActor1, TestActor2);
    float ExpectedDistance = FVector::Dist(TestActor1->GetActorLocation(), TestActor2->GetActorLocation());
    TestEqual("Distance calculation should be accurate", FMath::IsNearlyEqual(Distance, ExpectedDistance, 1.0f), true);

    // Test sector movement
    FVector NewPosition = FVector(2500.0f, 2500.0f, 2500.0f);
    PhysicsManager->SetWorldPosition(TestActor1, NewPosition);

    FSectorCoordinate NewSector = PhysicsManager->GetActorSector(TestActor1);
    TestEqual("Actor 1 should now be in sector (2,2,2)", NewSector, FSectorCoordinate(2, 2, 2));

    FVector RetrievedNewPos = PhysicsManager->GetWorldPosition(TestActor1);
    TestEqual("New position should be retrieved correctly", RetrievedNewPos, NewPosition);

    // Test serialization
    FOriginCenteredData SerializedData = PhysicsManager->SerializeActorData(TestActor2);
    TestEqual("Serialized sector should match", SerializedData.SectorCoord, FSectorCoordinate(1, 1, 1));

    // Test deserialization
    AActor* TestActor3 = TestWorld->SpawnActor<AActor>();
    PhysicsManager->DeserializeActorData(TestActor3, SerializedData);
    
    FSectorCoordinate DeserializedSector = PhysicsManager->GetActorSector(TestActor3);
    TestEqual("Deserialized sector should match", DeserializedSector, FSectorCoordinate(1, 1, 1));

    // Cleanup
    PhysicsManager->UnregisterActor(TestActor1);
    PhysicsManager->UnregisterActor(TestActor2);
    PhysicsManager->UnregisterActor(TestActor3);

    TestEqual("Actor 1 should be unregistered", PhysicsManager->IsActorRegistered(TestActor1), false);

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPersistentUniverseIntegrationTest, "Alexander.Persistence.Integration.FullWorkflow", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPersistentUniverseIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Create comprehensive test setup
    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor"));
        return false;
    }

    // Create both managers
    UPersistentUniverseManager* PersistenceManager = NewObject<UPersistentUniverseManager>(TestActor);
    UOriginCenteredPhysicsManager* PhysicsManager = NewObject<UOriginCenteredPhysicsManager>(TestActor);
    
    if (!PersistenceManager || !PhysicsManager)
    {
        AddError(TEXT("Failed to create managers"));
        return false;
    }

    PersistenceManager->RegisterComponent();
    PhysicsManager->RegisterComponent();
    PersistenceManager->BeginPlay();
    PhysicsManager->BeginPlay();

    // Create test player and register with physics
    AActor* TestPlayer = TestWorld->SpawnActor<AActor>();
    TestPlayer->SetActorLocation(FVector(5000.0f, 5000.0f, 5000.0f));
    PhysicsManager->RegisterActor(TestPlayer);
    PhysicsManager->SetPlayerOrigin(TestPlayer);

    // Create test ship
    AActor* TestShip = TestWorld->SpawnActor<AActor>();
    TestShip->SetActorLocation(FVector(10000.0f, 10000.0f, 10000.0f));
    PhysicsManager->RegisterActor(TestShip);

    // Save complete game state
    FString PlayerID = TEXT("IntegrationTestPlayer");
    FPlayerPersistentData PlayerData;
    PlayerData.PlayerID = PlayerID;
    PlayerData.PlayerName = TEXT("Integration Test Player");
    PlayerData.Level = 15;
    PlayerData.Experience = 5000;
    PlayerData.Credits = 75000;
    PlayerData.LastLocation = PhysicsManager->GetWorldPosition(TestPlayer);
    PlayerData.CurrentSystemID = TEXT("AlphaCentauri");

    // Save world state with actor positions
    FString WorldID = TEXT("IntegrationTestWorld");
    FWorldStateData WorldData;
    WorldData.WorldID = WorldID;
    WorldData.DynamicActorLocations.Add(TestPlayer->GetName(), PhysicsManager->GetWorldPosition(TestPlayer));
    WorldData.DynamicActorLocations.Add(TestShip->GetName(), PhysicsManager->GetWorldPosition(TestShip));
    WorldData.WorldFlags.Add(TEXT("GameStarted"), true);
    WorldData.CompletedEvents.Add(TEXT("ShipAcquired"));

    // Perform saves
    FSaveOperationResult PlayerSaveResult = PersistenceManager->SavePlayerData(PlayerID, PlayerData);
    FSaveOperationResult WorldSaveResult = PersistenceManager->SaveWorldState(WorldID, WorldData);
    
    TestEqual("Player save should succeed", PlayerSaveResult.bSuccess, true);
    TestEqual("World save should succeed", WorldSaveResult.bSuccess, true);

    // Simulate game shutdown and restart
    PhysicsManager->UnregisterActor(TestPlayer);
    PhysicsManager->UnregisterActor(TestShip);
    
    // Create new actors (simulating game restart)
    AActor* NewTestPlayer = TestWorld->SpawnActor<AActor>();
    AActor* NewTestShip = TestWorld->SpawnActor<AActor>();

    // Load game state
    bool bPlayerLoadSuccess = false;
    bool bWorldLoadSuccess = false;
    
    FPlayerPersistentData LoadedPlayerData = PersistenceManager->LoadPlayerData(PlayerID, bPlayerLoadSuccess);
    FWorldStateData LoadedWorldData = PersistenceManager->LoadWorldState(WorldID, bWorldLoadSuccess);
    
    TestEqual("Player load should succeed", bPlayerLoadSuccess, true);
    TestEqual("World load should succeed", bWorldLoadSuccess, true);

    // Restore actor positions
    if (LoadedWorldData.DynamicActorLocations.Contains(NewTestPlayer->GetName()))
    {
        FVector PlayerPosition = LoadedWorldData.DynamicActorLocations[NewTestPlayer->GetName()];
        NewTestPlayer->SetActorLocation(PlayerPosition);
        PhysicsManager->RegisterActor(NewTestPlayer);
    }

    if (LoadedWorldData.DynamicActorLocations.Contains(NewTestShip->GetName()))
    {
        FVector ShipPosition = LoadedWorldData.DynamicActorLocations[NewTestShip->GetName()];
        NewTestShip->SetActorLocation(ShipPosition);
        PhysicsManager->RegisterActor(NewTestShip);
    }

    // Verify restored state
    TestEqual("Restored player level should match", LoadedPlayerData.Level, 15);
    TestEqual("Restored credits should match", LoadedPlayerData.Credits, 75000);
    TestEqual("World flag should be set", LoadedWorldData.WorldFlags.Contains(TEXT("GameStarted")), true);
    TestEqual("Completed event should exist", LoadedWorldData.CompletedEvents.Contains(TEXT("ShipAcquired")), true);

    // Test statistics
    FPersistenceStatistics Stats = PersistenceManager->GetStatistics();
    TestEqual("Should have 2 saves", Stats.TotalSaves, 2);
    TestEqual("Should have 2 loads", Stats.TotalLoads, 2);
    TestEqual("Should have 0 failures", Stats.FailedOperations, 0);

    // Cleanup
    PersistenceManager->DeletePlayerData(PlayerID);
    PersistenceManager->DeleteWorldState(WorldID);

    TestWorld->DestroyWorld(false);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS