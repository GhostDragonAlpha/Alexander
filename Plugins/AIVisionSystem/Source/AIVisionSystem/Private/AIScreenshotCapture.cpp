// AIScreenshotCapture.cpp - Implementation

#include "AIScreenshotCapture.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"

AAIScreenshotCapture::AAIScreenshotCapture()
{
	PrimaryActorTick.bCanEverTick = true;
	CaptureCounter = 0;
}

void AAIScreenshotCapture::BeginPlay()
{
	Super::BeginPlay();
	
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Screenshot Capture Actor Ready"));
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Backend URL: %s"), *BackendURL);
	}
}

void AAIScreenshotCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAIScreenshotCapture::CaptureViewport(const FString& OutputFilename, int32 Width, int32 Height)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogTemp, Error, TEXT("AI Vision: No valid viewport found"));
		return;
	}

	// Create output directory if it doesn't exist
	FString OutputDir = FPaths::ProjectDir() / TEXT("Saved/AICaptures");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*OutputDir))
	{
		PlatformFile.CreateDirectory(*OutputDir);
	}

	FString FullPath = OutputDir / OutputFilename;

	// Request high-res screenshot
	FScreenshotRequest::RequestScreenshot(FullPath, false, false);

	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Screenshot saved to %s"), *FullPath);
	}

	// After a brief delay, send to AI backend
	FTimerHandle SendTimer;
	GetWorld()->GetTimerManager().SetTimer(SendTimer, [this, FullPath]()
	{
		FString MetadataJson = CaptureWithMetadata(FullPath);
		SendToAIBackend(FullPath, MetadataJson);
	}, 0.5f, false);
}

FString AAIScreenshotCapture::CaptureWithMetadata(const FString& OutputFilename)
{
	FSceneMetadata Metadata = GatherSceneMetadata();
	return MetadataToJson(Metadata);
}

FSceneMetadata AAIScreenshotCapture::GatherSceneMetadata()
{
	FSceneMetadata Metadata;
	
	if (!GetWorld())
	{
		return Metadata;
	}

	Metadata.LevelName = GetWorld()->GetMapName();
	Metadata.Timestamp = FDateTime::UtcNow();

	// Get all actors in scene
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
	
	Metadata.ActorCount = AllActors.Num();

	// Collect actor names and basic info
	for (AActor* Actor : AllActors)
	{
		if (Actor && !Actor->IsHidden())
		{
			FString ActorInfo = FString::Printf(TEXT("%s (%s)"), 
				*Actor->GetName(), 
				*Actor->GetClass()->GetName());
			Metadata.ActorNames.Add(ActorInfo);
		}
	}

	return Metadata;
}

FString AAIScreenshotCapture::MetadataToJson(const FSceneMetadata& Metadata)
{
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	
	JsonObj->SetStringField(TEXT("level_name"), Metadata.LevelName);
	JsonObj->SetNumberField(TEXT("actor_count"), Metadata.ActorCount);
	JsonObj->SetStringField(TEXT("timestamp"), Metadata.Timestamp.ToString());
	JsonObj->SetStringField(TEXT("development_goal"), DevelopmentGoal);

	TArray<TSharedPtr<FJsonValue>> ActorsArray;
	for (const FString& ActorName : Metadata.ActorNames)
	{
		ActorsArray.Add(MakeShareable(new FJsonValueString(ActorName)));
	}
	JsonObj->SetArrayField(TEXT("actors"), ActorsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

	return OutputString;
}

void AAIScreenshotCapture::SendToAIBackend(const FString& ImagePath, const FString& MetadataJson)
{
	if (BackendURL.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Vision: Backend URL not set"));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(BackendURL + TEXT("/api/analyze_scene"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Create JSON payload
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetStringField(TEXT("image_path"), ImagePath);
	JsonObj->SetStringField(TEXT("metadata"), MetadataJson);
	JsonObj->SetStringField(TEXT("user_goal"), DevelopmentGoal);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

	Request->SetContentAsString(OutputString);

	// Bind response callback
	Request->OnProcessRequestComplete().BindUObject(this, &AAIScreenshotCapture::OnAIResponseReceived);

	// Send request
	Request->ProcessRequest();

	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Sent screenshot to AI backend"));
	}
}

void AAIScreenshotCapture::OnAIResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AI Vision: Failed to get response from AI backend"));
		return;
	}

	FString ResponseStr = Response->GetContentAsString();
	
	if (bDebugMode)
	{
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Received response from AI"));
		UE_LOG(LogTemp, Log, TEXT("Response: %s"), *ResponseStr);
	}

	// Parse JSON response
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
	{
		// Extract AI suggestions/actions
		FString Analysis = JsonObj->GetStringField(TEXT("analysis"));
		
		UE_LOG(LogTemp, Log, TEXT("AI Analysis: %s"), *Analysis);
		
		// TODO: Execute AI-generated actions via Python executor
		// This will be handled by the Python integration
	}
}

void AAIScreenshotCapture::StartAutoCapture(float IntervalSeconds)
{
	StopAutoCapture(); // Clear any existing timer

	GetWorld()->GetTimerManager().SetTimer(AutoCaptureTimer, [this]()
	{
		FString Filename = FString::Printf(TEXT("auto_capture_%d.png"), CaptureCounter++);
		CaptureViewport(Filename);
	}, IntervalSeconds, true);

	UE_LOG(LogTemp, Log, TEXT("AI Vision: Auto-capture started (interval: %.1f seconds)"), IntervalSeconds);
}

void AAIScreenshotCapture::StopAutoCapture()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoCaptureTimer);
		UE_LOG(LogTemp, Log, TEXT("AI Vision: Auto-capture stopped"));
	}
}

void AAIScreenshotCapture::SetBackendURL(const FString& URL)
{
	BackendURL = URL;
	UE_LOG(LogTemp, Log, TEXT("AI Vision: Backend URL set to %s"), *URL);
}
