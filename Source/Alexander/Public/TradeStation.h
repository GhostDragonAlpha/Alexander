// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TradeStation.generated.h"

UCLASS()
class ALEXANDER_API ATradeStation : public AActor
{
	GENERATED_BODY()
	
public:
	ATradeStation();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;

	// Station properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Station")
	FString StationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Station")
	FString OwningFaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade Station")
	FString StationType;
};