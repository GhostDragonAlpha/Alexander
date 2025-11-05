// Copyright Epic Games, Inc. All Rights Reserved.

#include "MemorySystem.h"
#include "Engine/World.h"

UMemorySystem::UMemorySystem()
{
	LastConsolidationTime = 0.0f;
}

void UMemorySystem::ConsolidateMemories()
{
	if (!GetWorld()) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastConsolidationTime < ConsolidationInterval)
	{
		return;
	}

	// Promote important memories to long-term
	for (FMemoryFragment& Memory : Memories)
	{
		if (!Memory.bIsLongTerm && Memory.Importance > 0.7f)
		{
			PromoteToLongTerm(Memory);
		}
	}

	LastConsolidationTime = CurrentTime;
}

void UMemorySystem::DecayMemories(float DeltaTime)
{
	const float DecayRate = 0.0001f; // Memory decay rate

	for (FMemoryFragment& Memory : Memories)
	{
		if (!Memory.bIsLongTerm)
		{
			Memory.Importance = FMath::Max(0.0f, Memory.Importance - DecayRate * DeltaTime);
		}
	}

	// Remove completely decayed memories
	Memories.RemoveAll([](const FMemoryFragment& Memory)
	{
		return !Memory.bIsLongTerm && Memory.Importance <= 0.0f;
	});
}

TArray<FMemoryFragment> UMemorySystem::SearchMemoriesByType(EMemoryType Type)
{
	TArray<FMemoryFragment> Result;
	
	for (const FMemoryFragment& Memory : Memories)
	{
		if (Memory.MemoryType == Type)
		{
			Result.Add(Memory);
		}
	}
	
	return Result;
}

TArray<FMemoryFragment> UMemorySystem::SearchMemoriesByActor(const FString& ActorName)
{
	TArray<FMemoryFragment> Result;
	
	for (const FMemoryFragment& Memory : Memories)
	{
		if (Memory.AssociatedActors.Contains(ActorName))
		{
			Result.Add(Memory);
		}
	}
	
	return Result;
}

TArray<FMemoryFragment> UMemorySystem::SearchMemoriesByLocation(const FString& Location)
{
	TArray<FMemoryFragment> Result;
	
	for (const FMemoryFragment& Memory : Memories)
	{
		if (Memory.Location == Location)
		{
			Result.Add(Memory);
		}
	}
	
	return Result;
}

void UMemorySystem::PromoteToLongTerm(FMemoryFragment& Memory)
{
	Memory.bIsLongTerm = true;
	Memory.Importance = FMath::Min(1.0f, Memory.Importance * 1.2f);
}