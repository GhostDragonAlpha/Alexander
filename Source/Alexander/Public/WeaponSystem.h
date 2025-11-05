// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "WeaponSystem.generated.h"

// Forward declarations
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UParticleSystem;
class USoundBase;

/**
 * Weapon types available
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Laser UMETA(DisplayName = "Laser"),
	Plasma UMETA(DisplayName = "Plasma"),
	Kinetic UMETA(DisplayName = "Kinetic"),
	Missile UMETA(DisplayName = "Missile"),
	Torpedo UMETA(DisplayName = "Torpedo"),
	Railgun UMETA(DisplayName = "Railgun"),
	EMP UMETA(DisplayName = "EMP"),
	Mining UMETA(DisplayName = "Mining Laser")
};

/**
 * Weapon firing modes
 */
UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single UMETA(DisplayName = "Single Shot"),
	Burst UMETA(DisplayName = "Burst Fire"),
	Automatic UMETA(DisplayName = "Automatic"),
	Charged UMETA(DisplayName = "Charged Shot")
};

/**
 * Weapon mount points
 */
UENUM(BlueprintType)
enum class EWeaponMountPoint : uint8
{
	LeftWing UMETA(DisplayName = "Left Wing"),
	RightWing UMETA(DisplayName = "Right Wing"),
	Nose UMETA(DisplayName = "Nose"),
	TopTurret UMETA(DisplayName = "Top Turret"),
	BottomTurret UMETA(DisplayName = "Bottom Turret"),
	Internal UMETA(DisplayName = "Internal Bay")
};

/**
 * Weapon configuration data
 */
USTRUCT(BlueprintType)
struct FWeaponConfiguration : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName = TEXT("Unknown Weapon");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Laser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponFireMode FireMode = EWeaponFireMode::Single;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 1.0f; // Shots per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ProjectileSpeed = 3000.0f; // cm/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Range = 100000.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float EnergyPerShot = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float HeatPerShot = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 AmmoCapacity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Accuracy = 0.95f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UStaticMesh* ProjectileMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UParticleSystem* MuzzleFlash = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UParticleSystem* ImpactEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* ImpactSound = nullptr;
};

/**
 * Weapon hardpoint configuration
 */
USTRUCT(BlueprintType)
struct FWeaponHardpoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FName HardpointName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	EWeaponMountPoint MountPoint = EWeaponMountPoint::Nose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FWeaponConfiguration InstalledWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	bool bIsGimbaled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float GimbalRange = 15.0f; // Degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	int32 CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float CurrentHeat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float ReloadProgress = 0.0f;
};

/**
 * Targeting information
 */
USTRUCT(BlueprintType)
struct FTargetingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	FVector TargetVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	float DistanceToTarget = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	float TimeToImpact = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	FVector LeadPosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	bool bHasLock = false;

	UPROPERTY(BlueprintReadWrite, Category = "Targeting")
	float LockStrength = 0.0f; // 0-1
};

/**
 * Weapon System Component - manages all weapons on a ship
 */

/**
 * Wrapper for weapon group indices
 */
USTRUCT()
struct FWeaponGroup
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> WeaponIndices;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UWeaponSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponSystem();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// WEAPON CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FWeaponHardpoint> WeaponHardpoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	UDataTable* WeaponDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxHeat = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	float HeatDissipationRate = 5.0f; // Per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	float OverheatThreshold = 80.0f;

	// ============================================================================
	// WEAPON CONTROL
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireWeapon(int32 HardpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireAllWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireWeaponGroup(const TArray<int32>& GroupIndices);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StopFiring(int32 HardpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StopAllFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool CanFireWeapon(int32 HardpointIndex) const;

	// ============================================================================
	// TARGETING
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ClearTarget();

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	FTargetingInfo GetTargetingInfo() const { return CurrentTarget; }

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	bool HasTargetLock() const { return CurrentTarget.bHasLock; }

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	FVector CalculateLeadPosition(const FVector& TargetPosition, const FVector& TargetVelocity, float ProjectileSpeed);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void UpdateTargeting(float DeltaTime);

	// ============================================================================
	// WEAPON MANAGEMENT
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool InstallWeapon(int32 HardpointIndex, const FWeaponConfiguration& Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool RemoveWeapon(int32 HardpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void ReloadWeapon(int32 HardpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void ReloadAllWeapons();

	FWeaponHardpoint* GetHardpoint(int32 Index);

	// ============================================================================
	// HEAT MANAGEMENT
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Heat")
	float GetTotalHeat() const;

	UFUNCTION(BlueprintCallable, Category = "Heat")
	bool IsOverheating() const;

	UFUNCTION(BlueprintCallable, Category = "Heat")
	void VentHeat(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Heat")
	void UpdateHeat(float DeltaTime);

	// ============================================================================
	// AMMO MANAGEMENT
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	int32 GetTotalAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void AddAmmo(int32 HardpointIndex, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	bool HasAmmo(int32 HardpointIndex) const;

	// ============================================================================
	// WEAPON GROUPS
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Groups")
	void CreateWeaponGroup(const FName& GroupName, const TArray<int32>& HardpointIndices);

	UFUNCTION(BlueprintCallable, Category = "Groups")
	void FireWeaponGroupByName(const FName& GroupName);

	UFUNCTION(BlueprintCallable, Category = "Groups")
	TArray<int32> GetWeaponGroup(const FName& GroupName) const;

	// ============================================================================
	// ENERGY MANAGEMENT
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
	float MaxEnergy = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
	float CurrentEnergy = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy")
	float EnergyRechargeRate = 50.0f; // Per second

	UFUNCTION(BlueprintCallable, Category = "Energy")
	bool HasSufficientEnergy(float Required) const;

	UFUNCTION(BlueprintCallable, Category = "Energy")
	void ConsumeEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Energy")
	void UpdateEnergy(float DeltaTime);

	// ============================================================================
	// ADVANCED FEATURES
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Advanced")
	void EnableConvergence(bool bEnable, float ConvergenceDistance = 50000.0f);

	UFUNCTION(BlueprintCallable, Category = "Advanced")
	void SetFireLinked(bool bLinked);

	UFUNCTION(BlueprintCallable, Category = "Advanced")
	void CalculateOptimalFiringSolution(const FTargetingInfo& Target, FVector& OutAimPoint);

	// ============================================================================
	// STATUS & DEBUG
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Status")
	FString GetWeaponSystemStatus() const;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

protected:
	// Internal state
	UPROPERTY()
	FTargetingInfo CurrentTarget;

	UPROPERTY()
	TMap<FName, FWeaponGroup> WeaponGroups;

	UPROPERTY()
	bool bFireLinked = false;

	UPROPERTY()
	bool bConvergenceEnabled = false;

	UPROPERTY()
	float ConvergenceDistance = 50000.0f;

	// Firing state
	UPROPERTY()
	TMap<int32, float> LastFireTime;

	UPROPERTY()
	TMap<int32, bool> IsFiring;

	// Performance tracking
	int32 TotalShotsFired = 0;
	int32 TotalHits = 0;
	float AccuracyRating = 0.0f;

private:
	// Helper functions
	void FireProjectile(const FWeaponHardpoint& Hardpoint, const FVector& FireLocation, const FRotator& FireRotation);
	bool LineTraceWeaponHit(const FVector& Start, const FVector& End, FHitResult& OutHit);
	void ApplyRecoil(int32 HardpointIndex);
	void UpdateReloading(float DeltaTime);
	FVector GetHardpointWorldLocation(const FWeaponHardpoint& Hardpoint) const;
	FRotator GetHardpointWorldRotation(const FWeaponHardpoint& Hardpoint) const;
	FRotator CalculateGimbalAdjustment(const FWeaponHardpoint& Hardpoint, const FVector& TargetLocation);
};
