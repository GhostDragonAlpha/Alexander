// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SystemSelfTestInterface.h"
#include "PersistentUniverseManager.generated.h"

// Forward declarations
class AActor;

/**
 * Persistence data types
 */
UENUM(BlueprintType)
enum class EPersistenceType : uint8
{
    PlayerData      UMETA(DisplayName = "Player Data"),
    ShipData        UMETA(DisplayName = "Ship Data"),
    InventoryData   UMETA(DisplayName = "Inventory Data"),
    WorldState      UMETA(DisplayName = "World State"),
    FactionData     UMETA(DisplayName = "Faction Data"),
    EconomyData     UMETA(DisplayName = "Economy Data"),
    QuestData       UMETA(DisplayName = "Quest Data"),
    NPCState        UMETA(DisplayName = "NPC State")
};

/**
 * Save priority levels
 */
UENUM(BlueprintType)
enum class ESavePriority : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Player persistent data
 */
USTRUCT(BlueprintType)
struct FPlayerPersistentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 Experience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 Credits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FVector LastLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FRotator LastRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString CurrentSystemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float PlayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FDateTime LastSaveTime;

    FPlayerPersistentData()
        : Level(1)
        , Experience(0)
        , Credits(10000)
        , LastLocation(FVector::ZeroVector)
        , LastRotation(FRotator::ZeroRotator)
        , PlayTime(0.0f)
    {}
};

/**
 * World state data
 */
USTRUCT(BlueprintType)
struct FWorldStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString WorldID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    TMap<FString, FVector> DynamicActorLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    TMap<FString, bool> WorldFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FDateTime WorldTime;

    FWorldStateData()
    {}
};

/**
 * Save operation result
 */
USTRUCT(BlueprintType)
struct FSaveOperationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float SaveDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 BytesSaved;

    FSaveOperationResult()
        : bSuccess(false)
        , SaveDuration(0.0f)
        , BytesSaved(0)
    {}
};

/**
 * Persistence statistics
 */
USTRUCT(BlueprintType)
struct FPersistenceStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 TotalSaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 TotalLoads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 FailedOperations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float AverageSaveTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float AverageLoadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 TotalBytesWritten;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 TotalBytesRead;

    FPersistenceStatistics()
        : TotalSaves(0)
        , TotalLoads(0)
        , FailedOperations(0)
        , AverageSaveTime(0.0f)
        , AverageLoadTime(0.0f)
        , TotalBytesWritten(0)
        , TotalBytesRead(0)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, const FSaveOperationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAutoSaveTriggered, float, ElapsedTime, bool, bSuccess);

/**
 * Persistent Universe Manager Component
 * Manages data persistence, world state, and save/load operations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPersistentUniverseManager : public UActorComponent, public ISystemSelfTestInterface
{
    GENERATED_BODY()

public:
    UPersistentUniverseManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    //~ Begin ISystemSelfTestInterface interface
    virtual bool RunSelfTest_Implementation(FSystemTestResult& OutResult) override;
    virtual FString GetSystemName_Implementation() const override;
    virtual FString GetTestDescription_Implementation() const override;
    virtual bool IsReadyForTesting_Implementation() const override;
    //~ End ISystemSelfTestInterface interface

    // Save Operations
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FSaveOperationResult SavePlayerData(const FString& PlayerID, const FPlayerPersistentData& Data);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FSaveOperationResult SaveWorldState(const FString& WorldID, const FWorldStateData& Data);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FSaveOperationResult SaveAllData();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SaveAsync(EPersistenceType DataType, const FString& DataID);

    // Load Operations
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FPlayerPersistentData LoadPlayerData(const FString& PlayerID, bool& bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FWorldStateData LoadWorldState(const FString& WorldID, bool& bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool LoadAllData();

    // Auto-Save
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void EnableAutoSave(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SetAutoSaveInterval(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool IsAutoSaveEnabled() const;

    // Backup Management
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool CreateBackup(const FString& BackupName);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool RestoreBackup(const FString& BackupName);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    TArray<FString> GetAvailableBackups() const;

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool DeleteBackup(const FString& BackupName);

    // Cloud Sync
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void EnableCloudSync(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool SyncToCloud();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool SyncFromCloud();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool IsCloudSyncEnabled() const;

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool DeletePlayerData(const FString& PlayerID);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool DeleteWorldState(const FString& WorldID);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    TArray<FString> GetSavedPlayerIDs() const;

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool DoesSaveExist(const FString& SaveID) const;

    // Compression
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void EnableCompression(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool IsCompressionEnabled() const;

    // Encryption
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void EnableEncryption(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SetEncryptionKey(const FString& Key);

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool IsEncryptionEnabled() const;

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    FPersistenceStatistics GetStatistics() const;

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void ResetStatistics();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    int32 GetSaveFileSize(const FString& SaveID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Persistence")
    FOnSaveCompleted OnSaveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Persistence")
    FOnLoadCompleted OnLoadCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Persistence")
    FOnAutoSaveTriggered OnAutoSaveTriggered;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bEnableAutoSave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    float AutoSaveInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    int32 MaxBackups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bEnableCompression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bEnableEncryption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    bool bEnableCloudSync;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
    FString SaveDirectory;

private:
    // Data storage
    UPROPERTY()
    TMap<FString, FPlayerPersistentData> CachedPlayerData;

    UPROPERTY()
    TMap<FString, FWorldStateData> CachedWorldStates;

    float AutoSaveTimer;
    FPersistenceStatistics Statistics;
    FString EncryptionKey;

    // Helper functions
    FString GetSaveFilePath(EPersistenceType DataType, const FString& DataID) const;
    bool WriteToFile(const FString& FilePath, const TArray<uint8>& Data);
    bool ReadFromFile(const FString& FilePath, TArray<uint8>& OutData);
    TArray<uint8> CompressData(const TArray<uint8>& Data) const;
    TArray<uint8> DecompressData(const TArray<uint8>& Data) const;
    TArray<uint8> EncryptData(const TArray<uint8>& Data) const;
    TArray<uint8> DecryptData(const TArray<uint8>& Data) const;
    void TriggerAutoSave();
    void CleanupOldBackups();
    FString GenerateBackupName() const;
    bool ValidateSaveData(const TArray<uint8>& Data) const;
};
