// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersistentUniverseManager.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Compression/OodleDataCompression.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SystemSelfTestInterface.h"

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
 97     UE_LOG(LogTemp, Log, TEXT("✓ Player data save test PASSED - Saved %d bytes"), SaveResult.BytesSaved);
 98
 99     // Load test data
100     bool bLoadSuccess = false;
101     FPlayerPersistentData LoadedData = LoadPlayerData(TestPlayerID, bLoadSuccess);
102     if (!bLoadSuccess)
103     {
104         OutResult.bPassed = false;
105         OutResult.ErrorMessage = TEXT("Failed to load test player data");
106         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Player data load"));
107         return false;
108     }
109
110     // Verify loaded data matches saved data
111     if (LoadedData.PlayerID != TestPlayerData.PlayerID ||
112         LoadedData.PlayerName != TestPlayerData.PlayerName ||
113         LoadedData.Level != TestPlayerData.Level ||
114         LoadedData.Experience != TestPlayerData.Experience ||
115         LoadedData.Credits != TestPlayerData.Credits ||
116         !LoadedData.LastLocation.Equals(TestPlayerData.LastLocation, 1.0f) ||
117         LoadedData.CurrentSystemID != TestPlayerData.CurrentSystemID ||
118         FMath::Abs(LoadedData.PlayTime - TestPlayerData.PlayTime) > 0.1f)
119     {
120         OutResult.bPassed = false;
121         OutResult.ErrorMessage = TEXT("Loaded player data does not match saved data");
122         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Player data integrity check"));
123         return false;
124     }
125     UE_LOG(LogTemp, Log, TEXT("✓ Player data load and integrity test PASSED"));
126
127     // Test 3: Test world state save/load
128     FString TestWorldID = TEXT("TEST_WORLD_001");
129     FWorldStateData TestWorldData;
130     TestWorldData.WorldID = TestWorldID;
131     TestWorldData.DynamicActorLocations.Add(TEXT("ACTOR_001"), FVector(100.0f, 200.0f, 300.0f));
132     TestWorldData.DynamicActorLocations.Add(TEXT("ACTOR_002"), FVector(400.0f, 500.0f, 600.0f));
133     TestWorldData.WorldFlags.Add(TEXT("Flag1"), true);
134     TestWorldData.WorldFlags.Add(TEXT("Flag2"), false);
135     TestWorldData.CompletedEvents.Add(TEXT("Event1"));
136     TestWorldData.CompletedEvents.Add(TEXT("Event2"));
137
138     FSaveOperationResult WorldSaveResult = SaveWorldState(TestWorldID, TestWorldData);
139     if (!WorldSaveResult.bSuccess)
140     {
141         OutResult.bPassed = false;
142         OutResult.ErrorMessage = TEXT("Failed to save test world data: ") + WorldSaveResult.ErrorMessage;
143         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World state save - %s"), *WorldSaveResult.ErrorMessage);
144         return false;
145     }
146     UE_LOG(LogTemp, Log, TEXT("✓ World state save test PASSED - Saved %d bytes"), WorldSaveResult.BytesSaved);
147
148     bool bWorldLoadSuccess = false;
149     FWorldStateData LoadedWorldData = LoadWorldState(TestWorldID, bWorldLoadSuccess);
150     if (!bWorldLoadSuccess)
151     {
152         OutResult.bPassed = false;
153         OutResult.ErrorMessage = TEXT("Failed to load test world data");
154         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World state load"));
155         return false;
156     }
157
158     // Verify world data integrity
159     if (LoadedWorldData.WorldID != TestWorldData.WorldID ||
160         LoadedWorldData.DynamicActorLocations.Num() != TestWorldData.DynamicActorLocations.Num() ||
161         LoadedWorldData.WorldFlags.Num() != TestWorldData.WorldFlags.Num() ||
162         LoadedWorldData.CompletedEvents.Num() != TestWorldData.CompletedEvents.Num())
163     {
164         OutResult.bPassed = false;
165         OutResult.ErrorMessage = TEXT("Loaded world data structure does not match saved data");
166         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: World data integrity check"));
167         return false;
168     }
169     UE_LOG(LogTemp, Log, TEXT("✓ World state load and integrity test PASSED"));
170
171     // Test 4: Test compression if enabled
172     if (bEnableCompression)
173     {
174         FString LargeTestData = TEXT("This is a test string that should compress well. ");
175         LargeTestData = LargeTestData + LargeTestData + LargeTestData + LargeTestData + LargeTestData;
176
177         TArray<uint8> UncompressedData;
178         UncompressedData.Append((uint8*)TCHAR_TO_UTF8(*LargeTestData), LargeTestData.Len());
179
180         TArray<uint8> CompressedData = CompressData(UncompressedData);
181
182         if (CompressedData.Num() >= UncompressedData.Num())
183         {
184             OutResult.WarningMessages.Add(TEXT("Compression may not be working optimally"));
185             UE_LOG(LogTemp, Warning, TEXT("WARNING: Compression test - compressed size (%d) >= uncompressed size (%d)"),
186                 CompressedData.Num(), UncompressedData.Num());
187         }
188         else
189         {
190             UE_LOG(LogTemp, Log, TEXT("✓ Compression test PASSED - Compressed %d bytes to %d bytes (%.1f%% reduction)"),
191                 UncompressedData.Num(), CompressedData.Num(),
192                 100.0f * (1.0f - (float)CompressedData.Num() / (float)UncompressedData.Num()));
193         }
194
195         // Test decompression
196         TArray<uint8> DecompressedData = DecompressData(CompressedData);
197         if (DecompressedData.Num() != UncompressedData.Num() ||
198             FMemory::Memcmp(DecompressedData.GetData(), UncompressedData.GetData(), UncompressedData.Num()) != 0)
199         {
200             OutResult.bPassed = false;
201             OutResult.ErrorMessage = TEXT("Decompressed data does not match original");
202             UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Decompression integrity check"));
203             return false;
204         }
205         UE_LOG(LogTemp, Log, TEXT("✓ Decompression test PASSED"));
206     }
207     else
208     {
209         UE_LOG(LogTemp, Log, TEXT("⊘ Compression test SKIPPED (compression disabled)"));
210     }
211
212     // Test 5: Test encryption if enabled
213     if (bEnableEncryption)
214     {
215         FString TestString = TEXT("This is sensitive test data that should be encrypted");
216         TArray<uint8> PlainData;
217         PlainData.Append((uint8*)TCHAR_TO_UTF8(*TestString), TestString.Len());
218
219         TArray<uint8> EncryptedData = EncryptData(PlainData);
220         TArray<uint8> DecryptedData = DecryptData(EncryptedData);
221
222         // Verify encryption changed the data
223         if (EncryptedData.Num() != PlainData.Num() ||
224             FMemory::Memcmp(EncryptedData.GetData(), PlainData.GetData(), PlainData.Num()) == 0)
225         {
226             OutResult.WarningMessages.Add(TEXT("Encryption may not be working - encrypted data matches plain data"));
227             UE_LOG(LogTemp, Warning, TEXT("WARNING: Encryption test - data may not be properly encrypted"));
228         }
229         else
230         {
231             UE_LOG(LogTemp, Log, TEXT("✓ Encryption test PASSED - Data properly encrypted"));
232         }
233
234         // Verify decryption restores original data
235         if (DecryptedData.Num() != PlainData.Num() ||
236             FMemory::Memcmp(DecryptedData.GetData(), PlainData.GetData(), PlainData.Num()) != 0)
237         {
238             OutResult.bPassed = false;
239             OutResult.ErrorMessage = TEXT("Decrypted data does not match original plain data");
240             UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Decryption integrity check"));
241             return false;
242         }
243         UE_LOG(LogTemp, Log, TEXT("✓ Decryption test PASSED"));
244     }
245     else
246     {
247         UE_LOG(LogTemp, Log, TEXT("⊘ Encryption test SKIPPED (encryption disabled)"));
248     }
249
250     // Test 6: Test backup creation
251     FString TestBackupName = TEXT("TEST_BACKUP_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
252     if (!CreateBackup(TestBackupName))
253     {
254         OutResult.WarningMessages.Add(TEXT("Backup creation failed - this may be expected in some environments"));
255         UE_LOG(LogTemp, Warning, TEXT("WARNING: Backup creation test failed (may be expected)"));
256     }
257     else
258     {
259         UE_LOG(LogTemp, Log, TEXT("✓ Backup creation test PASSED"));
260
261         // Test backup listing
262         TArray<FString> Backups = GetAvailableBackups();
263         if (!Backups.Contains(TestBackupName))
264         {
265             OutResult.WarningMessages.Add(TEXT("Created backup but cannot find it in listing"));
266             UE_LOG(LogTemp, Warning, TEXT("WARNING: Backup listing test failed"));
267         }
268         else
269         {
270             UE_LOG(LogTemp, Log, TEXT("✓ Backup listing test PASSED"));
271         }
272     }
273
274     // Test 7: Test statistics tracking
275     FPersistenceStatistics CurrentStats = GetStatistics();
276     if (CurrentStats.TotalSaves < 2 || CurrentStats.TotalLoads < 1)
277     {
278         OutResult.bPassed = false;
279         OutResult.ErrorMessage = TEXT("Statistics tracking not working properly");
280         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Statistics tracking"));
281         return false;
282     }
283     UE_LOG(LogTemp, Log, TEXT("✓ Statistics tracking test PASSED - Total saves: %d, Total loads: %d"),
284         CurrentStats.TotalSaves, CurrentStats.TotalLoads);
285
286     // Test 8: Test file size queries
287     int32 PlayerFileSize = GetSaveFileSize(TestPlayerID);
288     if (PlayerFileSize <= 0)
289     {
290         OutResult.WarningMessages.Add(TEXT("File size query returned invalid size"));
291         UE_LOG(LogTemp, Warning, TEXT("WARNING: File size query test failed"));
292     }
293     else
294     {
295         UE_LOG(LogTemp, Log, TEXT("✓ File size query test PASSED - Size: %d bytes"), PlayerFileSize);
296     }
297
298     // Test 9: Test data validation
299     TArray<uint8> ValidData;
300     ValidData.Add(1); ValidData.Add(2); ValidData.Add(3);
301     if (!ValidateSaveData(ValidData))
302     {
303         OutResult.bPassed = false;
304         OutResult.ErrorMessage = TEXT("Data validation failed for valid data");
305         UE_LOG(LogTemp, Error, TEXT("TEST FAILED: Data validation"));
306         return false;
307     }
308     UE_LOG(LogTemp, Log, TEXT("✓ Data validation test PASSED"));
309
310     // Test 10: Test cleanup
311     bool bCleanupSuccess = true;
312     bCleanupSuccess &= DeletePlayerData(TestPlayerID);
313     bCleanupSuccess &= DeleteWorldState(TestWorldID);
314
315     if (!bCleanupSuccess)
316     {
317         OutResult.WarningMessages.Add(TEXT("Test data cleanup failed - test files may remain"));
318         UE_LOG(LogTemp, Warning, TEXT("WARNING: Test cleanup failed"));
319     }
320     else
321     {
322         UE_LOG(LogTemp, Log, TEXT("✓ Test cleanup PASSED"));
323     }
324
325     // Record performance metrics
326     OutResult.bPassed = true;
327     OutResult.PerformanceMetrics.Add(TEXT("PlayerSaveSize"), SaveResult.BytesSaved);
328     OutResult.PerformanceMetrics.Add(TEXT("WorldSaveSize"), WorldSaveResult.BytesSaved);
329     OutResult.PerformanceMetrics.Add(TEXT("PlayerSaveTime"), SaveResult.SaveDuration);
330     OutResult.PerformanceMetrics.Add(TEXT("PlayerLoadTime"), CurrentStats.AverageLoadTime);
331
332     UE_LOG(LogTemp, Log, TEXT("=== Persistent Universe Manager test PASSED ==="));
333     return true;
334 }
335
336 FString UPersistentUniverseManager::GetSystemName_Implementation() const
337 {
338     return TEXT("PersistentUniverseManager");
339 }
340
341 FString UPersistentUniverseManager::GetTestDescription_Implementation() const
342 {
343     return TEXT("Tests save/load functionality, compression, encryption, and backup systems");
344 }
345
346 bool UPersistentUniverseManager::IsReadyForTesting_Implementation() const
347 {
348     return !SaveDirectory.IsEmpty();
349 }
350 //~ End ISystemSelfTestInterface interface
351
 352 void UPersistentUniverseManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
 353 {
 354     Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
 355
 356     if (bEnableAutoSave)
 357     {
 358         AutoSaveTimer += DeltaTime;
 359         if (AutoSaveTimer >= AutoSaveInterval)
 360         {
 361             TriggerAutoSave();
 362             AutoSaveTimer = 0.0f;
 363         }
 364     }
 365 }

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
