// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandClaimManager.h"
#include "Planet.h"
#include "PlanetFarmingComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

ALandClaimManager::ALandClaimManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default configuration
	GridSize = 100.0f;
	MinClaimSeparation = 50.0f;
	BasePricePerSquareMeter = 10.0f;
	bEnableAbandonment = true;
	AbandonmentThresholdDays = 30;

	// Initialize price multipliers
	SizePriceMultipliers.Add(ELandClaimSize::Small, 1.0f);
	SizePriceMultipliers.Add(ELandClaimSize::Medium, 2.5f);
	SizePriceMultipliers.Add(ELandClaimSize::Large, 5.0f);
	SizePriceMultipliers.Add(ELandClaimSize::Huge, 10.0f);

	// Initialize monthly cost multipliers
	MonthlyCostMultipliers.Add(ELandClaimSize::Small, 0.1f);
	MonthlyCostMultipliers.Add(ELandClaimSize::Medium, 0.25f);
	MonthlyCostMultipliers.Add(ELandClaimSize::Large, 0.5f);
	MonthlyCostMultipliers.Add(ELandClaimSize::Huge, 1.0f);
}

void ALandClaimManager::BeginPlay()
{
	Super::BeginPlay();
}

void ALandClaimManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Process claim expiration periodically
	static float TimeAccumulator = 0.0f;
	TimeAccumulator += DeltaTime;

	if (TimeAccumulator >= 60.0f) // Process every minute
	{
		TimeAccumulator = 0.0f;
		ProcessClaimExpiration();
		UpdateClaimValues();
	}
}

void ALandClaimManager::InitializeForPlanet(APlanet* Planet)
{
	if (!Planet)
	{
		UE_LOG(LogTemp, Error, TEXT("LandClaimManager: Invalid planet provided"));
		return;
	}

	TargetPlanet = Planet;
	UE_LOG(LogTemp, Log, TEXT("LandClaimManager initialized for planet: %s"), *Planet->GetName());

	// Generate initial land claims for the planet
	GenerateInitialClaims();
}

void ALandClaimManager::GenerateInitialClaims()
{
	if (!TargetPlanet.IsValid())
	{
		return;
	}

	// Generate claims in a grid pattern around the planet
	int32 GridCount = 20; // 20x20 grid of potential claims
	float PlanetRadius = TargetPlanet->PlanetRadius * 100.0f; // Convert to Unreal units

	for (int32 X = -GridCount / 2; X < GridCount / 2; X++)
	{
		for (int32 Y = -GridCount / 2; Y < GridCount / 2; Y++)
		{
			FVector Location = FVector(
				X * GridSize * 10.0f,
				Y * GridSize * 10.0f,
				0.0f
			);

			// Check if location is on planet surface
			if (Location.Size() <= PlanetRadius * 1.1f) // Within 10% of surface
			{
				// Create unclaimed plot
				FLandClaim NewClaim;
				NewClaim.Location = Location;
				NewClaim.ClaimSize = ELandClaimSize::Small;
				NewClaim.Status = ELandClaimStatus::Unclaimed;
				NewClaim.ParentPlanet = TargetPlanet.Get();
				NewClaim.Bounds = CalculateClaimBounds(Location, ELandClaimSize::Small);
				NewClaim.PurchasePrice = CalculateClaimPrice(Location, ELandClaimSize::Small);
				NewClaim.MonthlyCost = CalculateMonthlyCost(Location, ELandClaimSize::Small);
				NewClaim.CurrentValue = NewClaim.PurchasePrice;

				// Analyze location suitability
				AnalyzeLocationSuitability(NewClaim);

				LandClaims.Add(NewClaim);
				ClaimIndexMap.Add(NewClaim.ClaimID, LandClaims.Num() - 1);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d initial land claims"), LandClaims.Num());
}

FLandClaimValidationResult ALandClaimManager::CreateLandClaim(FVector Location, ELandClaimSize Size, const FString& OwnerID, const FString& ClaimName)
{
	FLandClaimValidationResult Result = ValidateClaimLocation(Location, Size, OwnerID);

	if (!Result.bIsValid)
	{
		return Result;
	}

	// Create the claim
	FLandClaim NewClaim;
	NewClaim.ClaimID = FGuid::NewGuid();
	NewClaim.Location = Location;
	NewClaim.ClaimSize = Size;
	NewClaim.OwnerID = OwnerID;
	NewClaim.ClaimName = ClaimName;
	NewClaim.Status = ELandClaimStatus::Claimed;
	NewClaim.ParentPlanet = TargetPlanet.Get();
	NewClaim.Bounds = CalculateClaimBounds(Location, Size);
	NewClaim.PurchasePrice = CalculateClaimPrice(Location, Size);
	NewClaim.MonthlyCost = CalculateMonthlyCost(Location, Size);
	NewClaim.PurchaseDate = FDateTime::Now();
	NewClaim.LastPaymentDate = FDateTime::Now();
	NewClaim.CurrentValue = NewClaim.PurchasePrice;

	// Analyze location suitability
	AnalyzeLocationSuitability(NewClaim);

	// Add to collection
	LandClaims.Add(NewClaim);
	ClaimIndexMap.Add(NewClaim.ClaimID, LandClaims.Num() - 1);

	// Fire event
	OnLandClaimPurchased.Broadcast(NewClaim, OwnerID);

	UE_LOG(LogTemp, Log, TEXT("Land claim created: %s for owner %s"), *ClaimName, *OwnerID);

	Result.bIsValid = true;
	return Result;
}

FLandClaimValidationResult ALandClaimManager::ValidateClaimLocation(FVector Location, ELandClaimSize Size, const FString& OwnerID)
{
	FLandClaimValidationResult Result;

	// Check if location is valid
	if (!TargetPlanet.IsValid())
	{
		Result.ErrorMessage = TEXT("No target planet set");
		return Result;
	}

	// Calculate bounds
	FBox TestBounds = CalculateClaimBounds(Location, Size);

	// Check for overlapping claims
	if (CheckForOverlappingClaims(TestBounds))
	{
		Result.ErrorMessage = TEXT("Location overlaps with existing claim");
		
		// Suggest alternative locations
		Result.AlternativeLocations = FindValidClaimLocations();
		return Result;
	}

	// Check distance from other claims
	for (const FLandClaim& ExistingClaim : LandClaims)
	{
		if (ExistingClaim.Status == ELandClaimStatus::Claimed)
		{
			float Distance = FVector::Dist(Location, ExistingClaim.Location);
			float MinDistance = MinClaimSeparation + GetSizeInMeters(Size) + GetSizeInMeters(ExistingClaim.ClaimSize);
			
			if (Distance < MinDistance)
			{
				Result.ErrorMessage = TEXT("Too close to existing claim");
				return Result;
			}
		}
	}

	// Check planet bounds
	float PlanetRadius = TargetPlanet->PlanetRadius * 100.0f; // Convert to Unreal units
	float DistanceFromCenter = Location.Size();
	
	if (DistanceFromCenter > PlanetRadius * 1.5f) // Allow some margin
	{
		Result.ErrorMessage = TEXT("Location is too far from planet surface");
		return Result;
	}

	// Check reputation requirements (placeholder)
	float RequiredRep = 0.0f; // Would check against faction system
	if (!OwnerID.IsEmpty())
	{
		// TODO: Check actual faction reputation
	}

	Result.bIsValid = true;
	return Result;
}

bool ALandClaimManager::CheckForOverlappingClaims(const FBox& TestBounds) const
{
	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status != ELandClaimStatus::Unclaimed)
		{
			if (TestBounds.Intersect(Claim.Bounds))
			{
				return true;
			}
		}
	}
	return false;
}

bool ALandClaimManager::PurchaseLandClaim(const FGuid& ClaimID, const FString& OwnerID, float PaymentAmount)
{
	int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (!IndexPtr)
	{
		return false;
	}

	FLandClaim& Claim = LandClaims[*IndexPtr];
	
	if (Claim.Status != ELandClaimStatus::Unclaimed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Claim is not available for purchase"));
		return false;
	}

	if (PaymentAmount < Claim.PurchasePrice)
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient payment for claim"));
		return false;
	}

	// Process purchase
	Claim.Status = ELandClaimStatus::Claimed;
	Claim.OwnerID = OwnerID;
	Claim.PurchaseDate = FDateTime::Now();
	Claim.LastPaymentDate = FDateTime::Now();

	// Fire event
	OnLandClaimPurchased.Broadcast(Claim, OwnerID);

	UE_LOG(LogTemp, Log, TEXT("Land claim purchased: %s by %s"), *Claim.ClaimName, *OwnerID);
	return true;
}

bool ALandClaimManager::SellLandClaim(const FGuid& ClaimID, const FString& NewOwnerID, float SalePrice)
{
	int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (!IndexPtr)
	{
		return false;
	}

	FLandClaim& Claim = LandClaims[*IndexPtr];
	
	if (Claim.Status != ELandClaimStatus::Claimed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Claim is not available for sale"));
		return false;
	}

	// Process sale
	FString PreviousOwner = Claim.OwnerID;
	Claim.OwnerID = NewOwnerID;
	Claim.CurrentValue = SalePrice;
	Claim.LastPaymentDate = FDateTime::Now();

	// Fire event
	OnLandClaimSold.Broadcast(Claim, NewOwnerID);

	UE_LOG(LogTemp, Log, TEXT("Land claim sold: %s from %s to %s"), *Claim.ClaimName, *PreviousOwner, *NewOwnerID);
	return true;
}

bool ALandClaimManager::AbandonLandClaim(const FGuid& ClaimID, const FString& OwnerID)
{
	int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (!IndexPtr)
	{
		return false;
	}

	FLandClaim& Claim = LandClaims[*IndexPtr];
	
	if (Claim.Status != ELandClaimStatus::Claimed || Claim.OwnerID != OwnerID)
	{
		return false;
	}

	// Process abandonment
	Claim.Status = ELandClaimStatus::Abandoned;
	Claim.OwnerID = TEXT("");
	Claim.LastPaymentDate = FDateTime::Now();

	// Fire event
	OnLandClaimExpired.Broadcast(Claim);

	UE_LOG(LogTemp, Log, TEXT("Land claim abandoned: %s"), *Claim.ClaimName);
	return true;
}

FLandClaim ALandClaimManager::GetClaimByID(const FGuid& ClaimID) const
{
	const int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (IndexPtr)
	{
		return LandClaims[*IndexPtr];
	}
	return FLandClaim();
}

TArray<FLandClaim> ALandClaimManager::GetClaimsByOwner(const FString& OwnerID) const
{
	TArray<FLandClaim> Result;
	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.OwnerID == OwnerID)
		{
			Result.Add(Claim);
		}
	}
	return Result;
}

TArray<FLandClaim> ALandClaimManager::GetAllClaims() const
{
	return LandClaims;
}

TArray<FLandClaim> ALandClaimManager::GetUnclaimedPlots() const
{
	TArray<FLandClaim> Result;
	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Unclaimed)
		{
			Result.Add(Claim);
		}
	}
	return Result;
}

void ALandClaimManager::ProcessMonthlyPayments()
{
	FDateTime CurrentTime = FDateTime::Now();

	for (FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Claimed)
		{
			FTimespan TimeSinceLastPayment = CurrentTime - Claim.LastPaymentDate;
			
			if (TimeSinceLastPayment.GetDays() >= 30)
			{
				// Process payment (would deduct from player's account)
				bool PaymentSuccessful = true; // Placeholder - would check actual payment

				if (PaymentSuccessful)
				{
					Claim.LastPaymentDate = CurrentTime;
					UE_LOG(LogTemp, Log, TEXT("Monthly payment processed for claim: %s"), *Claim.ClaimName);
				}
				else
				{
					// Payment failed - mark for foreclosure
					Claim.Status = ELandClaimStatus::Foreclosed;
					OnLandClaimExpired.Broadcast(Claim);
					UE_LOG(LogTemp, Warning, TEXT("Claim foreclosed due to non-payment: %s"), *Claim.ClaimName);
				}
			}
		}
	}
}

void ALandClaimManager::UpdateClaimValues()
{
	FDateTime CurrentTime = FDateTime::Now();

	for (FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Claimed)
		{
			// Calculate appreciation based on time owned and improvements
			FTimespan TimeOwned = CurrentTime - Claim.PurchaseDate;
			float YearsOwned = TimeOwned.GetDays() / 365.0f;

			// Base appreciation: 2% per year
			float AppreciationRate = 0.02f;
			
			// Bonus for developed claims
			if (Claim.BuildingIDs.Num() > 0)
			{
				AppreciationRate += 0.01f; // Additional 1% for developed claims
			}

			// Bonus for farming suitability
			if (Claim.bSuitableForFarming)
			{
				AppreciationRate += 0.005f; // Additional 0.5% for farming land
			}

			// Update value
			float NewValue = Claim.PurchasePrice * FMath::Pow(1.0f + AppreciationRate, YearsOwned);
			Claim.CurrentValue = NewValue;
		}
	}
}

FLandClaimStatistics ALandClaimManager::GetClaimStatistics() const
{
	FLandClaimStatistics Stats;
	Stats.TotalClaims = LandClaims.Num();

	float TotalValue = 0.0f;
	FLandClaim MostValuable;

	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Claimed)
		{
			Stats.ClaimedPlots++;
			TotalValue += Claim.CurrentValue;

			// Track most valuable claim
			if (Claim.CurrentValue > MostValuable.CurrentValue)
			{
				MostValuable = Claim;
			}

			// Count claims by owner
			if (!Claim.OwnerID.IsEmpty())
			{
				int32* Count = Stats.ClaimsByOwner.Find(Claim.OwnerID);
				if (Count)
				{
					(*Count)++;
				}
				else
				{
					Stats.ClaimsByOwner.Add(Claim.OwnerID, 1);
				}
			}
		}
		else if (Claim.Status == ELandClaimStatus::Unclaimed)
		{
			Stats.UnclaimedPlots++;
		}
	}

	Stats.TotalValue = TotalValue;
	Stats.AverageValue = Stats.ClaimedPlots > 0 ? TotalValue / Stats.ClaimedPlots : 0.0f;
	Stats.MostValuableClaim = MostValuable;

	return Stats;
}

TArray<FVector> ALandClaimManager::FindSuitableFarmingLocations(int32 MaxResults) const
{
	TArray<FVector> Results;
	int32 FoundCount = 0;

	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.bSuitableForFarming && (Claim.Status == ELandClaimStatus::Unclaimed || Claim.Status == ELandClaimStatus::Claimed))
		{
			Results.Add(Claim.Location);
			FoundCount++;

			if (FoundCount >= MaxResults)
			{
				break;
			}
		}
	}

	return Results;
}

TArray<FVector> ALandClaimManager::FindSuitableBuildingLocations(int32 MaxResults) const
{
	TArray<FVector> Results;
	int32 FoundCount = 0;

	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.bSuitableForBuilding && (Claim.Status == ELandClaimStatus::Unclaimed || Claim.Status == ELandClaimStatus::Claimed))
		{
			Results.Add(Claim.Location);
			FoundCount++;

			if (FoundCount >= MaxResults)
			{
				break;
			}
		}
	}

	return Results;
}

FLandClaim ALandClaimManager::GetClaimAtLocation(FVector Location) const
{
	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.Bounds.IsInside(Location))
		{
			return Claim;
		}
	}
	return FLandClaim();
}

bool ALandClaimManager::AddBuildingToClaim(const FGuid& ClaimID, const FGuid& BuildingID)
{
	int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (!IndexPtr)
	{
		return false;
	}

	FLandClaim& Claim = LandClaims[*IndexPtr];
	Claim.BuildingIDs.Add(BuildingID);
	return true;
}

bool ALandClaimManager::RemoveBuildingFromClaim(const FGuid& ClaimID, const FGuid& BuildingID)
{
	int32* IndexPtr = ClaimIndexMap.Find(ClaimID);
	if (!IndexPtr)
	{
		return false;
	}

	FLandClaim& Claim = LandClaims[*IndexPtr];
	Claim.BuildingIDs.Remove(BuildingID);
	return true;
}

void ALandClaimManager::ProcessClaimExpiration()
{
	if (!bEnableAbandonment)
	{
		return;
	}

	FDateTime CurrentTime = FDateTime::Now();

	for (FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Claimed)
		{
			FTimespan TimeSinceLastPayment = CurrentTime - Claim.LastPaymentDate;
			
			if (TimeSinceLastPayment.GetDays() >= AbandonmentThresholdDays)
			{
				// Mark as abandoned
				Claim.Status = ELandClaimStatus::Abandoned;
				Claim.OwnerID = TEXT("");
				OnLandClaimExpired.Broadcast(Claim);
				UE_LOG(LogTemp, Log, TEXT("Claim abandoned due to inactivity: %s"), *Claim.ClaimName);
			}
		}
	}
}

TArray<FVector> ALandClaimManager::FindValidClaimLocations() const
{
	TArray<FVector> Locations;
	
	// Return a few valid locations as alternatives
	for (const FLandClaim& Claim : LandClaims)
	{
		if (Claim.Status == ELandClaimStatus::Unclaimed)
		{
			Locations.Add(Claim.Location);
			if (Locations.Num() >= 5) // Limit suggestions
			{
				break;
			}
		}
	}
	
	return Locations;
}

FBox ALandClaimManager::CalculateClaimBounds(FVector Location, ELandClaimSize Size) const
{
	float SizeMeters = GetSizeInMeters(Size);
	float HalfSize = SizeMeters / 2.0f;

	FVector Min = Location - FVector(HalfSize, HalfSize, HalfSize);
	FVector Max = Location + FVector(HalfSize, HalfSize, HalfSize);

	return FBox(Min, Max);
}

float ALandClaimManager::CalculateClaimPrice(FVector Location, ELandClaimSize Size) const
{
	float BaseSize = GetSizeInMeters(Size);
	float Area = BaseSize * BaseSize;
	float BasePrice = Area * BasePricePerSquareMeter;

	// Apply size multiplier
	float SizeMultiplier = SizePriceMultipliers.Contains(Size) ? SizePriceMultipliers[Size] : 1.0f;

	// Apply location multiplier (closer to landing zones = more expensive)
	float LocationMultiplier = 1.0f;
	if (TargetPlanet.IsValid())
	{
		// Would check distance to landing zones and other factors
		LocationMultiplier = FMath::RandRange(0.8f, 2.0f); // Random for now
	}

	return BasePrice * SizeMultiplier * LocationMultiplier;
}

float ALandClaimManager::CalculateMonthlyCost(FVector Location, ELandClaimSize Size) const
{
	float BasePrice = CalculateClaimPrice(Location, Size);
	float SizeMultiplier = MonthlyCostMultipliers.Contains(Size) ? MonthlyCostMultipliers[Size] : 0.1f;
	return BasePrice * SizeMultiplier;
}

void ALandClaimManager::AnalyzeLocationSuitability(FLandClaim& Claim) const
{
	if (!TargetPlanet.IsValid())
	{
		return;
	}

	// Check farming suitability using planet's farming component
	if (TargetPlanet->FarmingComponent)
	{
		Claim.bSuitableForFarming = TargetPlanet->FarmingComponent->IsSuitableForFarming(Claim.Location);
		Claim.SoilQuality = TargetPlanet->FarmingComponent->GetBaseSoilQuality(Claim.Location);
	}
	else
	{
		// Fallback analysis
		Claim.bSuitableForFarming = FMath::RandBool();
		Claim.SoilQuality = FMath::RandRange(0.3f, 0.9f);
	}

	// Check building suitability (basic slope and terrain check)
	Claim.bSuitableForBuilding = FMath::RandBool(); // Would do actual terrain analysis

	// Analyze resources
	Claim.AvailableResources.Add("Iron", FMath::RandRange(0.0f, 100.0f));
	Claim.AvailableResources.Add("Copper", FMath::RandRange(0.0f, 100.0f));
	Claim.AvailableResources.Add("Water", FMath::RandRange(0.0f, 100.0f));
	Claim.AvailableResources.Add("Organics", FMath::RandRange(0.0f, 100.0f));

	// Water availability
	Claim.WaterAvailability = FMath::RandRange(0.2f, 0.9f);
}

bool ALandClaimManager::CanAffordClaim(const FString& OwnerID, float Cost) const
{
	// Placeholder - would check player's actual currency
	return true;
}

float ALandClaimManager::GetSizeInMeters(ELandClaimSize Size) const
{
	switch (Size)
	{
		case ELandClaimSize::Small:
			return 100.0f;
		case ELandClaimSize::Medium:
			return 250.0f;
		case ELandClaimSize::Large:
			return 500.0f;
		case ELandClaimSize::Huge:
			return 1000.0f;
		default:
			return 100.0f;
	}
}