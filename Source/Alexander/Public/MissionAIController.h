// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FirstMissionGameMode.h"
#include "MissionAIController.generated.h"

class ASurfacePlayerCharacter;
class ASpaceship;
class ALandingGuidanceSystem;

/**
 * Mission Stage Actions - What the AI should do at each stage
 */
UENUM(BlueprintType)
enum class EMissionAIAction : uint8
{
	Idle UMETA(DisplayName = "Idle - Waiting"),
	WalkToSpaceship UMETA(DisplayName = "Walk To Spaceship"),
	EnterSpaceship UMETA(DisplayName = "Enter Spaceship"),
	WaitForTutorial UMETA(DisplayName = "Wait For Tutorial"),
	TakeOff UMETA(DisplayName = "Take Off"),
	FlyToMoon UMETA(DisplayName = "Fly To Moon"),
	LandOnMoon UMETA(DisplayName = "Land On Moon"),
	ExploreMoon UMETA(DisplayName = "Explore Moon"),
	ReturnToShip UMETA(DisplayName = "Return To Ship"),
	FlyToPlanet UMETA(DisplayName = "Fly To Planet"),
	LandOnPlanet UMETA(DisplayName = "Land On Planet"),
	ExitShip UMETA(DisplayName = "Exit Ship"),
	MissionComplete UMETA(DisplayName = "Mission Complete")
};

/**
 * AI Controller for automated mission playthrough
 * Controls character/ship autonomously through all mission stages
 */
UCLASS()
class ALEXANDER_API AMissionAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMissionAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	// ==================== Main Control ====================
	
	/** Start automated mission playthrough */
	UFUNCTION(BlueprintCallable, Category = "Mission AI")
	void StartAutomatedMission();
	
	/** Stop automated playthrough */
	UFUNCTION(BlueprintCallable, Category = "Mission AI")
	void StopAutomatedMission();
	
	/** Pause/Resume automation */
	UFUNCTION(BlueprintCallable, Category = "Mission AI")
	void SetAutomationPaused(bool bPaused);
	
	// ==================== Configuration ====================
	
	/** Enable automatic screenshots at each stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission AI")
	bool bCaptureScreenshots;
	
	/** Enable debug logging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission AI")
	bool bDebugMode;
	
	/** Movement speed multiplier (for faster demos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission AI", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SpeedMultiplier;
	
	/** Time to wait at each stage before proceeding (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission AI")
	float StageDelayTime;
	
	// ==================== Events ====================
	
	/** Called when AI completes an action */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIActionCompleted, EMissionAIAction, Action, bool, bSuccess);
	UPROPERTY(BlueprintAssignable, Category = "Mission AI")
	FOnAIActionCompleted OnAIActionCompleted;
	
	/** Called when AI reaches a mission milestone */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionMilestone, FString, MilestoneName);
	UPROPERTY(BlueprintAssignable, Category = "Mission AI")
	FOnMissionMilestone OnMissionMilestone;

protected:
	// ==================== Mission Tracking ====================
	
	/** Current mission stage */
	UPROPERTY(BlueprintReadOnly, Category = "Mission AI")
	EMissionStage CurrentMissionStage;
	
	/** Current AI action */
	UPROPERTY(BlueprintReadOnly, Category = "Mission AI")
	EMissionAIAction CurrentAction;
	
	/** Is automation active */
	UPROPERTY(BlueprintReadOnly, Category = "Mission AI")
	bool bIsAutomating;
	
	/** Is automation paused */
	UPROPERTY(BlueprintReadOnly, Category = "Mission AI")
	bool bIsPaused;
	
	/** Time spent on current action */
	float ActionTimer;
	
	/** Mission start time */
	float MissionStartTime;
	
	// ==================== References ====================
	
	/** Reference to mission game mode */
	UPROPERTY()
	AFirstMissionGameMode* MissionGameMode;
	
	/** Reference to surface character (when on foot) */
	UPROPERTY()
	ASurfacePlayerCharacter* SurfaceCharacter;
	
	/** Reference to spaceship (when flying) */
	UPROPERTY()
	ASpaceship* Spaceship;
	
	/** Reference to landing guidance system */
	UPROPERTY()
	ALandingGuidanceSystem* LandingGuidance;
	
	/** Target spaceship location */
	FVector SpaceshipLocation;
	
	/** Target moon location */
	FVector MoonLocation;
	
	/** Target planet location */
	FVector PlanetLocation;
	
	// ==================== Action Handlers ====================
	
	/** Handle mission stage change */
	UFUNCTION()
	void OnMissionStageChanged(EMissionStage NewStage);
	
	/** Execute current AI action */
	void ExecuteCurrentAction(float DeltaTime);
	
	/** Complete current action and move to next */
	void CompleteAction(bool bSuccess);
	
	/** Transition to next action based on mission stage */
	void TransitionToNextAction();
	
	// ==================== Ground Movement ====================
	
	/** Walk to spaceship using AI navigation */
	void Action_WalkToSpaceship(float DeltaTime);
	
	/** Enter spaceship */
	void Action_EnterSpaceship(float DeltaTime);
	
	/** Explore moon surface */
	void Action_ExploreMoon(float DeltaTime);
	
	/** Return to ship on moon */
	void Action_ReturnToShip(float DeltaTime);
	
	/** Exit ship */
	void Action_ExitShip(float DeltaTime);
	
	// ==================== Flight Control ====================
	
	/** Take off from surface */
	void Action_TakeOff(float DeltaTime);
	
	/** Fly to moon */
	void Action_FlyToMoon(float DeltaTime);
	
	/** Land on moon */
	void Action_LandOnMoon(float DeltaTime);
	
	/** Fly back to planet */
	void Action_FlyToPlanet(float DeltaTime);
	
	/** Land on planet */
	void Action_LandOnPlanet(float DeltaTime);
	
	// ==================== Helper Functions ====================
	
	/** Apply input to character */
	void ApplyCharacterInput(FVector2D MoveInput, FVector2D LookInput);
	
	/** Apply input to spaceship */
	void ApplySpaceshipInput(float Throttle, float Pitch, float Yaw, float Roll);
	
	/** Simulate button press */
	void SimulateButtonPress(FName ActionName);
	
	/** Find spaceship in level */
	void FindSpaceship();
	
	/** Find moon and planet locations */
	void FindCelestialBodies();
	
	/** Capture screenshot with stage name */
	void CaptureStageScreenshot(const FString& StageName);
	
	/** Log debug message */
	void LogDebug(const FString& Message);
	
	// ==================== State Tracking ====================
	
	/** Has found spaceship */
	bool bFoundSpaceship;
	
	/** Is currently in spaceship */
	bool bInSpaceship;
	
	/** Has taken off */
	bool bHasTakenOff;
	
	/** Has reached moon */
	bool bReachedMoon;
	
	/** Exploration distance traveled */
	float ExplorationDistance;
	
	/** Last exploration position */
	FVector LastExplorationPosition;
	
	/** Current flight target */
	FVector FlightTarget;
	
	/** Landing approach phase */
	int32 LandingPhase;

	/** Timer handle for delayed action transitions */
	FTimerHandle ActionTransitionTimerHandle;
};
