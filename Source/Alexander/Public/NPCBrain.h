// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NPCBrain.generated.h"

/**
 * NPC Brain - Core AI decision making system
 * Handles high-level AI behavior and decision making
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UNPCBrain : public UObject
{
	GENERATED_BODY()

public:
	UNPCBrain();

	// AI decision making
	UFUNCTION(BlueprintCallable, Category = "AI Brain")
	bool MakeDecision(const FString& Situation, TArray<FString>& AvailableActions);

	// Goal management
	UFUNCTION(BlueprintCallable, Category = "AI Brain")
	void SetGoal(const FString& NewGoal);

	UFUNCTION(BlueprintCallable, Category = "AI Brain")
	FString GetCurrentGoal() const { return CurrentGoal; }

	// Priority system
	UFUNCTION(BlueprintCallable, Category = "AI Brain")
	void UpdatePriorities();

	// State management
	UFUNCTION(BlueprintCallable, Category = "AI Brain")
	bool IsGoalCompleted() const { return bGoalCompleted; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "AI Brain")
	FString CurrentGoal;

	UPROPERTY(BlueprintReadOnly, Category = "AI Brain")
	bool bGoalCompleted;

	UPROPERTY(BlueprintReadOnly, Category = "AI Brain")
	TArray<FString> CurrentPriorities;

private:
	float DecisionTimer = 0.0f;
};