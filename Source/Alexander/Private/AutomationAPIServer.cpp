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
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Networking.h"
#include "Async/Async.h"

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

	// Check for command line port override: -HTTPPort=8081
	FString CommandLine = FCommandLine::Get();
	FString PortStr;
	if (FParse::Value(*CommandLine, TEXT("HTTPPort="), PortStr))
	{
		int32 OverridePort = FCString::Atoi(*PortStr);
		if (OverridePort > 0 && OverridePort < 65536)
		{
			ListenPort = OverridePort;
			UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Using command line port override: %d"), ListenPort);
		}
	}

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

	// Create TCP listener
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("AutomationAPI: Failed to get socket subsystem"));
		return false;
	}

	// Create endpoint (listen on all interfaces, specified port)
	FIPv4Address Address = FIPv4Address::Any;
	FIPv4Endpoint Endpoint(Address, ListenPort);

	// Create TCP listener
	TcpListener = new FTcpListener(Endpoint);
	TcpListener->OnConnectionAccepted().BindUObject(this, &UAutomationAPIServer::OnIncomingConnection);

	bIsRunning = true;
	NextShipID = 1;
	TotalRequestsProcessed = 0;
	TotalProcessingTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: TCP server listening on port %d"), ListenPort);
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HTTP automation server ready"));

	return true;
}

void UAutomationAPIServer::StopServer()
{
	if (!bIsRunning)
		return;

	// Stop and delete TCP listener
	if (TcpListener)
	{
		TcpListener->Stop();
		delete TcpListener;
		TcpListener = nullptr;
	}

	bIsRunning = false;
	TrackedShips.Empty();

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Server stopped"));
}

bool UAutomationAPIServer::OnIncomingConnection(FSocket* Socket, const FIPv4Endpoint& Endpoint)
{
	if (!Socket)
		return false;

	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Incoming connection from %s"), *Endpoint.ToString());
	}

	// Process request on game thread
	// NOTE: This keeps it simple and avoids threading issues
	FFunctionGraphTask::CreateAndDispatchWhenReady(
		[this, Socket]()
		{
			ProcessSocketRequest(Socket);
		},
		TStatId(),
		nullptr,
		ENamedThreads::GameThread
	);

	return true;
}

void UAutomationAPIServer::ProcessSocketRequest(FSocket* Socket)
{
	if (!Socket)
		return;

	// Read HTTP request
	TArray<uint8> ReceivedData;
	uint8 Buffer[1024];
	int32 BytesRead = 0;

	// Read data from socket
	while (Socket->Recv(Buffer, sizeof(Buffer) - 1, BytesRead))
	{
		if (BytesRead > 0)
		{
			ReceivedData.Append(Buffer, BytesRead);
			// Simple check for end of HTTP headers
			if (ReceivedData.Num() > 4)
			{
				bool bFoundEnd = false;
				for (int32 i = ReceivedData.Num() - 4; i >= 0 && i >= ReceivedData.Num() - 100; i--)
				{
					if (ReceivedData[i] == '\r' && ReceivedData[i+1] == '\n' &&
						ReceivedData[i+2] == '\r' && ReceivedData[i+3] == '\n')
					{
						bFoundEnd = true;
						break;
					}
				}
				if (bFoundEnd) break;
			}
		}
		if (BytesRead == 0) break;
	}

	// Parse HTTP request
	FString RequestString = FString(UTF8_TO_TCHAR((char*)ReceivedData.GetData()));
	TArray<FString> Lines;
	RequestString.ParseIntoArrayLines(Lines);

	if (Lines.Num() == 0)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		return;
	}

	// Parse request line (e.g., "GET /status HTTP/1.1")
	TArray<FString> RequestParts;
	Lines[0].ParseIntoArray(RequestParts, TEXT(" "), true);

	if (RequestParts.Num() < 2)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		return;
	}

	FString Method = RequestParts[0];
	FString Endpoint = RequestParts[1];

	// Extract body for POST requests (comes after \r\n\r\n separator)
	FString Body = TEXT("");
	int32 HeaderEndIndex = RequestString.Find(TEXT("\r\n\r\n"));
	if (HeaderEndIndex != INDEX_NONE)
	{
		// Parse Content-Length header to know how many bytes to read
		int32 ContentLength = 0;
		FString ContentLengthHeader = TEXT("Content-Length:");
		int32 ContentLengthIndex = RequestString.Find(ContentLengthHeader);
		if (ContentLengthIndex != INDEX_NONE)
		{
			int32 LineEndIndex = RequestString.Find(TEXT("\r\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, ContentLengthIndex);
			if (LineEndIndex != INDEX_NONE)
			{
				FString LengthValue = RequestString.Mid(ContentLengthIndex + ContentLengthHeader.Len(), LineEndIndex - ContentLengthIndex - ContentLengthHeader.Len());
				LengthValue.TrimStartAndEndInline();
				ContentLength = FCString::Atoi(*LengthValue);
			}
		}

		// Extract body (limit to Content-Length if specified)
		FString FullBody = RequestString.Mid(HeaderEndIndex + 4);
		if (ContentLength > 0 && ContentLength < FullBody.Len())
		{
			Body = FullBody.Left(ContentLength);
		}
		else
		{
			Body = FullBody;
		}
		Body.TrimStartAndEndInline();
	}

	// Handle request on game thread (we're already on game thread, so call directly)
	FString Response;
	HandleHTTPRequest(Endpoint, Method, Body, Response);

	// Send HTTP response
	FString HttpResponse = FString::Printf(TEXT("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %d\r\n\r\n%s"),
		Response.Len(), *Response);

	// Convert to UTF8 for socket transmission
	FTCHARToUTF8 Converter(*HttpResponse);
	int32 BytesSent = 0;
	Socket->Send((uint8*)Converter.Get(), Converter.Length(), BytesSent);

	// Close socket
	Socket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
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
	else if (Method == TEXT("POST") && Endpoint == TEXT("/apply_thrust"))
	{
		OutResponse = HandleApplyThrust(Body);
	}
	else if (Method == TEXT("GET") && (Endpoint.StartsWith(TEXT("/get_position/")) || Endpoint.StartsWith(TEXT("/get_position?"))))
	{
		FString ShipID;
		if (Endpoint.StartsWith(TEXT("/get_position/")))
		{
			// Path parameter style: /get_position/ship_3
			ShipID = Endpoint.RightChop(14); // Remove "/get_position/"
		}
		else
		{
			// Query parameter style: /get_position?ship_id=ship_3
			int32 QueryStart = Endpoint.Find(TEXT("ship_id="));
			if (QueryStart != INDEX_NONE)
			{
				ShipID = Endpoint.RightChop(QueryStart + 8); // Remove everything up to and including "ship_id="
				// Remove any trailing parameters (e.g., &other=value)
				int32 AmpersandPos = ShipID.Find(TEXT("&"));
				if (AmpersandPos != INDEX_NONE)
				{
					ShipID = ShipID.Left(AmpersandPos);
				}
			}
		}
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
	else if (Method == TEXT("GET") && Endpoint == TEXT("/get_player_pawn"))
	{
		OutResponse = HandleGetPlayerPawn();
	}
	else if (Method == TEXT("DELETE") && Endpoint.StartsWith(TEXT("/destroy_ship/")))
	{
		FString ShipID = Endpoint.RightChop(14); // Remove "/destroy_ship/"
		OutResponse = HandleDestroyShip(ShipID);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/submit_observation"))
	{
		OutResponse = HandleSubmitObservation(Body);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/validate_position"))
	{
		OutResponse = HandleValidatePosition(Body);
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
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSpawnShip RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Failed to parse JSON from body: '%s'"), *RequestBody);
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Parse location (supports both object {x,y,z} and array [x,y,z] formats)
	FVector SpawnLocation = FVector::ZeroVector;
	if (JsonObj->HasField(TEXT("location")))
	{
		const TSharedPtr<FJsonObject>* LocationObj;
		if (JsonObj->TryGetObjectField(TEXT("location"), LocationObj))
		{
			// Object format: {"x": 0, "y": 0, "z": 500}
			SpawnLocation.X = (*LocationObj)->GetNumberField(TEXT("x"));
			SpawnLocation.Y = (*LocationObj)->GetNumberField(TEXT("y"));
			SpawnLocation.Z = (*LocationObj)->GetNumberField(TEXT("z"));
		}
		else
		{
			// Array format: [x, y, z]
			TArray<TSharedPtr<FJsonValue>> LocationArray = JsonObj->GetArrayField(TEXT("location"));
			if (LocationArray.Num() >= 3)
			{
				SpawnLocation.X = LocationArray[0]->AsNumber();
				SpawnLocation.Y = LocationArray[1]->AsNumber();
				SpawnLocation.Z = LocationArray[2]->AsNumber();
			}
		}
	}

	// Parse rotation (optional, supports both object and array formats)
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (JsonObj->HasField(TEXT("rotation")))
	{
		const TSharedPtr<FJsonObject>* RotationObj;
		if (JsonObj->TryGetObjectField(TEXT("rotation"), RotationObj))
		{
			// Object format: {"pitch": 0, "yaw": 0, "roll": 0}
			SpawnRotation.Pitch = (*RotationObj)->GetNumberField(TEXT("pitch"));
			SpawnRotation.Yaw = (*RotationObj)->GetNumberField(TEXT("yaw"));
			SpawnRotation.Roll = (*RotationObj)->GetNumberField(TEXT("roll"));
		}
		else
		{
			// Array format: [pitch, yaw, roll]
			TArray<TSharedPtr<FJsonValue>> RotationArray = JsonObj->GetArrayField(TEXT("rotation"));
			if (RotationArray.Num() >= 3)
			{
				SpawnRotation.Pitch = RotationArray[0]->AsNumber();
				SpawnRotation.Yaw = RotationArray[1]->AsNumber();
				SpawnRotation.Roll = RotationArray[2]->AsNumber();
			}
		}
	}

	// Get ship class
	FString ShipClassPath = JsonObj->GetStringField(TEXT("ship_class"));
	if (ShipClassPath.IsEmpty())
	{
		ShipClassPath = TEXT("/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C");
	}

	// Load ship class using StaticLoadClass (works for both C++ and Blueprint classes)
	UClass* ShipClass = StaticLoadClass(AActor::StaticClass(), nullptr, *ShipClassPath);
	if (!ShipClass)
	{
		// Try alternative loading method for C++ classes
		ShipClass = FindObject<UClass>(ANY_PACKAGE, *ShipClassPath);
	}

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
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetInput RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: HandleSetInput - Invalid JSON"));
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	FString ShipID = JsonObj->GetStringField(TEXT("ship_id"));
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetInput - Looking for ship: %s"), *ShipID);

	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: HandleSetInput - Ship not found: %s"), *ShipID);
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetInput - Found ship: %s"), *Ship->GetName());

	// List all components on the ship for debugging
	TArray<UActorComponent*> Components;
	Ship->GetComponents(Components);
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Ship has %d components:"), Components.Num());
	for (UActorComponent* Component : Components)
	{
		UE_LOG(LogTemp, Log, TEXT("  - %s (%s)"), *Component->GetName(), *Component->GetClass()->GetName());
	}

	// Get FlightController component (optional)
	UFlightController* FlightController = Ship->FindComponentByClass<UFlightController>();
	bool bUsingPhysics = (FlightController == nullptr);

	if (FlightController)
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetInput - Using FlightController"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetInput - Using direct physics control"));
	}

	// Get root primitive for physics control
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Ship->GetRootComponent());

	// Parse thrust input (supports both object {x,y,z} and array [x,y,z] formats)
	if (JsonObj->HasField(TEXT("thrust")))
	{
		FVector ThrustInput = FVector::ZeroVector;
		const TSharedPtr<FJsonObject>* ThrustObj;
		if (JsonObj->TryGetObjectField(TEXT("thrust"), ThrustObj))
		{
			// Object format: {"x": 1.0, "y": 0.0, "z": 0.0}
			ThrustInput.X = (*ThrustObj)->GetNumberField(TEXT("x"));
			ThrustInput.Y = (*ThrustObj)->GetNumberField(TEXT("y"));
			ThrustInput.Z = (*ThrustObj)->GetNumberField(TEXT("z"));
		}
		else
		{
			// Array format: [x, y, z]
			TArray<TSharedPtr<FJsonValue>> ThrustArray = JsonObj->GetArrayField(TEXT("thrust"));
			if (ThrustArray.Num() >= 3)
			{
				ThrustInput.X = ThrustArray[0]->AsNumber();
				ThrustInput.Y = ThrustArray[1]->AsNumber();
				ThrustInput.Z = ThrustArray[2]->AsNumber();
			}
		}

		if (FlightController)
		{
			FlightController->SetThrustInput(ThrustInput);
		}
		else if (RootPrim && RootPrim->IsSimulatingPhysics())
		{
			// Apply force directly (scale by mass for consistent acceleration)
			float ForceMagnitude = 100000.0f; // Newtons
			FVector WorldForce = Ship->GetActorRotation().RotateVector(ThrustInput) * ForceMagnitude;
			RootPrim->AddForce(WorldForce);
			UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Applied physics force: %s"), *WorldForce.ToString());
		}
	}

	// Parse rotation input (supports both object and array formats)
	if (JsonObj->HasField(TEXT("rotation")))
	{
		FVector RotationInput = FVector::ZeroVector;
		const TSharedPtr<FJsonObject>* RotationObj;
		if (JsonObj->TryGetObjectField(TEXT("rotation"), RotationObj))
		{
			// Object format: {"pitch": 0.0, "yaw": 0.0, "roll": 0.0}
			RotationInput.X = (*RotationObj)->GetNumberField(TEXT("pitch"));
			RotationInput.Y = (*RotationObj)->GetNumberField(TEXT("yaw"));
			RotationInput.Z = (*RotationObj)->GetNumberField(TEXT("roll"));
		}
		else
		{
			// Array format: [pitch, yaw, roll]
			TArray<TSharedPtr<FJsonValue>> RotationArray = JsonObj->GetArrayField(TEXT("rotation"));
			if (RotationArray.Num() >= 3)
			{
				RotationInput.X = RotationArray[0]->AsNumber();
				RotationInput.Y = RotationArray[1]->AsNumber();
				RotationInput.Z = RotationArray[2]->AsNumber();
			}
		}

		if (FlightController)
		{
			FlightController->SetRotationInput(RotationInput);
		}
		else if (RootPrim && RootPrim->IsSimulatingPhysics())
		{
			// Apply torque directly
			float TorqueMagnitude = 1000000.0f; // Newton-meters
			FVector WorldTorque = Ship->GetActorRotation().RotateVector(RotationInput) * TorqueMagnitude;
			RootPrim->AddTorqueInRadians(WorldTorque);
			UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Applied physics torque: %s"), *WorldTorque.ToString());
		}
	}

	// Parse assist mode (only works with FlightController)
	if (FlightController && JsonObj->HasField(TEXT("assist_mode")))
	{
		int32 AssistModeInt = JsonObj->GetIntegerField(TEXT("assist_mode"));
		EFlightAssistMode AssistMode = static_cast<EFlightAssistMode>(AssistModeInt);
		FlightController->SetAssistMode(AssistMode);
	}

	FString Method = bUsingPhysics ? TEXT("physics") : TEXT("FlightController");
	return CreateJSONResponse(true, FString::Printf(TEXT("Input applied via %s"), *Method));
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

FString UAutomationAPIServer::HandleGetPlayerPawn()
{
	// Get the first player controller
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return CreateJSONResponse(false, TEXT("No player controller found"));
	}

	// Get the pawn
	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		return CreateJSONResponse(false, TEXT("Player has no pawn"));
	}

	// Check if this pawn is already tracked
	FString ExistingID;
	for (const auto& Pair : TrackedShips)
	{
		if (Pair.Value == Pawn)
		{
			ExistingID = Pair.Key;
			break;
		}
	}

	// If not tracked, register it
	if (ExistingID.IsEmpty())
	{
		ExistingID = TEXT("player_pawn");
		RegisterShip(Pawn, ExistingID);
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Registered player pawn as '%s'"), *ExistingID);
	}

	// Build response with pawn info
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ExistingID);
	ResponseData->SetStringField(TEXT("pawn_name"), Pawn->GetName());
	ResponseData->SetStringField(TEXT("pawn_class"), Pawn->GetClass()->GetName());

	// Check for FlightController
	UFlightController* FlightController = Pawn->FindComponentByClass<UFlightController>();
	ResponseData->SetBoolField(TEXT("has_flight_controller"), FlightController != nullptr);

	// List all components
	TArray<UActorComponent*> Components;
	Pawn->GetComponents(Components);
	TArray<TSharedPtr<FJsonValue>> ComponentsArray;
	for (UActorComponent* Component : Components)
	{
		ComponentsArray.Add(MakeShareable(new FJsonValueString(Component->GetClass()->GetName())));
	}
	ResponseData->SetArrayField(TEXT("components"), ComponentsArray);

	FVector Location = Pawn->GetActorLocation();
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.X)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Y)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Z)));
	ResponseData->SetArrayField(TEXT("location"), LocationArray);

	return CreateJSONResponse(true, TEXT("Player pawn retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleApplyThrust(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleApplyThrust RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Get ship ID
	FString ShipID = JsonObj->GetStringField(TEXT("ship_id"));
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	// Get thrust values
	float ThrustX = JsonObj->HasField(TEXT("thrust_x")) ? JsonObj->GetNumberField(TEXT("thrust_x")) : 0.0f;
	float ThrustY = JsonObj->HasField(TEXT("thrust_y")) ? JsonObj->GetNumberField(TEXT("thrust_y")) : 0.0f;
	float ThrustZ = JsonObj->HasField(TEXT("thrust_z")) ? JsonObj->GetNumberField(TEXT("thrust_z")) : 0.0f;

	// Get primitive component (for physics)
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Ship->GetRootComponent());
	if (!RootPrimitive)
	{
		return CreateJSONResponse(false, TEXT("Ship has no physics component"));
	}

	if (!RootPrimitive->IsSimulatingPhysics())
	{
		return CreateJSONResponse(false, TEXT("Ship is not simulating physics"));
	}

	// Apply force (AddForce expects force in Newtons, not acceleration)
	FVector Force(ThrustX, ThrustY, ThrustZ);
	RootPrimitive->AddForce(Force, NAME_None, true);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Applied thrust [%f, %f, %f] to ship '%s'"),
		ThrustX, ThrustY, ThrustZ, *ShipID);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);

	TArray<TSharedPtr<FJsonValue>> ThrustArray;
	ThrustArray.Add(MakeShareable(new FJsonValueNumber(ThrustX)));
	ThrustArray.Add(MakeShareable(new FJsonValueNumber(ThrustY)));
	ThrustArray.Add(MakeShareable(new FJsonValueNumber(ThrustZ)));
	ResponseData->SetArrayField(TEXT("thrust_applied"), ThrustArray);

	return CreateJSONResponse(true, TEXT("Thrust applied successfully"), ResponseData);
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

FString UAutomationAPIServer::HandleSubmitObservation(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSubmitObservation RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Parse observation data
	int32 ObserverID = JsonObj->GetIntegerField(TEXT("observer_id"));
	int32 TargetID = JsonObj->GetIntegerField(TEXT("target_id"));

	// Parse direction vector [x, y, z]
	TArray<TSharedPtr<FJsonValue>> DirectionArray = JsonObj->GetArrayField(TEXT("direction"));
	FVector Direction = FVector::ZeroVector;
	if (DirectionArray.Num() >= 3)
	{
		Direction.X = DirectionArray[0]->AsNumber();
		Direction.Y = DirectionArray[1]->AsNumber();
		Direction.Z = DirectionArray[2]->AsNumber();
	}

	float Distance = JsonObj->GetNumberField(TEXT("distance"));
	float ScaleFactor = JsonObj->GetNumberField(TEXT("scale_factor"));
	float Timestamp = JsonObj->HasField(TEXT("timestamp")) ? JsonObj->GetNumberField(TEXT("timestamp")) : GetWorld()->GetTimeSeconds();

	// Create observation measurement
	FObserverMeasurement Measurement;
	Measurement.ObserverID = ObserverID;
	Measurement.TargetID = TargetID;
	Measurement.ObserverPosition = FVector::ZeroVector; // Observer at their own origin
	Measurement.Direction = Direction.GetSafeNormal();
	Measurement.Distance = Distance;
	Measurement.ScaleFactor = ScaleFactor;
	Measurement.Timestamp = Timestamp;

	// Store observation using atomic operations to avoid any reference invalidation
	// Each FindOrAdd call is atomic and safe
	StoredObservations.FindOrAdd(TargetID).Add(Measurement);

	// Generate observation ID
	int32 ObservationID = NextObservationID++;

	// Get count using atomic access
	int32 TotalObservations = StoredObservations.FindOrAdd(TargetID).Num();

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Stored observation %d for target %d from observer %d (total: %d)"),
		ObservationID, TargetID, ObserverID, TotalObservations);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetNumberField(TEXT("observation_id"), ObservationID);
	ResponseData->SetNumberField(TEXT("target_id"), TargetID);
	ResponseData->SetNumberField(TEXT("observer_id"), ObserverID);
	ResponseData->SetNumberField(TEXT("total_observations"), TotalObservations);

	return CreateJSONResponse(true, TEXT("Observation recorded"), ResponseData);
}

FString UAutomationAPIServer::HandleValidatePosition(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleValidatePosition RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	int32 TargetID = JsonObj->GetIntegerField(TEXT("target_id"));

	// Parse observations array
	TArray<TSharedPtr<FJsonValue>> ObservationsArray = JsonObj->GetArrayField(TEXT("observations"));
	TArray<FObserverMeasurement> Measurements;

	for (const TSharedPtr<FJsonValue>& ObsValue : ObservationsArray)
	{
		const TSharedPtr<FJsonObject>* ObsObj;
		if (ObsValue->TryGetObject(ObsObj))
		{
			FObserverMeasurement Measurement;
			Measurement.ObserverID = (*ObsObj)->GetIntegerField(TEXT("observer_id"));
			Measurement.TargetID = TargetID;

			// Parse direction
			TArray<TSharedPtr<FJsonValue>> DirArray = (*ObsObj)->GetArrayField(TEXT("direction"));
			if (DirArray.Num() >= 3)
			{
				Measurement.Direction.X = DirArray[0]->AsNumber();
				Measurement.Direction.Y = DirArray[1]->AsNumber();
				Measurement.Direction.Z = DirArray[2]->AsNumber();
				Measurement.Direction.Normalize();
			}

			Measurement.Distance = (*ObsObj)->GetNumberField(TEXT("distance"));
			Measurement.ScaleFactor = (*ObsObj)->GetNumberField(TEXT("scale_factor"));
			Measurement.ObserverPosition = FVector::ZeroVector; // Each at own origin

			Measurements.Add(Measurement);
		}
	}

	if (Measurements.Num() < 2)
	{
		return CreateJSONResponse(false, TEXT("Need at least 2 observations for validation"));
	}

	// Create TriangulationValidator
	UTriangulationValidator* Validator = NewObject<UTriangulationValidator>();
	if (!Validator)
	{
		return CreateJSONResponse(false, TEXT("Failed to create TriangulationValidator"));
	}

	// Validate position
	FGeometricValidationResult ValidationResult = Validator->ValidatePosition(Measurements);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Validation result - Valid: %s, Confidence: %.2f, Error: %.2f, Method: %s"),
		ValidationResult.bIsValid ? TEXT("TRUE") : TEXT("FALSE"),
		ValidationResult.Confidence,
		ValidationResult.GeometricError,
		*ValidationResult.ValidationMethod);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetBoolField(TEXT("valid"), ValidationResult.bIsValid);
	ResponseData->SetNumberField(TEXT("confidence"), ValidationResult.Confidence);
	ResponseData->SetNumberField(TEXT("observer_count"), ValidationResult.ObserverCount);
	ResponseData->SetNumberField(TEXT("geometric_error"), ValidationResult.GeometricError);
	ResponseData->SetStringField(TEXT("validation_method"), ValidationResult.ValidationMethod);

	// Triangulated position
	TArray<TSharedPtr<FJsonValue>> PositionArray;
	PositionArray.Add(MakeShareable(new FJsonValueNumber(ValidationResult.TriangulatedPosition.X)));
	PositionArray.Add(MakeShareable(new FJsonValueNumber(ValidationResult.TriangulatedPosition.Y)));
	PositionArray.Add(MakeShareable(new FJsonValueNumber(ValidationResult.TriangulatedPosition.Z)));
	ResponseData->SetArrayField(TEXT("triangulated_position"), PositionArray);

	return CreateJSONResponse(ValidationResult.bIsValid,
		ValidationResult.bIsValid ? TEXT("Position validated") : TEXT("Position validation failed"),
		ResponseData);
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
