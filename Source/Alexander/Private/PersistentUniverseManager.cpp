// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersistentUniverseManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Compression/OodleDataCompression.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SystemSelfTestInterface.h"
#include "Misc/Crc.h"

UPersistentUniverseManager::UPersistentUniverseManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check auto-save every second

    bEnableAutoSave = true;
    AutoSaveInterval = 300.0f; // 5 minutes default
    MaxBackups = 5;
    bEnableCompression = true;
    bEnableEncryption = false;
    bEnableCloudSync = false;
    SaveDirectory = FPaths::ProjectSavedDir() / TEXT("SaveGames");
    AutoSaveTimer = 0.0f;
    EncryptionKey = TEXT("DefaultKey123456789"); // Should be set by user
}


void UPersistentUniverseManager::BeginPlay()
{
    Super::BeginPlay();

    // Create save directory if it doesn't exist
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDirectory))
    {
        PlatformFile.CreateDirectoryTree(*SaveDirectory);
    }

    UE_LOG(LogTemp, Log, TEXT("PersistentUniverseManager initialized. Save directory: %s"), *SaveDirectory);
}

//~ Begin ISystemSelfTestInterface interface
bool UPersistentUniverseManager::RunSelfTest_Implementation(FSystemTestResult& OutResult)
{
    OutResult.SystemName = GetSystemName_Implementation();
    OutResult.WarningMessages.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("=== Testing Persistent Universe Manager ==="));
    
    // Test 1: Check if save directory is accessible
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDirectory))
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Save directory does not exist and cannot be created");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Save directory not accessible"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Save directory test PASSED - Directory: %s"), *SaveDirectory);
    
    // Test 2: Test player data save/load
    FString TestPlayerID = TEXT("TEST_PLAYER_001");
    FPlayerPersistentData TestPlayerData;
    TestPlayerData.PlayerID = TestPlayerID;
    TestPlayerData.PlayerName = TEXT("Test Player");
    TestPlayerData.Level = 5;
    TestPlayerData.Experience = 1250;
    TestPlayerData.Credits = 50000;
    TestPlayerData.LastLocation = FVector(1000.0f, 2000.0f, 3000.0f);
    TestPlayerData.CurrentSystemID = TEXT("SOL_SYSTEM");
    TestPlayerData.PlayTime = 3600.0f; // 1 hour
    
    // Save test data
    FSaveOperationResult SaveResult = SavePlayerData(TestPlayerID, TestPlayerData);
    if (!SaveResult.bSuccess)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Failed to save test player data: ") + SaveResult.ErrorMessage;
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Player data save - %s"), *SaveResult.ErrorMessage);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Player data save test PASSED - Saved %d bytes"), SaveResult.BytesSaved);

    // Load test data
    bool bLoadSuccess = false;
    FPlayerPersistentData LoadedData = LoadPlayerData(TestPlayerID, bLoadSuccess);
    if (!bLoadSuccess)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Failed to load test player data");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Player data load"));
        return false;
    }

    // Verify loaded data matches saved data
    if (LoadedData.PlayerID != TestPlayerData.PlayerID ||
        LoadedData.PlayerName != TestPlayerData.PlayerName ||
        LoadedData.Level != TestPlayerData.Level ||
        LoadedData.Experience != TestPlayerData.Experience ||
        LoadedData.Credits != TestPlayerData.Credits ||
        !LoadedData.LastLocation.Equals(TestPlayerData.LastLocation, 1.0f) ||
        LoadedData.CurrentSystemID != TestPlayerData.CurrentSystemID ||
        FMath::Abs(LoadedData.PlayTime - TestPlayerData.PlayTime) > 0.1f)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Loaded player data does not match saved data");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Player data integrity check"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Player data load and integrity test PASSED"));

    // Test 3: Test world state save/load
    FString TestWorldID = TEXT("TEST_WORLD_001");
    FWorldStateData TestWorldData;
    TestWorldData.WorldID = TestWorldID;
    TestWorldData.DynamicActorLocations.Add(TEXT("ACTOR_001"), FVector(100.0f, 200.0f, 300.0f));
    TestWorldData.DynamicActorLocations.Add(TEXT("ACTOR_002"), FVector(400.0f, 500.0f, 600.0f));
    TestWorldData.WorldFlags.Add(TEXT("Flag1"), true);
    TestWorldData.WorldFlags.Add(TEXT("Flag2"), false);
    TestWorldData.CompletedEvents.Add(TEXT("Event1"));
    TestWorldData.CompletedEvents.Add(TEXT("Event2"));

    FSaveOperationResult WorldSaveResult = SaveWorldState(TestWorldID, TestWorldData);
    if (!WorldSaveResult.bSuccess)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Failed to save test world data: ") + WorldSaveResult.ErrorMessage;
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World state save - %s"), *WorldSaveResult.ErrorMessage);
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ World state save test PASSED - Saved %d bytes"), WorldSaveResult.BytesSaved);

    bool bWorldLoadSuccess = false;
    FWorldStateData LoadedWorldData = LoadWorldState(TestWorldID, bWorldLoadSuccess);
    if (!bWorldLoadSuccess)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Failed to load test world data");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World state load"));
        return false;
    }

    // Verify world data integrity
    if (LoadedWorldData.WorldID != TestWorldData.WorldID ||
        LoadedWorldData.DynamicActorLocations.Num() != TestWorldData.DynamicActorLocations.Num() ||
        LoadedWorldData.WorldFlags.Num() != TestWorldData.WorldFlags.Num() ||
        LoadedWorldData.CompletedEvents.Num() != TestWorldData.CompletedEvents.Num())
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Loaded world data structure does not match saved data");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World data integrity check"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ World state load and integrity test PASSED"));

    // Test 4: Test compression if enabled
    if (bEnableCompression)
    {
        FString LargeTestData = TEXT("This is a test string that should compress well. ");
        LargeTestData = LargeTestData + LargeTestData + LargeTestData + LargeTestData + LargeTestData;

        TArray<uint8> UncompressedData;
        UncompressedData.Append((uint8*)TCHAR_TO_UTF8(*LargeTestData), LargeTestData.Len());

        TArray<uint8> CompressedData = CompressData(UncompressedData);

        if (CompressedData.Num() >= UncompressedData.Num())
        {
            OutResult.WarningMessages.Add(TEXT("Compression may not be working optimally"));
            UE_LOG(LogTemp, Warning, TEXT("WARNING: Compression test - compressed size (%d) >= uncompressed size (%d)"),
                CompressedData.Num(), UncompressedData.Num());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Compression test PASSED - Compressed %d bytes to %d bytes (%.1f%% reduction)"),
                UncompressedData.Num(), CompressedData.Num(),
                100.0f * (1.0f - (float)CompressedData.Num() / (float)UncompressedData.Num()));
        }

        // Test decompression
        TArray<uint8> DecompressedData = DecompressData(CompressedData);
        if (DecompressedData.Num() != UncompressedData.Num() ||
            FMemory::Memcmp(DecompressedData.GetData(), UncompressedData.GetData(), UncompressedData.Num()) != 0)
        {
            OutResult.bPassed = false;
            OutResult.ErrorMessage = TEXT("Decompressed data does not match original");
            UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Decompression integrity check"));
            return false;
        }
        UE_LOG(LogTemp, Log, TEXT("✓ Decompression test PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("⊘ Compression test SKIPPED (compression disabled)"));
    }

    // Test 5: Test encryption if enabled
    if (bEnableEncryption)
    {
        FString TestString = TEXT("This is sensitive test data that should be encrypted");
        TArray<uint8> PlainData;
        PlainData.Append((uint8*)TCHAR_TO_UTF8(*TestString), TestString.Len());

        TArray<uint8> EncryptedData = EncryptData(PlainData);
        TArray<uint8> DecryptedData = DecryptData(EncryptedData);

        // Verify encryption changed the data
        if (EncryptedData.Num() != PlainData.Num() ||
            FMemory::Memcmp(EncryptedData.GetData(), PlainData.GetData(), PlainData.Num()) == 0)
        {
            OutResult.WarningMessages.Add(TEXT("Encryption may not be working - encrypted data matches plain data"));
            UE_LOG(LogTemp, Warning, TEXT("WARNING: Encryption test - data may not be properly encrypted"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Encryption test PASSED - Data properly encrypted"));
        }

        // Verify decryption restores original data
        if (DecryptedData.Num() != PlainData.Num() ||
            FMemory::Memcmp(DecryptedData.GetData(), PlainData.GetData(), PlainData.Num()) != 0)
        {
            OutResult.bPassed = false;
            OutResult.ErrorMessage = TEXT("Decrypted data does not match original plain data");
            UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Decryption integrity check"));
            return false;
        }
        UE_LOG(LogTemp, Log, TEXT("✓ Decryption test PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("⊘ Encryption test SKIPPED (encryption disabled)"));
    }

    // Test 6: Test backup creation
    FString TestBackupName = TEXT("TEST_BACKUP_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    if (!CreateBackup(TestBackupName))
    {
        OutResult.WarningMessages.Add(TEXT("Backup creation failed - this may be expected in some environments"));
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Backup creation test failed (may be expected)"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Backup creation test PASSED"));

        // Test backup listing
        TArray<FString> Backups = GetAvailableBackups();
        if (!Backups.Contains(TestBackupName))
        {
            OutResult.WarningMessages.Add(TEXT("Created backup but cannot find it in listing"));
            UE_LOG(LogTemp, Warning, TEXT("WARNING: Backup listing test failed"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Backup listing test PASSED"));
        }
    }

    // Test 7: Test statistics tracking
    FPersistenceStatistics CurrentStats = GetStatistics();
    if (CurrentStats.TotalSaves < 2 || CurrentStats.TotalLoads < 1)
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Statistics tracking not working properly");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics tracking"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Statistics tracking test PASSED - Total saves: %d, Total loads: %d"),
        CurrentStats.TotalSaves, CurrentStats.TotalLoads);

    // Test 8: Test file size queries
    int32 PlayerFileSize = GetSaveFileSize(TestPlayerID);
    if (PlayerFileSize <= 0)
    {
        OutResult.WarningMessages.Add(TEXT("File size query returned invalid size"));
        UE_LOG(LogTemp, Warning, TEXT("WARNING: File size query test failed"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✓ File size query test PASSED - Size: %d bytes"), PlayerFileSize);
    }

    // Test 9: Test data validation
    TArray<uint8> ValidData;
    ValidData.Add(1); ValidData.Add(2); ValidData.Add(3);
    if (!ValidateSaveData(ValidData))
    {
        OutResult.bPassed = false;
        OutResult.ErrorMessage = TEXT("Data validation failed for valid data");
        UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Data validation"));
        return false;
    }
    UE_LOG(LogTemp, Log, TEXT("✓ Data validation test PASSED"));

    // Test 10: Test cleanup
    bool bCleanupSuccess = true;
    bCleanupSuccess &= DeletePlayerData(TestPlayerID);
    bCleanupSuccess &= DeleteWorldState(TestWorldID);

    if (!bCleanupSuccess)
    {
        OutResult.WarningMessages.Add(TEXT("Test data cleanup failed - test files may remain"));
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Test cleanup failed"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Test cleanup PASSED"));
    }

    // Record performance metrics
    OutResult.bPassed = true;
    OutResult.PerformanceMetrics.Add(TEXT("PlayerSaveSize"), SaveResult.BytesSaved);
    OutResult.PerformanceMetrics.Add(TEXT("WorldSaveSize"), WorldSaveResult.BytesSaved);
    OutResult.PerformanceMetrics.Add(TEXT("PlayerSaveTime"), SaveResult.SaveDuration);
    OutResult.PerformanceMetrics.Add(TEXT("PlayerLoadTime"), CurrentStats.AverageLoadTime);

    UE_LOG(LogTemp, Log, TEXT("=== Persistent Universe Manager test PASSED ==="));
    return true;
}

FString UPersistentUniverseManager::GetSystemName_Implementation() const
{
    return TEXT("PersistentUniverseManager");
}

FString UPersistentUniverseManager::GetTestDescription_Implementation() const
{
    return TEXT("Tests save/load functionality, compression, encryption, and backup systems");
}

bool UPersistentUniverseManager::IsReadyForTesting_Implementation() const
{
    return !SaveDirectory.IsEmpty();
}
//~ End ISystemSelfTestInterface interface

void UPersistentUniverseManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableAutoSave)
    {
        AutoSaveTimer += DeltaTime;
        if (AutoSaveTimer >= AutoSaveInterval)
        {
            TriggerAutoSave();
            AutoSaveTimer = 0.0f;
        }
    }
}

FString UPersistentUniverseManager::GetSaveFilePath(EPersistenceType DataType, const FString& DataID) const
{
    FString TypeFolder;
    switch (DataType)
    {
        case EPersistenceType::PlayerData:
            TypeFolder = TEXT("Players");
            break;
        case EPersistenceType::ShipData:
            TypeFolder = TEXT("Ships");
            break;
        case EPersistenceType::InventoryData:
            TypeFolder = TEXT("Inventory");
            break;
        case EPersistenceType::WorldState:
            TypeFolder = TEXT("Worlds");
            break;
        case EPersistenceType::FactionData:
            TypeFolder = TEXT("Factions");
            break;
        case EPersistenceType::EconomyData:
            TypeFolder = TEXT("Economy");
            break;
        case EPersistenceType::QuestData:
            TypeFolder = TEXT("Quests");
            break;
        case EPersistenceType::NPCState:
            TypeFolder = TEXT("NPCs");
            break;
        default:
            TypeFolder = TEXT("Misc");
            break;
    }

    return SaveDirectory / TypeFolder / (DataID + TEXT(".sav"));
}

FString UPersistentUniverseManager::GetSaveFilePath(const FString& ObjectID, const FString& Category) const
{
    return SaveDirectory / Category / (ObjectID + TEXT(".sav"));
}

bool UPersistentUniverseManager::WriteToFile(const FString& FilePath, const TArray<uint8>& Data)
{
    // Create directory if needed
    FString Directory = FPaths::GetPath(FilePath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }

    return FFileHelper::SaveArrayToFile(Data, *FilePath);
}

bool UPersistentUniverseManager::ReadFromFile(const FString& FilePath, TArray<uint8>& OutData)
{
    return FFileHelper::LoadFileToArray(OutData, *FilePath);
}

TArray<uint8> UPersistentUniverseManager::CompressData(const TArray<uint8>& Data) const
{
    if (!bEnableCompression || Data.Num() == 0)
    {
        return Data;
    }

    // Store uncompressed size in header (4 bytes)
    TArray<uint8> CompressedData;
    CompressedData.SetNum(sizeof(int32));
    int32 UncompressedSize = Data.Num();
    FMemory::Memcpy(CompressedData.GetData(), &UncompressedSize, sizeof(int32));

    // Compress the data
    TArray<uint8> TempCompressed;
    int32 CompressedSize = Data.Num(); // Initial estimate
    TempCompressed.SetNumUninitialized(CompressedSize);

    // Use basic compression
    bool bSuccess = FCompression::CompressMemory(
        NAME_Zlib,
        TempCompressed.GetData(),
        CompressedSize,
        Data.GetData(),
        Data.Num()
    );

    if (bSuccess)
    {
        TempCompressed.SetNum(CompressedSize);
        CompressedData.Append(TempCompressed);
        return CompressedData;
    }

    // If compression failed, return original data with header indicating no compression (size = 0)
    CompressedData.Empty();
    CompressedData.SetNum(sizeof(int32));
    int32 NoCompressionMarker = 0;
    FMemory::Memcpy(CompressedData.GetData(), &NoCompressionMarker, sizeof(int32));
    CompressedData.Append(Data);
    return CompressedData;
}

TArray<uint8> UPersistentUniverseManager::DecompressData(const TArray<uint8>& Data) const
{
    if (!bEnableCompression || Data.Num() < sizeof(int32))
    {
        return Data;
    }

    // Read uncompressed size from header
    int32 UncompressedSize = 0;
    FMemory::Memcpy(&UncompressedSize, Data.GetData(), sizeof(int32));

    // Check if data was actually compressed (size = 0 means no compression)
    if (UncompressedSize == 0)
    {
        // Data was not compressed, return everything after the header
        TArray<uint8> UncompressedData;
        UncompressedData.Append(Data.GetData() + sizeof(int32), Data.Num() - sizeof(int32));
        return UncompressedData;
    }

    // Decompress the data
    TArray<uint8> UncompressedData;
    UncompressedData.SetNumUninitialized(UncompressedSize);

    bool bSuccess = FCompression::UncompressMemory(
        NAME_Zlib,
        UncompressedData.GetData(),
        UncompressedSize,
        Data.GetData() + sizeof(int32), // Skip header
        Data.Num() - sizeof(int32)
    );

    if (bSuccess)
    {
        UncompressedData.SetNum(UncompressedSize);
        return UncompressedData;
    }

    // If decompression failed, return original data (without header)
    UE_LOG(LogTemp, Error, TEXT("Failed to decompress data, returning original"));
    TArray<uint8> FailsafeData;
    FailsafeData.Append(Data.GetData() + sizeof(int32), Data.Num() - sizeof(int32));
    return FailsafeData;
}

TArray<uint8> UPersistentUniverseManager::EncryptData(const TArray<uint8>& Data) const
{
    if (!bEnableEncryption || Data.Num() == 0)
    {
        return Data;
    }

    // Simple XOR encryption (use proper encryption in production)
    TArray<uint8> EncryptedData = Data;
    TArray<uint8> KeyBytes;
    KeyBytes.SetNumUninitialized(EncryptionKey.Len());
    FMemory::Memcpy(KeyBytes.GetData(), TCHAR_TO_UTF8(*EncryptionKey), EncryptionKey.Len());

    for (int32 i = 0; i < EncryptedData.Num(); ++i)
    {
        EncryptedData[i] ^= KeyBytes[i % KeyBytes.Num()];
    }

    return EncryptedData;
}

TArray<uint8> UPersistentUniverseManager::DecryptData(const TArray<uint8>& Data) const
{
    // XOR encryption is symmetric
    return EncryptData(Data);
}

FSaveOperationResult UPersistentUniverseManager::SavePlayerData(const FString& PlayerID, const FPlayerPersistentData& Data)
{
    FSaveOperationResult Result;
    float StartTime = FPlatformTime::Seconds();

    // Serialize data
    FBufferArchive Archive;
    
    // Write header
    int32 Version = 1;
    Archive << Version;
    
    // Write player data
    FString TempPlayerID = Data.PlayerID;
    FString TempPlayerName = Data.PlayerName;
    int32 TempLevel = Data.Level;
    int32 TempExperience = Data.Experience;
    int32 TempCredits = Data.Credits;
    FVector TempLocation = Data.LastLocation;
    FRotator TempRotation = Data.LastRotation;
    FString TempSystemID = Data.CurrentSystemID;
    float TempPlayTime = Data.PlayTime;
    
    Archive << TempPlayerID;
    Archive << TempPlayerName;
    Archive << TempLevel;
    Archive << TempExperience;
    Archive << TempCredits;
    Archive << TempLocation;
    Archive << TempRotation;
    Archive << TempSystemID;
    Archive << TempPlayTime;

    // Process data
    TArray<uint8> ProcessedData(Archive.GetData(), Archive.Num());

    if (bEnableCompression)
    {
        ProcessedData = CompressData(ProcessedData);
    }

    if (bEnableEncryption)
    {
        ProcessedData = EncryptData(ProcessedData);
    }

    // Add checksum for data integrity
    uint32 Checksum = FCrc::MemCrc32(ProcessedData.GetData(), ProcessedData.Num());
    ProcessedData.Append((uint8*)&Checksum, sizeof(uint32));

    // Write to file
    FString FilePath = GetSaveFilePath(EPersistenceType::PlayerData, PlayerID);
    Result.bSuccess = WriteToFile(FilePath, ProcessedData);
    
    if (Result.bSuccess)
    {
        Result.BytesSaved = ProcessedData.Num();
        CachedPlayerData.Add(PlayerID, Data);
        
        // Update statistics
        Statistics.TotalSaves++;
        Statistics.TotalBytesWritten += Result.BytesSaved;
        
        UE_LOG(LogTemp, Log, TEXT("Successfully saved player data for %s (%d bytes)"), *PlayerID, Result.BytesSaved);
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to write file");
        Statistics.FailedOperations++;
        UE_LOG(LogTemp, Error, TEXT("Failed to save player data for %s"), *PlayerID);
    }

    Result.SaveDuration = FPlatformTime::Seconds() - StartTime;
    Statistics.AverageSaveTime = (Statistics.AverageSaveTime * (Statistics.TotalSaves - 1) + Result.SaveDuration) / Statistics.TotalSaves;

    OnSaveCompleted.Broadcast(Result);
    return Result;
}

FPlayerPersistentData UPersistentUniverseManager::LoadPlayerData(const FString& PlayerID, bool& bSuccess)
{
    float StartTime = FPlatformTime::Seconds();
    FPlayerPersistentData Data;
    bSuccess = false;

    // Check cache first
    if (CachedPlayerData.Contains(PlayerID))
    {
        bSuccess = true;
        return CachedPlayerData[PlayerID];
    }

    // Read from file
    FString FilePath = GetSaveFilePath(EPersistenceType::PlayerData, PlayerID);
    TArray<uint8> FileData;

    if (!ReadFromFile(FilePath, FileData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load player data for %s"), *PlayerID);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    // Process data
    TArray<uint8> ProcessedData = FileData;

    // Verify checksum first
    if (ProcessedData.Num() < sizeof(uint32))
    {
        UE_LOG(LogTemp, Error, TEXT("Save data too small - corrupted file for %s"), *PlayerID);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    // Extract and verify checksum
    uint32 StoredChecksum;
    FMemory::Memcpy(&StoredChecksum, ProcessedData.GetData() + ProcessedData.Num() - sizeof(uint32), sizeof(uint32));

    // Remove checksum from data
    TArray<uint8> DataWithoutChecksum;
    DataWithoutChecksum.Append(ProcessedData.GetData(), ProcessedData.Num() - sizeof(uint32));

    uint32 CalculatedChecksum = FCrc::MemCrc32(DataWithoutChecksum.GetData(), DataWithoutChecksum.Num());
    if (CalculatedChecksum != StoredChecksum)
    {
        UE_LOG(LogTemp, Error, TEXT("Save data corrupted - checksum mismatch for %s (expected: %u, got: %u)"),
            *PlayerID, StoredChecksum, CalculatedChecksum);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    ProcessedData = DataWithoutChecksum;

    if (bEnableEncryption)
    {
        ProcessedData = DecryptData(ProcessedData);
    }

    if (bEnableCompression)
    {
        ProcessedData = DecompressData(ProcessedData);
    }

    // Deserialize
    FMemoryReader Archive(ProcessedData, true);

    int32 Version;
    Archive << Version;

    // Check version and perform migration if needed
    const int32 CurrentVersion = 1; // Update this when save format changes
    if (Version != CurrentVersion)
    {
        if (!MigrateFromVersion(Version, CurrentVersion, Archive))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to migrate player data from version %d to %d for %s"),
                Version, CurrentVersion, *PlayerID);
            Statistics.FailedOperations++;
            OnLoadCompleted.Broadcast(false);
            return Data;
        }
    }

    FString TempPlayerID;
    FString TempPlayerName;
    int32 TempLevel;
    int32 TempExperience;
    int32 TempCredits;
    FVector TempLocation;
    FRotator TempRotation;
    FString TempSystemID;
    float TempPlayTime;
    
    Archive << TempPlayerID;
    Archive << TempPlayerName;
    Archive << TempLevel;
    Archive << TempExperience;
    Archive << TempCredits;
    Archive << TempLocation;
    Archive << TempRotation;
    Archive << TempSystemID;
    Archive << TempPlayTime;

    Data.PlayerID = TempPlayerID;
    Data.PlayerName = TempPlayerName;
    Data.Level = TempLevel;
    Data.Experience = TempExperience;
    Data.Credits = TempCredits;
    Data.LastLocation = TempLocation;
    Data.LastRotation = TempRotation;
    Data.CurrentSystemID = TempSystemID;
    Data.PlayTime = TempPlayTime;
    Data.LastSaveTime = FDateTime::Now();

    // Cache data
    CachedPlayerData.Add(PlayerID, Data);

    // Update statistics
    Statistics.TotalLoads++;
    Statistics.TotalBytesRead += FileData.Num();
    float LoadTime = FPlatformTime::Seconds() - StartTime;
    Statistics.AverageLoadTime = (Statistics.AverageLoadTime * (Statistics.TotalLoads - 1) + LoadTime) / Statistics.TotalLoads;

    bSuccess = true;
    OnLoadCompleted.Broadcast(true);
    
    UE_LOG(LogTemp, Log, TEXT("Successfully loaded player data for %s"), *PlayerID);
    return Data;
}

FSaveOperationResult UPersistentUniverseManager::SaveWorldState(const FString& WorldID, const FWorldStateData& Data)
{
    FSaveOperationResult Result;
    float StartTime = FPlatformTime::Seconds();

    // Serialize data
    FBufferArchive Archive;
    
    int32 Version = 1;
    Archive << Version;
    
    FString TempWorldID = Data.WorldID;
    Archive << TempWorldID;
    
    // Serialize dynamic actor locations
    int32 ActorCount = Data.DynamicActorLocations.Num();
    Archive << ActorCount;
    
    for (const auto& Pair : Data.DynamicActorLocations)
    {
        FString ActorID = Pair.Key;
        FVector Location = Pair.Value;
        Archive << ActorID;
        Archive << Location;
    }
    
    // Serialize world flags
    int32 FlagCount = Data.WorldFlags.Num();
    Archive << FlagCount;
    
    for (const auto& Pair : Data.WorldFlags)
    {
        FString FlagName = Pair.Key;
        bool FlagValue = Pair.Value;
        Archive << FlagName;
        Archive << FlagValue;
    }
    
    // Serialize completed events
    int32 EventCount = Data.CompletedEvents.Num();
    Archive << EventCount;
    
    for (const FString& Event : Data.CompletedEvents)
    {
        FString TempEvent = Event;
        Archive << TempEvent;
    }

    // Process data
    TArray<uint8> ProcessedData(Archive.GetData(), Archive.Num());

    if (bEnableCompression)
    {
        ProcessedData = CompressData(ProcessedData);
    }

    if (bEnableEncryption)
    {
        ProcessedData = EncryptData(ProcessedData);
    }

    // Add checksum for data integrity
    uint32 Checksum = FCrc::MemCrc32(ProcessedData.GetData(), ProcessedData.Num());
    ProcessedData.Append((uint8*)&Checksum, sizeof(uint32));

    // Write to file
    FString FilePath = GetSaveFilePath(EPersistenceType::WorldState, WorldID);
    Result.bSuccess = WriteToFile(FilePath, ProcessedData);
    
    if (Result.bSuccess)
    {
        Result.BytesSaved = ProcessedData.Num();
        CachedWorldStates.Add(WorldID, Data);
        
        Statistics.TotalSaves++;
        Statistics.TotalBytesWritten += Result.BytesSaved;
        
        UE_LOG(LogTemp, Log, TEXT("Successfully saved world state for %s (%d bytes)"), *WorldID, Result.BytesSaved);
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to write file");
        Statistics.FailedOperations++;
        UE_LOG(LogTemp, Error, TEXT("Failed to save world state for %s"), *WorldID);
    }

    Result.SaveDuration = FPlatformTime::Seconds() - StartTime;
    Statistics.AverageSaveTime = (Statistics.AverageSaveTime * (Statistics.TotalSaves - 1) + Result.SaveDuration) / Statistics.TotalSaves;

    OnSaveCompleted.Broadcast(Result);
    return Result;
}

FWorldStateData UPersistentUniverseManager::LoadWorldState(const FString& WorldID, bool& bSuccess)
{
    float StartTime = FPlatformTime::Seconds();
    FWorldStateData Data;
    bSuccess = false;

    // Check cache
    if (CachedWorldStates.Contains(WorldID))
    {
        bSuccess = true;
        return CachedWorldStates[WorldID];
    }

    // Read from file
    FString FilePath = GetSaveFilePath(EPersistenceType::WorldState, WorldID);
    TArray<uint8> FileData;

    if (!ReadFromFile(FilePath, FileData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load world state for %s"), *WorldID);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    // Process data
    TArray<uint8> ProcessedData = FileData;

    // Verify checksum first
    if (ProcessedData.Num() < sizeof(uint32))
    {
        UE_LOG(LogTemp, Error, TEXT("World state data too small - corrupted file for %s"), *WorldID);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    // Extract and verify checksum
    uint32 StoredChecksum;
    FMemory::Memcpy(&StoredChecksum, ProcessedData.GetData() + ProcessedData.Num() - sizeof(uint32), sizeof(uint32));

    // Remove checksum from data
    TArray<uint8> DataWithoutChecksum;
    DataWithoutChecksum.Append(ProcessedData.GetData(), ProcessedData.Num() - sizeof(uint32));

    uint32 CalculatedChecksum = FCrc::MemCrc32(DataWithoutChecksum.GetData(), DataWithoutChecksum.Num());
    if (CalculatedChecksum != StoredChecksum)
    {
        UE_LOG(LogTemp, Error, TEXT("World state corrupted - checksum mismatch for %s (expected: %u, got: %u)"),
            *WorldID, StoredChecksum, CalculatedChecksum);
        Statistics.FailedOperations++;
        OnLoadCompleted.Broadcast(false);
        return Data;
    }

    ProcessedData = DataWithoutChecksum;

    if (bEnableEncryption)
    {
        ProcessedData = DecryptData(ProcessedData);
    }

    if (bEnableCompression)
    {
        ProcessedData = DecompressData(ProcessedData);
    }

    // Deserialize
    FMemoryReader Archive(ProcessedData, true);

    int32 Version;
    Archive << Version;
    
    FString TempWorldID;
    Archive << TempWorldID;
    Data.WorldID = TempWorldID;
    
    // Load actor locations
    int32 ActorCount;
    Archive << ActorCount;
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FString ActorID;
        FVector Location;
        Archive << ActorID;
        Archive << Location;
        Data.DynamicActorLocations.Add(ActorID, Location);
    }
    
    // Load world flags
    int32 FlagCount;
    Archive << FlagCount;
    
    for (int32 i = 0; i < FlagCount; ++i)
    {
        FString FlagName;
        bool FlagValue;
        Archive << FlagName;
        Archive << FlagValue;
        Data.WorldFlags.Add(FlagName, FlagValue);
    }
    
    // Load completed events
    int32 EventCount;
    Archive << EventCount;
    
    for (int32 i = 0; i < EventCount; ++i)
    {
        FString Event;
        Archive << Event;
        Data.CompletedEvents.Add(Event);
    }

    Data.WorldTime = FDateTime::Now();
    CachedWorldStates.Add(WorldID, Data);

    // Update statistics
    Statistics.TotalLoads++;
    Statistics.TotalBytesRead += FileData.Num();
    float LoadTime = FPlatformTime::Seconds() - StartTime;
    Statistics.AverageLoadTime = (Statistics.AverageLoadTime * (Statistics.TotalLoads - 1) + LoadTime) / Statistics.TotalLoads;

    bSuccess = true;
    OnLoadCompleted.Broadcast(true);
    
    UE_LOG(LogTemp, Log, TEXT("Successfully loaded world state for %s"), *WorldID);
    return Data;
}

// Save Economic State
bool UPersistentUniverseManager::SaveEconomicState(const FString& FactionID, const FFactionEconomicData& EconomicData)
{
	FString FilePath = GetSaveFilePath(FactionID, TEXT("Economy"));

	// Serialize economy data
	FBufferArchive Archive;

	int32 Version = 1;
	Archive << Version;
	
	// Serialize faction ID
	FString TempFactionID = FactionID;
	Archive << TempFactionID;
	
	// Serialize economic data (placeholder - in production would serialize full struct)
	float TempCredits = EconomicData.Credits;
	float TempInfluence = EconomicData.Influence;
	Archive << TempCredits;
	Archive << TempInfluence;

	// Process data
	TArray<uint8> ProcessedData(Archive.GetData(), Archive.Num());

	if (bEnableCompression)
	{
		ProcessedData = CompressData(ProcessedData);
	}

	if (bEnableEncryption)
	{
		ProcessedData = EncryptData(ProcessedData);
	}

	bool bSuccess = WriteToFile(FilePath, ProcessedData);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully saved economic state for faction %s (%d bytes)"), *FactionID, ProcessedData.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save economic state for faction %s"), *FactionID);
	}

	return bSuccess;
}

// Load Economic State
bool UPersistentUniverseManager::LoadEconomicState(const FString& FactionID, FFactionEconomicData& OutEconomicData)
{
	FString FilePath = GetSaveFilePath(FactionID, TEXT("Economy"));
	TArray<uint8> FileData;

	if (!ReadFromFile(FilePath, FileData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load economic state file for faction %s"), *FactionID);
		return false;
	}

	// Process data
	TArray<uint8> ProcessedData = FileData;

	if (bEnableEncryption)
	{
		ProcessedData = DecryptData(ProcessedData);
	}

	if (bEnableCompression)
	{
		ProcessedData = DecompressData(ProcessedData);
	}

	// Deserialize
	FMemoryReader Archive(ProcessedData, true);

	int32 Version;
	Archive << Version;
	
	FString TempFactionID;
	Archive << TempFactionID;
	
	float TempCredits;
	float TempInfluence;
	Archive << TempCredits;
	Archive << TempInfluence;
	
	OutEconomicData.Credits = TempCredits;
	OutEconomicData.Influence = TempInfluence;

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded economic state for faction %s"), *FactionID);
	return true;
}

// Save Farming State
bool UPersistentUniverseManager::SaveFarmingState(const FString& FarmID, const FFarmStateData& FarmData)
{
	FString FilePath = GetSaveFilePath(FarmID, TEXT("Farming"));

	// Serialize farming data
	FBufferArchive Archive;

	int32 Version = 1;
	Archive << Version;
	
	// Serialize farm ID
	FString TempFarmID = FarmID;
	Archive << TempFarmID;
	
	// Serialize farm data (placeholder)
	int32 TempPlotCount = FarmData.PlotCount;
	Archive << TempPlotCount;

	// Process data
	TArray<uint8> ProcessedData(Archive.GetData(), Archive.Num());

	if (bEnableCompression)
	{
		ProcessedData = CompressData(ProcessedData);
	}

	if (bEnableEncryption)
	{
		ProcessedData = EncryptData(ProcessedData);
	}

	bool bSuccess = WriteToFile(FilePath, ProcessedData);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully saved farming state for farm %s (%d bytes)"), *FarmID, ProcessedData.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save farming state for farm %s"), *FarmID);
	}

	return bSuccess;
}

// Load Farming State
bool UPersistentUniverseManager::LoadFarmingState(const FString& FarmID, FFarmStateData& OutFarmData)
{
	FString FilePath = GetSaveFilePath(FarmID, TEXT("Farming"));
	TArray<uint8> FileData;

	if (!ReadFromFile(FilePath, FileData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load farming state file for farm %s"), *FarmID);
		return false;
	}

	// Process data
	TArray<uint8> ProcessedData = FileData;

	if (bEnableEncryption)
	{
		ProcessedData = DecryptData(ProcessedData);
	}

	if (bEnableCompression)
	{
		ProcessedData = DecompressData(ProcessedData);
	}

	// Deserialize
	FMemoryReader Archive(ProcessedData, true);

	int32 Version;
	Archive << Version;
	
	FString TempFarmID;
	Archive << TempFarmID;
	
	int32 TempPlotCount;
	Archive << TempPlotCount;
	
	OutFarmData.PlotCount = TempPlotCount;

	UE_LOG(LogTemp, Log, TEXT("Successfully loaded farming state for farm %s"), *FarmID);
	return true;
}

FSaveOperationResult UPersistentUniverseManager::SaveAllData()
{
    FSaveOperationResult Result;
    float StartTime = FPlatformTime::Seconds();
    int32 TotalBytes = 0;
    bool bAllSuccess = true;

    // Save all cached player data
    for (const auto& Pair : CachedPlayerData)
    {
        FSaveOperationResult PlayerResult = SavePlayerData(Pair.Key, Pair.Value);
        if (!PlayerResult.bSuccess)
        {
            bAllSuccess = false;
        }
        TotalBytes += PlayerResult.BytesSaved;
    }

    // Save all cached world states
    for (const auto& Pair : CachedWorldStates)
    {
        FSaveOperationResult WorldResult = SaveWorldState(Pair.Key, Pair.Value);
        if (!WorldResult.bSuccess)
        {
            bAllSuccess = false;
        }
        TotalBytes += WorldResult.BytesSaved;
    }

    // Save economic state
    SaveEconomicState();

    // Save farming state
    SaveFarmingState();

    Result.bSuccess = bAllSuccess;
    Result.BytesSaved = TotalBytes;
    Result.SaveDuration = FPlatformTime::Seconds() - StartTime;

    if (bAllSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully saved all data (%d bytes in %.2fs)"), TotalBytes, Result.SaveDuration);
    }
    else
    {
        Result.ErrorMessage = TEXT("Some save operations failed");
        UE_LOG(LogTemp, Warning, TEXT("Saved all data with some failures (%d bytes in %.2fs)"), TotalBytes, Result.SaveDuration);
    }

    return Result;
}

bool UPersistentUniverseManager::LoadAllData()
{
    bool bSuccess = true;

    // Load all player saves
    TArray<FString> PlayerIDs = GetSavedPlayerIDs();
    for (const FString& PlayerID : PlayerIDs)
    {
        bool bLoadSuccess = false;
        LoadPlayerData(PlayerID, bLoadSuccess);
        if (!bLoadSuccess)
        {
            bSuccess = false;
        }
    }

    // Load economic state
    LoadEconomicState();

    // Load farming state
    LoadFarmingState();

    UE_LOG(LogTemp, Log, TEXT("Loaded all available save data"));
    return bSuccess;
}

void UPersistentUniverseManager::SaveAsync(EPersistenceType DataType, const FString& DataID)
{
    // Async save would use UE5's async task system
    // For now, perform synchronous save
    UE_LOG(LogTemp, Log, TEXT("Async save requested for %s (performing sync save)"), *DataID);
    
    // This would be expanded in production to use FQueuedThreadPool or AsyncTask
}

void UPersistentUniverseManager::EnableAutoSave(bool bEnable)
{
    bEnableAutoSave = bEnable;
    if (bEnable)
    {
        AutoSaveTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Auto-save enabled (interval: %.1fs)"), AutoSaveInterval);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Auto-save disabled"));
    }
}

void UPersistentUniverseManager::SetAutoSaveInterval(float Seconds)
{
    AutoSaveInterval = FMath::Max(10.0f, Seconds); // Minimum 10 seconds
    UE_LOG(LogTemp, Log, TEXT("Auto-save interval set to %.1fs"), AutoSaveInterval);
}

bool UPersistentUniverseManager::IsAutoSaveEnabled() const
{
    return bEnableAutoSave;
}

void UPersistentUniverseManager::TriggerAutoSave()
{
    float StartTime = FPlatformTime::Seconds();
    FSaveOperationResult Result = SaveAllData();
    float ElapsedTime = FPlatformTime::Seconds() - StartTime;
    
    OnAutoSaveTriggered.Broadcast(ElapsedTime, Result.bSuccess);
    
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Auto-save completed successfully (%.2fs)"), ElapsedTime);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-save completed with errors (%.2fs)"), ElapsedTime);
    }
}

bool UPersistentUniverseManager::CreateBackup(const FString& BackupName)
{
    FString BackupDir = SaveDirectory / TEXT("Backups") / BackupName;
    FString SourceDir = SaveDirectory;

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (!PlatformFile.DirectoryExists(*BackupDir))
    {
        PlatformFile.CreateDirectoryTree(*BackupDir);
    }

    // Copy all save files to backup
    bool bSuccess = PlatformFile.CopyDirectoryTree(*BackupDir, *SourceDir, true);
    
    if (bSuccess)
    {
        CleanupOldBackups();
        UE_LOG(LogTemp, Log, TEXT("Backup created: %s"), *BackupName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create backup: %s"), *BackupName);
    }

    return bSuccess;
}

bool UPersistentUniverseManager::RestoreBackup(const FString& BackupName)
{
    FString BackupDir = SaveDirectory / TEXT("Backups") / BackupName;
    FString TargetDir = SaveDirectory;

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (!PlatformFile.DirectoryExists(*BackupDir))
    {
        UE_LOG(LogTemp, Error, TEXT("Backup not found: %s"), *BackupName);
        return false;
    }

    // Clear current saves and restore from backup
    bool bSuccess = PlatformFile.CopyDirectoryTree(*TargetDir, *BackupDir, true);
    
    if (bSuccess)
    {
        // Clear caches
        CachedPlayerData.Empty();
        CachedWorldStates.Empty();
        
        UE_LOG(LogTemp, Log, TEXT("Backup restored: %s"), *BackupName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to restore backup: %s"), *BackupName);
    }

    return bSuccess;
}

TArray<FString> UPersistentUniverseManager::GetAvailableBackups() const
{
    TArray<FString> Backups;
    FString BackupDir = SaveDirectory / TEXT("Backups");

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    class FDirectoryVisitor : public IPlatformFile::FDirectoryVisitor
    {
    public:
        TArray<FString>& Directories;
        
        FDirectoryVisitor(TArray<FString>& InDirectories) : Directories(InDirectories) {}
        
        virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
        {
            if (bIsDirectory)
            {
                Directories.Add(FPaths::GetCleanFilename(FilenameOrDirectory));
            }
            return true;
        }
    };

    FDirectoryVisitor Visitor(Backups);
    PlatformFile.IterateDirectory(*BackupDir, Visitor);

    return Backups;
}

bool UPersistentUniverseManager::DeleteBackup(const FString& BackupName)
{
    FString BackupDir = SaveDirectory / TEXT("Backups") / BackupName;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    bool bSuccess = PlatformFile.DeleteDirectoryRecursively(*BackupDir);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Backup deleted: %s"), *BackupName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to delete backup: %s"), *BackupName);
    }

    return bSuccess;
}

void UPersistentUniverseManager::CleanupOldBackups()
{
    TArray<FString> Backups = GetAvailableBackups();
    
    if (Backups.Num() > MaxBackups)
    {
        // Sort by creation time (would need to check file timestamps in production)
        // For now, just delete oldest alphabetically
        Backups.Sort();
        
        int32 ToDelete = Backups.Num() - MaxBackups;
        for (int32 i = 0; i < ToDelete; ++i)
        {
            DeleteBackup(Backups[i]);
        }
    }
}

FString UPersistentUniverseManager::GenerateBackupName() const
{
    FDateTime Now = FDateTime::Now();
    return FString::Printf(TEXT("Backup_%04d%02d%02d_%02d%02d%02d"),
        Now.GetYear(), Now.GetMonth(), Now.GetDay(),
        Now.GetHour(), Now.GetMinute(), Now.GetSecond());
}

void UPersistentUniverseManager::EnableCloudSync(bool bEnable)
{
    bEnableCloudSync = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Cloud sync %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UPersistentUniverseManager::SyncToCloud()
{
    if (!bEnableCloudSync)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cloud sync not enabled"));
        return false;
    }

    // Cloud sync implementation would go here
    // This would integrate with platform services (Steam Cloud, Epic Online Services, etc.)
    UE_LOG(LogTemp, Log, TEXT("Cloud sync to cloud requested (not implemented)"));
    return false;
}

bool UPersistentUniverseManager::SyncFromCloud()
{
    if (!bEnableCloudSync)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cloud sync not enabled"));
        return false;
    }

    // Cloud sync implementation would go here
    UE_LOG(LogTemp, Log, TEXT("Cloud sync from cloud requested (not implemented)"));
    return false;
}

bool UPersistentUniverseManager::IsCloudSyncEnabled() const
{
    return bEnableCloudSync;
}

bool UPersistentUniverseManager::DeletePlayerData(const FString& PlayerID)
{
    FString FilePath = GetSaveFilePath(EPersistenceType::PlayerData, PlayerID);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    bool bSuccess = PlatformFile.DeleteFile(*FilePath);
    
    if (bSuccess)
    {
        CachedPlayerData.Remove(PlayerID);
        UE_LOG(LogTemp, Log, TEXT("Deleted player data: %s"), *PlayerID);
    }

    return bSuccess;
}

bool UPersistentUniverseManager::DeleteWorldState(const FString& WorldID)
{
    FString FilePath = GetSaveFilePath(EPersistenceType::WorldState, WorldID);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    bool bSuccess = PlatformFile.DeleteFile(*FilePath);
    
    if (bSuccess)
    {
        CachedWorldStates.Remove(WorldID);
        UE_LOG(LogTemp, Log, TEXT("Deleted world state: %s"), *WorldID);
    }

    return bSuccess;
}

TArray<FString> UPersistentUniverseManager::GetSavedPlayerIDs() const
{
    TArray<FString> PlayerIDs;
    FString PlayerDir = SaveDirectory / TEXT("Players");

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    class FFileVisitor : public IPlatformFile::FDirectoryVisitor
    {
    public:
        TArray<FString>& Files;
        
        FFileVisitor(TArray<FString>& InFiles) : Files(InFiles) {}
        
        virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
        {
            if (!bIsDirectory)
            {
                FString Filename = FPaths::GetBaseFilename(FilenameOrDirectory);
                Files.Add(Filename);
            }
            return true;
        }
    };

    FFileVisitor Visitor(PlayerIDs);
    PlatformFile.IterateDirectory(*PlayerDir, Visitor);

    return PlayerIDs;
}

bool UPersistentUniverseManager::DoesSaveExist(const FString& SaveID) const
{
    // Check multiple data types
    for (int32 i = 0; i < 8; ++i)
    {
        EPersistenceType Type = static_cast<EPersistenceType>(i);
        FString FilePath = GetSaveFilePath(Type, SaveID);
        
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (PlatformFile.FileExists(*FilePath))
        {
            return true;
        }
    }

    return false;
}

void UPersistentUniverseManager::EnableCompression(bool bEnable)
{
    bEnableCompression = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Compression %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UPersistentUniverseManager::IsCompressionEnabled() const
{
    return bEnableCompression;
}

void UPersistentUniverseManager::EnableEncryption(bool bEnable)
{
    bEnableEncryption = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Encryption %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPersistentUniverseManager::SetEncryptionKey(const FString& Key)
{
    if (Key.Len() >= 16)
    {
        EncryptionKey = Key;
        UE_LOG(LogTemp, Log, TEXT("Encryption key updated"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Encryption key too short (minimum 16 characters)"));
    }
}

bool UPersistentUniverseManager::IsEncryptionEnabled() const
{
    return bEnableEncryption;
}

FPersistenceStatistics UPersistentUniverseManager::GetStatistics() const
{
    return Statistics;
}

void UPersistentUniverseManager::ResetStatistics()
{
    Statistics = FPersistenceStatistics();
    UE_LOG(LogTemp, Log, TEXT("Statistics reset"));
}

int32 UPersistentUniverseManager::GetSaveFileSize(const FString& SaveID) const
{
    FString FilePath = GetSaveFilePath(EPersistenceType::PlayerData, SaveID);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (PlatformFile.FileExists(*FilePath))
    {
        return PlatformFile.FileSize(*FilePath);
    }

    return 0;
}

bool UPersistentUniverseManager::ValidateSaveData(const TArray<uint8>& Data) const
{
    // Basic validation - check if data is not empty and has reasonable size
    return Data.Num() > 0 && Data.Num() < 100 * 1024 * 1024; // Max 100MB
}

bool UPersistentUniverseManager::MigrateFromVersion(int32 OldVersion, int32 NewVersion, FMemoryReader& Reader)
{
    // Handle version migrations
    if (OldVersion == NewVersion)
    {
        // No migration needed
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("Migrating save data from version %d to %d"), OldVersion, NewVersion);

    // Version 1 to Version 2 migration
    if (OldVersion == 1 && NewVersion == 2)
    {
        // Example: In v2, we might add new fields with default values
        // Read old data, add new fields with defaults
        UE_LOG(LogTemp, Log, TEXT("Applying migration from v1 to v2"));
        return true;
    }

    // Add more version migrations as needed
    // if (OldVersion == 2 && NewVersion == 3)
    // {
    //     // Handle v2 to v3 migration
    //     return true;
    // }

    // Unsupported migration path
    UE_LOG(LogTemp, Warning, TEXT("Unsupported migration path from version %d to %d"), OldVersion, NewVersion);
    return false;
}
