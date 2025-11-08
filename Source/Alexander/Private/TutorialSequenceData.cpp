// Copyright Epic Games, Inc. All Rights Reserved.

#include "TutorialSequenceData.h"

TArray<FTutorialStep> UTutorialSequenceData::GetStepsByContext(ETutorialContext ContextFilter) const
{
	TArray<FTutorialStep> FilteredSteps;

	if (ContextFilter == Context || ContextFilter == ETutorialContext::General)
	{
		FilteredSteps = Steps;
	}

	return FilteredSteps;
}

FTutorialStep UTutorialSequenceData::GetStepByID(FName StepID) const
{
	for (const FTutorialStep& Step : Steps)
	{
		if (Step.StepID == StepID)
		{
			return Step;
		}
	}

	// Return empty step if not found
	return FTutorialStep();
}

bool UTutorialSequenceData::IsSequenceValidForPlayer(int32 PlayerLevel, const TArray<FName>& CompletedSequences) const
{
	// Check if prerequisites are met
	if (!ArePrerequisitesMet(CompletedSequences))
	{
		return false;
	}

	// Check if already completed and not repeatable
	if (!bRepeatable && CompletedSequences.Contains(SequenceID))
	{
		return false;
	}

	// Level-based filtering could be added here
	// For now, all sequences are valid if prerequisites are met

	return true;
}

int32 UTutorialSequenceData::GetTotalObjectiveCount() const
{
	int32 TotalObjectives = 0;

	for (const FTutorialStep& Step : Steps)
	{
		TotalObjectives += Step.Objectives.Num();
	}

	return TotalObjectives;
}

float UTutorialSequenceData::GetProgressPercentage(const TArray<FName>& CompletedSteps) const
{
	if (Steps.Num() == 0)
	{
		return 0.0f;
	}

	int32 CompletedCount = 0;

	for (const FTutorialStep& Step : Steps)
	{
		if (CompletedSteps.Contains(Step.StepID))
		{
			CompletedCount++;
		}
	}

	return static_cast<float>(CompletedCount) / static_cast<float>(Steps.Num()) * 100.0f;
}

TArray<FName> UTutorialSequenceData::GetNextAvailableSteps(const TArray<FName>& CompletedSteps) const
{
	TArray<FName> AvailableSteps;

	for (const FTutorialStep& Step : Steps)
	{
		// Skip if already completed
		if (CompletedSteps.Contains(Step.StepID))
		{
			continue;
		}

		// Check if all required previous steps are completed
		bool bCanStart = true;
		for (const FName& RequiredStep : Step.RequiredPreviousSteps)
		{
			if (!CompletedSteps.Contains(RequiredStep))
			{
				bCanStart = false;
				break;
			}
		}

		if (bCanStart)
		{
			AvailableSteps.Add(Step.StepID);
		}
	}

	return AvailableSteps;
}

bool UTutorialSequenceData::ArePrerequisitesMet(const TArray<FName>& CompletedSequences) const
{
	for (const FName& PrereqSequence : PrerequisiteSequences)
	{
		if (!CompletedSequences.Contains(PrereqSequence))
		{
			return false;
		}
	}

	return true;
}
