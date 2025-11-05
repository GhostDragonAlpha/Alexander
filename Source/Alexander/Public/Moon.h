// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OrbitalBody.h"
#include "Moon.generated.h"

UCLASS(Blueprintable)
class ALEXANDER_API AMoon : public AOrbitalBody
{
	GENERATED_BODY()

public:
	AMoon();

protected:
	virtual void BeginPlay() override;

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MoonMesh;

	// Visual Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moon Properties")
	float MoonScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moon Properties")
	FLinearColor MoonColor;
};
