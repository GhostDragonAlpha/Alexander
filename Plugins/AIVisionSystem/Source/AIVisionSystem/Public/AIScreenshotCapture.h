// AIScreenshotCapture.h - Screenshot capture actor for AI vision

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Http.h"
#include "AIScreenshotCapture.generated.h"

USTRUCT(BlueprintType)
struct FSceneMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "AI Vision")
	FString LevelName;

	UPROPERTY(BlueprintReadWrite, Category = "AI Vision")
	TArray<FString> ActorNames;

	UPROPERTY(BlueprintReadWrite, Category = "AI Vision")
	int32 ActorCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "AI Vision")
	FDateTime Timestamp = FDateTime::Now();
};

UCLASS()
class AIVISIONSYSTEM_API AAIScreenshotCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	AAIScreenshotCapture();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Capture current viewport and save to disk
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	void CaptureViewport(const FString& OutputFilename, int32 Width = 1920, int32 Height = 1080);

	// Capture with scene metadata (actors, positions, etc.)
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	FString CaptureWithMetadata(const FString& OutputFilename);

	// Start automatic capture at intervals
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	void StartAutoCapture(float IntervalSeconds = 5.0f);

	// Stop automatic capture
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	void StopAutoCapture();

	// Set AI backend URL
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	void SetBackendURL(const FString& URL);

	// Send screenshot to AI backend for analysis
	UFUNCTION(BlueprintCallable, Category = "AI Vision")
	void SendToAIBackend(const FString& ImagePath, const FString& MetadataJson);

	// Get development goal for AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision")
	FString DevelopmentGoal;

	// AI Backend URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision")
	FString BackendURL = TEXT("http://localhost:8000");

	// Enable debug logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Vision")
	bool bDebugMode = true;

private:
	FTimerHandle AutoCaptureTimer;
	int32 CaptureCounter = 0;

	// Gather scene metadata
	FSceneMetadata GatherSceneMetadata();

	// Convert metadata to JSON
	FString MetadataToJson(const FSceneMetadata& Metadata);

	// Handle HTTP response from AI backend
	void OnAIResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Save texture to PNG file
	bool SaveTextureToPNG(UTexture2D* Texture, const FString& FilePath);
};
