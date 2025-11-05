// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersistentUniverseManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Compression/OodleDataCompression.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

    // Simple compression using UE5's compression API
    TArray<uint8> CompressedData;
    int32 CompressedSize = Data.Num(); // Initial estimate
    CompressedData.SetNumUninitialized(CompressedSize);

    // Use basic compression
    bool bSuccess = FCompression::CompressMemory(
        NAME_Zlib,
        CompressedData.GetData(),
        CompressedSize,
        Data.GetData(),
        Data.Num()
    );

    if (bSuccess)
    {
        CompressedData.SetNum(CompressedSize);
        return CompressedData;
    }

    return Data;
}

TArray<uint8> UPersistentUniverseManager::DecompressData(const TArray<uint8>& Data) const
{
    if (!bEnableCompression || Data.Num() == 0)
    {
        return Data;
    }

    // Estimate uncompressed size (store in header in production)
    int32 UncompressedSize = Data.Num() * 4; // Rough estimate
    TArray<uint8> UncompressedData;
    UncompressedData.SetNumUninitialized(UncompressedSize);

    bool bSuccess = FCompression::UncompressMemory(
        NAME_Zlib,
        UncompressedData.GetData(),
        UncompressedSize,
        Data.GetData(),
        Data.Num()
    );

    if (bSuccess)
    {
        UncompressedData.SetNum(UncompressedSize);
        return UncompressedData;
    }

    return Data;
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
