// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestLevelCentralHub.h"
#include "Testing/NavigationKioskWidget.h"
#include "Testing/TestResultsWidget.h"
#include "Testing/TestLevelGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATestLevelCentralHub::ATestLevelCentralHub()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create visual components
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	KioskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KioskMesh"));
	KioskMesh->SetupAttachment(RootComponent);

	SignageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SignageMesh"));
	SignageMesh->SetupAttachment(RootComponent);

	// Create widget components
	NavigationKioskComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NavigationKioskComponent"));
	NavigationKioskComponent->SetupAttachment(RootComponent);
	NavigationKioskComponent->SetWidgetSpace(EWidgetSpace::World);
	NavigationKioskComponent->SetDrawSize(FVector2D(1000.0f, 750.0f));

	ResultsDisplayComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ResultsDisplayComponent"));
	ResultsDisplayComponent->SetupAttachment(RootComponent);
	ResultsDisplayComponent->SetWidgetSpace(EWidgetSpace::World);
	ResultsDisplayComponent->SetDrawSize(FVector2D(800.0f, 600.0f));

	SettingsPanelComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SettingsPanelComponent"));
	SettingsPanelComponent->SetupAttachment(RootComponent);
	SettingsPanelComponent->SetWidgetSpace(EWidgetSpace::World);
	SettingsPanelComponent->SetDrawSize(FVector2D(600.0f, 800.0f));

	// Create lighting
	KioskLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KioskLight"));
	KioskLight->SetupAttachment(RootComponent);
	KioskLight->SetLightColor(FLinearColor(0.0f, 1.0f, 1.0f)); // Cyan
	KioskLight->SetIntensity(5000.0f);

	AmbientLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AmbientLight"));
	AmbientLight->SetupAttachment(RootComponent);
	AmbientLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f)); // White
	AmbientLight->SetIntensity(2000.0f);

	// Create audio
	AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
	AmbientAudio->SetupAttachment(RootComponent);
	AmbientAudio->bAutoActivate = false;
}

void ATestLevelCentralHub::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the hub
	InitializeHub();
	RegisterWithGameMode();

	// Set up initial state
	if (bAutoShowNavigationOnSpawn)
	{
		ShowNavigationKiosk();
	}

	// Play welcome sequence
	PlayWelcomeSequence();
}

void ATestLevelCentralHub::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastUpdate += DeltaTime;
	if (TimeSinceLastUpdate >= HubUpdateInterval)
	{
		UpdateHubStatus();
		TimeSinceLastUpdate = 0.0f;
	}
}

void ATestLevelCentralHub::InitializeHub()
{
	if (bHubInitialized)
	{
		return;
	}

	// Create widgets
	CreateWidgets();

	// Setup lighting and audio
	SetupLighting();
	SetupAudio();

	// Find or create spawn point
	HandlePlayerSpawn();

	bHubInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("Test Level Central Hub initialized"));
}

void ATestLevelCentralHub::RegisterWithGameMode()
{
	// Get the game mode
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	GameModeRef = Cast<ATestLevelGameMode>(GameMode);

	if (GameModeRef)
	{
		// Bind to game mode events
		GameModeRef->OnTestsStarted.AddDynamic(this, &ATestLevelCentralHub::OnTestsStarted);
		GameModeRef->OnAllTestsCompleted.AddDynamic(this, &ATestLevelCentralHub::OnTestsCompleted);
		GameModeRef->OnStationChanged.AddDynamic(this, &ATestLevelCentralHub::OnStationChanged);

		UE_LOG(LogTemp, Log, TEXT("Central Hub registered with Test Level Game Mode"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find TestLevelGameMode for Central Hub"));
	}
}

void ATestLevelCentralHub::CreateWidgets()
{
	// Create navigation kiosk widget
	if (NavigationKioskClass && NavigationKioskComponent)
	{
		NavigationKioskWidget = CreateWidget<UNavigationKioskWidget>(GetWorld(), NavigationKioskClass);
		if (NavigationKioskWidget)
		{
			NavigationKioskComponent->SetWidget(NavigationKioskWidget);
			NavigationKioskWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Create results widget
	if (ResultsWidgetClass && ResultsDisplayComponent)
	{
		ResultsWidget = CreateWidget<UTestResultsWidget>(GetWorld(), ResultsWidgetClass);
		if (ResultsWidget)
		{
			ResultsDisplayComponent->SetWidget(ResultsWidget);
			ResultsWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Create settings widget
	if (SettingsWidgetClass && SettingsPanelComponent)
	{
		SettingsWidget = CreateWidget<UUserWidget>(GetWorld(), SettingsWidgetClass);
		if (SettingsWidget)
		{
			SettingsPanelComponent->SetWidget(SettingsWidget);
			SettingsWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ATestLevelCentralHub::SetupLighting()
{
	// Position lights appropriately
	if (KioskLight && KioskMesh)
	{
		KioskLight->SetRelativeLocation(KioskMesh->GetRelativeLocation() + FVector(0.0f, 0.0f, 200.0f));
	}

	if (AmbientLight)
	{
		AmbientLight->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));
	}
}

void ATestLevelCentralHub::SetupAudio()
{
	// Set up ambient audio
	if (AmbientAudio && AmbientAudio->Sound)
	{
		AmbientAudio->Play();
	}
}

void ATestLevelCentralHub::HandlePlayerSpawn()
{
	// Find existing player start or create one
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() > 0)
	{
		HubSpawnPoint = Cast<APlayerStart>(PlayerStarts[0]);
	}
	else
	{
		// Create a player start at origin
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("HubSpawnPoint");
		HubSpawnPoint = GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (HubSpawnPoint)
	{
		UE_LOG(LogTemp, Log, TEXT("Hub spawn point ready"));
	}
}

void ATestLevelCentralHub::ShowNavigationKiosk()
{
	if (NavigationKioskWidget)
	{
		NavigationKioskWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateWidgetVisibility();
	}
}

void ATestLevelCentralHub::ShowResultsDisplay()
{
	if (ResultsWidget)
	{
		ResultsWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateWidgetVisibility();
	}
}

void ATestLevelCentralHub::ShowSettingsPanel()
{
	if (SettingsWidget)
	{
		SettingsWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateWidgetVisibility();
	}
}

void ATestLevelCentralHub::HideAllWidgets()
{
	if (NavigationKioskWidget)
	{
		NavigationKioskWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ResultsWidget)
	{
		ResultsWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (SettingsWidget)
	{
		SettingsWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ATestLevelCentralHub::UpdateHubStatus()
{
	// Update visual indicators based on test state
	if (GameModeRef)
	{
		bool bTestsRunning = GameModeRef->AreTestsRunning();
		float Progress = GameModeRef->GetOverallProgress();

		// Update lighting based on test state
		if (bTestsRunning)
		{
			KioskLight->SetLightColor(FLinearColor(1.0f, 1.0f, 0.0f)); // Yellow for running
		}
		else if (Progress >= 1.0f)
		{
			KioskLight->SetLightColor(FLinearColor(0.0f, 1.0f, 0.0f)); // Green for completed
		}
		else
		{
			KioskLight->SetLightColor(FLinearColor(0.0f, 1.0f, 1.0f)); // Cyan for ready
		}
	}
}

void ATestLevelCentralHub::PlayWelcomeSequence()
{
	// Play welcome sound
	if (WelcomeSound && AmbientAudio)
	{
		AmbientAudio->SetSound(WelcomeSound);
		AmbientAudio->Play();
	}

	// Could add more welcome effects here (particles, animations, etc.)
}

void ATestLevelCentralHub::HighlightActiveStation(const FString& StationName)
{
	CurrentActiveStation = StationName;

	// Update navigation kiosk to highlight the active station
	if (NavigationKioskWidget)
	{
		// This would be implemented in the widget's blueprint
		// NavigationKioskWidget->HighlightStation(StationName);
	}
}

bool ATestLevelCentralHub::TeleportPlayerToStation(const FString& StationName)
{
	if (GameModeRef)
	{
		bool bSuccess = GameModeRef->TeleportPlayerToStation(StationName);

		if (bSuccess && TeleportSound && AmbientAudio)
		{
			AmbientAudio->SetSound(TeleportSound);
			AmbientAudio->Play();
		}

		return bSuccess;
	}

	return false;
}

void ATestLevelCentralHub::ReturnPlayerToHub()
{
	if (GameModeRef)
	{
		GameModeRef->TeleportPlayerToHub();

		if (TeleportSound && AmbientAudio)
		{
			AmbientAudio->SetSound(TeleportSound);
			AmbientAudio->Play();
		}
	}
}

void ATestLevelCentralHub::UpdateWidgetVisibility()
{
	// Ensure only one major widget is visible at a time
	// This is a simple implementation - could be more sophisticated
}

void ATestLevelCentralHub::OnTestsStarted(const FString& TestName)
{
	UpdateHubStatus();
	HideAllWidgets();
	ShowResultsDisplay();
}

void ATestLevelCentralHub::OnTestsCompleted(const FTestResults& Results)
{
	UpdateHubStatus();
	ShowResultsDisplay();
}

void ATestLevelCentralHub::OnStationChanged(const FString& StationName)
{
	HighlightActiveStation(StationName);
	UpdateHubStatus();
}
