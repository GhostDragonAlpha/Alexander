// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionTemplateDataAsset.h"

TArray<FMissionTemplateData> UMissionTemplateDataAssetClass::GetTemplatesByType(EMissionTypeDataAsset Type) const
{
	TArray<FMissionTemplateData> Result;

	for (const FMissionTemplateData& Template : Templates)
	{
		if (Template.Type == Type)
		{
			Result.Add(Template);
		}
	}

	return Result;
}

TArray<FMissionTemplateData> UMissionTemplateDataAssetClass::GetTemplatesForFaction(FName FactionID) const
{
	TArray<FMissionTemplateData> Result;

	for (const FMissionTemplateData& Template : Templates)
	{
		if (Template.FactionID == FactionID || Template.FactionID == NAME_None)
		{
			Result.Add(Template);
		}
	}

	return Result;
}

TArray<FMissionTemplateData> UMissionTemplateDataAssetClass::GetTemplatesForContext(const FMissionContextDataAsset& Context) const
{
	TArray<FMissionTemplateData> Result;

	for (const FMissionTemplateData& Template : Templates)
	{
		if (IsTemplateValidForContext(Template, Context))
		{
			Result.Add(Template);
		}
	}

	return Result;
}

FMissionTemplateData UMissionTemplateDataAssetClass::GetRandomTemplate(EMissionTypeDataAsset Type, int32 PlayerLevel) const
{
	TArray<FMissionTemplateData> ValidTemplates;

	for (const FMissionTemplateData& Template : Templates)
	{
		if (Template.Type == Type && 
			PlayerLevel >= Template.MinPlayerLevel && 
			PlayerLevel <= Template.MaxPlayerLevel)
		{
			// Add based on weight
			for (int32 i = 0; i < Template.GenerationWeight; i++)
			{
				ValidTemplates.Add(Template);
			}
		}
	}

	if (ValidTemplates.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ValidTemplates.Num() - 1);
		return ValidTemplates[RandomIndex];
	}

	return FMissionTemplateData();
}

bool UMissionTemplateDataAssetClass::IsTemplateValidForContext(
	const FMissionTemplateData& Template, 
	const FMissionContextDataAsset& Context) const
{
	// Check player level
	if (Context.PlayerLevel < Template.MinPlayerLevel || 
		Context.PlayerLevel > Template.MaxPlayerLevel)
	{
		return false;
	}

	// Check required tags
	for (const FName& RequiredTag : Template.RequiredTags)
	{
		if (!Context.AvailableTags.Contains(RequiredTag))
		{
			return false;
		}
	}

	// Check excluded tags
	for (const FName& ExcludedTag : Template.ExcludedTags)
	{
		if (Context.AvailableTags.Contains(ExcludedTag))
		{
			return false;
		}
	}

	// Check faction reputation
	if (Template.FactionID != NAME_None && 
		Template.FactionID == Context.NearestFaction)
	{
		if (Context.FactionReputation < Template.MinReputation)
		{
			return false;
		}
	}

	// Check danger level
	if (!Context.bAllowDangerousMissions && Template.DifficultyLevel > 5)
	{
		return false;
	}

	return true;
}

TArray<FName> UMissionTemplateDataAssetClass::GetAllTemplateIDs() const
{
	TArray<FName> IDs;

	for (const FMissionTemplateData& Template : Templates)
	{
		IDs.Add(Template.TemplateID);
	}

	return IDs;
}

FMissionTemplateData UMissionTemplateDataAssetClass::FindTemplateByID(FName TemplateID) const
{
	for (const FMissionTemplateData& Template : Templates)
	{
		if (Template.TemplateID == TemplateID)
		{
			return Template;
		}
	}

	return FMissionTemplateData();
}

TArray<FMissionTemplateData> UMissionTemplateDataAssetClass::GetTemplatesByDifficulty(
	int32 MinDifficulty, 
	int32 MaxDifficulty) const
{
	TArray<FMissionTemplateData> Result;

	for (const FMissionTemplateData& Template : Templates)
	{
		if (Template.DifficultyLevel >= MinDifficulty && 
			Template.DifficultyLevel <= MaxDifficulty)
		{
			Result.Add(Template);
		}
	}

	return Result;
}
