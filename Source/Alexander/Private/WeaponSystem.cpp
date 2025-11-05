// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"

UWeaponSystem::UWeaponSystem()
{
PrimaryComponentTick.bCanEverTick = true;
PrimaryComponentTick.TickInterval = 0.016f; // ~60 Hz for responsive combat

// Initialize energy
CurrentEnergy = MaxEnergy;

// Initialize targeting
CurrentTarget = FTargetingInfo();
}

void UWeaponSystem::BeginPlay()
{
Super::BeginPlay();

// Initialize hardpoints
for (FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
Hardpoint.CurrentAmmo = Hardpoint.InstalledWeapon.AmmoCapacity;
Hardpoint.CurrentHeat = 0.0f;
Hardpoint.bIsReloading = false;
Hardpoint.ReloadProgress = 0.0f;
}

UE_LOG(LogTemp, Log, TEXT("WeaponSystem initialized with %d hardpoints"), WeaponHardpoints.Num());
}

void UWeaponSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
FActorComponentTickFunction* ThisTickFunction)
{
Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

// Update targeting
UpdateTargeting(DeltaTime);

// Update heat dissipation
UpdateHeat(DeltaTime);

// Update energy recharge
UpdateEnergy(DeltaTime);

// Update reloading weapons
UpdateReloading(DeltaTime);

// Draw debug if enabled
if (bShowDebug)
{
DrawDebugInfo();
}
}

// ============================================================================
// WEAPON CONTROL
// ============================================================================

void UWeaponSystem::FireWeapon(int32 HardpointIndex)
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return;
}

FWeaponHardpoint& Hardpoint = WeaponHardpoints[HardpointIndex];

if (!CanFireWeapon(HardpointIndex))
{
return;
}

// Check fire rate
float CurrentTime = GetWorld()->GetTimeSeconds();
float* LastTime = LastFireTime.Find(HardpointIndex);
float TimeSinceLastFire = LastTime ? (CurrentTime - *LastTime) : 999.0f;
float FireInterval = 1.0f / Hardpoint.InstalledWeapon.FireRate;

if (TimeSinceLastFire < FireInterval)
{
return;
}

// Consume resources
ConsumeEnergy(Hardpoint.InstalledWeapon.EnergyPerShot);
Hardpoint.CurrentAmmo--;
Hardpoint.CurrentHeat += Hardpoint.InstalledWeapon.HeatPerShot;

// Get firing position and direction
FVector FireLocation = GetHardpointWorldLocation(Hardpoint);
FRotator FireRotation = GetHardpointWorldRotation(Hardpoint);

// Apply gimbal adjustment if targeting
if (Hardpoint.bIsGimbaled && CurrentTarget.bHasLock)
{
FRotator GimbalAdjustment = CalculateGimbalAdjustment(Hardpoint, CurrentTarget.LeadPosition);
FireRotation = FireRotation + GimbalAdjustment;
}

// Apply convergence if enabled
if (bConvergenceEnabled)
{
FVector ConvergencePoint = FireLocation + (FireRotation.Vector() * ConvergenceDistance);
FireRotation = (ConvergencePoint - FireLocation).Rotation();
}

// Apply accuracy variation
float AccuracySpread = (1.0f - Hardpoint.InstalledWeapon.Accuracy) * 5.0f; // Max 5 degrees spread
FireRotation.Pitch += FMath::FRandRange(-AccuracySpread, AccuracySpread);
FireRotation.Yaw += FMath::FRandRange(-AccuracySpread, AccuracySpread);

// Fire the projectile
FireProjectile(Hardpoint, FireLocation, FireRotation);

// Play effects
if (Hardpoint.InstalledWeapon.MuzzleFlash)
{
UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), 
Hardpoint.InstalledWeapon.MuzzleFlash, FireLocation, FireRotation);
}

if (Hardpoint.InstalledWeapon.FireSound)
{
UGameplayStatics::PlaySoundAtLocation(this, 
Hardpoint.InstalledWeapon.FireSound, FireLocation);
}

// Update fire time
LastFireTime.Add(HardpointIndex, CurrentTime);
IsFiring.Add(HardpointIndex, true);

// Track statistics
TotalShotsFired++;

// Apply recoil
ApplyRecoil(HardpointIndex);

// Check for reload
if (Hardpoint.CurrentAmmo <= 0)
{
ReloadWeapon(HardpointIndex);
}

if (bShowDebug)
{
UE_LOG(LogTemp, Log, TEXT("Fired weapon %s from hardpoint %d"), 
*Hardpoint.InstalledWeapon.WeaponName.ToString(), HardpointIndex);
}
}

void UWeaponSystem::FireAllWeapons()
{
for (int32 i = 0; i < WeaponHardpoints.Num(); i++)
{
if (bFireLinked)
{
// Fire all weapons simultaneously with linked fire
FireWeapon(i);
}
else
{
// Fire weapons sequentially
if (CanFireWeapon(i))
{
FireWeapon(i);
break; // Fire one weapon per call
}
}
}
}

void UWeaponSystem::FireWeaponGroup(const TArray<int32>& GroupIndices)
{
for (int32 Index : GroupIndices)
{
FireWeapon(Index);
}
}

void UWeaponSystem::StopFiring(int32 HardpointIndex)
{
IsFiring.Add(HardpointIndex, false);
}

void UWeaponSystem::StopAllFiring()
{
for (int32 i = 0; i < WeaponHardpoints.Num(); i++)
{
StopFiring(i);
}
}

bool UWeaponSystem::CanFireWeapon(int32 HardpointIndex) const
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return false;
}

const FWeaponHardpoint& Hardpoint = WeaponHardpoints[HardpointIndex];

// Check if reloading
if (Hardpoint.bIsReloading)
{
return false;
}

// Check ammo
if (Hardpoint.CurrentAmmo <= 0)
{
return false;
}

// Check energy
if (!HasSufficientEnergy(Hardpoint.InstalledWeapon.EnergyPerShot))
{
return false;
}

// Check heat
if (IsOverheating())
{
return false;
}

return true;
}

// ============================================================================
// TARGETING
// ============================================================================

void UWeaponSystem::SetTarget(AActor* NewTarget)
{
if (!NewTarget)
{
ClearTarget();
return;
}

CurrentTarget.TargetActor = NewTarget;
CurrentTarget.bHasLock = false;
CurrentTarget.LockStrength = 0.0f;

UE_LOG(LogTemp, Log, TEXT("Target acquired: %s"), *NewTarget->GetName());
}

void UWeaponSystem::ClearTarget()
{
CurrentTarget = FTargetingInfo();
}

FVector UWeaponSystem::CalculateLeadPosition(const FVector& TargetPosition, 
const FVector& TargetVelocity, float ProjectileSpeed)
{
if (ProjectileSpeed <= 0.0f)
{
return TargetPosition;
}

FVector ShipLocation = GetOwner()->GetActorLocation();
FVector ToTarget = TargetPosition - ShipLocation;
float Distance = ToTarget.Size();

// Calculate time to intercept
float TimeToIntercept = Distance / ProjectileSpeed;

// Calculate lead position
FVector LeadPosition = TargetPosition + (TargetVelocity * TimeToIntercept);

return LeadPosition;
}

void UWeaponSystem::UpdateTargeting(float DeltaTime)
{
if (!CurrentTarget.TargetActor)
{
return;
}

// Update target information
CurrentTarget.TargetLocation = CurrentTarget.TargetActor->GetActorLocation();
CurrentTarget.TargetVelocity = CurrentTarget.TargetActor->GetVelocity();
CurrentTarget.DistanceToTarget = FVector::Dist(
GetOwner()->GetActorLocation(), 
CurrentTarget.TargetLocation
);

// Calculate lead position for primary weapon
if (WeaponHardpoints.Num() > 0)
{
float ProjectileSpeed = WeaponHardpoints[0].InstalledWeapon.ProjectileSpeed;
CurrentTarget.LeadPosition = CalculateLeadPosition(
CurrentTarget.TargetLocation,
CurrentTarget.TargetVelocity,
ProjectileSpeed
);

CurrentTarget.TimeToImpact = CurrentTarget.DistanceToTarget / ProjectileSpeed;
}

// Update lock strength
FVector ToTarget = CurrentTarget.TargetLocation - GetOwner()->GetActorLocation();
FVector Forward = GetOwner()->GetActorForwardVector();
float DotProduct = FVector::DotProduct(ToTarget.GetSafeNormal(), Forward);

if (DotProduct > 0.95f) // Within 18 degrees
{
CurrentTarget.LockStrength = FMath::Min(CurrentTarget.LockStrength + DeltaTime * 0.5f, 1.0f);
}
else
{
CurrentTarget.LockStrength = FMath::Max(CurrentTarget.LockStrength - DeltaTime * 2.0f, 0.0f);
}

CurrentTarget.bHasLock = CurrentTarget.LockStrength >= 0.8f;
}

// ============================================================================
// WEAPON MANAGEMENT
// ============================================================================

bool UWeaponSystem::InstallWeapon(int32 HardpointIndex, const FWeaponConfiguration& Weapon)
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return false;
}

FWeaponHardpoint& Hardpoint = WeaponHardpoints[HardpointIndex];
Hardpoint.InstalledWeapon = Weapon;
Hardpoint.CurrentAmmo = Weapon.AmmoCapacity;
Hardpoint.CurrentHeat = 0.0f;
Hardpoint.bIsReloading = false;

UE_LOG(LogTemp, Log, TEXT("Installed weapon %s on hardpoint %d"), 
*Weapon.WeaponName.ToString(), HardpointIndex);

return true;
}

bool UWeaponSystem::RemoveWeapon(int32 HardpointIndex)
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return false;
}

WeaponHardpoints[HardpointIndex].InstalledWeapon = FWeaponConfiguration();
return true;
}

void UWeaponSystem::ReloadWeapon(int32 HardpointIndex)
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return;
}

FWeaponHardpoint& Hardpoint = WeaponHardpoints[HardpointIndex];
Hardpoint.bIsReloading = true;
Hardpoint.ReloadProgress = 0.0f;

UE_LOG(LogTemp, Log, TEXT("Reloading weapon %s on hardpoint %d"), 
*Hardpoint.InstalledWeapon.WeaponName.ToString(), HardpointIndex);
}

void UWeaponSystem::ReloadAllWeapons()
{
for (int32 i = 0; i < WeaponHardpoints.Num(); i++)
{
if (WeaponHardpoints[i].CurrentAmmo < WeaponHardpoints[i].InstalledWeapon.AmmoCapacity)
{
ReloadWeapon(i);
}
}
}

FWeaponHardpoint* UWeaponSystem::GetHardpoint(int32 Index)
{
if (WeaponHardpoints.IsValidIndex(Index))
{
return &WeaponHardpoints[Index];
}
return nullptr;
}

// ============================================================================
// HEAT MANAGEMENT
// ============================================================================

float UWeaponSystem::GetTotalHeat() const
{
float TotalHeat = 0.0f;
for (const FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
TotalHeat += Hardpoint.CurrentHeat;
}
return TotalHeat;
}

bool UWeaponSystem::IsOverheating() const
{
return GetTotalHeat() > OverheatThreshold;
}

void UWeaponSystem::VentHeat(float Amount)
{
// Distribute heat venting across all weapons
float HeatPerWeapon = Amount / FMath::Max(1, WeaponHardpoints.Num());

for (FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
Hardpoint.CurrentHeat = FMath::Max(0.0f, Hardpoint.CurrentHeat - HeatPerWeapon);
}
}

void UWeaponSystem::UpdateHeat(float DeltaTime)
{
// Natural heat dissipation
float DissipationAmount = HeatDissipationRate * DeltaTime;

for (FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
Hardpoint.CurrentHeat = FMath::Max(0.0f, Hardpoint.CurrentHeat - DissipationAmount);
}
}

// ============================================================================
// AMMO MANAGEMENT
// ============================================================================

int32 UWeaponSystem::GetTotalAmmo() const
{
int32 TotalAmmo = 0;
for (const FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
TotalAmmo += Hardpoint.CurrentAmmo;
}
return TotalAmmo;
}

void UWeaponSystem::AddAmmo(int32 HardpointIndex, int32 Amount)
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return;
}

FWeaponHardpoint& Hardpoint = WeaponHardpoints[HardpointIndex];
Hardpoint.CurrentAmmo = FMath::Min(
Hardpoint.CurrentAmmo + Amount,
Hardpoint.InstalledWeapon.AmmoCapacity
);
}

bool UWeaponSystem::HasAmmo(int32 HardpointIndex) const
{
if (!WeaponHardpoints.IsValidIndex(HardpointIndex))
{
return false;
}

return WeaponHardpoints[HardpointIndex].CurrentAmmo > 0;
}

// ============================================================================
// WEAPON GROUPS
// ============================================================================

void UWeaponSystem::CreateWeaponGroup(const FName& GroupName, const TArray<int32>& HardpointIndices)
{
	FWeaponGroup NewGroup;
	NewGroup.WeaponIndices = HardpointIndices;
	WeaponGroups.Add(GroupName, NewGroup);
	UE_LOG(LogTemp, Log, TEXT("Created weapon group '%s' with %d weapons"),
		*GroupName.ToString(), HardpointIndices.Num());
}

void UWeaponSystem::FireWeaponGroupByName(const FName& GroupName)
{
	const FWeaponGroup* Group = WeaponGroups.Find(GroupName);
	if (Group)
	{
		FireWeaponGroup(Group->WeaponIndices);
	}
}

TArray<int32> UWeaponSystem::GetWeaponGroup(const FName& GroupName) const
{
	const FWeaponGroup* Group = WeaponGroups.Find(GroupName);
	return Group ? Group->WeaponIndices : TArray<int32>();
}

// ============================================================================
// ENERGY MANAGEMENT
// ============================================================================

bool UWeaponSystem::HasSufficientEnergy(float Required) const
{
return CurrentEnergy >= Required;
}

void UWeaponSystem::ConsumeEnergy(float Amount)
{
CurrentEnergy = FMath::Max(0.0f, CurrentEnergy - Amount);
}

void UWeaponSystem::UpdateEnergy(float DeltaTime)
{
// Recharge energy
CurrentEnergy = FMath::Min(MaxEnergy, CurrentEnergy + (EnergyRechargeRate * DeltaTime));
}

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

void UWeaponSystem::EnableConvergence(bool bEnable, float Distance)
{
bConvergenceEnabled = bEnable;
ConvergenceDistance = Distance;

UE_LOG(LogTemp, Log, TEXT("Weapon convergence %s at %.0f cm"), 
bEnable ? TEXT("enabled") : TEXT("disabled"), Distance);
}

void UWeaponSystem::SetFireLinked(bool bLinked)
{
bFireLinked = bLinked;
UE_LOG(LogTemp, Log, TEXT("Fire linking %s"), bLinked ? TEXT("enabled") : TEXT("disabled"));
}

void UWeaponSystem::CalculateOptimalFiringSolution(const FTargetingInfo& Target, FVector& OutAimPoint)
{
if (!Target.TargetActor || WeaponHardpoints.Num() == 0)
{
OutAimPoint = FVector::ZeroVector;
return;
}

// Use lead calculation for optimal solution
float ProjectileSpeed = WeaponHardpoints[0].InstalledWeapon.ProjectileSpeed;
OutAimPoint = CalculateLeadPosition(Target.TargetLocation, Target.TargetVelocity, ProjectileSpeed);
}

// ============================================================================
// STATUS & DEBUG
// ============================================================================

FString UWeaponSystem::GetWeaponSystemStatus() const
{
FString Status = TEXT("=== WEAPON SYSTEM STATUS ===\n\n");

Status += FString::Printf(TEXT("Energy: %.0f / %.0f (%.1f%%)\n"), 
CurrentEnergy, MaxEnergy, (CurrentEnergy / MaxEnergy) * 100.0f);
Status += FString::Printf(TEXT("Total Heat: %.1f / %.1f\n"), GetTotalHeat(), MaxHeat);
Status += FString::Printf(TEXT("Overheating: %s\n"), IsOverheating() ? TEXT("YES") : TEXT("No"));
Status += FString::Printf(TEXT("Total Ammo: %d\n"), GetTotalAmmo());
Status += FString::Printf(TEXT("Shots Fired: %d\n"), TotalShotsFired);
Status += FString::Printf(TEXT("Hits: %d\n"), TotalHits);
Status += FString::Printf(TEXT("Accuracy: %.1f%%\n\n"), 
TotalShotsFired > 0 ? (float)TotalHits / TotalShotsFired * 100.0f : 0.0f);

if (CurrentTarget.bHasLock)
{
Status += FString::Printf(TEXT("Target: %s\n"), *CurrentTarget.TargetActor->GetName());
Status += FString::Printf(TEXT("Distance: %.0f m\n"), CurrentTarget.DistanceToTarget / 100.0f);
Status += FString::Printf(TEXT("Lock Strength: %.0f%%\n"), CurrentTarget.LockStrength * 100.0f);
}
else
{
Status += TEXT("Target: None\n");
}

Status += TEXT("\nHardpoints:\n");
for (int32 i = 0; i < WeaponHardpoints.Num(); i++)
{
const FWeaponHardpoint& HP = WeaponHardpoints[i];
Status += FString::Printf(TEXT("%d. %s - Ammo: %d/%d, Heat: %.1f\n"),
i, *HP.InstalledWeapon.WeaponName.ToString(), 
HP.CurrentAmmo, HP.InstalledWeapon.AmmoCapacity, HP.CurrentHeat);
}

return Status;
}

void UWeaponSystem::DrawDebugInfo()
{
if (!GetWorld())
{
return;
}

// Draw hardpoint locations
for (int32 i = 0; i < WeaponHardpoints.Num(); ++i)
{
    const FWeaponHardpoint& Hardpoint = WeaponHardpoints[i];
    FVector Location = GetHardpointWorldLocation(Hardpoint);
    FColor Color = CanFireWeapon(i) ? FColor::Green : FColor::Red;
    DrawDebugSphere(GetWorld(), Location, 20.0f, 8, Color, false, -1.0f);
}

// Draw targeting info
if (CurrentTarget.bHasLock)
{
FVector ShipLocation = GetOwner()->GetActorLocation();
DrawDebugLine(GetWorld(), ShipLocation, CurrentTarget.TargetLocation, 
FColor::Yellow, false, -1.0f, 0, 2.0f);
DrawDebugSphere(GetWorld(), CurrentTarget.LeadPosition, 50.0f, 12, 
FColor::Orange, false, -1.0f);
}
}

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

void UWeaponSystem::FireProjectile(const FWeaponHardpoint& Hardpoint, 
const FVector& FireLocation, const FRotator& FireRotation)
{
// Perform line trace for hit detection (simplified - would spawn actual projectile)
FVector Start = FireLocation;
FVector End = Start + (FireRotation.Vector() * Hardpoint.InstalledWeapon.Range);

FHitResult HitResult;
if (LineTraceWeaponHit(Start, End, HitResult))
{
// Apply damage
if (HitResult.GetActor())
{
UGameplayStatics::ApplyDamage(HitResult.GetActor(), 
Hardpoint.InstalledWeapon.Damage, nullptr, GetOwner(), nullptr);

TotalHits++;
}

// Spawn impact effect
if (Hardpoint.InstalledWeapon.ImpactEffect)
{
UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), 
Hardpoint.InstalledWeapon.ImpactEffect, 
HitResult.Location, HitResult.Normal.Rotation());
}

// Play impact sound
if (Hardpoint.InstalledWeapon.ImpactSound)
{
UGameplayStatics::PlaySoundAtLocation(this, 
Hardpoint.InstalledWeapon.ImpactSound, HitResult.Location);
}

// Draw debug line
if (bShowDebug)
{
DrawDebugLine(GetWorld(), Start, HitResult.Location, 
FColor::Red, false, 0.5f, 0, 2.0f);
}
}
else if (bShowDebug)
{
DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.5f, 0, 1.0f);
}
}

bool UWeaponSystem::LineTraceWeaponHit(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
FCollisionQueryParams QueryParams;
QueryParams.AddIgnoredActor(GetOwner());
QueryParams.bTraceComplex = false;

return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, 
ECC_Visibility, QueryParams);
}

void UWeaponSystem::ApplyRecoil(int32 HardpointIndex)
{
// Apply recoil force to owner
// This would integrate with ship physics
if (AActor* Owner = GetOwner())
{
// Calculate recoil direction (opposite of fire direction)
FVector RecoilForce = -GetHardpointWorldRotation(WeaponHardpoints[HardpointIndex]).Vector() * 1000.0f;
// Would apply force to ship's physics component
}
}

void UWeaponSystem::UpdateReloading(float DeltaTime)
{
for (FWeaponHardpoint& Hardpoint : WeaponHardpoints)
{
if (Hardpoint.bIsReloading)
{
Hardpoint.ReloadProgress += DeltaTime / Hardpoint.InstalledWeapon.ReloadTime;

if (Hardpoint.ReloadProgress >= 1.0f)
{
// Reload complete
Hardpoint.CurrentAmmo = Hardpoint.InstalledWeapon.AmmoCapacity;
Hardpoint.bIsReloading = false;
Hardpoint.ReloadProgress = 0.0f;

UE_LOG(LogTemp, Log, TEXT("Reload complete for %s"), 
*Hardpoint.InstalledWeapon.WeaponName.ToString());
}
}
}
}

FVector UWeaponSystem::GetHardpointWorldLocation(const FWeaponHardpoint& Hardpoint) const
{
if (AActor* Owner = GetOwner())
{
return Owner->GetActorTransform().TransformPosition(Hardpoint.RelativeLocation);
}
return Hardpoint.RelativeLocation;
}

FRotator UWeaponSystem::GetHardpointWorldRotation(const FWeaponHardpoint& Hardpoint) const
{
if (AActor* Owner = GetOwner())
{
FTransform OwnerTransform = Owner->GetActorTransform();
return (OwnerTransform.GetRotation() * Hardpoint.RelativeRotation.Quaternion()).Rotator();
}
return Hardpoint.RelativeRotation;
}

FRotator UWeaponSystem::CalculateGimbalAdjustment(const FWeaponHardpoint& Hardpoint, 
const FVector& TargetLocation)
{
FVector HardpointLocation = GetHardpointWorldLocation(Hardpoint);
FRotator HardpointRotation = GetHardpointWorldRotation(Hardpoint);

// Calculate direction to target
FVector ToTarget = (TargetLocation - HardpointLocation).GetSafeNormal();
FRotator TargetRotation = ToTarget.Rotation();

// Calculate gimbal adjustment (limited by gimbal range)
FRotator Adjustment = TargetRotation - HardpointRotation;
Adjustment.Pitch = FMath::Clamp(Adjustment.Pitch, -Hardpoint.GimbalRange, Hardpoint.GimbalRange);
Adjustment.Yaw = FMath::Clamp(Adjustment.Yaw, -Hardpoint.GimbalRange, Hardpoint.GimbalRange);
Adjustment.Roll = 0.0f;

return Adjustment;
}
