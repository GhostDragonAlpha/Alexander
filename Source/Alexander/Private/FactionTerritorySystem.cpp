// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactionTerritorySystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"

UFactionTerritorySystem::UFactionTerritorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second

    bEnableAIFactions = true;
    TerritoryUpdateInterval = 5.0f;
    ConflictUpdateInterval = 2.0f;
    MaxActiveConflicts = 10;
    bEnableDynamicBorders = true;

    TerritoryUpdateTimer = 0.0f;
    ConflictUpdateTimer = 0.0f;
    BorderUpdateTimer = 0.0f;
    AIUpdateTimer = 0.0f;
}

void UFactionTerritorySystem::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("FactionTerritorySystem initialized"));
}

void UFactionTerritorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update timers
    TerritoryUpdateTimer += DeltaTime;
    ConflictUpdateTimer += DeltaTime;
    BorderUpdateTimer += DeltaTime;
    AIUpdateTimer += DeltaTime;

    // Territory updates
    if (TerritoryUpdateTimer >= TerritoryUpdateInterval)
    {
        UpdateTerritoryControl(DeltaTime);
        TerritoryUpdateTimer = 0.0f;
    }

    // Conflict updates
    if (ConflictUpdateTimer >= ConflictUpdateInterval)
    {
        UpdateConflicts(DeltaTime);
        ConflictUpdateTimer = 0.0f;
    }

    // Border tension updates
    if (BorderUpdateTimer >= 10.0f && bEnableDynamicBorders)
    {
        UpdateBorderTensions(DeltaTime);
        BorderUpdateTimer = 0.0f;
    }

    // AI faction behavior
    if (AIUpdateTimer >= 30.0f && bEnableAIFactions)
    {
        SimulateFactionBehavior(DeltaTime);
        AIUpdateTimer = 0.0f;
    }
}

FString UFactionTerritorySystem::CreateFaction(const FString& FactionName, EFactionType Type, EGovernmentType Government)
{
    FFactionData NewFaction;
    NewFaction.FactionID = GenerateFactionID();
    NewFaction.FactionName = FactionName;
    NewFaction.FactionType = Type;
    NewFaction.GovernmentType = Government;
    NewFaction.Power = 100;
    NewFaction.Wealth = 10000;
    NewFaction.Influence = 50;

    // Set faction color based on type
    switch (Type)
    {
        case EFactionType::Military:
            NewFaction.FactionColor = FLinearColor(0.2f, 0.4f, 0.8f);
            break;
        case EFactionType::Corporate:
            NewFaction.FactionColor = FLinearColor(0.8f, 0.6f, 0.2f);
            break;
        case EFactionType::Pirate:
            NewFaction.FactionColor = FLinearColor(0.8f, 0.2f, 0.2f);
            break;
        default:
            NewFaction.FactionColor = FLinearColor::Gray;
            break;
    }

    Factions.Add(NewFaction.FactionID, NewFaction);
    
    UE_LOG(LogTemp, Log, TEXT("Created faction: %s (%s)"), *FactionName, *NewFaction.FactionID);
    return NewFaction.FactionID;
}

FFactionData UFactionTerritorySystem::GetFactionData(const FString& FactionID, bool& bSuccess) const
{
    const FFactionData* Faction = Factions.Find(FactionID);
    bSuccess = (Faction != nullptr);
    return bSuccess ? *Faction : FFactionData();
}

TArray<FFactionData> UFactionTerritorySystem::GetAllFactions() const
{
    TArray<FFactionData> AllFactions;
    for (const auto& Pair : Factions)
    {
        AllFactions.Add(Pair.Value);
    }
    return AllFactions;
}

void UFactionTerritorySystem::SetFactionPower(const FString& FactionID, int32 Power)
{
    FFactionData* Faction = Factions.Find(FactionID);
    if (Faction)
    {
        Faction->Power = FMath::Clamp(Power, 0, 10000);
    }
}

void UFactionTerritorySystem::ModifyFactionWealth(const FString& FactionID, int32 Amount)
{
    FFactionData* Faction = Factions.Find(FactionID);
    if (Faction)
    {
        Faction->Wealth = FMath::Max(0, Faction->Wealth + Amount);
    }
}

TArray<FString> UFactionTerritorySystem::GetFactionsByType(EFactionType Type) const
{
    TArray<FString> MatchingFactions;
    for (const auto& Pair : Factions)
    {
        if (Pair.Value.FactionType == Type)
        {
            MatchingFactions.Add(Pair.Key);
        }
    }
    return MatchingFactions;
}

bool UFactionTerritorySystem::ClaimTerritory(const FString& TerritoryID, const FString& FactionID)
{
    if (!CanClaimTerritory(FactionID, TerritoryID))
    {
        return false;
    }

    FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (!Territory)
    {
        // Create new territory
        FTerritoryData NewTerritory;
        NewTerritory.TerritoryID = TerritoryID;
        NewTerritory.ControllingFaction = FactionID;
        NewTerritory.Status = ETerritoryStatus::Controlled;
        NewTerritory.ControlStrength = 100.0f;
        Territories.Add(TerritoryID, NewTerritory);
    }
    else
    {
        TransferTerritory(TerritoryID, FactionID);
    }

    // Update faction's controlled systems
    FFactionData* Faction = Factions.Find(FactionID);
    if (Faction && !Faction->ControlledSystems.Contains(TerritoryID))
    {
        Faction->ControlledSystems.Add(TerritoryID);
    }

    OnTerritoryChanged.Broadcast(TerritoryID, FactionID);
    UE_LOG(LogTemp, Log, TEXT("Territory %s claimed by faction %s"), *TerritoryID, *FactionID);
    return true;
}

bool UFactionTerritorySystem::ContestedTerritory(const FString& TerritoryID, const FString& AttackingFactionID)
{
    FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (!Territory || Territory->ControllingFaction == AttackingFactionID)
    {
        return false;
    }

    Territory->Status = ETerritoryStatus::Contested;
    Territory->ContestingFactions.Add(AttackingFactionID, 10.0f);

    // Start conflict
    StartConflict(Territory->ControllingFaction, AttackingFactionID, TerritoryID);

    UE_LOG(LogTemp, Log, TEXT("Territory %s contested by faction %s"), *TerritoryID, *AttackingFactionID);
    return true;
}

FTerritoryData UFactionTerritorySystem::GetTerritoryData(const FString& TerritoryID, bool& bSuccess) const
{
    const FTerritoryData* Territory = Territories.Find(TerritoryID);
    bSuccess = (Territory != nullptr);
    return bSuccess ? *Territory : FTerritoryData();
}

TArray<FTerritoryData> UFactionTerritorySystem::GetFactionTerritories(const FString& FactionID) const
{
    TArray<FTerritoryData> FactionTerritories;
    for (const auto& Pair : Territories)
    {
        if (Pair.Value.ControllingFaction == FactionID)
        {
            FactionTerritories.Add(Pair.Value);
        }
    }
    return FactionTerritories;
}

FString UFactionTerritorySystem::GetTerritoryController(const FString& TerritoryID) const
{
    const FTerritoryData* Territory = Territories.Find(TerritoryID);
    return Territory ? Territory->ControllingFaction : FString();
}

void UFactionTerritorySystem::UpdateTerritoryControl(float DeltaTime)
{
    for (auto& Pair : Territories)
    {
        FTerritoryData& Territory = Pair.Value;
        
        if (Territory.Status == ETerritoryStatus::Contested)
        {
            // Process contested territory
            for (auto& ContestPair : Territory.ContestingFactions)
            {
                FFactionData* Faction = Factions.Find(ContestPair.Key);
                if (Faction)
                {
                    float InfluenceGain = Faction->Power * 0.01f * DeltaTime;
                    ContestPair.Value = FMath::Clamp(ContestPair.Value + InfluenceGain, 0.0f, 100.0f);
                }
            }

            // Check if attacker has gained control
            for (const auto& ContestPair : Territory.ContestingFactions)
            {
                if (ContestPair.Value >= 75.0f)
                {
                    TransferTerritory(Territory.TerritoryID, ContestPair.Key);
                    break;
                }
            }
        }
    }
}

void UFactionTerritorySystem::ModifyReputation(const FString& FactionID, int32 Amount)
{
    FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    
    if (!Reputation)
    {
        FFactionReputation NewReputation;
        NewReputation.FactionID = FactionID;
        NewReputation.ReputationPoints = Amount;
        PlayerReputations.Add(FactionID, NewReputation);
        Reputation = &PlayerReputations[FactionID];
    }
    else
    {
        Reputation->ReputationPoints = FMath::Clamp(Reputation->ReputationPoints + Amount, -100, 100);
    }

    UpdateAllegianceFromReputation(FactionID);
    PropagateReputationEffects(FactionID, Amount);

    OnReputationChanged.Broadcast(FactionID, Reputation->ReputationPoints);
    UE_LOG(LogTemp, Log, TEXT("Reputation with faction %s changed by %d (total: %d)"), *FactionID, Amount, Reputation->ReputationPoints);
}

int32 UFactionTerritorySystem::GetReputation(const FString& FactionID) const
{
    const FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    return Reputation ? Reputation->ReputationPoints : 0;
}

EFactionAllegiance UFactionTerritorySystem::GetAllegiance(const FString& FactionID) const
{
    const FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    return Reputation ? Reputation->Allegiance : EFactionAllegiance::Neutral;
}

TArray<FFactionReputation> UFactionTerritorySystem::GetAllReputations() const
{
    TArray<FFactionReputation> Reputations;
    for (const auto& Pair : PlayerReputations)
    {
        Reputations.Add(Pair.Value);
    }
    return Reputations;
}

bool UFactionTerritorySystem::HasAccessPermission(const FString& FactionID) const
{
    const FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    return Reputation ? Reputation->bHasAccessPermission : false;
}

void UFactionTerritorySystem::GrantAccessPermission(const FString& FactionID, bool bGrant)
{
    FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    if (Reputation)
    {
        Reputation->bHasAccessPermission = bGrant;
    }
}

void UFactionTerritorySystem::SetFactionRelation(const FString& Faction1ID, const FString& Faction2ID, EFactionAllegiance Relation)
{
if (!FactionRelations.Contains(Faction1ID))
    {
        FFactionRelations NewRelations;
        FactionRelations.Add(Faction1ID, NewRelations);
    }
    FactionRelations[Faction1ID].Relations.Add(Faction2ID, Relation);

    // Reciprocal relation
    if (!FactionRelations.Contains(Faction2ID))
    {
        FFactionRelations NewRelations;
        FactionRelations.Add(Faction2ID, NewRelations);
    }
    FactionRelations[Faction2ID].Relations.Add(Faction1ID, Relation);
}

EFactionAllegiance UFactionTerritorySystem::GetFactionRelation(const FString& Faction1ID, const FString& Faction2ID) const
{
    if (const FFactionRelations* Relations = FactionRelations.Find(Faction1ID))
    {
        if (const EFactionAllegiance* Relation = Relations->Relations.Find(Faction2ID))
        {
            return *Relation;
        }
    }
    return EFactionAllegiance::Neutral;
}

void UFactionTerritorySystem::FormAlliance(const FString& Faction1ID, const FString& Faction2ID)
{
    SetFactionRelation(Faction1ID, Faction2ID, EFactionAllegiance::Allied);
    
    FFactionData* Faction1 = Factions.Find(Faction1ID);
    FFactionData* Faction2 = Factions.Find(Faction2ID);
    
    if (Faction1) Faction1->AlliedFactions.AddUnique(Faction2ID);
    if (Faction2) Faction2->AlliedFactions.AddUnique(Faction1ID);

    UE_LOG(LogTemp, Log, TEXT("Alliance formed between %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionTerritorySystem::BreakAlliance(const FString& Faction1ID, const FString& Faction2ID)
{
    SetFactionRelation(Faction1ID, Faction2ID, EFactionAllegiance::Neutral);
    
    FFactionData* Faction1 = Factions.Find(Faction1ID);
    FFactionData* Faction2 = Factions.Find(Faction2ID);
    
    if (Faction1) Faction1->AlliedFactions.Remove(Faction2ID);
    if (Faction2) Faction2->AlliedFactions.Remove(Faction1ID);

    UE_LOG(LogTemp, Log, TEXT("Alliance broken between %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionTerritorySystem::DeclareWar(const FString& Faction1ID, const FString& Faction2ID)
{
    SetFactionRelation(Faction1ID, Faction2ID, EFactionAllegiance::AtWar);
    
    FFactionData* Faction1 = Factions.Find(Faction1ID);
    FFactionData* Faction2 = Factions.Find(Faction2ID);
    
    if (Faction1) Faction1->HostileFactions.AddUnique(Faction2ID);
    if (Faction2) Faction2->HostileFactions.AddUnique(Faction1ID);

    UE_LOG(LogTemp, Warning, TEXT("War declared between %s and %s"), *Faction1ID, *Faction2ID);
}

void UFactionTerritorySystem::MakePeace(const FString& Faction1ID, const FString& Faction2ID)
{
    SetFactionRelation(Faction1ID, Faction2ID, EFactionAllegiance::Neutral);
    
    FFactionData* Faction1 = Factions.Find(Faction1ID);
    FFactionData* Faction2 = Factions.Find(Faction2ID);
    
    if (Faction1) Faction1->HostileFactions.Remove(Faction2ID);
    if (Faction2) Faction2->HostileFactions.Remove(Faction1ID);

    UE_LOG(LogTemp, Log, TEXT("Peace made between %s and %s"), *Faction1ID, *Faction2ID);
}

FString UFactionTerritorySystem::StartConflict(const FString& Faction1ID, const FString& Faction2ID, const FString& TerritoryID)
{
    if (ActiveConflicts.Num() >= MaxActiveConflicts)
    {
        return FString();
    }

    FFactionConflict Conflict;
    Conflict.ConflictID = GenerateConflictID();
    Conflict.Faction1ID = Faction1ID;
    Conflict.Faction2ID = Faction2ID;
    Conflict.DisputedTerritoryID = TerritoryID;
    Conflict.ConflictIntensity = 50.0f;
    Conflict.StartTime = FDateTime::Now();
    Conflict.bIsActive = true;

    ActiveConflicts.Add(Conflict);
    OnConflictStarted.Broadcast(Faction1ID, Faction2ID, TerritoryID);

    UE_LOG(LogTemp, Warning, TEXT("Conflict started: %s vs %s over %s"), *Faction1ID, *Faction2ID, *TerritoryID);
    return Conflict.ConflictID;
}

void UFactionTerritorySystem::EndConflict(const FString& ConflictID, const FString& WinnerFactionID)
{
    for (int32 i = 0; i < ActiveConflicts.Num(); ++i)
    {
        if (ActiveConflicts[i].ConflictID == ConflictID)
        {
            ActiveConflicts[i].bIsActive = false;
            
            if (!WinnerFactionID.IsEmpty() && !ActiveConflicts[i].DisputedTerritoryID.IsEmpty())
            {
                ClaimTerritory(ActiveConflicts[i].DisputedTerritoryID, WinnerFactionID);
            }
            
            ActiveConflicts.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Conflict %s ended. Winner: %s"), *ConflictID, *WinnerFactionID);
            break;
        }
    }
}

TArray<FFactionConflict> UFactionTerritorySystem::GetActiveConflicts() const
{
    return ActiveConflicts;
}

bool UFactionTerritorySystem::IsInConflict(const FString& Faction1ID, const FString& Faction2ID) const
{
    for (const FFactionConflict& Conflict : ActiveConflicts)
    {
        if ((Conflict.Faction1ID == Faction1ID && Conflict.Faction2ID == Faction2ID) ||
            (Conflict.Faction1ID == Faction2ID && Conflict.Faction2ID == Faction1ID))
        {
            return true;
        }
    }
    return false;
}

void UFactionTerritorySystem::UpdateConflicts(float DeltaTime)
{
    for (FFactionConflict& Conflict : ActiveConflicts)
    {
        // Intensity naturally decreases over time
        Conflict.ConflictIntensity = FMath::Max(0.0f, Conflict.ConflictIntensity - DeltaTime * 0.5f);
        
        // End conflict if intensity too low
        if (Conflict.ConflictIntensity < 10.0f)
        {
            EndConflict(Conflict.ConflictID, FString());
        }
    }
}

TArray<FFactionMission> UFactionTerritorySystem::GetAvailableMissions(const FString& FactionID) const
{
    TArray<FFactionMission> Missions;
    for (const FFactionMission& Mission : AvailableMissions)
    {
        if (Mission.FactionID == FactionID)
        {
            int32 PlayerRep = GetReputation(FactionID);
            if (PlayerRep >= Mission.RequiredReputation)
            {
                Missions.Add(Mission);
            }
        }
    }
    return Missions;
}

bool UFactionTerritorySystem::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < AvailableMissions.Num(); ++i)
    {
        if (AvailableMissions[i].MissionID == MissionID)
        {
            FFactionMission& Mission = AvailableMissions[i];
            ModifyReputation(Mission.FactionID, Mission.ReputationReward);
            
            FFactionReputation* Rep = PlayerReputations.Find(Mission.FactionID);
            if (Rep)
            {
                Rep->MissionsCompleted++;
            }
            
            if (!Mission.bIsRepeatable)
            {
                AvailableMissions.RemoveAt(i);
            }
            
            return true;
        }
    }
    return false;
}

void UFactionTerritorySystem::GenerateFactionMissions(const FString& FactionID, int32 Count)
{
    // Generate procedural missions for faction
    for (int32 i = 0; i < Count; ++i)
    {
        FFactionMission Mission;
        Mission.MissionID = FString::Printf(TEXT("MISSION_%lld_%d"), FDateTime::Now().GetTicks(), i);
        Mission.FactionID = FactionID;
        Mission.MissionName = TEXT("Generated Mission ") + FString::FromInt(i);
        Mission.ReputationReward = FMath::RandRange(5, 20);
        Mission.CreditReward = FMath::RandRange(500, 5000);
        AvailableMissions.Add(Mission);
    }
}

void UFactionTerritorySystem::CreateBorderZone(const FString& Faction1ID, const FString& Faction2ID)
{
    FBorderZone Zone;
    Zone.ZoneID = FString::Printf(TEXT("BORDER_%s_%s"), *Faction1ID, *Faction2ID);
    Zone.Faction1ID = Faction1ID;
    Zone.Faction2ID = Faction2ID;
    Zone.TensionLevel = 25.0f;
    BorderZones.Add(Zone);
}

TArray<FBorderZone> UFactionTerritorySystem::GetBorderZones() const
{
    return BorderZones;
}

void UFactionTerritorySystem::UpdateBorderTensions(float DeltaTime)
{
    for (FBorderZone& Zone : BorderZones)
    {
        EFactionAllegiance Relation = GetFactionRelation(Zone.Faction1ID, Zone.Faction2ID);
        
        if (Relation == EFactionAllegiance::AtWar)
        {
            Zone.TensionLevel = FMath::Min(100.0f, Zone.TensionLevel + 10.0f * DeltaTime);
        }
        else if (Relation == EFactionAllegiance::Allied)
        {
            Zone.TensionLevel = FMath::Max(0.0f, Zone.TensionLevel - 5.0f * DeltaTime);
        }
        
        Zone.bIsHotZone = (Zone.TensionLevel > 75.0f);
    }
}

void UFactionTerritorySystem::SpreadInfluence(const FString& FactionID, const FString& TerritoryID, float Amount)
{
    FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (Territory)
    {
        Territory->ContestingFactions.FindOrAdd(FactionID) += Amount;
    }
}

float UFactionTerritorySystem::GetFactionInfluence(const FString& FactionID, const FString& TerritoryID) const
{
    const FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (Territory && Territory->ContestingFactions.Contains(FactionID))
    {
        return Territory->ContestingFactions[FactionID];
    }
    return 0.0f;
}

FString UFactionTerritorySystem::GetDominantFaction(const FString& TerritoryID) const
{
    const FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (!Territory)
    {
        return FString();
    }
    
    return Territory->ControllingFaction;
}

void UFactionTerritorySystem::SimulateFactionBehavior(float DeltaTime)
{
    // AI factions make decisions
    for (const auto& Pair : Factions)
    {
        if (!Pair.Value.bIsPlayable)
        {
            ProcessFactionExpansion(Pair.Key);
            ProcessFactionDiplomacy(Pair.Key);
            UpdateFactionEconomy(Pair.Key, DeltaTime);
        }
    }
}

void UFactionTerritorySystem::ProcessFactionExpansion(const FString& FactionID)
{
    // AI expansion logic would go here
}

void UFactionTerritorySystem::ProcessFactionDiplomacy(const FString& FactionID)
{
    // AI diplomacy logic would go here
}

void UFactionTerritorySystem::UpdateAllegianceFromReputation(const FString& FactionID)
{
    FFactionReputation* Reputation = PlayerReputations.Find(FactionID);
    if (!Reputation)
    {
        return;
    }

    EFactionAllegiance OldAllegiance = Reputation->Allegiance;
    
    if (Reputation->ReputationPoints >= 75)
    {
        Reputation->Allegiance = EFactionAllegiance::Allied;
    }
    else if (Reputation->ReputationPoints >= 25)
    {
        Reputation->Allegiance = EFactionAllegiance::Friendly;
    }
    else if (Reputation->ReputationPoints >= -25)
    {
        Reputation->Allegiance = EFactionAllegiance::Neutral;
    }
    else if (Reputation->ReputationPoints >= -75)
    {
        Reputation->Allegiance = EFactionAllegiance::Unfriendly;
    }
    else
    {
        Reputation->Allegiance = EFactionAllegiance::Hostile;
    }

    if (OldAllegiance != Reputation->Allegiance)
    {
        OnAllegianceChanged.Broadcast(Reputation->Allegiance);
    }
}

bool UFactionTerritorySystem::CanClaimTerritory(const FString& FactionID, const FString& TerritoryID) const
{
    const FFactionData* Faction = Factions.Find(FactionID);
    return (Faction && Faction->Power >= 50);
}

void UFactionTerritorySystem::TransferTerritory(const FString& TerritoryID, const FString& NewFactionID)
{
    FTerritoryData* Territory = Territories.Find(TerritoryID);
    if (!Territory)
    {
        return;
    }

    FString OldFactionID = Territory->ControllingFaction;
    Territory->ControllingFaction = NewFactionID;
    Territory->Status = ETerritoryStatus::Controlled;
    Territory->ControlStrength = 100.0f;
    Territory->ContestingFactions.Empty();

    // Update faction controlled systems
    FFactionData* OldFaction = Factions.Find(OldFactionID);
    if (OldFaction)
    {
        OldFaction->ControlledSystems.Remove(TerritoryID);
    }

    FFactionData* NewFaction = Factions.Find(NewFactionID);
    if (NewFaction)
    {
        NewFaction->ControlledSystems.AddUnique(TerritoryID);
    }
}

void UFactionTerritorySystem::CalculateFactionPower(const FString& FactionID)
{
    FFactionData* Faction = Factions.Find(FactionID);
    if (!Faction)
    {
        return;
    }

    int32 TerritoryPower = Faction->ControlledSystems.Num() * 10;
    int32 WealthPower = Faction->Wealth / 1000;
    Faction->Power = 100 + TerritoryPower + WealthPower;
}

FString UFactionTerritorySystem::GenerateFactionID() const
{
    return FString::Printf(TEXT("FACTION_%lld"), FDateTime::Now().GetTicks());
}

FString UFactionTerritorySystem::GenerateConflictID() const
{
    return FString::Printf(TEXT("CONFLICT_%lld"), FDateTime::Now().GetTicks());
}

void UFactionTerritorySystem::PropagateReputationEffects(const FString& FactionID, int32 Amount)
{
    // Reputation changes affect allied/hostile factions
    const FFactionData* Faction = Factions.Find(FactionID);
    if (!Faction)
    {
        return;
    }

    for (const FString& AlliedID : Faction->AlliedFactions)
    {
        ModifyReputation(AlliedID, Amount / 4);
    }

    for (const FString& HostileID : Faction->HostileFactions)
    {
        ModifyReputation(HostileID, -Amount / 2);
    }
}

bool UFactionTerritorySystem::AreFactionsAllied(const FString& Faction1ID, const FString& Faction2ID) const
{
    return GetFactionRelation(Faction1ID, Faction2ID) == EFactionAllegiance::Allied;
}

void UFactionTerritorySystem::UpdateFactionEconomy(const FString& FactionID, float DeltaTime)
{
    FFactionData* Faction = Factions.Find(FactionID);
    if (!Faction)
    {
        return;
    }

    // Generate income from territories
    int32 Income = Faction->ControlledSystems.Num() * 100;
    Faction->Wealth += Income;
}
