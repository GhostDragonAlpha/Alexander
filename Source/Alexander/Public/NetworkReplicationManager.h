// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "NetworkReplicationManager.generated.h"

// Forward declarations
class AActor;
class APawn;

/**
 * Replication priority levels
 */
UENUM(BlueprintType)
enum class EReplicationPriority : uint8
{
    VeryLow     UMETA(DisplayName = "Very Low"),
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    VeryHigh    UMETA(DisplayName = "Very High"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Network relevancy distance settings
 */
UENUM(BlueprintType)
enum class ERelevancyDistance : uint8
{
    Close       UMETA(DisplayName = "Close (5km)"),
    Medium      UMETA(DisplayName = "Medium (15km)"),
    Far         UMETA(DisplayName = "Far (50km)"),
    VeryFar     UMETA(DisplayName = "Very Far (100km)"),
    Unlimited   UMETA(DisplayName = "Unlimited")
};

/**
 * Entity types for replication prioritization
 */
UENUM(BlueprintType)
enum class EEntityType : uint8
{
    Player          UMETA(DisplayName = "Player"),
    PlayerShip      UMETA(DisplayName = "Player Ship"),
    NPC             UMETA(DisplayName = "NPC"),
    NPCShip         UMETA(DisplayName = "NPC Ship"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Station         UMETA(DisplayName = "Station"),
    Planet          UMETA(DisplayName = "Planet"),
    Asteroid        UMETA(DisplayName = "Asteroid"),
    Effect          UMETA(DisplayName = "Effect"),
    Pickup          UMETA(DisplayName = "Pickup"),
    Environmental   UMETA(DisplayName = "Environmental")
};

/**
 * Replicated entity information
 */
USTRUCT(BlueprintType)
struct FReplicatedEntityInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    EEntityType EntityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    EReplicationPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    float RelevancyRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    FVector LastReplicatedLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    FRotator LastReplicatedRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    FVector LastReplicatedVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    float LastReplicationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    bool bIsDormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
    int32 NetworkID;

    FReplicatedEntityInfo()
        : Actor(nullptr)
        , EntityType(EEntityType::Environmental)
        , Priority(EReplicationPriority::Normal)
        , RelevancyRadius(50000.0f)
        , LastReplicatedLocation(FVector::ZeroVector)
        , LastReplicatedRotation(FRotator::ZeroRotator)
        , LastReplicatedVelocity(FVector::ZeroVector)
        , LastReplicationTime(0.0f)
        , bIsDormant(false)
        , NetworkID(0)
    {}
};

/**
 * Client prediction state
 */
USTRUCT(BlueprintType)
struct FClientPredictionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    FVector PredictedLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    FRotator PredictedRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    FVector PredictedVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    float PredictionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prediction")
    float ErrorTolerance;

    FClientPredictionState()
        : PredictedLocation(FVector::ZeroVector)
        , PredictedRotation(FRotator::ZeroRotator)
        , PredictedVelocity(FVector::ZeroVector)
        , PredictionTime(0.0f)
        , ErrorTolerance(100.0f)
    {}
};

/**
 * Network statistics
 */
USTRUCT(BlueprintType)
struct FAlexanderNetworkStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 EntitiesReplicated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 BytesSent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 BytesReceived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    float AverageLatency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    float PacketLoss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
    int32 ActiveConnections;

    FAlexanderNetworkStatistics()
        : EntitiesReplicated(0)
        , BytesSent(0)
        , BytesReceived(0)
        , AverageLatency(0.0f)
        , PacketLoss(0.0f)
        , ActiveConnections(0)
    {}
};

/**
 * Server meshing zone information
 */
USTRUCT(BlueprintType)
struct FServerMeshZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    FString ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    FString ServerNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    int32 PlayerCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    float ServerLoad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server Mesh")
    bool bIsActive;

    FServerMeshZone()
        : ZoneCenter(FVector::ZeroVector)
        , ZoneRadius(100000.0f)
        , PlayerCount(0)
        , EntityCount(0)
        , ServerLoad(0.0f)
        , bIsActive(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityRegistered, int32, NetworkID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityUnregistered, int32, NetworkID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplicationError, int32, NetworkID, const FString&, ErrorMessage);

/**
 * Network Replication Manager Component
 * Manages entity replication, client prediction, and server meshing
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UNetworkReplicationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNetworkReplicationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Entity Registration
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    int32 RegisterEntity(AActor* Actor, EEntityType EntityType, EReplicationPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    bool UnregisterEntity(int32 NetworkID);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    FReplicatedEntityInfo GetEntityInfo(int32 NetworkID) const;

    // Replication Control
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void SetEntityPriority(int32 NetworkID, EReplicationPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void SetEntityDormancy(int32 NetworkID, bool bDormant);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void ForceReplication(int32 NetworkID);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    bool ShouldReplicateEntity(int32 NetworkID, const FVector& ViewerLocation) const;

    // Client Prediction
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void EnableClientPrediction(int32 NetworkID, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    FClientPredictionState PredictEntityState(int32 NetworkID, float DeltaTime) const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void ReconcileClientPrediction(int32 NetworkID, const FVector& ServerLocation, const FRotator& ServerRotation);

    // Relevancy Management
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    TArray<int32> GetRelevantEntities(const FVector& ViewerLocation, float MaxDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void SetRelevancyRadius(int32 NetworkID, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    bool IsEntityRelevant(int32 NetworkID, const FVector& ViewerLocation) const;

    // Server Meshing
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    FServerMeshZone CreateServerZone(const FVector& Center, float Radius, const FString& ServerNodeID);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void UpdateServerZone(const FString& ZoneID, int32 PlayerCount, int32 EntityCount, float ServerLoad);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    TArray<FServerMeshZone> GetActiveZones() const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    FServerMeshZone GetZoneForLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    bool RequestZoneTransfer(int32 NetworkID, const FString& TargetZoneID);

    // Bandwidth Management
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void SetBandwidthLimit(int32 BytesPerSecond);

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    int32 GetCurrentBandwidthUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    float GetReplicationBudget() const;

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    FAlexanderNetworkStatistics GetNetworkStatistics() const;

    UFUNCTION(BlueprintCallable, Category = "Network Replication")
    void ResetStatistics();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    float TickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    int32 MaxEntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    bool bEnableClientPrediction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    bool bEnableServerMeshing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    float MaxRelevancyDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Replication")
    int32 BandwidthLimitBytesPerSecond;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Network Replication")
    FOnEntityRegistered OnEntityRegistered;

    UPROPERTY(BlueprintAssignable, Category = "Network Replication")
    FOnEntityUnregistered OnEntityUnregistered;

    UPROPERTY(BlueprintAssignable, Category = "Network Replication")
    FOnReplicationError OnReplicationError;

private:
    // Entity storage
    UPROPERTY()
    TMap<int32, FReplicatedEntityInfo> RegisteredEntities;

    UPROPERTY()
    TMap<int32, FClientPredictionState> PredictionStates;

    UPROPERTY()
    TMap<FString, FServerMeshZone> ServerZones;

    int32 NextNetworkID;
    FAlexanderNetworkStatistics Statistics;
    float TimeSinceLastStatReset;
    int32 CurrentFrameBytesUsed;

    // Helper functions
    float CalculateReplicationPriority(const FReplicatedEntityInfo& Entity, const FVector& ViewerLocation) const;
    bool ShouldReplicateThisFrame(const FReplicatedEntityInfo& Entity, float DeltaTime) const;
    void UpdateEntityReplication(int32 NetworkID, float DeltaTime);
    void ProcessReplicationQueue(float DeltaTime);
    float GetPriorityMultiplier(EReplicationPriority Priority) const;
    float GetRelevancyDistance(ERelevancyDistance Distance) const;
    void UpdateNetworkStatistics(float DeltaTime);
    bool IsWithinBandwidthBudget(int32 EstimatedBytes) const;
    void OptimizeReplicationForBandwidth();
    FServerMeshZone* FindBestZoneForEntity(const FVector& Location);
    void BalanceServerLoad();
};
