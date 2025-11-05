// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerTimeComponent.h"
#include "RelativitySystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPlayerTimeComponent::UPlayerTimeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second

	SetIsReplicatedByDefault(true);

	BiologicalAge = 25.0f;
	TimeDebt = 0.0;
	CurrentTimeDilation = 1.0f;
	TotalRealTime = 0.0;
	TotalWorldTime = 0.0;
	CurrentVelocity = 0.0f;
	CurrentAltitude = 0.0f;

	bAutoRegister = true;
	UpdateFrequency = 0.5f;
	bShowDilationWarnings = true;

	UpdateTimer = 0.0f;
	bInitialized = false;
}

void UPlayerTimeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegister)
	{
		Initialize();
	}
}

void UPlayerTimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromRelativitySystem();

	Super::EndPlay(EndPlayReason);
}

void UPlayerTimeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized)
	{
		return;
	}

	// Update timer
	UpdateTimer += DeltaTime;

	// Update at specified frequency
	if (UpdateTimer >= UpdateFrequency)
	{
		UpdateTimer = 0.0f;
		UpdateFromRelativitySystem();
	}
}

void UPlayerTimeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerTimeComponent, BiologicalAge);
	DOREPLIFETIME(UPlayerTimeComponent, TimeDebt);
	DOREPLIFETIME(UPlayerTimeComponent, CurrentTimeDilation);
	DOREPLIFETIME(UPlayerTimeComponent, TotalRealTime);
	DOREPLIFETIME(UPlayerTimeComponent, TotalWorldTime);
	DOREPLIFETIME(UPlayerTimeComponent, CurrentVelocity);
	DOREPLIFETIME(UPlayerTimeComponent, CurrentAltitude);
}

// ============================================================================
// TIME MANAGEMENT
// ============================================================================

void UPlayerTimeComponent::Initialize(float StartingAge)
{
	if (bInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTimeComponent: Already initialized"));
		return;
	}

	BiologicalAge = StartingAge;
	TimeDebt = 0.0;
	CurrentTimeDilation = 1.0f;
	TotalRealTime = 0.0;
	TotalWorldTime = 0.0;

	// Find owning player controller
	OwningPlayerController = FindOwningPlayerController();

	// Register with RelativitySystem
	RegisterWithRelativitySystem();

	bInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("PlayerTimeComponent: Initialized for player %s with age %.1f"),
		OwningPlayerController ? *OwningPlayerController->GetName() : TEXT("Unknown"),
		StartingAge);
}

void UPlayerTimeComponent::AdvanceBiologicalAge(float SecondsDelta)
{
	const float SecondsPerYear = 31536000.0f; // 365 days
	BiologicalAge += SecondsDelta / SecondsPerYear;
}

void UPlayerTimeComponent::UpdateFromRelativitySystem()
{
	if (!RelativitySystem || !OwningPlayerController)
	{
		return;
	}

	// Get latest data from RelativitySystem
	FPlayerTimeData Data = RelativitySystem->GetPlayerTimeData(OwningPlayerController);

	BiologicalAge = Data.BiologicalAge;
	TimeDebt = Data.TimeDebt;
	TotalRealTime = Data.TotalRealTimeSeconds;
	TotalWorldTime = Data.TotalWorldTimeSeconds;
	CurrentVelocity = Data.CurrentVelocity;
	CurrentAltitude = Data.CurrentAltitude;

	// Calculate current time dilation
	APawn* Pawn = OwningPlayerController->GetPawn();
	if (Pawn && RelativitySystem)
	{
		CurrentTimeDilation = RelativitySystem->GetCombinedTimeDilation(Pawn);

		// Show warnings if enabled
		if (bShowDilationWarnings && CurrentTimeDilation < 0.5f)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerTimeComponent: Extreme time dilation %.2f!"), CurrentTimeDilation);
		}
	}
}

// ============================================================================
// UI QUERIES
// ============================================================================

FString UPlayerTimeComponent::GetFormattedTimeDebt() const
{
	if (!RelativitySystem || !OwningPlayerController)
	{
		return TEXT("0 seconds");
	}

	return RelativitySystem->GetFormattedTimeDebt(OwningPlayerController);
}

FString UPlayerTimeComponent::GetFormattedBiologicalAge() const
{
	return FString::Printf(TEXT("%.1f years"), BiologicalAge);
}

FString UPlayerTimeComponent::GetFormattedSessionTime() const
{
	// Convert seconds to hours, minutes, seconds
	int32 TotalSeconds = FMath::FloorToInt(TotalRealTime);
	int32 Hours = TotalSeconds / 3600;
	int32 Minutes = (TotalSeconds % 3600) / 60;
	int32 Seconds = TotalSeconds % 60;

	if (Hours > 0)
	{
		return FString::Printf(TEXT("%d hours, %d minutes"), Hours, Minutes);
	}
	else if (Minutes > 0)
	{
		return FString::Printf(TEXT("%d minutes, %d seconds"), Minutes, Seconds);
	}
	else
	{
		return FString::Printf(TEXT("%d seconds"), Seconds);
	}
}

FLinearColor UPlayerTimeComponent::GetTimeDilationColor() const
{
	if (!RelativitySystem)
	{
		return FLinearColor::Green;
	}

	return RelativitySystem->GetTimeDilationColor(CurrentTimeDilation);
}

int32 UPlayerTimeComponent::GetTimeDebtSeverity() const
{
	if (!RelativitySystem || !OwningPlayerController)
	{
		return 0;
	}

	return RelativitySystem->GetTimeDebtSeverity(OwningPlayerController);
}

bool UPlayerTimeComponent::IsExperiencingExtremeDilation() const
{
	return CurrentTimeDilation < 0.5f;
}

float UPlayerTimeComponent::GetVelocityPercentageOfLightSpeed() const
{
	const float SpeedOfLight = 299792458.0f; // m/s
	return (CurrentVelocity / SpeedOfLight) * 100.0f;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void UPlayerTimeComponent::RegisterWithRelativitySystem()
{
	if (!OwningPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTimeComponent: Cannot register - no player controller"));
		return;
	}

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTimeComponent: Cannot register - no game instance"));
		return;
	}

	RelativitySystem = GameInstance->GetSubsystem<URelativitySystem>();
	if (!RelativitySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerTimeComponent: Cannot register - RelativitySystem not found"));
		return;
	}

	RelativitySystem->RegisterPlayer(OwningPlayerController, BiologicalAge);

	UE_LOG(LogTemp, Log, TEXT("PlayerTimeComponent: Registered with RelativitySystem"));
}

void UPlayerTimeComponent::UnregisterFromRelativitySystem()
{
	if (RelativitySystem && OwningPlayerController)
	{
		RelativitySystem->UnregisterPlayer(OwningPlayerController);
		UE_LOG(LogTemp, Log, TEXT("PlayerTimeComponent: Unregistered from RelativitySystem"));
	}

	RelativitySystem = nullptr;
	OwningPlayerController = nullptr;
}

APlayerController* UPlayerTimeComponent::FindOwningPlayerController() const
{
	// Check if owner is a player controller
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		return PC;
	}

	// Check if owner is a pawn with a player controller
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn)
	{
		return Cast<APlayerController>(Pawn->GetController());
	}

	return nullptr;
}
