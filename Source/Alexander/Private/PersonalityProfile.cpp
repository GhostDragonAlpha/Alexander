// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersonalityProfile.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions

UPersonalityProfile::UPersonalityProfile()
{
	PersonalityArchetype = TEXT("Balanced");
}

void UPersonalityProfile::InitializeRandomPersonality()
{
	Traits.Empty();
	
	// Initialize with random values
	Traits.Add(FPersonalityTrait{TEXT("openness"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("conscientiousness"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("extraversion"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("agreeableness"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("neuroticism"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("kindness"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("aggression"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	Traits.Add(FPersonalityTrait{TEXT("curiosity"), FMath::FRandRange(0.0f, 1.0f), 1.0f});
	
	CalculateArchetype();
}

void UPersonalityProfile::SetTrait(const FString& TraitName, float Value)
{
	for (FPersonalityTrait& Trait : Traits)
	{
		if (Trait.TraitName == TraitName)
		{
			Trait.Value = FMath::Clamp(Value, 0.0f, 1.0f);
			CalculateArchetype();
			return;
		}
	}
	
	// Add new trait if not found
	Traits.Add(FPersonalityTrait{TraitName, FMath::Clamp(Value, 0.0f, 1.0f), 1.0f});
	CalculateArchetype();
}

float UPersonalityProfile::GetTraitValue(const FString& TraitName) const
{
	for (const FPersonalityTrait& Trait : Traits)
	{
		if (Trait.TraitName == TraitName)
		{
			return Trait.Value;
		}
	}
	
	return 0.5f; // Default fallback
}

FString UPersonalityProfile::GetPersonalityType() const
{
	return PersonalityArchetype;
}

bool UPersonalityProfile::IsCompatibleWith(UPersonalityProfile* OtherProfile) const
{
	if (!OtherProfile) return false;
	
	float CompatibilityScore = 0.0f;
	int32 TraitCount = 0;
	
	// Compare major traits
	TArray<FString> MajorTraits = {TEXT("extraversion"), TEXT("agreeableness"), TEXT("openness")};
	
	for (const FString& TraitName : MajorTraits)
	{
		float MyValue = GetTraitValue(TraitName);
		float OtherValue = OtherProfile->GetTraitValue(TraitName);
		
		// Calculate compatibility based on trait differences
		float Difference = FMath::Abs(MyValue - OtherValue);
		CompatibilityScore += (1.0f - Difference);
		TraitCount++;
	}
	
	if (TraitCount > 0)
	{
		CompatibilityScore /= TraitCount;
	}
	
	return CompatibilityScore > 0.6f;
}

float UPersonalityProfile::GetSocialModifier() const
{
	float Extraversion = GetTraitValue("extraversion");
	float Agreeableness = GetTraitValue("agreeableness");
	float Neuroticism = GetTraitValue("neuroticism");
	
	return (Extraversion + Agreeableness - Neuroticism) / 2.0f;
}

float UPersonalityProfile::GetCombatModifier() const
{
	float Aggression = GetTraitValue("aggression");
	float Neuroticism = GetTraitValue("neuroticism");
	float Conscientiousness = GetTraitValue("conscientiousness");
	
	return (Aggression + Neuroticism + (1.0f - Conscientiousness)) / 3.0f;
}

float UPersonalityProfile::GetTradeModifier() const
{
	float Conscientiousness = GetTraitValue("conscientiousness");
	float Agreeableness = GetTraitValue("agreeableness");
	float Openness = GetTraitValue("openness");
	
	return (Conscientiousness + Agreeableness + Openness) / 3.0f;
}

void UPersonalityProfile::CalculateArchetype()
{
	float Extraversion = GetTraitValue("extraversion");
	float Agreeableness = GetTraitValue("agreeableness");
	float Openness = GetTraitValue("openness");
	float Aggression = GetTraitValue("aggression");
	float Conscientiousness = GetTraitValue("conscientiousness");
	
	// Determine archetype based on dominant traits
	if (Extraversion > 0.7f && Agreeableness > 0.7f)
	{
		PersonalityArchetype = TEXT("Social Butterfly");
	}
	else if (Aggression > 0.7f && Extraversion > 0.5f)
	{
		PersonalityArchetype = TEXT("Aggressive Leader");
	}
	else if (Openness > 0.7f && Conscientiousness > 0.5f)
	{
		PersonalityArchetype = TEXT("Curious Scholar");
	}
	else if (Agreeableness > 0.7f && Conscientiousness > 0.7f)
	{
		PersonalityArchetype = TEXT("Kind Helper");
	}
	else if (Aggression > 0.7f)
	{
		PersonalityArchetype = TEXT("Hostile Warrior");
	}
	else if (Openness > 0.7f)
	{
		PersonalityArchetype = TEXT("Adventurous Explorer");
	}
	else if (Conscientiousness > 0.7f)
	{
		PersonalityArchetype = TEXT("Disciplined Worker");
	}
	else if (Extraversion < 0.3f)
	{
		PersonalityArchetype = TEXT("Quiet Observer");
	}
	else
	{
		PersonalityArchetype = TEXT("Balanced Individual");
	}
}