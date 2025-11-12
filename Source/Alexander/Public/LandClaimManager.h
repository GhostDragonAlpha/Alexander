// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "LandClaimManager.generated.h"

// Forward declarations
class APlanet;
class AFaction;

// Land claim sizes
UENUM(BlueprintType)
enum class ELandClaimSize : uint8
{
	Small UMETA(DisplayName = "Small (100x100m)"),
	Medium UMETA(DisplayName = "Medium (250x250m)"),
	Large UMETA(DisplayName = "Large (500x500m)"),
	Huge UMETA(DisplayName = "Huge (1000x1000m)")
};

// Land claim status
UENUM(BlueprintType)
enum class ELandClaimStatus : uint8
{
	Unclaimed UMETA(DisplayName = "Unclaimed"),
	Claimed UMETA(DisplayName = "Claimed"),
	Disputed UMETA(DisplayName = "Disputed"),
	Abandoned UMETA(DisplayName = "Abandoned"),
	Foreclosed UMETA(DisplayName = "Foreclosed"),
	Reserved UMETA(DisplayName = "Reserved")
};

// Land claim data structure
USTRUCT(BlueprintType)
struct FLandClaim
{
	GENERATED_BODY()

	// Unique claim ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FGuid ClaimID;

	// Claim owner (player or faction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FString OwnerID;

	// Claim name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FString ClaimName;

	// Center location of the claim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FVector Location;

	// Claim size type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	ELandClaimSize ClaimSize;

	// Claim status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	ELandClaimStatus Status;

	// Planet this claim is on
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	TWeakObjectPtr<APlanet> ParentPlanet;

	// Claim bounds (calculated from location and size)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FBox Bounds;

	// Claim purchase price
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float PurchasePrice;

	// Monthly maintenance cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float MonthlyCost;

	// Date claim was purchased
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FDateTime PurchaseDate;

	// Last payment date
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	FDateTime LastPaymentDate;

	// Faction reputation requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float RequiredReputation;

	// Resources available in this claim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	TMap<FName, float> AvailableResources;

	// Soil quality (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float SoilQuality;

	// Water availability (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float WaterAvailability;

	// Claim value (appreciates over time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	float CurrentValue;

	// Is this claim suitable for farming?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	bool bSuitableForFarming;

	// Is this claim suitable for building?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	bool bSuitableForBuilding;

	// Number of days until claim is considered abandoned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	int32 AbandonmentDays;

	// Buildings constructed on this claim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Claim")
	TArray<FGuid> BuildingIDs;

	// Constructor
	FLandClaim()
	{
		ClaimID = FGuid::NewGuid();
		Status = ELandClaimStatus::Unclaimed;
		PurchasePrice = 0.0f;
		MonthlyCost = 0.0f;
		RequiredReputation = 0.0f;
		SoilQuality = 0.5f;
		WaterAvailability = 0.5f;
		CurrentValue = 0.0f;
		bSuitableForFarming = false;
		bSuitableForBuilding = false;
		AbandonmentDays = 30;
	}
};

// Land claim validation result
USTRUCT(BlueprintType)
struct FLandClaimValidationResult
{
	GENERATED_BODY()

	// Is the claim valid?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bIsValid;

	// Error message if invalid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	FString ErrorMessage;

	// Warning messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	TArray<FString> Warnings;

	// Suggested alternatives if invalid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	TArray<FVector> AlternativeLocations;

	FLandClaimValidationResult()
	{
		bIsValid = false;
	}
};

// Land claim statistics
USTRUCT(BlueprintType)
struct FLandClaimStatistics
{
	GENERATED_BODY()

	// Total number of claims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalClaims;

	// Number of claimed plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 ClaimedPlots;

	// Number of unclaimed plots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 UnclaimedPlots;

	// Total value of all claims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float TotalValue;

	// Average claim value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	float AverageValue;

	// Most valuable claim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	FLandClaim MostValuableClaim;

	// Claims by owner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	TMap<FString, int32> ClaimsByOwner;

	FLandClaimStatistics()
	{
		TotalClaims = 0;
		ClaimedPlots = 0;
		UnclaimedPlots = 0;
		TotalValue = 0.0f;
		AverageValue = 0.0f;
	}
};

// Delegate for claim events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLandClaimPurchased, const FLandClaim&, Claim, const FString&, OwnerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLandClaimSold, const FLandClaim&, Claim, const FString&, NewOwnerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandClaimExpired, const FLandClaim&, Claim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandClaimDisputed, const FLandClaim&, Claim);

/**
 * Land Claim Manager - Handles planetary land ownership, validation, and management
 */
UCLASS()
class ALEXANDER_API ALandClaimManager : public AActor
{
	GENERATED_BODY()

public:
	ALandClaimManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Initialize the land claim manager for a specific planet
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	void InitializeForPlanet(APlanet* Planet);

	// Create a new land claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	FLandClaimValidationResult CreateLandClaim(FVector Location, ELandClaimSize Size, const FString& OwnerID, const FString& ClaimName);

	// Purchase a land claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool PurchaseLandClaim(const FGuid& ClaimID, const FString& OwnerID, float PaymentAmount);

	// Sell a land claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool SellLandClaim(const FGuid& ClaimID, const FString& NewOwnerID, float SalePrice);

	// Abandon a land claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool AbandonLandClaim(const FGuid& ClaimID, const FString& OwnerID);

	// Validate if a location is available for claiming
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	FLandClaimValidationResult ValidateClaimLocation(FVector Location, ELandClaimSize Size, const FString& OwnerID = TEXT(""));

	// Check if a claim overlaps with existing claims
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool CheckForOverlappingClaims(const FBox& TestBounds) const;

	// Get claim by ID
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	FLandClaim GetClaimByID(const FGuid& ClaimID) const;

	// Get all claims for an owner
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	TArray<FLandClaim> GetClaimsByOwner(const FString& OwnerID) const;

	// Get all claims on a planet
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	TArray<FLandClaim> GetAllClaims() const;

	// Get unclaimed plots
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	TArray<FLandClaim> GetUnclaimedPlots() const;

	// Process monthly payments
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	void ProcessMonthlyPayments();

	// Update claim values (appreciation/depreciation)
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	void UpdateClaimValues();

	// Get claim statistics
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	FLandClaimStatistics GetClaimStatistics() const;

	// Find suitable farming locations
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	TArray<FVector> FindSuitableFarmingLocations(int32 MaxResults = 10) const;

	// Find suitable building locations
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	TArray<FVector> FindSuitableBuildingLocations(int32 MaxResults = 10) const;

	// Get claim at location
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	FLandClaim GetClaimAtLocation(FVector Location) const;

	// Add building to claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool AddBuildingToClaim(const FGuid& ClaimID, const FGuid& BuildingID);

	// Remove building from claim
	UFUNCTION(BlueprintCallable, Category = "Land Claims")
	bool RemoveBuildingFromClaim(const FGuid& ClaimID, const FGuid& BuildingID);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Land Claims")
	FOnLandClaimPurchased OnLandClaimPurchased;

	UPROPERTY(BlueprintAssignable, Category = "Land Claims")
	FOnLandClaimSold OnLandClaimSold;

	UPROPERTY(BlueprintAssignable, Category = "Land Claims")
	FOnLandClaimExpired OnLandClaimExpired;

	UPROPERTY(BlueprintAssignable, Category = "Land Claims")
	FOnLandClaimDisputed OnLandClaimDisputed;

protected:
	// Planet this manager is for
	UPROPERTY()
	TWeakObjectPtr<APlanet> TargetPlanet;

	// All land claims
	UPROPERTY()
	TArray<FLandClaim> LandClaims;

	// Claim lookup by ID
	UPROPERTY()
	TMap<FGuid, int32> ClaimIndexMap;

	// Grid size for claim placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float GridSize;

	// Minimum distance between claims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MinClaimSeparation;

	// Price multipliers for different claim sizes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	TMap<ELandClaimSize, float> SizePriceMultipliers;

	// Monthly cost multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	TMap<ELandClaimSize, float> MonthlyCostMultipliers;

	// Base price per square meter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	float BasePricePerSquareMeter;

	// Enable/disable claim abandonment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnableAbandonment;

	// Days until claim is considered abandoned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 AbandonmentThresholdDays;

private:
	// Generate initial land claims for the planet
	void GenerateInitialClaims();

	// Calculate claim bounds based on location and size
	FBox CalculateClaimBounds(FVector Location, ELandClaimSize Size) const;

	// Calculate claim price based on location and size
	float CalculateClaimPrice(FVector Location, ELandClaimSize Size) const;

	// Calculate monthly cost for a claim
	float CalculateMonthlyCost(FVector Location, ELandClaimSize Size) const;

	// Analyze location suitability
	void AnalyzeLocationSuitability(FLandClaim& Claim) const;

	// Check if owner can afford claim
	bool CanAffordClaim(const FString& OwnerID, float Cost) const;

	// Process claim expiration
	void ProcessClaimExpiration();

	// Find valid claim locations in grid pattern
	TArray<FVector> FindValidClaimLocations() const;

	// Get size in meters
	float GetSizeInMeters(ELandClaimSize Size) const;
};