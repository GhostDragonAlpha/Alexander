// Copyright Epic Games, Inc. All Rights Reserved.

#include "NPCBrain.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"

UNPCBrain::UNPCBrain()
{
	CurrentGoal = TEXT("Idle");
	bGoalCompleted = false;
}

bool UNPCBrain::MakeDecision(const FString& Situation, TArray<FString>& AvailableActions)
{
	// Simple decision making logic
	if (AvailableActions.Num() == 0)
	{
		return false;
	}

	// For now, just return a random action
	// In a full implementation, this would use complex AI reasoning
	int32 RandomIndex = FMath::RandRange(0, AvailableActions.Num() - 1);
	
	// Clear and set the chosen action
	AvailableActions = { AvailableActions[RandomIndex] };
	
	return true;
}

void UNPCBrain::SetGoal(const FString& NewGoal)
{
	CurrentGoal = NewGoal;
	bGoalCompleted = false;
}

void UNPCBrain::UpdatePriorities()
{
	// Simple priority update
	CurrentPriorities.Empty();
	CurrentPriorities.Add(TEXT("Survival"));
	CurrentPriorities.Add(TEXT("Social"));
	CurrentPriorities.Add(TEXT("Economic"));
}