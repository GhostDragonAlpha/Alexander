#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerState.h"
#include "NetworkManager.h"
#include "EconomySystem.h"
#include "MissionSystem.h"
#include "VoiceChatSystem.h"
#include "SpaceStationHub.generated.h"

UENUM(BlueprintType)
enum class EHubZoneType : uint8
{
    SpawnZone,
    SocialZone,
    TradingZone,
    MissionZone,
    CustomizationZone,
    HangarZone,
    MedicalZone,
    AdminZone
};

UENUM(BlueprintType)
enum class EHubState : uint8
{
    Active,
    Maintenance,
    Lockdown,
    EventMode
};

USTRUCT(BlueprintType)
struct FHubZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHubZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator ZoneRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresAccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ZoneProperties;
};

USTRUCT(BlueprintType)
struct FHubEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AffectedZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> EventModifiers;
};

/**
 * Wrapper for players in zones
 */
USTRUCT()
struct FSpaceStationZonePlayerList
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<APlayerState*> Players;
};

UCLASS()
class ALEXANDER_API ASpaceStationHub : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStationHub();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Hub Management
    UFUNCTION(BlueprintCallable, Category = "Hub Management")
    void InitializeHub();

    UFUNCTION(BlueprintCallable, Category = "Hub Management")
    void ShutdownHub();

    UFUNCTION(BlueprintCallable, Category = "Hub Management")
    bool IsHubActive() const { return CurrentHubState == EHubState::Active; }

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void RegisterZone(const FHubZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void UnregisterZone(const FName& ZoneName);

    FHubZone* GetZone(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    TArray<FHubZone> GetZonesByType(EHubZoneType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    bool CanPlayerEnterZone(APlayerState* Player, const FName& ZoneName);

    // Player Management
    UFUNCTION(BlueprintCallable, Category = "Player Management")
    void OnPlayerEnterHub(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    void OnPlayerLeaveHub(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    void MovePlayerToZone(APlayerState* Player, const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Player Management")
    TArray<APlayerState*> GetPlayersInZone(const FName& ZoneName);

    // Event Management
    UFUNCTION(BlueprintCallable, Category = "Event Management")
    void StartHubEvent(const FHubEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Event Management")
    void EndHubEvent(const FName& EventName);

    UFUNCTION(BlueprintCallable, Category = "Event Management")
    bool IsEventActive(const FName& EventName);

    UFUNCTION(BlueprintCallable, Category = "Event Management")
    TArray<FHubEvent> GetActiveEvents();

    // Social Features
    UFUNCTION(BlueprintCallable, Category = "Social")
    void EnableVoiceChatInZone(const FName& ZoneName, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void SetZoneSocialRadius(const FName& ZoneName, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<APlayerState*> GetPlayersInSocialRange(APlayerState* Player, float Range);

    // Trading Integration
    UFUNCTION(BlueprintCallable, Category = "Trading")
    void RefreshTradingPost();

    UFUNCTION(BlueprintCallable, Category = "Trading")
    TArray<FMarketItem> GetAvailableItems();

    UFUNCTION(BlueprintCallable, Category = "Trading")
    bool ProcessTrade(APlayerState* Buyer, APlayerState* Seller, const FMarketItem& Item, int32 Quantity);

    // Mission Integration
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void RefreshMissionBoard();

    UFUNCTION(BlueprintCallable, Category = "Missions")
    TArray<FMissionData> GetAvailableMissions(APlayerState* Player);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool AcceptMission(APlayerState* Player, const FMissionData& Mission);

    // Hub State Management
    UFUNCTION(BlueprintCallable, Category = "Hub State")
    void SetHubState(EHubState NewState);

    UFUNCTION(BlueprintCallable, Category = "Hub State")
    EHubState GetHubState() const { return CurrentHubState; }

    // Statistics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetTotalPlayerCount() const { return PlayersInHub.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetZonePlayerCount(const FName& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetAveragePlayerTimeInHub() const;

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    TMap<FName, int32> GetZonePopulationStats();

protected:
    // Hub Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    FName HubName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    FString HubDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    int32 MaxPlayersInHub;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    float HubRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    bool bEnableVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    bool bEnableTrading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hub Config")
    bool bEnableMissions;

    // Zone System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone System")
    TMap<FName, FHubZone> RegisteredZones;

    TMap<FName, FSpaceStationZonePlayerList> PlayersInZones;

    // Player Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
    TSet<APlayerState*> PlayersInHub;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
    TMap<APlayerState*, float> PlayerEntryTimes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
    TMap<APlayerState*, FName> PlayerCurrentZones;

    // Event System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
    TArray<FHubEvent> ActiveEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events")
    TMap<FName, float> EventStartTimes;

    // Hub State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hub State")
    EHubState CurrentHubState;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* HubBounds;

    // System References
    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    // class UNetworkManager* NetworkManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UEconomySystem* EconomySystem;

    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    // class UMissionSystem* MissionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UVoiceChatSystem* VoiceChatSystem;

private:
    // Internal Functions
    void InitializeZones();
    void SetupZoneTriggers();
    void UpdateHubEvents(float DeltaTime);
    void ProcessPlayerInteractions(float DeltaTime);
    void UpdateZonePopulations();
    void HandleHubStateChange(EHubState OldState, EHubState NewState);

    // Zone Management Internal
    void CreateZoneTrigger(const FHubZone& Zone);
    void OnZoneTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    void OnZoneTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Event Management Internal
    void ApplyEventEffects(const FHubEvent& Event);
    void RemoveEventEffects(const FHubEvent& Event);
    void BroadcastEventUpdate(const FHubEvent& Event, bool bStarted);

    // Player Management Internal
    void RegisterPlayerInZone(APlayerState* Player, const FName& ZoneName);
    void UnregisterPlayerFromZone(APlayerState* Player, const FName& ZoneName);
    void UpdatePlayerZoneLocation(APlayerState* Player);

    // Statistics Internal
    void UpdateHubStatistics();
    float CalculateHubActivityLevel() const;

    // Timers
    UPROPERTY()
    FTimerHandle EventUpdateTimer;

    UPROPERTY()
    FTimerHandle StatisticsUpdateTimer;

    UPROPERTY()
    FTimerHandle TradingRefreshTimer;

    UPROPERTY()
    FTimerHandle MissionRefreshTimer;

    // Constants
    static constexpr float EVENT_UPDATE_INTERVAL = 1.0f;
    static constexpr float STATISTICS_UPDATE_INTERVAL = 5.0f;
    static constexpr float TRADING_REFRESH_INTERVAL = 300.0f; // 5 minutes
    static constexpr float MISSION_REFRESH_INTERVAL = 600.0f; // 10 minutes
    static constexpr float DEFAULT_SOCIAL_RADIUS = 500.0f;
};