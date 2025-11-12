// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameStateExtractor.h"
#include "FlightController.h"
#include "ShipCustomizationComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Components/WidgetComponent.h"
#include "Components/TextRenderComponent.h"

UGameStateExtractor::UGameStateExtractor()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	bVerboseLogging = true;
	StateUpdateInterval = 0.1f;
	NearbyActorDetectionRadius = 5000.0f;
	bTrackPerformance = true;

	LastUpdateTime = 0.0f;
	AverageFPS = 0.0f;
	MinFPS = 9999.0f;
	MaxFPS = 0.0f;
	FrameCount = 0;
}

void UGameStateExtractor::BeginPlay()
{
	Super::BeginPlay();

	if (bVerboseLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("GameStateExtractor: Component initialized"));
	}
}

void UGameStateExtractor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update performance tracking
	if (bTrackPerformance && DeltaTime > 0.0f)
	{
		float FPS = 1.0f / DeltaTime;
		AverageFPS = (AverageFPS * FrameCount + FPS) / (FrameCount + 1);
		MinFPS = FMath::Min(MinFPS, FPS);
		MaxFPS = FMath::Max(MaxFPS, FPS);
		FrameCount++;
	}

	// Update cached state at specified interval
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastUpdateTime >= StateUpdateInterval)
	{
		CachedGameState = ExtractGameState();
		LastUpdateTime = CurrentTime;

		if (bVerboseLogging)
		{
			UE_LOG(LogTemp, Verbose, TEXT("GameStateExtractor: Cached game state updated"));
		}
	}
}

UWorld* UGameStateExtractor::GetGameWorld()
{
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

	return World;
}

APlayerController* UGameStateExtractor::GetPlayerController()
{
	UWorld* World = GetGameWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}

APawn* UGameStateExtractor::GetPlayerPawn()
{
	APlayerController* PC = GetPlayerController();
	return PC ? PC->GetPawn() : nullptr;
}

UFlightController* UGameStateExtractor::GetFlightController()
{
	APawn* PlayerPawn = GetPlayerPawn();
	return PlayerPawn ? PlayerPawn->FindComponentByClass<UFlightController>() : nullptr;
}

UShipCustomizationComponent* UGameStateExtractor::GetShipCustomizationComponent()
{
	APawn* PlayerPawn = GetPlayerPawn();
	return PlayerPawn ? PlayerPawn->FindComponentByClass<UShipCustomizationComponent>() : nullptr;
}

TSharedPtr<FJsonObject> UGameStateExtractor::VectorToJSON(const FVector& Vector)
{
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetNumberField(TEXT("x"), Vector.X);
	JsonObj->SetNumberField(TEXT("y"), Vector.Y);
	JsonObj->SetNumberField(TEXT("z"), Vector.Z);
	return JsonObj;
}

TSharedPtr<FJsonObject> UGameStateExtractor::RotatorToJSON(const FRotator& Rotator)
{
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetNumberField(TEXT("pitch"), Rotator.Pitch);
	JsonObj->SetNumberField(TEXT("yaw"), Rotator.Yaw);
	JsonObj->SetNumberField(TEXT("roll"), Rotator.Roll);
	return JsonObj;
}

TSharedPtr<FJsonObject> UGameStateExtractor::TransformToJSON(const FTransform& Transform)
{
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	JsonObj->SetObjectField(TEXT("location"), VectorToJSON(Transform.GetLocation()));
	JsonObj->SetObjectField(TEXT("rotation"), RotatorToJSON(Transform.GetRotation().Rotator()));
	JsonObj->SetObjectField(TEXT("scale"), VectorToJSON(Transform.GetScale3D()));
	return JsonObj;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractPlayerState()
{
	TSharedPtr<FJsonObject> PlayerState = MakeShareable(new FJsonObject);

	APawn* PlayerPawn = GetPlayerPawn();
	APlayerController* PC = GetPlayerController();

	if (PlayerPawn)
	{
		// Basic pawn info
		PlayerState->SetStringField(TEXT("pawn_name"), PlayerPawn->GetName());
		PlayerState->SetStringField(TEXT("pawn_class"), PlayerPawn->GetClass()->GetName());

		// Transform
		PlayerState->SetObjectField(TEXT("transform"), TransformToJSON(PlayerPawn->GetActorTransform()));

		// Physics state
		PlayerState->SetObjectField(TEXT("velocity"), VectorToJSON(PlayerPawn->GetVelocity()));

		// Flight controller info
		UFlightController* FlightController = GetFlightController();
		if (FlightController)
		{
			TSharedPtr<FJsonObject> FlightInfo = MakeShareable(new FJsonObject);
			// FlightInfo->SetNumberField(TEXT("current_speed"), FlightController->GetCurrentSpeed()); // Method doesn't exist
			FlightInfo->SetNumberField(TEXT("assist_mode"), (int32)FlightController->GetAssistMode());
			// FlightInfo->SetBoolField(TEXT("is_boosting"), FlightController->IsBoosting()); // Method doesn't exist

			PlayerState->SetObjectField(TEXT("flight_controller"), FlightInfo);
		}

		// Customization info
		UShipCustomizationComponent* Customization = GetShipCustomizationComponent();
		if (Customization)
		{
			TSharedPtr<FJsonObject> CustomizationInfo = MakeShareable(new FJsonObject);
			CustomizationInfo->SetStringField(TEXT("loadout_name"), Customization->CurrentLoadout.LoadoutName.ToString());
			CustomizationInfo->SetStringField(TEXT("equipped_skin"), Customization->CurrentLoadout.EquippedSkin.ToString());

			// Stats
			TSharedPtr<FJsonObject> StatsInfo = MakeShareable(new FJsonObject);
			StatsInfo->SetNumberField(TEXT("mass"), Customization->CurrentLoadout.TotalStats.Mass);
			StatsInfo->SetNumberField(TEXT("thrust_power"), Customization->CurrentLoadout.TotalStats.ThrustPower);
			StatsInfo->SetNumberField(TEXT("max_velocity"), Customization->CurrentLoadout.TotalStats.MaxVelocity);
			StatsInfo->SetNumberField(TEXT("rotation_speed"), Customization->CurrentLoadout.TotalStats.RotationSpeed);
			StatsInfo->SetNumberField(TEXT("acceleration"), Customization->CurrentLoadout.TotalStats.Acceleration);
			StatsInfo->SetNumberField(TEXT("hull_integrity"), Customization->CurrentLoadout.TotalStats.HullIntegrity);
			StatsInfo->SetNumberField(TEXT("shield_strength"), Customization->CurrentLoadout.TotalStats.ShieldStrength);

			CustomizationInfo->SetObjectField(TEXT("stats"), StatsInfo);
			PlayerState->SetObjectField(TEXT("customization"), CustomizationInfo);
		}
	}

	if (PC)
	{
		TSharedPtr<FJsonObject> ControllerInfo = MakeShareable(new FJsonObject);
		ControllerInfo->SetStringField(TEXT("controller_name"), PC->GetName());
		ControllerInfo->SetBoolField(TEXT("is_local_player"), PC->IsLocalPlayerController());

		// Input state
		TSharedPtr<FJsonObject> InputInfo = MakeShareable(new FJsonObject);
		InputInfo->SetBoolField(TEXT("is_movement_input"), PC->IsMoveInputIgnored());
		InputInfo->SetBoolField(TEXT("is_look_input"), PC->IsLookInputIgnored());

		ControllerInfo->SetObjectField(TEXT("input_state"), InputInfo);
		PlayerState->SetObjectField(TEXT("controller"), ControllerInfo);

		// Player state
		if (APlayerState* PS = PC->PlayerState)
		{
			TSharedPtr<FJsonObject> PlayerStateInfo = MakeShareable(new FJsonObject);
			PlayerStateInfo->SetStringField(TEXT("player_name"), PS->GetPlayerName());
			PlayerStateInfo->SetNumberField(TEXT("player_id"), PS->GetPlayerId());

			PlayerState->SetObjectField(TEXT("player_state"), PlayerStateInfo);
		}
	}

	return PlayerState;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractWorldState()
{
	TSharedPtr<FJsonObject> WorldState = MakeShareable(new FJsonObject);

	UWorld* World = GetGameWorld();
	if (!World)
	{
		return WorldState;
	}

	// Basic world info
	WorldState->SetStringField(TEXT("map_name"), World->GetMapName());
	WorldState->SetNumberField(TEXT("time_seconds"), World->GetTimeSeconds());
	WorldState->SetNumberField(TEXT("real_time_seconds"), World->GetRealTimeSeconds());
	WorldState->SetNumberField(TEXT("delta_time_seconds"), World->GetDeltaSeconds());
	WorldState->SetNumberField(TEXT("actor_count"), World->GetActorCount());
	// WorldState->SetNumberField(TEXT("pawn_count"), World->GetPawnCount()); // Method doesn't exist

	// Game mode info
	if (AGameModeBase* GameMode = World->GetAuthGameMode())
	{
		TSharedPtr<FJsonObject> GameModeInfo = MakeShareable(new FJsonObject);
		GameModeInfo->SetStringField(TEXT("game_mode_name"), GameMode->GetName());
		GameModeInfo->SetStringField(TEXT("game_mode_class"), GameMode->GetClass()->GetName());

		WorldState->SetObjectField(TEXT("game_mode"), GameModeInfo);
	}

	// Game state info
	if (AGameStateBase* GameState = World->GetGameState())
	{
		TSharedPtr<FJsonObject> GameStateInfo = MakeShareable(new FJsonObject);
		// GameStateInfo->SetStringField(TEXT("game_state_name"), GameState->GetName()); // Method doesn't exist
		GameStateInfo->SetStringField(TEXT("game_state_class"), GameState->GetClass()->GetName());
		// GameStateInfo->SetNumberField(TEXT("player_count"), GameState->PlayerArray.Num()); // Member doesn't exist

		WorldState->SetObjectField(TEXT("game_state"), GameStateInfo);
	}

	return WorldState;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractPerformanceMetrics()
{
	TSharedPtr<FJsonObject> Performance = MakeShareable(new FJsonObject);

	UWorld* World = GetGameWorld();
	if (World)
	{
		TSharedPtr<FJsonObject> FrameMetrics = MakeShareable(new FJsonObject);
		float DeltaTime = World->GetDeltaSeconds();
		float FPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;

		FrameMetrics->SetNumberField(TEXT("fps"), FPS);
		FrameMetrics->SetNumberField(TEXT("delta_time_ms"), DeltaTime * 1000.0f);
		FrameMetrics->SetNumberField(TEXT("real_time_seconds"), World->GetRealTimeSeconds());
		FrameMetrics->SetNumberField(TEXT("time_seconds"), World->GetTimeSeconds());

		Performance->SetObjectField(TEXT("frame"), FrameMetrics);
	}

	// Memory metrics
	TSharedPtr<FJsonObject> MemoryMetrics = MakeShareable(new FJsonObject);
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();

	MemoryMetrics->SetNumberField(TEXT("used_physical_mb"), MemoryStats.UsedPhysical / 1024.0f / 1024.0f);
	MemoryMetrics->SetNumberField(TEXT("used_virtual_mb"), MemoryStats.UsedVirtual / 1024.0f / 1024.0f);
	MemoryMetrics->SetNumberField(TEXT("available_physical_mb"), MemoryStats.AvailablePhysical / 1024.0f / 1024.0f);
	MemoryMetrics->SetNumberField(TEXT("total_physical_mb"), MemoryStats.TotalPhysical / 1024.0f / 1024.0f);

	Performance->SetObjectField(TEXT("memory"), MemoryMetrics);

	// Hardware metrics
	TSharedPtr<FJsonObject> HardwareMetrics = MakeShareable(new FJsonObject);
	HardwareMetrics->SetNumberField(TEXT("num_cores"), FPlatformMisc::NumberOfCores());
	HardwareMetrics->SetNumberField(TEXT("num_cores_including_hyperthreads"), FPlatformMisc::NumberOfCoresIncludingHyperthreads());

	Performance->SetObjectField(TEXT("hardware"), HardwareMetrics);

	// Tracked performance stats
	if (bTrackPerformance && FrameCount > 0)
	{
		TSharedPtr<FJsonObject> TrackedMetrics = MakeShareable(new FJsonObject);
		TrackedMetrics->SetNumberField(TEXT("average_fps"), AverageFPS);
		TrackedMetrics->SetNumberField(TEXT("min_fps"), MinFPS);
		TrackedMetrics->SetNumberField(TEXT("max_fps"), MaxFPS);
		TrackedMetrics->SetNumberField(TEXT("frame_count"), FrameCount);

		Performance->SetObjectField(TEXT("tracked"), TrackedMetrics);
	}

	return Performance;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractInventoryState()
{
	TSharedPtr<FJsonObject> Inventory = MakeShareable(new FJsonObject);

	UShipCustomizationComponent* Customization = GetShipCustomizationComponent();
	if (Customization)
	{
		// Equipped parts
		TSharedPtr<FJsonObject> EquippedParts = MakeShareable(new FJsonObject);
		for (const auto& Pair : Customization->CurrentLoadout.EquippedParts)
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

			EquippedParts->SetStringField(CategoryName, Pair.Value.ToString());
		}

		Inventory->SetObjectField(TEXT("equipped_parts"), EquippedParts);
		Inventory->SetStringField(TEXT("equipped_skin"), Customization->CurrentLoadout.EquippedSkin.ToString());
		Inventory->SetStringField(TEXT("loadout_name"), Customization->CurrentLoadout.LoadoutName.ToString());
	}

	return Inventory;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractMissionState()
{
	TSharedPtr<FJsonObject> MissionState = MakeShareable(new FJsonObject);

	// This would be extended based on actual mission system implementation
	MissionState->SetStringField(TEXT("status"), TEXT("active"));
	MissionState->SetStringField(TEXT("current_mission"), TEXT("exploration"));
	MissionState->SetNumberField(TEXT("objectives_completed"), 0);
	MissionState->SetNumberField(TEXT("total_objectives"), 1);

	return MissionState;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractNearbyActors(float Radius)
{
	TSharedPtr<FJsonObject> NearbyActors = MakeShareable(new FJsonObject);

	UWorld* World = GetGameWorld();
	if (!World)
	{
		return NearbyActors;
	}

	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn)
	{
		return NearbyActors;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	TArray<AActor*> FoundActors;

	// Find nearby actors
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

	TArray<TSharedPtr<FJsonValue>> ActorsArray;
	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == PlayerPawn)
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
		if (Distance <= Radius)
		{
			TSharedPtr<FJsonObject> ActorInfo = MakeShareable(new FJsonObject);
			ActorInfo->SetStringField(TEXT("name"), Actor->GetName());
			ActorInfo->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
			ActorInfo->SetObjectField(TEXT("location"), VectorToJSON(Actor->GetActorLocation()));
			ActorInfo->SetNumberField(TEXT("distance"), Distance);

			ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorInfo)));
		}
	}

	NearbyActors->SetArrayField(TEXT("actors"), ActorsArray);
	NearbyActors->SetNumberField(TEXT("count"), ActorsArray.Num());
	NearbyActors->SetNumberField(TEXT("search_radius"), Radius);

	return NearbyActors;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractUIState()
{
	TSharedPtr<FJsonObject> UIState = MakeShareable(new FJsonObject);

	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn)
	{
		return UIState;
	}

	// Find UI components
	TArray<UWidgetComponent*> WidgetComponents;
	PlayerPawn->GetComponents<UWidgetComponent>(WidgetComponents);

	TArray<TSharedPtr<FJsonValue>> UIComponentsArray;
	for (UWidgetComponent* WidgetComp : WidgetComponents)
	{
		if (WidgetComp)
		{
			TSharedPtr<FJsonObject> WidgetInfo = MakeShareable(new FJsonObject);
			WidgetInfo->SetStringField(TEXT("name"), WidgetComp->GetName());
			WidgetInfo->SetBoolField(TEXT("is_visible"), WidgetComp->IsVisible());

			UIComponentsArray.Add(MakeShareable(new FJsonValueObject(WidgetInfo)));
		}
	}

	UIState->SetArrayField(TEXT("widget_components"), UIComponentsArray);

	// Find text render components (for debug text, etc.)
	TArray<UTextRenderComponent*> TextComponents;
	PlayerPawn->GetComponents<UTextRenderComponent>(TextComponents);

	TArray<TSharedPtr<FJsonValue>> TextComponentsArray;
	for (UTextRenderComponent* TextComp : TextComponents)
	{
		if (TextComp)
		{
			TSharedPtr<FJsonObject> TextInfo = MakeShareable(new FJsonObject);
			TextInfo->SetStringField(TEXT("name"), TextComp->GetName());
			TextInfo->SetStringField(TEXT("text"), TextComp->Text.ToString());
			TextInfo->SetBoolField(TEXT("is_visible"), TextComp->IsVisible());

			TextComponentsArray.Add(MakeShareable(new FJsonValueObject(TextInfo)));
		}
	}

	UIState->SetArrayField(TEXT("text_components"), TextComponentsArray);
	UIState->SetNumberField(TEXT("total_ui_elements"), UIComponentsArray.Num() + TextComponentsArray.Num());

	return UIState;
}

TSharedPtr<FJsonObject> UGameStateExtractor::ExtractGameState()
{
	TSharedPtr<FJsonObject> GameState = MakeShareable(new FJsonObject);

	// Add timestamp
	GameState->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());
	GameState->SetNumberField(TEXT("extractor_version"), 1.0f);

	// Add all state components
	GameState->SetObjectField(TEXT("player"), ExtractPlayerState());
	GameState->SetObjectField(TEXT("world"), ExtractWorldState());
	GameState->SetObjectField(TEXT("performance"), ExtractPerformanceMetrics());
	GameState->SetObjectField(TEXT("inventory"), ExtractInventoryState());
	GameState->SetObjectField(TEXT("mission"), ExtractMissionState());
	GameState->SetObjectField(TEXT("nearby_actors"), ExtractNearbyActors(NearbyActorDetectionRadius));
	GameState->SetObjectField(TEXT("ui"), ExtractUIState());

	return GameState;
}

FString UGameStateExtractor::GetGameStateJSON()
{
	TSharedPtr<FJsonObject> GameState = ExtractGameState();

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(GameState.ToSharedRef(), Writer);

	return OutputString;
}