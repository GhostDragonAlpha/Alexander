// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OriginCenteredPhysicsManager.generated.h"

// Forward declarations
class AActor;
class USceneComponent;

/**
 * Sector coordinate structure
 */
USTRUCT(BlueprintType)
struct FSectorCoordinate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 Y;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 Z;

    FSectorCoordinate() : X(0), Y(0), Z(0) {}
    FSectorCoordinate(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

    bool operator==(const FSectorCoordinate& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    bool operator!=(const FSectorCoordinate& Other) const
    {
        return !(*this == Other);
    }

    FString ToString() const
    {
        return FString::Printf(TEXT("(%d, %d, %d)"), X, Y, Z);
    }
};

/**
 * Origin-centered physics data for an actor
 */
USTRUCT(BlueprintType)
struct FOriginCenteredData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FSectorCoordinate SectorCoord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FRotator LocalRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector LocalVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector LocalAcceleration;

    FOriginCenteredData()
        : SectorCoord(0, 0, 0)
        , LocalPosition(FVector::ZeroVector)
        , LocalRotation(FRotator::ZeroRotator)
        , LocalVelocity(FVector::ZeroVector)
        , LocalAcceleration(FVector::ZeroVector)
    {}
};

/**
 * Physics sector bounds
 */
USTRUCT(BlueprintType)
struct FPhysicsSectorBounds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SectorSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxCoordinateValue;

    FPhysicsSectorBounds()
        : SectorSize(100000.0f) // 100km sectors
        , MaxCoordinateValue(1000000.0f) // 1000km max from origin
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorChanged, AActor*, Actor, FSectorCoordinate, NewSector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOriginShifted, FSectorCoordinate, NewOrigin);

/**
 * Origin-Centered Physics Manager
 * Manages relative positioning system to handle large-scale space environments
 * without floating-point precision issues
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UOriginCenteredPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UOriginCenteredPhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Actor Registration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsActorRegistered(AActor* Actor) const;

    // Position Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetWorldPosition(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetWorldPosition(AActor* Actor, const FVector& WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void TeleportActorTo(AActor* Actor, const FVector& WorldPosition);

    // Velocity and Acceleration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetWorldVelocity(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetWorldVelocity(AActor* Actor, const FVector& WorldVelocity);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddWorldVelocity(AActor* Actor, const FVector& DeltaVelocity);

    // Sector Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    FSectorCoordinate GetActorSector(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetActorSector(AActor* Actor, const FSectorCoordinate& Sector);

    UFUNCTION(BlueprintPure, Category = "Physics")
    FSectorCoordinate WorldPositionToSector(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintPure, Category = "Physics")
    FVector SectorToWorldPosition(const FSectorCoordinate& Sector, const FVector& LocalPosition) const;

    // Origin Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPlayerOrigin(AActor* PlayerActor);

    UFUNCTION(BlueprintPure, Category = "Physics")
    FSectorCoordinate GetCurrentOrigin() const { return CurrentOrigin; }

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ShiftOrigin(const FSectorCoordinate& NewOrigin);

    // Distance Calculations
    UFUNCTION(BlueprintPure, Category = "Physics")
    float GetDistanceBetweenActors(AActor* ActorA, AActor* ActorB) const;

    UFUNCTION(BlueprintPure, Category = "Physics")
    float GetDistanceToPlayer(AActor* Actor) const;

    // Precision Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RebaseActorPosition(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RebaseAllPositions();

    // Serialization
    UFUNCTION(BlueprintCallable, Category = "Physics")
    FOriginCenteredData SerializeActorData(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void DeserializeActorData(AActor* Actor, const FOriginCenteredData& Data);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics")
    FOnSectorChanged OnSectorChanged;

    UPROPERTY(BlueprintAssignable, Category = "Physics")
    FOnOriginShifted OnOriginShifted;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FPhysicsSectorBounds SectorBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    AActor* PlayerPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bAutoRebaseEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AutoRebaseDistance;

private:
    // Actor data storage
    UPROPERTY()
    TMap<AActor*, FOriginCenteredData> ActorDataMap;

    // Current origin sector (player's sector)
    UPROPERTY()
    FSectorCoordinate CurrentOrigin;

    // Player reference for distance calculations
    UPROPERTY()
    AActor* PlayerActor;

    // Helper functions
    void UpdateActorSector(AActor* Actor, const FSectorCoordinate& NewSector);
    void UpdateActorTransform(AActor* Actor);
    bool ShouldRebaseActor(AActor* Actor) const;
    FVector CalculateWorldVelocity(const FOriginCenteredData& Data) const;
};

// Hash function for FSectorCoordinate
inline uint32 GetTypeHash(const FSectorCoordinate& Sector)
{
    return HashCombine(HashCombine(GetTypeHash(Sector.X), GetTypeHash(Sector.Y)), GetTypeHash(Sector.Z));
}