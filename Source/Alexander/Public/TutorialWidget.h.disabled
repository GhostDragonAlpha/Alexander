// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialSystem.h"
#include "TutorialWidget.generated.h"

/**
 * UI widget for displaying tutorial information
 */
UCLASS()
class ALEXANDER_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void UpdateTutorialDisplay();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnSkipButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void OnNextButtonClicked();

	// Blueprint-exposed properties
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	FText CurrentTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	FText CurrentDescription;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	FText CurrentHint;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 CurrentStep = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	int32 TotalSteps = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	float StepProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	bool bCanSkip = true;

protected:
	UPROPERTY()
	UTutorialSystem* TutorialSystem;
};
