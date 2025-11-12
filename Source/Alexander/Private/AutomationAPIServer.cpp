// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutomationAPIServer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HighResScreenshot.h"
#include "FlightController.h"
#include "ShipCustomizationComponent.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Networking.h"
#include "Async/Async.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Engine/Texture2D.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#endif

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

#if WITH_EDITOR
	// Register PIE state change callback to clean up ships when PIE ends
	FEditorDelegates::EndPIE.AddUObject(this, &UAutomationAPIServer::OnPIEEnded);
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Registered PIE end callback"));
#endif
}

void UAutomationAPIServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if WITH_EDITOR
	// Unregister PIE state change callback
	FEditorDelegates::EndPIE.RemoveAll(this);
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Unregistered PIE end callback"));
#endif

	StopServer();
	Super::EndPlay(EndPlayReason);
}

void UAutomationAPIServer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRunning)
		return;

	// Clean up destroyed ships (thread-safe)
	{
		FScopeLock Lock(&TrackedShipsLock);
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

	try
	{
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

		// Add null terminator for safe string conversion
		ReceivedData.Add(0);

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
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("AutomationAPI: Exception in ProcessSocketRequest - closing socket"));
		if (Socket)
		{
			Socket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		}
	}
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

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleHTTPRequest called - Method='%s', Endpoint='%s'"), *Method, *Endpoint);

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
	else if (Method == TEXT("POST") && Endpoint == TEXT("/possess"))
	{
		OutResponse = HandlePossess(Body);
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
	else if (Method == TEXT("POST") && Endpoint == TEXT("/apply_ship_customization"))
	{
		OutResponse = HandleApplyShipCustomization(Body);
	}
	else if (Method == TEXT("GET") && Endpoint.StartsWith(TEXT("/get_ship_customization/")))
	{
		FString ShipID = Endpoint.RightChop(24); // Remove "/get_ship_customization/"
		OutResponse = HandleGetShipCustomization(ShipID);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/equip_ship_part"))
	{
		OutResponse = HandleEquipShipPart(Body);
	}
	else if (Method == TEXT("GET") && Endpoint.StartsWith(TEXT("/get_ship_loadout/")))
	{
		FString ShipID = Endpoint.RightChop(18); // Remove "/get_ship_loadout/"
		OutResponse = HandleGetShipLoadout(ShipID);
	}
	// NEW ENDPOINTS FOR AUTONOMOUS PLAYTESTING
	else if (Method == TEXT("POST") && Endpoint == TEXT("/receive_screenshot"))
	{
		OutResponse = HandleReceiveScreenshot(Body);
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/game_state"))
	{
		OutResponse = HandleGetGameState();
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/execute_command"))
	{
		OutResponse = HandleExecuteCommand(Body);
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/performance_metrics"))
	{
		OutResponse = HandlePerformanceMetrics();
	}
	// STREAMING OPTIMIZATION ENDPOINTS
	else if (Method == TEXT("GET") && Endpoint == TEXT("/streaming_metrics"))
	{
		OutResponse = HandleStreamingMetrics();
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/set_streaming_strategy"))
	{
		OutResponse = HandleSetStreamingStrategy(Body);
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/asset_loading_status"))
	{
		OutResponse = HandleAssetLoadingStatus();
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/texture_memory_stats"))
	{
		OutResponse = HandleTextureMemoryStats();
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/mesh_memory_stats"))
	{
		OutResponse = HandleMeshMemoryStats();
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/force_memory_optimization"))
	{
		OutResponse = HandleForceMemoryOptimization(Body);
	}
	// TICK OPTIMIZATION ENDPOINTS
	else if (Method == TEXT("GET") && Endpoint == TEXT("/tick_stats"))
	{
		OutResponse = HandleTickStats();
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/set_actor_priority"))
	{
		OutResponse = HandleSetActorPriority(Body);
	}
	else if (Method == TEXT("GET") && Endpoint == TEXT("/dormant_actors"))
	{
		OutResponse = HandleDormantActors();
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/reset_tick_optimization"))
	{
		OutResponse = HandleResetTickOptimization(Body);
	}
#if WITH_EDITOR
	// ============================================================================
	// MATERIAL & TEXTURE QUERY ROUTING
	// ============================================================================
	else if ((Method == TEXT("GET") || Method == TEXT("POST")) && Endpoint == TEXT("/materials/list"))
	{
		OutResponse = HandleMaterialsList(Body);
	}
	else if (Method == TEXT("GET") && Endpoint.StartsWith(TEXT("/materials/get_properties/")))
	{
		FString MaterialPath = Endpoint.RightChop(26); // Remove "/materials/get_properties/"
		OutResponse = HandleGetMaterialProperties(MaterialPath);
	}
	else if ((Method == TEXT("GET") || Method == TEXT("POST")) && Endpoint == TEXT("/textures/list"))
	{
		OutResponse = HandleTexturesList(Body);
	}
	// ============================================================================
	// PIE CONTROL ROUTING
	// ============================================================================
	else if (Method == TEXT("POST") && Endpoint == TEXT("/pie/start"))
	{
		OutResponse = HandlePlayInEditor(Body);
	}
	else if (Method == TEXT("POST") && Endpoint == TEXT("/pie/stop"))
	{
		OutResponse = HandleStopPlaying();
	}
#endif // WITH_EDITOR
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

	// Find active game world (PIE world required for spawning)
	UWorld* World = nullptr;

#if WITH_EDITOR
	// In editor, look for PIE world only (editor world doesn't support gameplay spawning)
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("PIE not running - start PIE first with POST /pie/start"));
	}
#else
	// In packaged game, use GetWorld()
	World = GetWorld();
	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}
#endif

	// Spawn ship
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Ship = World->SpawnActor<AActor>(ShipClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Ship)
	{
		return CreateJSONResponse(false, TEXT("Failed to spawn ship"));
	}

	// Register ship
	FString ShipID = GenerateShipID();
	UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: About to register ship %s at %p, TrackedShips.Num() before = %d"), *ShipID, Ship, TrackedShips.Num());
	RegisterShip(Ship, ShipID);
	UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: After RegisterShip, TrackedShips.Num() = %d"), TrackedShips.Num());

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
	// Acquire lock, find ship, get location, release lock (minimize lock time)
	FVector Location = FVector::ZeroVector;
	bool bShipFound = false;
	{
		FScopeLock Lock(&TrackedShipsLock);
		AActor** FoundShip = TrackedShips.Find(ShipID);
		if (FoundShip && IsValid(*FoundShip))
		{
			Location = (*FoundShip)->GetActorLocation();
			bShipFound = true;
		}
	}
	// Lock released here

	if (!bShipFound)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> PositionObj = MakeShareable(new FJsonObject);
	PositionObj->SetNumberField(TEXT("x"), Location.X);
	PositionObj->SetNumberField(TEXT("y"), Location.Y);
	PositionObj->SetNumberField(TEXT("z"), Location.Z);
	ResponseData->SetObjectField(TEXT("position"), PositionObj);

	return CreateJSONResponse(true, TEXT("Position retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleGetVelocity(const FString& ShipID)
{
	// Acquire lock, find ship, get velocity, release lock (minimize lock time)
	FVector Velocity = FVector::ZeroVector;
	float Speed = 0.0f;
	bool bShipFound = false;
	{
		FScopeLock Lock(&TrackedShipsLock);
		AActor** FoundShip = TrackedShips.Find(ShipID);
		if (FoundShip && IsValid(*FoundShip))
		{
			Velocity = (*FoundShip)->GetVelocity();
			Speed = Velocity.Size();
			bShipFound = true;
		}
	}
	// Lock released here

	if (!bShipFound)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> VelocityObj = MakeShareable(new FJsonObject);
	VelocityObj->SetNumberField(TEXT("x"), Velocity.X);
	VelocityObj->SetNumberField(TEXT("y"), Velocity.Y);
	VelocityObj->SetNumberField(TEXT("z"), Velocity.Z);
	ResponseData->SetObjectField(TEXT("velocity"), VelocityObj);
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

	// Extract just the base filename (Unreal ignores directory paths in screenshot requests)
	FString BaseFilename = FPaths::GetBaseFilename(Filename);

	// Request screenshot
	FScreenshotRequest::RequestScreenshot(BaseFilename, false, false);

	// Build expected file path where Unreal will save it
	FString ProjectDir = FPaths::ProjectDir();
	FString ScreenshotDir = FPaths::Combine(ProjectDir, TEXT("Saved"), TEXT("Screenshots"));

#if WITH_EDITOR
	ScreenshotDir = FPaths::Combine(ScreenshotDir, TEXT("WindowsEditor"));
#else
	ScreenshotDir = FPaths::Combine(ScreenshotDir, TEXT("Windows"));
#endif

	FString ExpectedPath = FPaths::Combine(ScreenshotDir, BaseFilename + TEXT(".png"));

	// Poll for file existence (screenshot is asynchronous)
	IFileManager& FileManager = IFileManager::Get();
	int32 MaxAttempts = 50; // 5 seconds total (50 * 100ms)
	bool bFileExists = false;

	for (int32 i = 0; i < MaxAttempts; ++i)
	{
		if (FileManager.FileExists(*ExpectedPath))
		{
			bFileExists = true;
			break;
		}
		FPlatformProcess::Sleep(0.1f); // Wait 100ms between checks
	}

	if (!bFileExists)
	{
		UE_LOG(LogTemp, Warning, TEXT("Screenshot file not found after 5s: %s"), *ExpectedPath);
		return CreateJSONResponse(false, FString::Printf(TEXT("Screenshot timeout - file not created: %s"), *ExpectedPath));
	}

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("filename"), BaseFilename);
	ResponseData->SetStringField(TEXT("full_path"), ExpectedPath);
	ResponseData->SetBoolField(TEXT("file_exists"), true);

	return CreateJSONResponse(true, TEXT("Screenshot captured and verified"), ResponseData);
}

FString UAutomationAPIServer::HandleStatus()
{
	return GetServerStatus();
}

FString UAutomationAPIServer::HandleListShips()
{
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ShipsArray;

	// Copy TrackedShips map quickly while holding lock
	TMap<FString, AActor*> ShipsCopy;
	{
		FScopeLock Lock(&TrackedShipsLock);
		ShipsCopy = TrackedShips;
		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleListShips - Copied %d ships"), ShipsCopy.Num());
		}
	}
	// Lock released here - access ship properties without holding lock

	// Iterate through copy and access ship properties
	for (const auto& Pair : ShipsCopy)
	{
		// Enhanced logging for debugging
		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Ship %s - Actor ptr: %p, IsValid: %d"),
				*Pair.Key,
				Pair.Value,
				IsValid(Pair.Value) ? 1 : 0);
		}

		if (IsValid(Pair.Value))
		{
			try
			{
				TSharedPtr<FJsonObject> ShipObj = MakeShareable(new FJsonObject);
				ShipObj->SetStringField(TEXT("ship_id"), Pair.Key);

				FString ShipName = Pair.Value->GetName();
				ShipObj->SetStringField(TEXT("ship_name"), ShipName);

				FVector Location = Pair.Value->GetActorLocation();

				TArray<TSharedPtr<FJsonValue>> LocationArray;
				LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.X)));
				LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Y)));
				LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Z)));
				ShipObj->SetArrayField(TEXT("location"), LocationArray);

				ShipsArray.Add(MakeShareable(new FJsonValueObject(ShipObj)));

				if (bVerboseLogging)
				{
					UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: Successfully added ship %s to response array"), *Pair.Key);
				}
			}
			catch (...)
			{
				UE_LOG(LogTemp, Error, TEXT("AutomationAPI: Exception accessing ship %s"), *Pair.Key);
			}
		}
	}

	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: HandleListShips - Final ShipsArray.Num() = %d"), ShipsArray.Num());
	}

	ResponseData->SetArrayField(TEXT("ships"), ShipsArray);
	ResponseData->SetNumberField(TEXT("count"), ShipsArray.Num());

	return CreateJSONResponse(true, TEXT("Ships listed"), ResponseData);
}

FString UAutomationAPIServer::HandleGetPlayerPawn()
{
	// Find PIE world (same pattern as HandleSpawnShip)
	UWorld* World = nullptr;

#if WITH_EDITOR
	// In editor, look for PIE world
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	// In packaged builds
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("PIE world not found - start PIE first"));
	}

	// Get the first player controller
	APlayerController* PC = World->GetFirstPlayerController();
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

FString UAutomationAPIServer::HandlePossess(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandlePossess RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Get target pawn ID
	FString TargetID = JsonObj->GetStringField(TEXT("target_id"));

	// Get the target pawn/ship from tracked ships
	AActor* TargetActor = GetShipByID(TargetID);
	if (!TargetActor)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Target not found: %s"), *TargetID));
	}

	// Cast to Pawn (since we need APawn for possession)
	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!TargetPawn)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Target '%s' is not a Pawn (class: %s)"), *TargetID, *TargetActor->GetClass()->GetName()));
	}

	// Get the first player controller
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return CreateJSONResponse(false, TEXT("No player controller found"));
	}

	// Get current pawn before possession
	APawn* OldPawn = PC->GetPawn();
	FString OldPawnName = OldPawn ? OldPawn->GetName() : TEXT("None");

	// Perform possession
	PC->Possess(TargetPawn);

	// Give it a frame to take effect
	FPlatformProcess::Sleep(0.1f);

	// Verify possession succeeded
	APawn* NewPawn = PC->GetPawn();
	if (NewPawn != TargetPawn)
	{
		return CreateJSONResponse(false, TEXT("Possession failed - controller did not possess target"));
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Player controller possessed '%s' (was: '%s')"), *TargetPawn->GetName(), *OldPawnName);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("target_id"), TargetID);
	ResponseData->SetStringField(TEXT("target_name"), TargetPawn->GetName());
	ResponseData->SetStringField(TEXT("old_pawn_name"), OldPawnName);
	ResponseData->SetStringField(TEXT("target_class"), TargetPawn->GetClass()->GetName());

	// Get new position
	FVector Location = TargetPawn->GetActorLocation();
	TArray<TSharedPtr<FJsonValue>> LocationArray;
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.X)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Y)));
	LocationArray.Add(MakeShareable(new FJsonValueNumber(Location.Z)));
	ResponseData->SetArrayField(TEXT("location"), LocationArray);

	return CreateJSONResponse(true, FString::Printf(TEXT("Possessed '%s'"), *TargetPawn->GetName()), ResponseData);
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
	StoredObservations.FindOrAdd(TargetID).Measurements.Add(Measurement);

	// Generate observation ID
	int32 ObservationID = NextObservationID++;

	// Get count using atomic access
	int32 TotalObservations = StoredObservations.FindOrAdd(TargetID).Measurements.Num();

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

FString UAutomationAPIServer::HandleApplyShipCustomization(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleApplyShipCustomization RequestBody: '%s'"), *RequestBody);

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

	// Get ShipCustomizationComponent
	UShipCustomizationComponent* CustomizationComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
	if (!CustomizationComp)
	{
		return CreateJSONResponse(false, TEXT("Ship has no ShipCustomizationComponent"));
	}

	// Parse customization parameters
	FShipStats CustomStats;
	bool bStatsModified = false;

	if (JsonObj->HasField(TEXT("mass")))
	{
		CustomStats.Mass = JsonObj->GetNumberField(TEXT("mass"));
		bStatsModified = true;
	}

	if (JsonObj->HasField(TEXT("thrust_power")))
	{
		CustomStats.ThrustPower = JsonObj->GetNumberField(TEXT("thrust_power"));
		bStatsModified = true;
	}

	if (JsonObj->HasField(TEXT("max_velocity")))
	{
		CustomStats.MaxVelocity = JsonObj->GetNumberField(TEXT("max_velocity"));
		bStatsModified = true;
	}

	if (JsonObj->HasField(TEXT("rotation_speed")))
	{
		CustomStats.RotationSpeed = JsonObj->GetNumberField(TEXT("rotation_speed"));
		bStatsModified = true;
	}

	if (JsonObj->HasField(TEXT("acceleration")))
	{
		CustomStats.Acceleration = JsonObj->GetNumberField(TEXT("acceleration"));
		bStatsModified = true;
	}

	if (!bStatsModified)
	{
		return CreateJSONResponse(false, TEXT("No customization stats provided"));
	}

	// Apply stats to the loadout
	CustomizationComp->CurrentLoadout.TotalStats = CustomStats;
	CustomizationComp->ApplyStatsToFlightController();

	// Build response with applied stats
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);
	ResponseData->SetNumberField(TEXT("mass"), CustomStats.Mass);
	ResponseData->SetNumberField(TEXT("thrust_power"), CustomStats.ThrustPower);
	ResponseData->SetNumberField(TEXT("max_velocity"), CustomStats.MaxVelocity);
	ResponseData->SetNumberField(TEXT("rotation_speed"), CustomStats.RotationSpeed);
	ResponseData->SetNumberField(TEXT("acceleration"), CustomStats.Acceleration);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Applied customization to ship '%s'"), *ShipID);

	return CreateJSONResponse(true, TEXT("Ship customization applied"), ResponseData);
}

FString UAutomationAPIServer::HandleGetShipCustomization(const FString& ShipID)
{
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	// Get ShipCustomizationComponent
	UShipCustomizationComponent* CustomizationComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
	if (!CustomizationComp)
	{
		return CreateJSONResponse(false, TEXT("Ship has no ShipCustomizationComponent"));
	}

	// Get current stats
	FShipStats CurrentStats = CustomizationComp->GetTotalStats();

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);
	ResponseData->SetNumberField(TEXT("mass"), CurrentStats.Mass);
	ResponseData->SetNumberField(TEXT("thrust_power"), CurrentStats.ThrustPower);
	ResponseData->SetNumberField(TEXT("max_velocity"), CurrentStats.MaxVelocity);
	ResponseData->SetNumberField(TEXT("rotation_speed"), CurrentStats.RotationSpeed);
	ResponseData->SetNumberField(TEXT("acceleration"), CurrentStats.Acceleration);
	ResponseData->SetNumberField(TEXT("hull_integrity"), CurrentStats.HullIntegrity);
	ResponseData->SetNumberField(TEXT("shield_strength"), CurrentStats.ShieldStrength);
	ResponseData->SetNumberField(TEXT("weapon_damage"), CurrentStats.WeaponDamage);
	ResponseData->SetNumberField(TEXT("energy_capacity"), CurrentStats.EnergyCapacity);
	ResponseData->SetNumberField(TEXT("energy_regen_rate"), CurrentStats.EnergyRegenRate);

	return CreateJSONResponse(true, TEXT("Ship customization retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleEquipShipPart(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleEquipShipPart RequestBody: '%s'"), *RequestBody);

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

	// Get ShipCustomizationComponent
	UShipCustomizationComponent* CustomizationComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
	if (!CustomizationComp)
	{
		return CreateJSONResponse(false, TEXT("Ship has no ShipCustomizationComponent"));
	}

	// Parse parameters
	int32 CategoryInt = JsonObj->GetIntegerField(TEXT("part_category"));
	EShipPartCategory Category = static_cast<EShipPartCategory>(CategoryInt);
	FString PartIDString = JsonObj->GetStringField(TEXT("part_id"));
	FName PartID = FName(*PartIDString);

	// Equip the part
	bool bSuccess = CustomizationComp->EquipPart(Category, PartID);
	if (!bSuccess)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Failed to equip part: %s"), *PartIDString));
	}

	// Get updated loadout
	FShipLoadout CurrentLoadout = CustomizationComp->CurrentLoadout;

	// Build response with updated loadout
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);
	ResponseData->SetStringField(TEXT("equipped_part_id"), PartIDString);
	ResponseData->SetNumberField(TEXT("part_category"), CategoryInt);

	// Add equipped parts map
	TSharedPtr<FJsonObject> EquippedPartsObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : CurrentLoadout.EquippedParts)
	{
		FString CategoryName;
		switch (Pair.Key)
		{
			case EShipPartCategory::Engine: CategoryName = TEXT("Engine"); break;
			case EShipPartCategory::Thrusters: CategoryName = TEXT("Thrusters"); break;
			case EShipPartCategory::Hull: CategoryName = TEXT("Hull"); break;
			case EShipPartCategory::Wings: CategoryName = TEXT("Wings"); break;
			case EShipPartCategory::Cockpit: CategoryName = TEXT("Cockpit"); break;
			case EShipPartCategory::Weapon: CategoryName = TEXT("Weapon"); break;
			case EShipPartCategory::Shield: CategoryName = TEXT("Shield"); break;
			case EShipPartCategory::Utility: CategoryName = TEXT("Utility"); break;
			default: CategoryName = TEXT("Unknown"); break;
		}
		EquippedPartsObj->SetStringField(CategoryName, Pair.Value.ToString());
	}
	ResponseData->SetObjectField(TEXT("equipped_parts"), EquippedPartsObj);

	// Add total stats
	FShipStats TotalStats = CurrentLoadout.TotalStats;
	TSharedPtr<FJsonObject> StatsObj = MakeShareable(new FJsonObject);
	StatsObj->SetNumberField(TEXT("mass"), TotalStats.Mass);
	StatsObj->SetNumberField(TEXT("thrust_power"), TotalStats.ThrustPower);
	StatsObj->SetNumberField(TEXT("max_velocity"), TotalStats.MaxVelocity);
	StatsObj->SetNumberField(TEXT("rotation_speed"), TotalStats.RotationSpeed);
	ResponseData->SetObjectField(TEXT("total_stats"), StatsObj);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Equipped part '%s' to ship '%s'"), *PartIDString, *ShipID);

	return CreateJSONResponse(true, TEXT("Ship part equipped"), ResponseData);
}

FString UAutomationAPIServer::HandleGetShipLoadout(const FString& ShipID)
{
	AActor* Ship = GetShipByID(ShipID);
	if (!Ship)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Ship not found: %s"), *ShipID));
	}

	// Get ShipCustomizationComponent
	UShipCustomizationComponent* CustomizationComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
	if (!CustomizationComp)
	{
		return CreateJSONResponse(false, TEXT("Ship has no ShipCustomizationComponent"));
	}

	// Get current loadout
	FShipLoadout CurrentLoadout = CustomizationComp->CurrentLoadout;

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("ship_id"), ShipID);
	ResponseData->SetStringField(TEXT("loadout_name"), CurrentLoadout.LoadoutName.ToString());
	ResponseData->SetStringField(TEXT("equipped_skin"), CurrentLoadout.EquippedSkin.ToString());

	// Add equipped parts map
	TSharedPtr<FJsonObject> EquippedPartsObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : CurrentLoadout.EquippedParts)
	{
		FString CategoryName;
		switch (Pair.Key)
		{
			case EShipPartCategory::Engine: CategoryName = TEXT("Engine"); break;
			case EShipPartCategory::Thrusters: CategoryName = TEXT("Thrusters"); break;
			case EShipPartCategory::Hull: CategoryName = TEXT("Hull"); break;
			case EShipPartCategory::Wings: CategoryName = TEXT("Wings"); break;
			case EShipPartCategory::Cockpit: CategoryName = TEXT("Cockpit"); break;
			case EShipPartCategory::Weapon: CategoryName = TEXT("Weapon"); break;
			case EShipPartCategory::Shield: CategoryName = TEXT("Shield"); break;
			case EShipPartCategory::Utility: CategoryName = TEXT("Utility"); break;
			default: CategoryName = TEXT("Unknown"); break;
		}
		EquippedPartsObj->SetStringField(CategoryName, Pair.Value.ToString());
	}
	ResponseData->SetObjectField(TEXT("equipped_parts"), EquippedPartsObj);

	// Add total stats
	FShipStats TotalStats = CurrentLoadout.TotalStats;
	TSharedPtr<FJsonObject> StatsObj = MakeShareable(new FJsonObject);
	StatsObj->SetNumberField(TEXT("mass"), TotalStats.Mass);
	StatsObj->SetNumberField(TEXT("thrust_power"), TotalStats.ThrustPower);
	StatsObj->SetNumberField(TEXT("max_velocity"), TotalStats.MaxVelocity);
	StatsObj->SetNumberField(TEXT("rotation_speed"), TotalStats.RotationSpeed);
	StatsObj->SetNumberField(TEXT("acceleration"), TotalStats.Acceleration);
	StatsObj->SetNumberField(TEXT("hull_integrity"), TotalStats.HullIntegrity);
	StatsObj->SetNumberField(TEXT("shield_strength"), TotalStats.ShieldStrength);
	StatsObj->SetNumberField(TEXT("weapon_damage"), TotalStats.WeaponDamage);
	StatsObj->SetNumberField(TEXT("energy_capacity"), TotalStats.EnergyCapacity);
	StatsObj->SetNumberField(TEXT("energy_regen_rate"), TotalStats.EnergyRegenRate);
	ResponseData->SetObjectField(TEXT("total_stats"), StatsObj);

	return CreateJSONResponse(true, TEXT("Ship loadout retrieved"), ResponseData);
}

void UAutomationAPIServer::RegisterShip(AActor* Ship, const FString& ShipID)
{
	if (Ship)
	{
		FScopeLock Lock(&TrackedShipsLock);
		TrackedShips.Add(ShipID, Ship);
		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Registered ship %s (%s)"), *ShipID, *Ship->GetName());
		}
	}
}

void UAutomationAPIServer::UnregisterShip(const FString& ShipID)
{
	FScopeLock Lock(&TrackedShipsLock);
	TrackedShips.Remove(ShipID);
	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Unregistered ship %s"), *ShipID);
	}
}

AActor* UAutomationAPIServer::GetShipByID(const FString& ShipID)
{
	FScopeLock Lock(&TrackedShipsLock);
	AActor** FoundShip = TrackedShips.Find(ShipID);
	return (FoundShip && IsValid(*FoundShip)) ? *FoundShip : nullptr;
}

TArray<AActor*> UAutomationAPIServer::GetAllShips()
{
	FScopeLock Lock(&TrackedShipsLock);
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

#if WITH_EDITOR
FString UAutomationAPIServer::HandlePlayInEditor(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandlePlayInEditor - Starting PIE"));

	// Check if already in PIE
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			return CreateJSONResponse(false, TEXT("PIE already running"));
		}
	}

	// Request PIE start using GEditor
	if (GEditor)
	{
		FRequestPlaySessionParams Params;
		GEditor->RequestPlaySession(Params);
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: PIE start requested"));
		return CreateJSONResponse(true, TEXT("PIE start requested - may take a few seconds"));
	}

	return CreateJSONResponse(false, TEXT("GEditor not available"));
}

FString UAutomationAPIServer::HandleStopPlaying()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleStopPlaying - Stopping PIE"));

	// Check if PIE is running
	bool bPIERunning = false;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			bPIERunning = true;
			break;
		}
	}

	if (!bPIERunning)
	{
		return CreateJSONResponse(false, TEXT("PIE not running"));
	}

	// Request PIE stop using GEditor
	if (GEditor)
	{
		GEditor->RequestEndPlayMap();
		UE_LOG(LogTemp, Log, TEXT("AutomationAPI: PIE stop requested"));
		return CreateJSONResponse(true, TEXT("PIE stop requested"));
	}

	return CreateJSONResponse(false, TEXT("GEditor not available"));
}
#endif // WITH_EDITOR

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
	// Use platform time instead of world time, so this works even without a world context
	float CurrentTime = static_cast<float>(FPlatformTime::Seconds());

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

#if WITH_EDITOR
void UAutomationAPIServer::OnPIEEnded(bool bIsSimulating)
{
	// Thread-safe cleanup of tracked ships when PIE ends
	FScopeLock Lock(&TrackedShipsLock);

	int32 ShipsCleared = TrackedShips.Num();
	TrackedShips.Empty();

	UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: PIE ended - cleared %d tracked ships"), ShipsCleared);
}
#endif

// ============================================================================
// MATERIAL & TEXTURE QUERY HANDLERS
// ============================================================================

#if WITH_EDITOR
FString UAutomationAPIServer::HandleMaterialsList(const FString& RequestBody)
{
	// Get the Asset Registry
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return CreateJSONResponse(false, TEXT("AssetRegistry not available"));
	}

	// Parse optional filter from request body
	FString PathFilter = TEXT("");
	if (!RequestBody.IsEmpty())
	{
		TSharedPtr<FJsonObject> RequestJSON = ParseJSON(RequestBody);
		if (RequestJSON.IsValid() && RequestJSON->HasField(TEXT("path_filter")))
		{
			PathFilter = RequestJSON->GetStringField(TEXT("path_filter"));
		}
	}

	// Build filter for UMaterial and UMaterialInstance assets
	FARFilter Filter;
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.Material")));
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.MaterialInstance")));
	Filter.bRecursiveClasses = true;

	// Get assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry->GetAssets(Filter, AssetDataList);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> MaterialsArray;

	for (const FAssetData& AssetData : AssetDataList)
	{
		FString AssetPath = AssetData.GetObjectPathString();

		// Apply path filter if specified
		if (!PathFilter.IsEmpty() && !AssetPath.Contains(PathFilter))
		{
			continue;
		}

		TSharedPtr<FJsonObject> MaterialObj = MakeShareable(new FJsonObject);
		MaterialObj->SetStringField(TEXT("path"), AssetPath);
		MaterialObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
		MaterialObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		MaterialObj->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());

		MaterialsArray.Add(MakeShareable(new FJsonValueObject(MaterialObj)));
	}

	ResponseData->SetArrayField(TEXT("materials"), MaterialsArray);
	ResponseData->SetNumberField(TEXT("count"), MaterialsArray.Num());
	if (!PathFilter.IsEmpty())
	{
		ResponseData->SetStringField(TEXT("path_filter"), PathFilter);
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Found %d materials"), MaterialsArray.Num());

	return CreateJSONResponse(true, FString::Printf(TEXT("Found %d materials"), MaterialsArray.Num()), ResponseData);
}

FString UAutomationAPIServer::HandleGetMaterialProperties(const FString& MaterialPath)
{
	// Try to load the material
	UMaterial* Material = LoadObject<UMaterial>(nullptr, *MaterialPath);

	if (!Material)
	{
		// Try loading as MaterialInstance
		UMaterialInstance* MaterialInstance = LoadObject<UMaterialInstance>(nullptr, *MaterialPath);
		if (!MaterialInstance)
		{
			return CreateJSONResponse(false, FString::Printf(TEXT("Failed to load material: %s"), *MaterialPath));
		}

		// Get base material from instance
		Material = MaterialInstance->GetBaseMaterial();
		if (!Material)
		{
			return CreateJSONResponse(false, TEXT("Failed to get base material from instance"));
		}
	}

	// Build response with material properties
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("material_path"), MaterialPath);
	ResponseData->SetStringField(TEXT("material_name"), Material->GetName());

	// Material properties
	ResponseData->SetBoolField(TEXT("two_sided"), Material->IsTwoSided());
	ResponseData->SetStringField(TEXT("blend_mode"), StaticEnum<EBlendMode>()->GetNameStringByValue((int64)Material->BlendMode));
	ResponseData->SetStringField(TEXT("shading_model"), StaticEnum<EMaterialShadingModel>()->GetNameStringByValue((int64)Material->GetShadingModels().GetFirstShadingModel()));

	// Count textures used by this material
	TArray<UTexture*> OutTextures;
	Material->GetUsedTextures(OutTextures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::SM5, true);
	ResponseData->SetNumberField(TEXT("num_textures"), OutTextures.Num());

	// List texture paths
	TArray<TSharedPtr<FJsonValue>> TexturesArray;
	for (UTexture* Texture : OutTextures)
	{
		if (Texture)
		{
			TSharedPtr<FJsonObject> TexObj = MakeShareable(new FJsonObject);
			TexObj->SetStringField(TEXT("path"), Texture->GetPathName());
			TexObj->SetStringField(TEXT("name"), Texture->GetName());
			TexturesArray.Add(MakeShareable(new FJsonValueObject(TexObj)));
		}
	}
	ResponseData->SetArrayField(TEXT("textures"), TexturesArray);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Retrieved properties for material '%s'"), *MaterialPath);

	return CreateJSONResponse(true, TEXT("Material properties retrieved"), ResponseData);
}

FString UAutomationAPIServer::HandleTexturesList(const FString& RequestBody)
{
	// Get the Asset Registry
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return CreateJSONResponse(false, TEXT("AssetRegistry not available"));
	}

	// Parse optional filter from request body
	FString PathFilter = TEXT("");
	if (!RequestBody.IsEmpty())
	{
		TSharedPtr<FJsonObject> RequestJSON = ParseJSON(RequestBody);
		if (RequestJSON.IsValid() && RequestJSON->HasField(TEXT("path_filter")))
		{
			PathFilter = RequestJSON->GetStringField(TEXT("path_filter"));
		}
	}

	// Build filter for UTexture2D assets
	FARFilter Filter;
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.Texture2D")));
	Filter.bRecursiveClasses = true;

	// Get assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry->GetAssets(Filter, AssetDataList);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> TexturesArray;

	for (const FAssetData& AssetData : AssetDataList)
	{
		FString AssetPath = AssetData.GetObjectPathString();

		// Apply path filter if specified
		if (!PathFilter.IsEmpty() && !AssetPath.Contains(PathFilter))
		{
			continue;
		}

		TSharedPtr<FJsonObject> TextureObj = MakeShareable(new FJsonObject);
		TextureObj->SetStringField(TEXT("path"), AssetPath);
		TextureObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
		TextureObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());

		// Try to get texture dimensions and format from asset tags
		FString SizeX, SizeY, Format;
		if (AssetData.GetTagValue("SizeX", SizeX))
		{
			TextureObj->SetNumberField(TEXT("width"), FCString::Atoi(*SizeX));
		}
		if (AssetData.GetTagValue("SizeY", SizeY))
		{
			TextureObj->SetNumberField(TEXT("height"), FCString::Atoi(*SizeY));
		}
		if (AssetData.GetTagValue("Format", Format))
		{
			TextureObj->SetStringField(TEXT("format"), Format);
		}

		TexturesArray.Add(MakeShareable(new FJsonValueObject(TextureObj)));
	}

	ResponseData->SetArrayField(TEXT("textures"), TexturesArray);
	ResponseData->SetNumberField(TEXT("count"), TexturesArray.Num());
	if (!PathFilter.IsEmpty())
	{
		ResponseData->SetStringField(TEXT("path_filter"), PathFilter);
	}

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Found %d textures"), TexturesArray.Num());

	return CreateJSONResponse(true, FString::Printf(TEXT("Found %d textures"), TexturesArray.Num()), ResponseData);
}
#endif // WITH_EDITOR

// ============================================================================
// AUTONOMOUS PLAYTESTING ENDPOINTS
// ============================================================================

FString UAutomationAPIServer::HandleReceiveScreenshot(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleReceiveScreenshot RequestBody size: %d bytes"), RequestBody.Len());

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutomationAPI: HandleReceiveScreenshot - Invalid JSON"));
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Extract base64 image data
	FString Base64Image = JsonObj->GetStringField(TEXT("image_base64"));
	if (Base64Image.IsEmpty())
	{
		return CreateJSONResponse(false, TEXT("No image data provided"));
	}

	// Extract metadata
	TSharedPtr<FJsonObject> MetadataObj;
	if (JsonObj->HasField(TEXT("metadata")))
	{
		const TSharedPtr<FJsonObject>* MetadataPtr;
		if (JsonObj->TryGetObjectField(TEXT("metadata"), MetadataPtr))
		{
			MetadataObj = *MetadataPtr;
		}
	}

	// Process the screenshot (store for analysis, trigger AI processing, etc.)
	FString ScreenshotID = FString::Printf(TEXT("screenshot_%d"), FDateTime::Now().ToUnixTimestamp());

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Received screenshot %s (%d bytes)"), *ScreenshotID, Base64Image.Len());

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("screenshot_id"), ScreenshotID);
	ResponseData->SetNumberField(TEXT("image_size"), Base64Image.Len());
	ResponseData->SetStringField(TEXT("status"), TEXT("received"));

	if (MetadataObj.IsValid())
	{
		ResponseData->SetObjectField(TEXT("metadata"), MetadataObj);
	}

	return CreateJSONResponse(true, FString::Printf(TEXT("Screenshot received: %s"), *ScreenshotID), ResponseData);
}

FString UAutomationAPIServer::HandleGetGameState()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleGetGameState"));

	// Find active game world
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	// Get player controller and pawn
	APlayerController* PC = World->GetFirstPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	// Build comprehensive game state
	TSharedPtr<FJsonObject> GameState = MakeShareable(new FJsonObject);

	// Player state
	TSharedPtr<FJsonObject> PlayerState = MakeShareable(new FJsonObject);
	if (PlayerPawn)
	{
		FVector Location = PlayerPawn->GetActorLocation();
		FRotator Rotation = PlayerPawn->GetActorRotation();

		TSharedPtr<FJsonObject> PositionObj = MakeShareable(new FJsonObject);
		PositionObj->SetNumberField(TEXT("x"), Location.X);
		PositionObj->SetNumberField(TEXT("y"), Location.Y);
		PositionObj->SetNumberField(TEXT("z"), Location.Z);
		PlayerState->SetObjectField(TEXT("position"), PositionObj);

		TSharedPtr<FJsonObject> RotationObj = MakeShareable(new FJsonObject);
		RotationObj->SetNumberField(TEXT("pitch"), Rotation.Pitch);
		RotationObj->SetNumberField(TEXT("yaw"), Rotation.Yaw);
		RotationObj->SetNumberField(TEXT("roll"), Rotation.Roll);
		PlayerState->SetObjectField(TEXT("rotation"), RotationObj);

		PlayerState->SetStringField(TEXT("pawn_name"), PlayerPawn->GetName());
		PlayerState->SetStringField(TEXT("pawn_class"), PlayerPawn->GetClass()->GetName());

		// Check for FlightController
		UFlightController* FlightController = PlayerPawn->FindComponentByClass<UFlightController>();
		PlayerState->SetBoolField(TEXT("has_flight_controller"), FlightController != nullptr);
	}

	GameState->SetObjectField(TEXT("player"), PlayerState);

	// World state
	TSharedPtr<FJsonObject> WorldState = MakeShareable(new FJsonObject);
	WorldState->SetNumberField(TEXT("time_seconds"), World->GetTimeSeconds());
	WorldState->SetNumberField(TEXT("real_time_seconds"), World->GetRealTimeSeconds());
	WorldState->SetNumberField(TEXT("delta_time_seconds"), World->GetDeltaSeconds());
	WorldState->SetStringField(TEXT("map_name"), World->GetMapName());

	GameState->SetObjectField(TEXT("world"), WorldState);

	// Game mode state
	AGameModeBase* GameMode = World->GetAuthGameMode();
	if (GameMode)
	{
		TSharedPtr<FJsonObject> GameModeState = MakeShareable(new FJsonObject);
		GameModeState->SetStringField(TEXT("game_mode_name"), GameMode->GetName());
		GameModeState->SetStringField(TEXT("game_mode_class"), GameMode->GetClass()->GetName());
		GameState->SetObjectField(TEXT("game_mode"), GameModeState);
	}

	// Performance metrics
	TSharedPtr<FJsonObject> Performance = MakeShareable(new FJsonObject);
	Performance->SetNumberField(TEXT("fps"), 1.0f / World->GetDeltaSeconds());
	Performance->SetNumberField(TEXT("delta_ms"), World->GetDeltaSeconds() * 1000.0f);

	GameState->SetObjectField(TEXT("performance"), Performance);

	return CreateJSONResponse(true, TEXT("Game state retrieved"), GameState);
}

FString UAutomationAPIServer::HandleExecuteCommand(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleExecuteCommand RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	FString Command = JsonObj->GetStringField(TEXT("command"));
	if (Command.IsEmpty())
	{
		return CreateJSONResponse(false, TEXT("No command provided"));
	}

	// Find active game world
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	// Get player controller to execute command
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return CreateJSONResponse(false, TEXT("No player controller found"));
	}

	// Execute console command
	PC->ConsoleCommand(Command);

	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: Executed console command: %s"), *Command);

	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("command"), Command);
	ResponseData->SetStringField(TEXT("status"), TEXT("executed"));

	return CreateJSONResponse(true, FString::Printf(TEXT("Command executed: %s"), *Command), ResponseData);
}

FString UAutomationAPIServer::HandlePerformanceMetrics()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandlePerformanceMetrics"));

	// Find active game world
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	// Build performance metrics
	TSharedPtr<FJsonObject> Metrics = MakeShareable(new FJsonObject);

	// Frame time metrics
	float DeltaTime = World->GetDeltaSeconds();
	float FPS = 1.0f / DeltaTime;

	TSharedPtr<FJsonObject> FrameMetrics = MakeShareable(new FJsonObject);
	FrameMetrics->SetNumberField(TEXT("fps"), FPS);
	FrameMetrics->SetNumberField(TEXT("delta_time_ms"), DeltaTime * 1000.0f);
	FrameMetrics->SetNumberField(TEXT("real_time_seconds"), World->GetRealTimeSeconds());
	FrameMetrics->SetNumberField(TEXT("time_seconds"), World->GetTimeSeconds());

	Metrics->SetObjectField(TEXT("frame"), FrameMetrics);

	// Memory metrics (if platform supports it)
	TSharedPtr<FJsonObject> MemoryMetrics = MakeShareable(new FJsonObject);
	MemoryMetrics->SetNumberField(TEXT("physical_memory_mb"), FPlatformMemory::GetStats().UsedPhysical / 1024.0f / 1024.0f);
	MemoryMetrics->SetNumberField(TEXT("virtual_memory_mb"), FPlatformMemory::GetStats().UsedVirtual / 1024.0f / 1024.0f);
	MemoryMetrics->SetNumberField(TEXT("available_physical_mb"), FPlatformMemory::GetStats().AvailablePhysical / 1024.0f / 1024.0f);

	Metrics->SetObjectField(TEXT("memory"), MemoryMetrics);

	// Hardware metrics
	TSharedPtr<FJsonObject> HardwareMetrics = MakeShareable(new FJsonObject);
	HardwareMetrics->SetNumberField(TEXT("num_cores"), FPlatformMisc::NumberOfCores());
	HardwareMetrics->SetNumberField(TEXT("num_cores_including_hyperthreads"), FPlatformMisc::NumberOfCoresIncludingHyperthreads());

	Metrics->SetObjectField(TEXT("hardware"), HardwareMetrics);

	// Unreal specific metrics
	TSharedPtr<FJsonObject> UnrealMetrics = MakeShareable(new FJsonObject);
	UnrealMetrics->SetNumberField(TEXT("num_actors"), World->GetActorCount());
	UnrealMetrics->SetNumberField(TEXT("num_pawns"), World->GetNumPawns());

	Metrics->SetObjectField(TEXT("unreal"), UnrealMetrics);

	return CreateJSONResponse(true, TEXT("Performance metrics retrieved"), Metrics);
}

FString UAutomationAPIServer::HandleStreamingMetrics()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleStreamingMetrics"));
	// TODO: Implement streaming metrics when MemoryOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Streaming metrics not implemented - MemoryOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleSetStreamingStrategy(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetStreamingStrategy RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Find MemoryOptimizationManager
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	UMemoryOptimizationManager* MemoryManager = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		MemoryManager = It->FindComponentByClass<UMemoryOptimizationManager>();
		if (MemoryManager)
		{
			break;
		}
	}

	if (!MemoryManager)
	{
		return CreateJSONResponse(false, TEXT("MemoryOptimizationManager not found"));
	}

	// Parse strategy from request
	FString StrategyStr = JsonObj->GetStringField(TEXT("strategy"));
	EMemoryOptimizationStrategy NewStrategy = EMemoryOptimizationStrategy::Balanced;

	if (StrategyStr == TEXT("Aggressive"))
	{
		NewStrategy = EMemoryOptimizationStrategy::Aggressive;
	}
	else if (StrategyStr == TEXT("Balanced"))
	{
		NewStrategy = EMemoryOptimizationStrategy::Balanced;
	}
	else if (StrategyStr == TEXT("Conservative"))
	{
		NewStrategy = EMemoryOptimizationStrategy::Conservative;
	}
	else if (StrategyStr == TEXT("Disabled"))
	{
		NewStrategy = EMemoryOptimizationStrategy::Disabled;
	}
	else
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Invalid strategy: %s"), *StrategyStr));
	}

	// Set new strategy
	MemoryManager->SetOptimizationStrategy(NewStrategy);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("strategy"), StrategyStr);
	ResponseData->SetStringField(TEXT("status"), TEXT("applied"));

	return CreateJSONResponse(true, FString::Printf(TEXT("Streaming strategy set to: %s"), *StrategyStr), ResponseData);
}

FString UAutomationAPIServer::HandleAssetLoadingStatus()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleAssetLoadingStatus"));

	// TODO: Implement asset loading status when MemoryOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Asset loading status not implemented - MemoryOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleTextureMemoryStats()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleTextureMemoryStats"));

	// TODO: Implement texture memory stats when MemoryOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Texture memory stats not implemented - MemoryOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleMeshMemoryStats()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleMeshMemoryStats"));

	// TODO: Implement mesh memory stats when MemoryOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Mesh memory stats not implemented - MemoryOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleForceMemoryOptimization(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleForceMemoryOptimization"));

	// TODO: Implement force memory optimization when MemoryOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Force memory optimization not implemented - MemoryOptimizationManager not available"));
}

// ============================================================================
// TICK OPTIMIZATION ENDPOINTS
// ============================================================================

FString UAutomationAPIServer::HandleTickStats()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleTickStats"));

	// TODO: Implement tick stats when TickOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Tick stats not implemented - TickOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleSetActorPriority(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleSetActorPriority RequestBody: '%s'"), *RequestBody);

	TSharedPtr<FJsonObject> JsonObj = ParseJSON(RequestBody);
	if (!JsonObj.IsValid())
	{
		return CreateJSONResponse(false, TEXT("Invalid JSON"));
	}

	// Find TickOptimizationManager
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	UTickOptimizationManager* TickManager = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TickManager = It->FindComponentByClass<UTickOptimizationManager>();
		if (TickManager)
		{
			break;
		}
	}

	if (!TickManager)
	{
		return CreateJSONResponse(false, TEXT("TickOptimizationManager not found"));
	}

	// Parse actor ID and priority from request
	FString ActorID = JsonObj->GetStringField(TEXT("actor_id"));
	FString PriorityStr = JsonObj->GetStringField(TEXT("priority"));

	if (ActorID.IsEmpty() || PriorityStr.IsEmpty())
	{
		return CreateJSONResponse(false, TEXT("Missing actor_id or priority"));
	}

	// Find actor by ID (search all actors in world)
	AActor* TargetActor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetName() == ActorID)
		{
			TargetActor = *It;
			break;
		}
	}

	if (!TargetActor)
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Actor not found: %s"), *ActorID));
	}

	// Parse priority
	ETickPriority Priority = ETickPriority::Medium;
	if (PriorityStr == TEXT("Critical")) Priority = ETickPriority::Critical;
	else if (PriorityStr == TEXT("High")) Priority = ETickPriority::High;
	else if (PriorityStr == TEXT("Medium")) Priority = ETickPriority::Medium;
	else if (PriorityStr == TEXT("Low")) Priority = ETickPriority::Low;
	else if (PriorityStr == TEXT("VeryLow")) Priority = ETickPriority::VeryLow;
	else if (PriorityStr == TEXT("Dormant")) Priority = ETickPriority::Dormant;
	else
	{
		return CreateJSONResponse(false, FString::Printf(TEXT("Invalid priority: %s"), *PriorityStr));
	}

	// Set actor priority
	TickManager->SetActorPriority(TargetActor, Priority);

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("actor_id"), ActorID);
	ResponseData->SetStringField(TEXT("actor_name"), TargetActor->GetName());
	ResponseData->SetStringField(TEXT("priority"), PriorityStr);
	ResponseData->SetStringField(TEXT("status"), TEXT("applied"));

	return CreateJSONResponse(true, FString::Printf(TEXT("Actor priority set: %s -> %s"), *ActorID, *PriorityStr), ResponseData);
}

FString UAutomationAPIServer::HandleDormantActors()
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleDormantActors"));

	// TODO: Implement dormant actors when TickOptimizationManager is available
	return CreateJSONResponse(false, TEXT("Dormant actors not implemented - TickOptimizationManager not available"));
}

FString UAutomationAPIServer::HandleResetTickOptimization(const FString& RequestBody)
{
	UE_LOG(LogTemp, Log, TEXT("AutomationAPI: HandleResetTickOptimization"));

	// Find TickOptimizationManager
	UWorld* World = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}
#else
	World = GetWorld();
#endif

	if (!World)
	{
		return CreateJSONResponse(false, TEXT("No active game world"));
	}

	UTickOptimizationManager* TickManager = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TickManager = It->FindComponentByClass<UTickOptimizationManager>();
		if (TickManager)
		{
			break;
		}
	}

	if (!TickManager)
	{
		return CreateJSONResponse(false, TEXT("TickOptimizationManager not found"));
	}

	// Reset tick optimization
	TickManager->ResetTickOptimization();

	// Build response
	TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
	ResponseData->SetStringField(TEXT("status"), TEXT("reset"));
	ResponseData->SetStringField(TEXT("message"), TEXT("All tick rates reset to default"));

	return CreateJSONResponse(true, TEXT("Tick optimization reset"), ResponseData);
}
