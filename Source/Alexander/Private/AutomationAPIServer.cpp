// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutomationAPIServer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HighResScreenshot.h"
#include "FlightController.h"

UAutomationAPIServer::UAutomationAPIServer()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.016f; // ~60fps

	bServerEnabled = true;
	bVerboseLogging = true;
	ListenPort = 8080;
	MaxRequestsPerSecond = 60;
}

void UAutomationAPIServer::BeginPlay()
{
	Super::BeginPlay();

	if (bServerEnabled)
	{
		StartServer();
	}
}

void UAutomationAPIServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopServer();
	Super::EndPlay(EndPlayReason);
}

void UAutomationAPIServer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRunning)
		return;

	// Clean up destroyed ships
	TArray<FString> ShipsToRemove;
	for (const auto& Pair : TrackedShips)
	{
		if (!IsValid(Pair.Value))
		{
			ShipsToRemove.Add(Pair.Key);
		}
	}

	for (const FString& ShipID : ShipsToRemove)
	{
		TrackedShips.Remove(ShipID);
		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Removed destroyed ship %s"), *ShipID);
		}
	}
}

bool UAutomationAPIServer::StartServer()
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Server already running"));
		return false;
	}

	bIsRunning = true;
	NextShipID = 1;
	TotalRequestsProcessed = 0;
	TotalProcessingTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Server started on port %d"), ListenPort);
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Console commands ready for automation"));

	return true;
}

void UAutomationAPIServer::StopServer()
{
	if (!bIsRunning)
		return;

	bIsRunning = false;
	TrackedShips.Empty();

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Server stopped"));
}

FString UAutomationAPIServer::GetServerStatus() const
{
	TSharedPtr<FJsonObject> StatusObj = MakeShareable(new FJsonObject);
	StatusObj->SetBoolField(TEXT("running"), bIsRunning);
	StatusObj->SetNumberField(TEXT("port"), ListenPort);
	StatusObj->SetNumberField(TEXT("tracked_ships"), TrackedShips.Num());
	StatusObj->SetNumberField(TEXT("total_requests"), TotalRequestsProcessed);
	StatusObj->SetNumberField(TEXT("avg_processing_time_ms"),
		TotalRequestsProcessed > 0 ? (TotalProcessingTime / TotalRequestsProcessed) * 1000.0f : 0.0f);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(StatusObj.ToSharedRef(), Writer);

	return OutputString;
}

void UAutomationAPIServer::HandleHTTPRequest(const FString& Endpoint, const FString& Method, const FString& Body, FString& OutResponse)
{
	if (!CheckRateLimit())
	{
		OutResponse = CreateJSONResponse(false, TEXT("Rate limit exceeded"));
		return;
	}

	double StartTime = FPlatformTime::Seconds();

	// Route requests
	if (Method == TEXT("POST") && Endpoint == TEXT("/spawn_ship"))
	{
		OutResponse = HandleSpawnShip(Body);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/set_input"))
	{
		OutResponse = HandleSetInput(Body);
	}
	else if (Method == TEXT("GET") && Endpoint.StartsWith(TEXT("/get_position/")))
	{
		FString ShipID = Endpoint.RightChop(14); // Remove "/get_position/"
		OutResponse = HandleGetPosition(ShipID);
	}
	else if (Method == TEXT("GET") && Endpoint.StartsWith(TEXT("/get_velocity/")))
	{
		FString ShipID = Endpoint.RightChop(14); // Remove "/get_velocity/"
		OutResponse = HandleGetVelocity(ShipID);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/screenshot"))
	{
		OutResponse = HandleScreenshot(Body);
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/status"))
	{
		OutResponse = HandleStatus();
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/list_ships"))
	{
		OutResponse = HandleListShips();
	}
	else if (Method == TEXT("DELETE") && Endpoint.StartsWith(TEXT("/destroy_ship/")))
	{
		FString ShipID = Endpoint.RightChop(14); // Remove "/destroy_ship/"
		OutResponse = HandleDestroyShip(ShipID);
	}
	else
	{
		OutResponse = CreateJSONResponse(false, FString::Printf(TEXT("Unknown endpoint: %s %s"), *Method, *Endpoint));
	}

	float ProcessingTime = FPlatformTime::Seconds() - StartTime;
	TotalRequestsProcessed++;
	TotalProcessingTime += ProcessingTime;

	LogRequest(Endpoint, Method, ProcessingTime);
}

FString UAutomationAPIServer::HandleSpawnShip(const FString& RequestBody)
{
	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Parse location
	TArray<TSharedPtr<FJsonValue>> LocationArray = JsonObj->GetArrayField(TEXT("location"));
	FVector SpawnLocation = FVector::ZeroVector;
	if (LocationArray.Num() >= 3)
	{
		SpawnLocation.X = LocationArray[0]->AsNumber();
		SpawnLocation.Y = LocationArray[1]->AsNumber();
		SpawnLocation.Z = LocationArray[2]->AsNumber();
	}

	// Parse rotation (optional)
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (JsonObj->HasField(TEXT("rotation")))
	{
		TArray<TSharedPtr<FJsonValue>> RotationArray = JsonObj->GetArrayField(TEXT("rotation"));
		if (RotationArray.Num() >= 3)
		{
			SpawnRotation.Pitch = RotationArray[0]->AsNumber();
			SpawnRotation.Yaw = RotationArray[1]->AsNumber();
			SpawnRotation.Roll = RotationArray[2]->AsNumber();
		}
	}

	// Get ship class
	FString ShipClassPath = JsonObj->GetStringField(TEXT("ship_class"));
	if (ShipClassPath.IsEmpty())
	{
		ShipClassPath = TEXT("/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C");
	}

	// Load ship class
	UClass* ShipClass = LoadObject<UClass>(nullptr, *ShipClassPath);
	if (!ShipClass || !ValidateShipClass(ShipClass))
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Failed to load ship class: %s"), *ShipClassPath));
	}

	// Spawn ship
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Ship = GetWorld()->SpawnActor<AActor>(ShipClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Ship)
	{
		return CreateJSONResponse(false, TEXT("Failed to spawn ship"));
	}

	// Register ship
	FString ShipID = GenerateShipID();
	RegisterShip(Ship, ShipID);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);
	ResponseData->SetStringField(TEXT("ship_name"), Ship->GetName());

	TArray<TSharedPtr<FJsonValue>> LocationResponseArray;
	LocationResponseArray.Add(MakeShareable(new FJsonValueNumber(SpawnLocation.X)));
	LocationResponseArray.Add(MakeShareable(new FJsonValueNumber(SpawnLocation.Y)));
	LocationResponseArray.Add(MakeShareable(new FJsonValueNumber(SpawnLocation.Z)));
	ResponseData->SetArrayField(TEXT("location"), LocationResponseArray);

	return CreateJSONResponse(true, FString::Printf(TEXT("Ship spawned: %s"), *ShipID), ResponseData);
}

FString UAutomationAPIServer::HandleSetInput(const FString& RequestBody)
{
	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	FString ShipID = JsonObj->GetStringField(TEXT("ship_id"));
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	// Get FlightController component
	UFlightController* FlightController = Ship->FindComponentByClass<UFlightController>();
	if (!FlightController)
	{
		return CreateJSONResponse(false, TEXT("Ship has no FlightController component"));
	}

	// Parse thrust input
	if (JsonObj->HasField(TEXT("thrust")))
	{
		TArray<TSharedPtr<FJsonValue>> ThrustArray = JsonObj->GetArrayField(TEXT("thrust"));
		if (ThrustArray.Num() >= 3)
		{
			FVector ThrustInput(
				ThrustArray[0]->AsNumber(),
				ThrustArray[1]->AsNumber(),
				ThrustArray[2]->AsNumber()
			);
			FlightController->SetThrustInput(ThrustInput);
		}
	}

	// Parse rotation input
	if (JsonObj->HasField(TEXT("rotation")))
	{
		TArray<TSharedPtr<FJsonValue>> RotationArray = JsonObj->GetArrayField(TEXT("rotation"));
		if (RotationArray.Num() >= 3)
		{
			FVector RotationInput(
				RotationArray[0]->AsNumber(),
				RotationArray[1]->AsNumber(),
				RotationArray[2]->AsNumber()
			);
			FlightController->SetRotationInput(RotationInput);
		}
	}

	// Parse assist mode
	if (JsonObj->HasField(TEXT("assist_mode")))
	{
		int32 AssistModeInt = JsonObj->GetIntegerField(TEXT("assist_mode"));
		EFlightAssistMode AssistMode = static_cast<EFlightAssistMode>(AssistModeInt);
		FlightController->SetAssistMode(AssistMode);
	}

	return CreateJSONResponse(true, TEXT("Input applied"));
}

FString UAutomationAPIServer::HandleGetPosition(const FString& ShipID)
{
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	FVector Location = Ship->GetActorLocation();

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.X)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Y)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Z)));
	ResponseData->SetArrayField(TEXT("position"), LocationArray);

	return CreateJSONResponse(true, TEXT("Position retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleGetVelocity(const FString& ShipID)
{
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	FVector Velocity = Ship->GetVelocity();
	float Speed = Velocity.Size();

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> VelocityArray;
	VelocityArray.Add(MakeShareable(new FJsonValueNumber(Velocity.X)));
	VelocityArray.Add(MakeShareable(new FJsonValueNumber(Velocity.Y)));
	VelocityArray.Add(MakeShareable(new FJsonValueNumber(Velocity.Z)));
	ResponseData->SetArrayField(TEXT("velocity"), VelocityArray);
	ResponseData->SetNumberField(TEXT("speed"), Speed);

	return CreateJSONResponse(true, TEXT("Velocity retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleScreenshot(const FString& RequestBody)
{
	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	FString Filename = JsonObj->GetStringField(TEXT("filename"));
	if (Filename.IsEmpty())
	{
		Filename = FString::Printf(TEXT("automation_screenshot_%s"), *FDateTime::Now().ToString());
	}

	// Request screenshot
	FScreenshotRequest::RequestScreenshot(Filename, false, false);

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("filename"), Filename);

	return CreateJSONResponse(true, TEXT("Screenshot requested"), ResponseData);
}

FString UAutomationAPIServer::HandleStatus()
{
	return GetServerStatus();
}

FString UAutomationAPIServer::HandleListShips()
{
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ShipsArray;

	for (const auto& Pair : TrackedShips)
	{
		if (IsValid(Pair.Value))
		{
			TSharedPtr<FJsonObject> ShipObj = MakeShareable(new FJsonObject);
			ShipObj->SetStringField(TEXT("ship_id"), Pair.Key);
			ShipObj->SetStringField(TEXT("ship_name"), Pair.Value->GetName());

			FVector Location = Pair.Value->GetActorLocation();
			TArray<TSharedPtr<FJsonValue>> LocationArray;
			LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.X)));
			LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Y)));
			LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Z)));
			ShipObj->SetArrayField(TEXT("location"), LocationArray);

			ShipsArray.Add(MakeShareable(new FJsonValueObject(ShipObj)));
		}
	}

	ResponseData->SetArrayField(TEXT("ships"), ShipsArray);
	ResponseData->SetNumberField(TEXT("count"), ShipsArray.Num());

	return CreateJSONResponse(true, TEXT("Ships listed"), ResponseData);
}

FString UAutomationAPIServer::HandleDestroyShip(const FString& ShipID)
{
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	Ship->Destroy();
	UnregisterShip(ShipID);

	return CreateJSONResponse(true, FString::Printf(TEXT("Ship destroyed: %s"), *ShipID));
}

void UAutomationAPIServer::RegisterShip(AActor* Ship, const FString& ShipID)
{
	if (Ship)
	{
		TrackedShips.Add(ShipID, Ship);
		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Registered ship %s (%s)"), *ShipID, *Ship->GetName());
		}
	}
}

void UAutomationAPIServer::UnregisterShip(const FString& ShipID)
{
	TrackedShips.Remove(ShipID);
	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Unregistered ship %s"), *ShipID);
	}
}

AActor* UAutomationAPIServer::GetShipByID(const FString& ShipID)
{
	AActor** FoundShip = TrackedShips.Find(ShipID);
	return (FoundShip && IsValid(*FoundShip)) ? *FoundShip : nullptr;
}

TArray<AActor*> UAutomationAPIServer::GetAllShips()
{
	TArray<AActor*> Ships;
	for (const auto& Pair : TrackedShips)
	{
		if (IsValid(Pair.Value))
		{
			Ships.Add(Pair.Value);
		}
	}
	return Ships;
}

TSharedPtr<FJsonObject> UAutomationAPIServer::ParseJSON(const FString& JSONString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		return JsonObject;
	}

	return nullptr;
}

FString UAutomationAPIServer::CreateJSONResponse(bool bSuccess, const FString& Message, const TSharedPtr<FJsonObject>& Data)
{
	TSharedPtr<FJsonObject> ResponseObj = MakeShareable(new FJsonObject);
	ResponseObj->SetBoolField(TEXT("success"), bSuccess);
	ResponseObj->SetStringField(TEXT("message"), Message);
	ResponseObj->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());

	if (Data.IsValid())
	{
		ResponseObj->SetObjectField(TEXT("data"), Data);
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);

	return OutputString;
}

bool UAutomationAPIServer::CheckRateLimit()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Reset counter every second
	if (CurrentTime - LastRequestTime >= 1.0f)
	{
		RequestsThisSecond = 0;
		LastRequestTime = CurrentTime;
	}

	RequestsThisSecond++;
	return RequestsThisSecond <= MaxRequestsPerSecond;
}

void UAutomationAPIServer::LogRequest(const FString& Endpoint, const FString& Method, float ProcessingTime)
{
	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: %s %s - %.2fms"),
			*Method, *Endpoint, ProcessingTime * 1000.0f);
	}
}

FString UAutomationAPIServer::GenerateShipID()
{
	return FString::Printf(TEXT("ship_%d"), NextShipID++);
}

bool UAutomationAPIServer::ValidateShipClass(UClass* ShipClass)
{
	return ShipClass && ShipClass->IsChildOf(AActor::StaticClass());
}
