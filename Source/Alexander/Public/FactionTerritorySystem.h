// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "FactionTerritorySystem.generated.h"

// Forward declarations
class AActor;

/**
 * Faction types
 */
UENUM(BlueprintType)
enum class EFactionType : uint8
{
    Military        UMETA(DisplayName = "Military"),
    Corporate       UMETA(DisplayName = "Corporate"),
    Pirate          UMETA(DisplayName = "Pirate"),
    Traders         UMETA(DisplayName = "Traders"),
    Scientists      UMETA(DisplayName = "Scientists"),
    Religious       UMETA(DisplayName = "Religious"),
    Colonists       UMETA(DisplayName = "Colonists"),
    Independent     UMETA(DisplayName = "Independent"),
    Alien           UMETA(DisplayName = "Alien"),
    Rebel           UMETA(DisplayName = "Rebel")
};

/**
 * Faction allegiance
 */
UENUM(BlueprintType)
enum class EFactionAllegiance : uint8
{
    Friendly        UMETA(DisplayName = "Friendly"),
    Allied          UMETA(DisplayName = "Allied"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Unfriendly      UMETA(DisplayName = "Unfriendly"),
    Hostile         UMETA(DisplayName = "Hostile"),
    AtWar           UMETA(DisplayName = "At War")
};

/**
 * Territory control status
 */
UENUM(BlueprintType)
enum class ETerritoryStatus : uint8
{
    Controlled      UMETA(DisplayName = "Controlled"),
    Contested       UMETA(DisplayName = "Contested"),
    UnderSiege      UMETA(DisplayName = "Under Siege"),
    Blockaded       UMETA(DisplayName = "Blockaded"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

/**
 * Faction government type
 */
UENUM(BlueprintType)
enum class EGovernmentType : uint8
{
    Democracy       UMETA(DisplayName = "Democracy"),
    Dictatorship    UMETA(DisplayName = "Dictatorship"),
    Corporate       UMETA(DisplayName = "Corporate"),
    Theocracy       UMETA(DisplayName = "Theocracy"),
    Anarchy         UMETA(DisplayName = "Anarchy"),
    Feudal          UMETA(DisplayName = "Feudal"),
    Confederation   UMETA(DisplayName = "Confederation"),
    Collective      UMETA(DisplayName = "Collective")
};

/**
 * Faction data
 */
USTRUCT(BlueprintType)
struct FFactionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FString FactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FString FactionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    EFactionType FactionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    EGovernmentType GovernmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    FLinearColor FactionColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 Power; // Military/Economic power

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 Wealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 Influence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    TArray<FString> ControlledSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    TArray<FString> AlliedFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    TArray<FString> HostileFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    bool bIsPlayable;

    FFactionData()
        : FactionType(EFactionType::Independent)
        , GovernmentType(EGovernmentType::Democracy)
        , FactionColor(FLinearColor::White)
        , Power(100)
        , Wealth(10000)
        , Influence(50)
        , bIsPlayable(false)
    {}
};

/**
 * Territory data
 */
USTRUCT(BlueprintType)
struct FTerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FString TerritoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FString SystemID; // Which star system

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FString ControllingFaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    ETerritoryStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float ControlStrength; // 0-100%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TMap<FString, float> ContestingFactions; // FactionID -> Influence %

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 MilitaryPresence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 Population;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TaxRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FString> Resources; // Available resources

    FTerritoryData()
        : Status(ETerritoryStatus::Neutral)
        , ControlStrength(0.0f)
        , MilitaryPresence(0)
        , Population(0)
        , TaxRate(0.05f)
    {}
};

/**
 * Faction reputation
 */
USTRUCT(BlueprintType)
struct FFactionReputation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    FString FactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    int32 ReputationPoints; // -100 to +100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    EFactionAllegiance Allegiance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    int32 MissionsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    int32 TradeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    bool bHasAccessPermission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    TArray<FString> UnlockedBenefits;

    FFactionReputation()
        : ReputationPoints(0)
        , Allegiance(EFactionAllegiance::Neutral)
        , MissionsCompleted(0)
        , TradeVolume(0)
        , bHasAccessPermission(false)
    {}
};

/**
 * Faction conflict
 */
USTRUCT(BlueprintType)
struct FFactionConflict
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString ConflictID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString Faction1ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString Faction2ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FString DisputedTerritoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    float ConflictIntensity; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    bool bIsActive;

    FFactionConflict()
        : ConflictIntensity(0.0f)
        , bIsActive(false)
    {}
};

/**
 * Faction mission
 */
USTRUCT(BlueprintType)
struct FFactionMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString FactionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ReputationReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 CreditReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredReputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsRepeatable;

    FFactionMission()
        : ReputationReward(10)
        , CreditReward(1000)
        , RequiredReputation(0)
        , bIsRepeatable(false)
    {}
};

/**
 * Border zone
 */
USTRUCT(BlueprintType)
struct FBorderZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    FString ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    FString Faction1ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    FString Faction2ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    TArray<FString> BorderSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    float TensionLevel; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
    bool bIsHotZone;

    FBorderZone()
        : TensionLevel(0.0f)
        , bIsHotZone(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryChanged, const FString&, TerritoryID, const FString&, NewFactionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReputationChanged, const FString&, FactionID, int32, NewReputation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConflictStarted, const FString&, Faction1ID, const FString&, Faction2ID, const FString&, TerritoryID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllegianceChanged, EFactionAllegiance, NewAllegiance);

/**
 * Wrapper for faction relations (nested TMap)
 */
USTRUCT()
struct FFactionRelations
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, EFactionAllegiance> Relations;
};

/**
 * Faction Territory System Component
 * Manages faction relationships, territorial control, reputation, and conflicts
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UFactionTerritorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UFactionTerritorySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Faction Management
    UFUNCTION(BlueprintCallable, Category = "Faction")
    FString CreateFaction(const FString& FactionName, EFactionType Type, EGovernmentType Government);

    UFUNCTION(BlueprintCallable, Category = "Faction")
    FFactionData GetFactionData(const FString& FactionID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "Faction")
    TArray<FFactionData> GetAllFactions() const;

    UFUNCTION(BlueprintCallable, Category = "Faction")
    void SetFactionPower(const FString& FactionID, int32 Power);

    UFUNCTION(BlueprintCallable, Category = "Faction")
    void ModifyFactionWealth(const FString& FactionID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Faction")
    TArray<FString> GetFactionsByType(EFactionType Type) const;

    // Territory Control
    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool ClaimTerritory(const FString& TerritoryID, const FString& FactionID);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool ContestedTerritory(const FString& TerritoryID, const FString& AttackingFactionID);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FTerritoryData GetTerritoryData(const FString& TerritoryID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    TArray<FTerritoryData> GetFactionTerritories(const FString& FactionID) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FString GetTerritoryController(const FString& TerritoryID) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void UpdateTerritoryControl(float DeltaTime);

    // Reputation System
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void ModifyReputation(const FString& FactionID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    int32 GetReputation(const FString& FactionID) const;

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    EFactionAllegiance GetAllegiance(const FString& FactionID) const;

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    TArray<FFactionReputation> GetAllReputations() const;

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    bool HasAccessPermission(const FString& FactionID) const;

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void GrantAccessPermission(const FString& FactionID, bool bGrant);

    // Faction Relations
    UFUNCTION(BlueprintCallable, Category = "Relations")
    void SetFactionRelation(const FString& Faction1ID, const FString& Faction2ID, EFactionAllegiance Relation);

    UFUNCTION(BlueprintCallable, Category = "Relations")
    EFactionAllegiance GetFactionRelation(const FString& Faction1ID, const FString& Faction2ID) const;

    UFUNCTION(BlueprintCallable, Category = "Relations")
    void FormAlliance(const FString& Faction1ID, const FString& Faction2ID);

    UFUNCTION(BlueprintCallable, Category = "Relations")
    void BreakAlliance(const FString& Faction1ID, const FString& Faction2ID);

    UFUNCTION(BlueprintCallable, Category = "Relations")
    void DeclareWar(const FString& Faction1ID, const FString& Faction2ID);

    UFUNCTION(BlueprintCallable, Category = "Relations")
    void MakePeace(const FString& Faction1ID, const FString& Faction2ID);

    // Conflicts
    UFUNCTION(BlueprintCallable, Category = "Conflict")
    FString StartConflict(const FString& Faction1ID, const FString& Faction2ID, const FString& TerritoryID);

    UFUNCTION(BlueprintCallable, Category = "Conflict")
    void EndConflict(const FString& ConflictID, const FString& WinnerFactionID);

    UFUNCTION(BlueprintCallable, Category = "Conflict")
    TArray<FFactionConflict> GetActiveConflicts() const;

    UFUNCTION(BlueprintCallable, Category = "Conflict")
    bool IsInConflict(const FString& Faction1ID, const FString& Faction2ID) const;

    UFUNCTION(BlueprintCallable, Category = "Conflict")
    void UpdateConflicts(float DeltaTime);

    // Missions
    UFUNCTION(BlueprintCallable, Category = "Missions")
    TArray<FFactionMission> GetAvailableMissions(const FString& FactionID) const;

    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void GenerateFactionMissions(const FString& FactionID, int32 Count);

    // Border Management
    UFUNCTION(BlueprintCallable, Category = "Border")
    void CreateBorderZone(const FString& Faction1ID, const FString& Faction2ID);

    UFUNCTION(BlueprintCallable, Category = "Border")
    TArray<FBorderZone> GetBorderZones() const;

    UFUNCTION(BlueprintCallable, Category = "Border")
    void UpdateBorderTensions(float DeltaTime);

    // Influence System
    UFUNCTION(BlueprintCallable, Category = "Influence")
    void SpreadInfluence(const FString& FactionID, const FString& TerritoryID, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Influence")
    float GetFactionInfluence(const FString& FactionID, const FString& TerritoryID) const;

    UFUNCTION(BlueprintCallable, Category = "Influence")
    FString GetDominantFaction(const FString& TerritoryID) const;

    // AI Behavior
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SimulateFactionBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ProcessFactionExpansion(const FString& FactionID);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ProcessFactionDiplomacy(const FString& FactionID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Faction")
    FOnTerritoryChanged OnTerritoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Faction")
    FOnReputationChanged OnReputationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Faction")
    FOnConflictStarted OnConflictStarted;

    UPROPERTY(BlueprintAssignable, Category = "Faction")
    FOnAllegianceChanged OnAllegianceChanged;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    bool bEnableAIFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    float TerritoryUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    float ConflictUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    int32 MaxActiveConflicts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    bool bEnableDynamicBorders;

private:
    UPROPERTY()
    TMap<FString, FFactionData> Factions;

    UPROPERTY()
    TMap<FString, FTerritoryData> Territories;

    UPROPERTY()
    TMap<FString, FFactionReputation> PlayerReputations;

    UPROPERTY()
    TArray<FFactionConflict> ActiveConflicts;

    UPROPERTY()
    TArray<FBorderZone> BorderZones;

    UPROPERTY()
    TArray<FFactionMission> AvailableMissions;

    UPROPERTY()
    TMap<FString, FFactionRelations> FactionRelations;

    float TerritoryUpdateTimer;
    float ConflictUpdateTimer;
    float BorderUpdateTimer;
    float AIUpdateTimer;

    // Helper functions
    void UpdateAllegianceFromReputation(const FString& FactionID);
    bool CanClaimTerritory(const FString& FactionID, const FString& TerritoryID) const;
    void TransferTerritory(const FString& TerritoryID, const FString& NewFactionID);
    void CalculateFactionPower(const FString& FactionID);
    FString GenerateFactionID() const;
    FString GenerateConflictID() const;
    void PropagateReputationEffects(const FString& FactionID, int32 Amount);
    bool AreFactionsAllied(const FString& Faction1ID, const FString& Faction2ID) const;
    void UpdateFactionEconomy(const FString& FactionID, float DeltaTime);
};
