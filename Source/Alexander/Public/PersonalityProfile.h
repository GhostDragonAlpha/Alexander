// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIDialogueSystem.h"
#include "PersonalityProfile.generated.h"

/**
 * Personality Profile - Manages NPC personality traits and behaviors
 * Defines how an NPC thinks, feels, and acts
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UPersonalityProfile : public UObject
{
	GENERATED_BODY()

public:
	UPersonalityProfile();

	// Personality traits
	UPROPERTY(BlueprintReadOnly, Category = "Personality")
	TArray<FPersonalityTrait> Traits;

	// Personality operations
	UFUNCTION(BlueprintCallable, Category = "Personality")
	void InitializeRandomPersonality();

	UFUNCTION(BlueprintCallable, Category = "Personality")
	void SetTrait(const FString& TraitName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Personality")
	float GetTraitValue(const FString& TraitName) const;

	// Personality analysis
	UFUNCTION(BlueprintCallable, Category = "Personality")
	FString GetPersonalityType() const;

	UFUNCTION(BlueprintCallable, Category = "Personality")
	bool IsCompatibleWith(UPersonalityProfile* OtherProfile) const;

	// Behavior modifiers
	UFUNCTION(BlueprintCallable, Category = "Personality")
	float GetSocialModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Personality")
	float GetCombatModifier() const;

	UFUNCTION(BlueprintCallable, Category = "Personality")
	float GetTradeModifier() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Personality")
	FString PersonalityArchetype;

private:
	void CalculateArchetype();
};