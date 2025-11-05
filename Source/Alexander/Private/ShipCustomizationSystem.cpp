// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipCustomizationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

UShipCustomizationSystem::UShipCustomizationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;

    MaxInventorySize = 100;
    bAutoCalculatePerformance = true;
    bEnforceRequirements = true;
    bPerformanceDirty = true;
}

void UShipCustomizationSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeSlots();
    
    // Set default colors
    CurrentLoadout.PrimaryColor = FLinearColor::White;
    CurrentLoadout.SecondaryColor = FLinearColor(0.2f, 0.2f, 0.2f);
    CurrentLoadout.PaintScheme = EPaintScheme::Solid;
    CurrentLoadout.LoadoutID = GenerateLoadoutID();
    CurrentLoadout.LoadoutName = TEXT("Default Loadout");

    UE_LOG(LogTemp, Log, TEXT("ShipCustomizationSystem initialized"));
}

void UShipCustomizationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UShipCustomizationSystem::InitializeSlots()
{
    ComponentSlots.Empty();

    // Create slots based on ship template
    for (const auto& Pair : CurrentShipTemplate.ComponentSlots)
    {
        FComponentSlot Slot;
        Slot.SlotID = Pair.Key;
        Slot.SlotName = Pair.Key;
        Slot.AcceptedType = Pair.Value;
        Slot.bIsRequired = (Pair.Value == EShipComponentType::Hull || 
                           Pair.Value == EShipComponentType::Engine ||
                           Pair.Value == EShipComponentType::PowerPlant);
        Slot.bIsOccupied = false;

        ComponentSlots.Add(Pair.Key, Slot);
    }
}

bool UShipCustomizationSystem::InstallComponent(const FString& ComponentID, const FString& SlotID)
{
    // Find component in inventory
    FShipComponentData* Component = FindComponentInInventory(ComponentID);
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Component not found in inventory: %s"), *ComponentID);
        return false;
    }

    // Check if slot exists
    FComponentSlot* Slot = ComponentSlots.Find(SlotID);
    if (!Slot)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot: %s"), *SlotID);
        return false;
    }

    // Validate compatibility
    if (!CanInstallComponent(ComponentID, SlotID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Component %s cannot be installed in slot %s"), *ComponentID, *SlotID);
        return false;
    }

    // Remove existing component if slot is occupied
    if (Slot->bIsOccupied)
    {
        RemoveComponent(SlotID);
    }

    // Install component
    Component->bIsInstalled = true;
    Component->SlotID = SlotID;
    Slot->bIsOccupied = true;
    Slot->InstalledComponentID = ComponentID;

    CurrentLoadout.Components.Add(SlotID, *Component);

    bPerformanceDirty = true;
    if (bAutoCalculatePerformance)
    {
        RecalculatePerformance();
    }

    OnComponentInstalled.Broadcast(ComponentID, SlotID);
    ApplyComponentVisuals(*Component, SlotID);

    UE_LOG(LogTemp, Log, TEXT("Installed component %s in slot %s"), *ComponentID, *SlotID);
    return true;
}

bool UShipCustomizationSystem::RemoveComponent(const FString& SlotID)
{
    FComponentSlot* Slot = ComponentSlots.Find(SlotID);
    if (!Slot || !Slot->bIsOccupied)
    {
        return false;
    }

    FString ComponentID = Slot->InstalledComponentID;
    
    // Find and update component
    FShipComponentData* Component = FindComponentInInventory(ComponentID);
    if (Component)
    {
        Component->bIsInstalled = false;
        Component->SlotID.Empty();
    }

    // Remove from loadout
    CurrentLoadout.Components.Remove(SlotID);

    // Update slot
    Slot->bIsOccupied = false;
    Slot->InstalledComponentID.Empty();

    bPerformanceDirty = true;
    if (bAutoCalculatePerformance)
    {
        RecalculatePerformance();
    }

    OnComponentRemoved.Broadcast(ComponentID, SlotID);
    RemoveComponentVisuals(SlotID);

    UE_LOG(LogTemp, Log, TEXT("Removed component %s from slot %s"), *ComponentID, *SlotID);
    return true;
}

bool UShipCustomizationSystem::SwapComponents(const FString& SlotID1, const FString& SlotID2)
{
    FComponentSlot* Slot1 = ComponentSlots.Find(SlotID1);
    FComponentSlot* Slot2 = ComponentSlots.Find(SlotID2);

    if (!Slot1 || !Slot2 || !Slot1->bIsOccupied || !Slot2->bIsOccupied)
    {
        return false;
    }

    FString ComponentID1 = Slot1->InstalledComponentID;
    FString ComponentID2 = Slot2->InstalledComponentID;

    // Validate swap compatibility
    FShipComponentData* Comp1 = FindComponentInInventory(ComponentID1);
    FShipComponentData* Comp2 = FindComponentInInventory(ComponentID2);

    if (!Comp1 || !Comp2)
    {
        return false;
    }

    if (Comp1->ComponentType != Slot2->AcceptedType || Comp2->ComponentType != Slot1->AcceptedType)
    {
        return false;
    }

    // Perform swap
    RemoveComponent(SlotID1);
    RemoveComponent(SlotID2);
    InstallComponent(ComponentID1, SlotID2);
    InstallComponent(ComponentID2, SlotID1);

    return true;
}

FShipComponentData UShipCustomizationSystem::GetInstalledComponent(const FString& SlotID, bool& bSuccess) const
{
    const FShipComponentData* Component = CurrentLoadout.Components.Find(SlotID);
    bSuccess = (Component != nullptr);
    return bSuccess ? *Component : FShipComponentData();
}

TArray<FShipComponentData> UShipCustomizationSystem::GetAllInstalledComponents() const
{
    TArray<FShipComponentData> Components;
    for (const auto& Pair : CurrentLoadout.Components)
    {
        Components.Add(Pair.Value);
    }
    return Components;
}

bool UShipCustomizationSystem::CanInstallComponent(const FString& ComponentID, const FString& SlotID) const
{
    const FShipComponentData* Component = FindComponentInInventory(ComponentID);
    const FComponentSlot* Slot = ComponentSlots.Find(SlotID);

    if (!Component || !Slot)
    {
        return false;
    }

    // Check type compatibility
    if (Component->ComponentType != Slot->AcceptedType)
    {
        return false;
    }

    // Check power requirements
    if (bEnforceRequirements)
    {
        float TotalPower = GetTotalPowerConsumption() + Component->PowerRequirement;
        if (TotalPower > CurrentShipTemplate.MaxPowerOutput)
        {
            return false;
        }

        // Check mass requirements
        float TotalMass = GetTotalMass() + Component->Mass;
        if (TotalMass > CurrentShipTemplate.MaxMass)
        {
            return false;
        }
    }

    return true;
}

FString UShipCustomizationSystem::FindCompatibleSlot(const FString& ComponentID) const
{
    const FShipComponentData* Component = FindComponentInInventory(ComponentID);
    if (!Component)
    {
        return FString();
    }

    // Find first available slot of matching type
    for (const auto& Pair : ComponentSlots)
    {
        if (Pair.Value.AcceptedType == Component->ComponentType && !Pair.Value.bIsOccupied)
        {
            if (CanInstallComponent(ComponentID, Pair.Key))
            {
                return Pair.Key;
            }
        }
    }

    return FString();
}

void UShipCustomizationSystem::AddComponentToInventory(const FShipComponentData& Component)
{
    if (ComponentInventory.Num() >= MaxInventorySize)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory full!"));
        return;
    }

    ComponentInventory.Add(Component);
    UE_LOG(LogTemp, Log, TEXT("Added component to inventory: %s"), *Component.ComponentName);
}

bool UShipCustomizationSystem::RemoveComponentFromInventory(const FString& ComponentID)
{
    for (int32 i = 0; i < ComponentInventory.Num(); ++i)
    {
        if (ComponentInventory[i].ComponentID == ComponentID)
        {
            ComponentInventory.RemoveAt(i);
            return true;
        }
    }
    return false;
}

TArray<FShipComponentData> UShipCustomizationSystem::GetInventoryComponents() const
{
    return ComponentInventory;
}

TArray<FShipComponentData> UShipCustomizationSystem::GetInventoryComponentsByType(EShipComponentType Type) const
{
    TArray<FShipComponentData> FilteredComponents;
    for (const FShipComponentData& Component : ComponentInventory)
    {
        if (Component.ComponentType == Type)
        {
            FilteredComponents.Add(Component);
        }
    }
    return FilteredComponents;
}

int32 UShipCustomizationSystem::GetInventoryCount() const
{
    return ComponentInventory.Num();
}

FString UShipCustomizationSystem::SaveLoadout(const FString& LoadoutName)
{
    FShipLoadout NewLoadout = CurrentLoadout;
    NewLoadout.LoadoutID = GenerateLoadoutID();
    NewLoadout.LoadoutName = LoadoutName.IsEmpty() ? TEXT("Custom Loadout") : LoadoutName;

    SavedLoadouts.Add(NewLoadout.LoadoutID, NewLoadout);

    UE_LOG(LogTemp, Log, TEXT("Saved loadout: %s"), *LoadoutName);
    return NewLoadout.LoadoutID;
}

bool UShipCustomizationSystem::LoadLoadout(const FString& LoadoutID)
{
    const FShipLoadout* Loadout = SavedLoadouts.Find(LoadoutID);
    if (!Loadout)
    {
        return false;
    }

    // Remove all current components
    TArray<FString> SlotIDs;
    ComponentSlots.GetKeys(SlotIDs);
    for (const FString& SlotID : SlotIDs)
    {
        RemoveComponent(SlotID);
    }

    // Install loadout components
    CurrentLoadout = *Loadout;
    for (const auto& Pair : Loadout->Components)
    {
        InstallComponent(Pair.Value.ComponentID, Pair.Key);
    }

    OnLoadoutChanged.Broadcast(LoadoutID);
    UpdateShipVisuals();

    UE_LOG(LogTemp, Log, TEXT("Loaded loadout: %s"), *Loadout->LoadoutName);
    return true;
}

bool UShipCustomizationSystem::DeleteLoadout(const FString& LoadoutID)
{
    return SavedLoadouts.Remove(LoadoutID) > 0;
}

TArray<FShipLoadout> UShipCustomizationSystem::GetSavedLoadouts() const
{
    TArray<FShipLoadout> Loadouts;
    for (const auto& Pair : SavedLoadouts)
    {
        Loadouts.Add(Pair.Value);
    }
    return Loadouts;
}

FShipLoadout UShipCustomizationSystem::GetCurrentLoadout() const
{
    return CurrentLoadout;
}

void UShipCustomizationSystem::SetPrimaryColor(FLinearColor Color)
{
    CurrentLoadout.PrimaryColor = Color;
    UpdateShipVisuals();
}

void UShipCustomizationSystem::SetSecondaryColor(FLinearColor Color)
{
    CurrentLoadout.SecondaryColor = Color;
    UpdateShipVisuals();
}

void UShipCustomizationSystem::SetPaintScheme(EPaintScheme Scheme)
{
    CurrentLoadout.PaintScheme = Scheme;
    OnPaintSchemeChanged.Broadcast(Scheme);
    UpdateShipVisuals();
}

void UShipCustomizationSystem::ApplyDecal(const FString& DecalID)
{
    CurrentLoadout.DecalID = DecalID;
    UpdateShipVisuals();
}

void UShipCustomizationSystem::ApplyMaterialPreset(const FString& PresetID)
{
    // Preset application would be implemented here
    UE_LOG(LogTemp, Log, TEXT("Applied material preset: %s"), *PresetID);
}

FLinearColor UShipCustomizationSystem::GetPrimaryColor() const
{
    return CurrentLoadout.PrimaryColor;
}

FLinearColor UShipCustomizationSystem::GetSecondaryColor() const
{
    return CurrentLoadout.SecondaryColor;
}

EPaintScheme UShipCustomizationSystem::GetPaintScheme() const
{
    return CurrentLoadout.PaintScheme;
}

FShipPerformanceStats UShipCustomizationSystem::CalculatePerformance() const
{
    if (!bPerformanceDirty)
    {
        return CachedPerformance;
    }

    FShipPerformanceStats Stats;

    // Base stats from ship template
    Stats.MaxSpeed = 1000.0f;
    Stats.Acceleration = 100.0f;
    Stats.MaxShield = 0.0f;
    Stats.ShieldRegenRate = 0.0f;
    Stats.Armor = 100.0f;
    Stats.PowerOutput = CurrentShipTemplate.MaxPowerOutput;
    Stats.CargoCapacity = 0.0f;
    Stats.FuelCapacity = 0.0f;
    Stats.SensorRange = 1000.0f;

    // Apply component bonuses
    for (const auto& Pair : CurrentLoadout.Components)
    {
        const FShipComponentData& Component = Pair.Value;

        // Add component-specific bonuses
        if (Component.Stats.Contains(TEXT("Speed")))
        {
            Stats.MaxSpeed += Component.Stats[TEXT("Speed")];
        }
        if (Component.Stats.Contains(TEXT("Shield")))
        {
            Stats.MaxShield += Component.Stats[TEXT("Shield")];
        }
        if (Component.Stats.Contains(TEXT("ShieldRegen")))
        {
            Stats.ShieldRegenRate += Component.Stats[TEXT("ShieldRegen")];
        }
        if (Component.Stats.Contains(TEXT("Armor")))
        {
            Stats.Armor += Component.Stats[TEXT("Armor")];
        }
        if (Component.Stats.Contains(TEXT("Cargo")))
        {
            Stats.CargoCapacity += Component.Stats[TEXT("Cargo")];
        }
        if (Component.Stats.Contains(TEXT("Fuel")))
        {
            Stats.FuelCapacity += Component.Stats[TEXT("Fuel")];
        }
        if (Component.Stats.Contains(TEXT("Sensor")))
        {
            Stats.SensorRange += Component.Stats[TEXT("Sensor")];
        }

        Stats.PowerConsumption += Component.PowerRequirement;
        Stats.TotalMass += Component.Mass;
    }

    // Apply rarity multipliers
    float RarityBonus = 1.0f + (CalculateStatBonus(TEXT("Rarity")) * 0.1f);
    Stats.MaxSpeed *= RarityBonus;
    Stats.MaxShield *= RarityBonus;

    return Stats;
}

float UShipCustomizationSystem::GetTotalPowerConsumption() const
{
    float TotalPower = 0.0f;
    for (const auto& Pair : CurrentLoadout.Components)
    {
        TotalPower += Pair.Value.PowerRequirement;
    }
    return TotalPower;
}

float UShipCustomizationSystem::GetTotalMass() const
{
    float TotalMass = 0.0f;
    for (const auto& Pair : CurrentLoadout.Components)
    {
        TotalMass += Pair.Value.Mass;
    }
    return TotalMass;
}

bool UShipCustomizationSystem::IsLoadoutValid() const
{
    // Check required slots
    for (const auto& Pair : ComponentSlots)
    {
        if (Pair.Value.bIsRequired && !Pair.Value.bIsOccupied)
        {
            return false;
        }
    }

    // Check power and mass limits
    if (GetTotalPowerConsumption() > CurrentShipTemplate.MaxPowerOutput)
    {
        return false;
    }

    if (GetTotalMass() > CurrentShipTemplate.MaxMass)
    {
        return false;
    }

    return true;
}

TArray<FString> UShipCustomizationSystem::GetLoadoutWarnings() const
{
    TArray<FString> Warnings;

    // Check required components
    for (const auto& Pair : ComponentSlots)
    {
        if (Pair.Value.bIsRequired && !Pair.Value.bIsOccupied)
        {
            Warnings.Add(FString::Printf(TEXT("Missing required component: %s"), *Pair.Value.SlotName));
        }
    }

    // Check power
    float PowerUsage = GetTotalPowerConsumption();
    if (PowerUsage > CurrentShipTemplate.MaxPowerOutput)
    {
        Warnings.Add(FString::Printf(TEXT("Power overload: %.0f / %.0f"), PowerUsage, CurrentShipTemplate.MaxPowerOutput));
    }

    // Check mass
    float Mass = GetTotalMass();
    if (Mass > CurrentShipTemplate.MaxMass)
    {
        Warnings.Add(FString::Printf(TEXT("Overweight: %.0f / %.0f kg"), Mass, CurrentShipTemplate.MaxMass));
    }

    return Warnings;
}

void UShipCustomizationSystem::SetShipTemplate(const FShipTemplate& Template)
{
    CurrentShipTemplate = Template;
    InitializeSlots();
}

FShipTemplate UShipCustomizationSystem::GetShipTemplate() const
{
    return CurrentShipTemplate;
}

TArray<FComponentSlot> UShipCustomizationSystem::GetComponentSlots() const
{
    TArray<FComponentSlot> Slots;
    for (const auto& Pair : ComponentSlots)
    {
        Slots.Add(Pair.Value);
    }
    return Slots;
}

void UShipCustomizationSystem::UpdateShipVisuals()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Find all static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* Mesh : MeshComponents)
    {
        ApplyColorToMesh(Mesh);
    }

    UE_LOG(LogTemp, Log, TEXT("Updated ship visuals"));
}

void UShipCustomizationSystem::ApplyColorToMesh(UStaticMeshComponent* Mesh)
{
    if (!Mesh)
    {
        return;
    }

    // Create dynamic material instance
    for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
    {
        UMaterialInterface* Material = Mesh->GetMaterial(i);
        if (Material)
        {
            UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynMaterial)
            {
                DynMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), CurrentLoadout.PrimaryColor);
                DynMaterial->SetVectorParameterValue(TEXT("SecondaryColor"), CurrentLoadout.SecondaryColor);
                Mesh->SetMaterial(i, DynMaterial);
            }
        }
    }
}

void UShipCustomizationSystem::SavePreset(const FString& PresetName, const FString& Description)
{
    FCustomizationPreset Preset;
    Preset.PresetID = GenerateLoadoutID();
    Preset.PresetName = PresetName;
    Preset.Description = Description;
    Preset.Loadout = CurrentLoadout;
    Preset.bIsDefault = false;

    Presets.Add(Preset);
    UE_LOG(LogTemp, Log, TEXT("Saved preset: %s"), *PresetName);
}

bool UShipCustomizationSystem::LoadPreset(const FString& PresetID)
{
    for (const FCustomizationPreset& Preset : Presets)
    {
        if (Preset.PresetID == PresetID)
        {
            CurrentLoadout = Preset.Loadout;
            UpdateShipVisuals();
            return true;
        }
    }
    return false;
}

TArray<FCustomizationPreset> UShipCustomizationSystem::GetAvailablePresets() const
{
    return Presets;
}

FShipPerformanceStats UShipCustomizationSystem::CompareLoadouts(const FString& LoadoutID1, const FString& LoadoutID2) const
{
    // Return difference between two loadouts
    FShipPerformanceStats DiffStats;
    // Implementation would calculate differences
    return DiffStats;
}

float UShipCustomizationSystem::GetComponentStatValue(const FString& ComponentID, const FString& StatName) const
{
    const FShipComponentData* Component = FindComponentInInventory(ComponentID);
    if (Component && Component->Stats.Contains(StatName))
    {
        return Component->Stats[StatName];
    }
    return 0.0f;
}

int32 UShipCustomizationSystem::GetLoadoutCost() const
{
    int32 TotalCost = CurrentShipTemplate.BaseCost;
    for (const auto& Pair : CurrentLoadout.Components)
    {
        TotalCost += Pair.Value.Cost;
    }
    return TotalCost;
}

void UShipCustomizationSystem::RecalculatePerformance()
{
    CachedPerformance = CalculatePerformance();
    bPerformanceDirty = false;
}

FShipComponentData* UShipCustomizationSystem::FindComponentInInventory(const FString& ComponentID)
{
    for (FShipComponentData& Component : ComponentInventory)
    {
        if (Component.ComponentID == ComponentID)
        {
            return &Component;
        }
    }
    return nullptr;
}

const FShipComponentData* UShipCustomizationSystem::FindComponentInInventory(const FString& ComponentID) const
{
    for (const FShipComponentData& Component : ComponentInventory)
    {
        if (Component.ComponentID == ComponentID)
        {
            return &Component;
        }
    }
    return nullptr;
}

bool UShipCustomizationSystem::ValidateComponentRequirements(const FShipComponentData& Component) const
{
    // Check if component level matches ship requirements
    // Check if manufacturer is compatible
    // Additional validation logic
    return true;
}

float UShipCustomizationSystem::CalculateStatBonus(const FString& StatName) const
{
    float Bonus = 0.0f;
    for (const auto& Pair : CurrentLoadout.Components)
    {
        if (Pair.Value.Stats.Contains(StatName))
        {
            Bonus += Pair.Value.Stats[StatName];
        }
    }
    return Bonus;
}

void UShipCustomizationSystem::ApplyComponentVisuals(const FShipComponentData& Component, const FString& SlotID)
{
    // Apply visual effects for installed component
    // This would spawn visual representations, attach meshes, etc.
    UE_LOG(LogTemp, Log, TEXT("Applied visuals for component in slot %s"), *SlotID);
}

void UShipCustomizationSystem::RemoveComponentVisuals(const FString& SlotID)
{
    // Remove visual effects for component
    UE_LOG(LogTemp, Log, TEXT("Removed visuals from slot %s"), *SlotID);
}

FString UShipCustomizationSystem::GenerateLoadoutID() const
{
    return FString::Printf(TEXT("LOADOUT_%lld"), FDateTime::Now().GetTicks());
}
