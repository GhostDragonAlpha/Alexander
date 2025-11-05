// Copyright Epic Games, Inc. All Rights Reserved.
// Agent 3 Full Implementation

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "CelestialBodyComponent.generated.h"

// Forward declarations
class UCelestialBodyRegistry;
class UStaticMeshComponent;

UCLASS(ClassGroup=(CelestialScaling), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UCelestialBodyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCelestialBodyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
	float Mass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
	float Radius;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Celestial Body")
	float CurrentScaleFactor;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Celestial Body")
	float TargetScaleFactor;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Celestial Body")
	float DistanceToPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
	FName BodyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
	FString BodyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	bool bEnableDynamicScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float ScaleTransitionSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float MinScaleFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float MaxScaleFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	bool bEnableGravity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	float GravityMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "LOD")
	int32 CurrentLODLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	void UpdateScale(const FVector& PlayerPosition);

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	void ApplyScaleImmediate(float NewScale);

	void SmoothScaleTransition(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	FVector CalculateGravitationalForce(const FVector& TargetPosition, float TargetMass) const;

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	FVector CalculateGravitationalAcceleration(const FVector& Position) const;

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	void UpdateLODSystem();

	// Agent 2 interface compatibility
	UFUNCTION(BlueprintCallable, Category = "Celestial")
	FName GetBodyName() const { return BodyID; }

	UFUNCTION(BlueprintCallable, Category = "Celestial")
	float GetMass() const { return Mass; }

	UFUNCTION(BlueprintCallable, Category = "Celestial")
	float GetRadius() const { return Radius; }

	void ApplyPositionOffset(const FVector& Offset);
	void UpdateScaleForDistance(float Distance);

	void DrawDebugVisualization();

	UFUNCTION(BlueprintCallable, Category = "Celestial Body")
	FString GetStatusInfo() const;

private:
	UPROPERTY()
	UStaticMeshComponent* VisualMesh;

	FVector OriginalScale;
	bool bIsRegistered;

	void RegisterWithSubsystem();
	void UnregisterFromSubsystem();
	void CacheVisualMeshComponent();
	float CalculateScaleFactorForDistance(float Distance) const;
	void ApplyScaleToActor(float Scale);

	static constexpr double GravitationalConstant = 6.67430e-11;
	static constexpr float KilometersToCm = 100000.0f;
};
