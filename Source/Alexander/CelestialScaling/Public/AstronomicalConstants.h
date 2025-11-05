// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlayerOriginManager.h" // For FVirtualPosition definition
#include "AstronomicalConstants.generated.h" // MUST be last include

// Forward declarations
class AActor;

USTRUCT(BlueprintType)
struct ALEXANDER_API FCelestialBodyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	double Mass = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	double Radius = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	double StandardGravParam = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	double OrbitalRadius = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	double OrbitalPeriod = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Celestial Data")
	FString BodyName;

	FCelestialBodyData()
		: Mass(0.0), Radius(0.0), StandardGravParam(0.0)
		, OrbitalRadius(0.0), OrbitalPeriod(0.0), BodyName(TEXT("Unknown"))
	{}

	FCelestialBodyData(double InMass, double InRadius, double InStandardGravParam,
					   double InOrbitalRadius, double InOrbitalPeriod, const FString& InName)
		: Mass(InMass), Radius(InRadius), StandardGravParam(InStandardGravParam)
		, OrbitalRadius(InOrbitalRadius), OrbitalPeriod(InOrbitalPeriod), BodyName(InName)
	{}
};

// FVirtualPosition moved to PlayerOriginManager.h to avoid duplicate definition

USTRUCT(BlueprintType)
struct ALEXANDER_API FScaleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Data")
	float CurrentScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Data")
	float TargetScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Data")
	float TransitionSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Data")
	FVector BaseVisualSize;

	FScaleData()
		: CurrentScale(1.0f), TargetScale(1.0f), TransitionSpeed(1.0f), BaseVisualSize(FVector::OneVector)
	{}

	FScaleData(float InCurrentScale, float InTargetScale, float InTransitionSpeed, const FVector& InBaseSize)
		: CurrentScale(InCurrentScale), TargetScale(InTargetScale)
		, TransitionSpeed(InTransitionSpeed), BaseVisualSize(InBaseSize)
	{}
};

USTRUCT(BlueprintType)
struct ALEXANDER_API FGravitationalForceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravitational Force")
	FVector ForceVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravitational Force")
	float ForceMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravitational Force")
	AActor* SourceBody = nullptr;

	FGravitationalForceData()
		: ForceVector(FVector::ZeroVector), ForceMagnitude(0.0f), SourceBody(nullptr)
	{}

	FGravitationalForceData(const FVector& InForceVector, float InForceMagnitude, AActor* InSourceBody)
		: ForceVector(InForceVector), ForceMagnitude(InForceMagnitude), SourceBody(InSourceBody)
	{}
};

namespace CelestialScalingConstants
{
	static constexpr float MinScaleFactor = 0.0001f;
	static constexpr float MaxScaleFactor = 10000.0f;
	static constexpr float DefaultScaleFactor = 1.0f;
	static constexpr double MaxOriginOffset = 1000000.0;
	static constexpr double RecenterThreshold = 500000.0;
	static constexpr float MinPositionUpdateThreshold = 0.1f;
	static constexpr float SphereOfInfluenceMultiplier = 1.2f;
	static constexpr float VisualInfluenceMultiplier = 3.0f;
	static constexpr float NearFieldDistance = 100000.0f;
	static constexpr float FarFieldDistance = 1000000.0f;
	static constexpr float DefaultTransitionSpeed = 2.0f;
	static constexpr float MinScaleDifference = 0.01f;
	static constexpr float ScaleSmoothingFactor = 0.1f;

	namespace SolSystem
	{
		static constexpr double G = 6.67430e-20;

		namespace Sun
		{
			static constexpr double Mass = 1.98892e30;
			static constexpr double Radius = 696000.0;
			static constexpr double StandardGravParam = 1.32712440018e11;
		}

		namespace Earth
		{
			static constexpr double Mass = 5.97237e24;
			static constexpr double Radius = 6371.0;
			static constexpr double StandardGravParam = 3.986004418e5;
			static constexpr double OrbitalRadius = 149597870.7;
			static constexpr double OrbitalPeriod = 31558149.7635;
		}

		namespace Moon
		{
			static constexpr double Mass = 7.342e22;
			static constexpr double Radius = 1737.4;
			static constexpr double StandardGravParam = 4.9048695e3;
			static constexpr double OrbitalRadius = 384400.0;
			static constexpr double OrbitalPeriod = 2360591.5;
		}

		namespace Mars
		{
			static constexpr double Mass = 6.4171e23;
			static constexpr double Radius = 3389.5;
			static constexpr double StandardGravParam = 4.282837e4;
			static constexpr double OrbitalRadius = 227939200.0;
			static constexpr double OrbitalPeriod = 59355072.0;
		}

		namespace Jupiter
		{
			static constexpr double Mass = 1.8982e27;
			static constexpr double Radius = 69911.0;
			static constexpr double StandardGravParam = 1.26686534e8;
			static constexpr double OrbitalRadius = 778570000.0;
			static constexpr double OrbitalPeriod = 374335776.0;
		}
	}
}

UCLASS(BlueprintType)
class ALEXANDER_API UAstronomicalConstantsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Data")
	static FCelestialBodyData GetCelestialBodyData(const FString& BodyName);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Calculations")
	static float CalculateSphereOfInfluence(double SemiMajorAxis, double PrimaryMass, double SecondaryMass);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Calculations")
	static FVector CalculateGravitationalForce(double Mass1, double Mass2, const FVector& Position1, const FVector& Position2);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Calculations")
	static float CalculateEscapeVelocity(double Mass, double Radius);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Validation")
	static bool IsValidScaleFactor(float ScaleFactor);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Validation")
	static float ClampScaleFactor(float ScaleFactor);

	UFUNCTION(BlueprintCallable, Category = "Celestial Scaling|Validation")
	static bool RequiresRecentering(const FVector& PositionOffset);

	// Note: FVirtualPosition functions moved to PlayerOriginManager
};
