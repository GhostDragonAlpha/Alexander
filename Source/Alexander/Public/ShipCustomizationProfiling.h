// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Ship Customization Performance Profiling
 *
 * This header provides macros and stat declarations for profiling the ship customization system.
 * Enable ENABLE_SHIP_CUSTOMIZATION_PROFILING to activate profiling.
 */

// Enable/disable ship customization profiling
// Set to 0 to disable all profiling overhead in shipping builds
#ifndef ENABLE_SHIP_CUSTOMIZATION_PROFILING
	#define ENABLE_SHIP_CUSTOMIZATION_PROFILING !(UE_BUILD_SHIPPING)
#endif

// ============================================================================
// STAT GROUP DECLARATION
// ============================================================================

DECLARE_STATS_GROUP(TEXT("Ship Customization"), STATGROUP_ShipCustomization, STATCAT_Advanced);

// ============================================================================
// STAT DECLARATIONS - Customization Operations
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("CalculateTotalStats"), STAT_ShipCustomization_CalculateStats, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("EquipPart"), STAT_ShipCustomization_EquipPart, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("UnequipPart"), STAT_ShipCustomization_UnequipPart, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("EquipSkin"), STAT_ShipCustomization_EquipSkin, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("ApplyStatsToFlightController"), STAT_ShipCustomization_ApplyStats, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetEquippedPart"), STAT_ShipCustomization_GetEquippedPart, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetEquippedSkin"), STAT_ShipCustomization_GetEquippedSkin, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Data Queries
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("GetPartData"), STAT_ShipCustomization_GetPartData, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetSkinData"), STAT_ShipCustomization_GetSkinData, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetPartsInCategory"), STAT_ShipCustomization_GetPartsInCategory, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetUnlockedPartsInCategory"), STAT_ShipCustomization_GetUnlockedParts, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetAllSkins"), STAT_ShipCustomization_GetAllSkins, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("GetUnlockedSkins"), STAT_ShipCustomization_GetUnlockedSkins, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Progression
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("UnlockPart"), STAT_ShipCustomization_UnlockPart, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("UnlockSkin"), STAT_ShipCustomization_UnlockSkin, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("AddXP"), STAT_ShipCustomization_AddXP, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("AddCredits"), STAT_ShipCustomization_AddCredits, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("CheckLevelUp"), STAT_ShipCustomization_CheckLevelUp, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Loadout Management
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("SaveLoadout"), STAT_ShipCustomization_SaveLoadout, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("LoadLoadout"), STAT_ShipCustomization_LoadLoadout, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("DeleteLoadout"), STAT_ShipCustomization_DeleteLoadout, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Visual Updates
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("UpdateShipVisuals"), STAT_ShipCustomization_UpdateVisuals, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("ApplySkinMaterial"), STAT_ShipCustomization_ApplySkin, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Save/Load
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("SaveCustomizationData"), STAT_ShipCustomization_SaveData, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("LoadCustomizationData"), STAT_ShipCustomization_LoadData, STATGROUP_ShipCustomization);

// ============================================================================
// STAT DECLARATIONS - Initialization
// ============================================================================

DECLARE_CYCLE_STAT(TEXT("BeginPlay"), STAT_ShipCustomization_BeginPlay, STATGROUP_ShipCustomization);
DECLARE_CYCLE_STAT(TEXT("InitializeDefaultLoadout"), STAT_ShipCustomization_InitLoadout, STATGROUP_ShipCustomization);

// ============================================================================
// MEMORY STATS
// ============================================================================

DECLARE_MEMORY_STAT(TEXT("ShipCustomization Memory"), STAT_ShipCustomization_Memory, STATGROUP_ShipCustomization);

// ============================================================================
// CUSTOM PROFILING MACROS
// ============================================================================

#if ENABLE_SHIP_CUSTOMIZATION_PROFILING

	/**
	 * PROFILE_SHIP_CUSTOMIZATION_SCOPE - Profile a section of code
	 * Usage: PROFILE_SHIP_CUSTOMIZATION_SCOPE(FunctionName);
	 *
	 * This creates a scoped counter that tracks execution time.
	 * The stat will appear in: stat ShipCustomization
	 */
	#define PROFILE_SHIP_CUSTOMIZATION_SCOPE(StatName) \
		SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_##StatName)

	/**
	 * PROFILE_SHIP_CUSTOMIZATION_FRAME_BEGIN - Mark start of frame-critical code
	 * Not typically needed; SCOPE_CYCLE_COUNTER handles frame tracking
	 */
	#define PROFILE_SHIP_CUSTOMIZATION_FRAME_BEGIN() \
		{}

	/**
	 * LOG_SHIP_CUSTOMIZATION_PERF - Log performance warning if exceeds threshold
	 * Usage: LOG_SHIP_CUSTOMIZATION_PERF(TimeTaken, 0.1f);
	 */
	#define LOG_SHIP_CUSTOMIZATION_PERF(Time, Budget) \
		if (Time > Budget) { \
			UE_LOG(LogTemp, Warning, TEXT("[ShipCustomization] Performance budget exceeded: %.3fms / %.3fms"), Time * 1000.0f, Budget * 1000.0f); \
		}

#else

	// Empty macros when profiling is disabled
	#define PROFILE_SHIP_CUSTOMIZATION_SCOPE(StatName)
	#define PROFILE_SHIP_CUSTOMIZATION_FRAME_BEGIN()
	#define LOG_SHIP_CUSTOMIZATION_PERF(Time, Budget)

#endif

// ============================================================================
// PROFILING HELPER UTILITIES
// ============================================================================

#if ENABLE_SHIP_CUSTOMIZATION_PROFILING

/**
 * FShipCustomizationProfileScope
 * RAII-style helper for manual profiling with timing
 *
 * Usage:
 *   {
 *       FShipCustomizationProfileScope Scope(TEXT("MyOperation"));
 *       // ... code to profile ...
 *   } // Logs execution time on scope exit
 */
class FShipCustomizationProfileScope
{
public:
	explicit FShipCustomizationProfileScope(const TCHAR* InName, float InBudgetMs = 1000.0f)
		: Name(InName)
		, BudgetMs(InBudgetMs)
		, StartTime(FPlatformTime::Seconds())
	{
	}

	~FShipCustomizationProfileScope()
	{
		double ElapsedSeconds = FPlatformTime::Seconds() - StartTime;
		float ElapsedMs = static_cast<float>(ElapsedSeconds) * 1000.0f;

		if (ElapsedMs > BudgetMs)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShipCustomization] %s exceeded budget: %.3fms / %.1fms"),
				Name, ElapsedMs, BudgetMs);
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("[ShipCustomization] %s: %.3fms"),
				Name, ElapsedMs);
		}
	}

private:
	const TCHAR* Name;
	float BudgetMs;
	double StartTime;
};

#endif

// ============================================================================
// PERFORMANCE MONITORING STRUCTS
// ============================================================================

/**
 * FShipCustomizationPerformanceMetrics
 * Stores performance metrics for analysis
 */
struct FShipCustomizationPerformanceMetrics
{
	// Timing data (in milliseconds)
	float LastFrameTimeMs = 0.0f;
	float AvgFrameTimeMs = 0.0f;
	float MaxFrameTimeMs = 0.0f;
	float MinFrameTimeMs = 1000.0f;

	// Per-operation timings
	float LastStatCalcTimeMs = 0.0f;
	float LastEquipPartTimeMs = 0.0f;
	float LastApplyStatsTimeMs = 0.0f;

	// Memory stats
	uint64 MemoryUsageBytes = 0;
	uint64 PeakMemoryBytes = 0;

	// Frame counters
	uint32 FrameCount = 0;
	uint32 OperationCount = 0;

	// Helper to check if exceeding budget
	bool IsWithinBudget(float TimeMs, float BudgetMs) const
	{
		return TimeMs <= BudgetMs;
	}

	// Reset metrics
	void Reset()
	{
		LastFrameTimeMs = 0.0f;
		AvgFrameTimeMs = 0.0f;
		MaxFrameTimeMs = 0.0f;
		MinFrameTimeMs = 1000.0f;
		LastStatCalcTimeMs = 0.0f;
		LastEquipPartTimeMs = 0.0f;
		LastApplyStatsTimeMs = 0.0f;
		MemoryUsageBytes = 0;
		FrameCount = 0;
		OperationCount = 0;
	}
};
