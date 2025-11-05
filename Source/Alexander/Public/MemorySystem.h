// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIDialogueSystem.h"
#include "MemorySystem.generated.h"

/**
 * Memory System - Manages NPC memories and learning
 * Handles long-term and short-term memory storage and retrieval
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API UMemorySystem : public UObject
{
	GENERATED_BODY()

public:
	UMemorySystem();

	// Memory storage
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	TArray<FMemoryFragment> Memories;

	// Memory operations
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ConsolidateMemories();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void DecayMemories(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	int32 GetMemoryCount() const { return Memories.Num(); }

	// Memory search
	UFUNCTION(BlueprintCallable, Category = "Memory")
	TArray<FMemoryFragment> SearchMemoriesByType(EMemoryType Type);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	TArray<FMemoryFragment> SearchMemoriesByActor(const FString& ActorName);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	TArray<FMemoryFragment> SearchMemoriesByLocation(const FString& Location);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	float LastConsolidationTime;

	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	float ConsolidationInterval = 300.0f; // 5 minutes

private:
	void PromoteToLongTerm(FMemoryFragment& Memory);
};