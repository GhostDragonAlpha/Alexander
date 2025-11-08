// Copyright Epic Games, Inc. All Rights Reserved.

#include "DialogueContentDatabase.h"

TArray<FDialogueEntry> UDialogueContentDatabase::GetGreetingsForPersonality(FName Personality) const
{
	if (Greetings.Contains(Personality))
	{
		return Greetings[Personality].Entries;
	}

	// Return neutral/default greetings if personality not found
	if (Greetings.Contains(FName("Neutral")))
	{
		return Greetings[FName("Neutral")].Entries;
	}

	return TArray<FDialogueEntry>();
}

TArray<FDialogueEntry> UDialogueContentDatabase::GetFarewellsForPersonality(FName Personality) const
{
	if (Farewells.Contains(Personality))
	{
		return Farewells[Personality].Entries;
	}

	// Return neutral/default farewells if personality not found
	if (Farewells.Contains(FName("Neutral")))
	{
		return Farewells[FName("Neutral")].Entries;
	}

	return TArray<FDialogueEntry>();
}

TArray<FDialogueEntry> UDialogueContentDatabase::GetDialogueForContext(
	const FAIDialogueContext& Context,
	EDialogueCategory Category) const
{
	TArray<FDialogueEntry> ValidDialogue;
	TArray<FDialogueEntry> SourceDialogue;

	// Get source dialogue based on category
	switch (Category)
	{
	case EDialogueCategory::Greeting:
		if (Greetings.Contains(Context.NPCPersonality))
		{
			SourceDialogue = Greetings[Context.NPCPersonality].Entries;
		}
		break;

	case EDialogueCategory::Farewell:
		if (Farewells.Contains(Context.NPCPersonality))
		{
			SourceDialogue = Farewells[Context.NPCPersonality].Entries;
		}
		break;

	case EDialogueCategory::Mission:
		SourceDialogue = MissionDialogue;
		break;

	case EDialogueCategory::Trading:
		SourceDialogue = TradingDialogue;
		break;

	case EDialogueCategory::Combat:
		SourceDialogue = CombatDialogue;
		break;

	case EDialogueCategory::Information:
		SourceDialogue = InformationDialogue;
		break;

	case EDialogueCategory::Ambient:
		SourceDialogue = AmbientDialogue;
		break;

	case EDialogueCategory::Reaction:
		if (Context.ConversationContext != NAME_None &&
			ReactionDialogue.Contains(Context.ConversationContext))
		{
			SourceDialogue = ReactionDialogue[Context.ConversationContext].Entries;
		}
		break;

	default:
		SourceDialogue = AllDialogue;
		break;
	}

	// Filter by context
	for (const FDialogueEntry& Entry : SourceDialogue)
	{
		if (IsDialogueValidForContext(Entry, Context))
		{
			ValidDialogue.Add(Entry);
		}
	}

	return ValidDialogue;
}

FDialogueEntry UDialogueContentDatabase::GetRandomDialogue(
	const FAIDialogueContext& Context,
	EDialogueCategory Category) const
{
	TArray<FDialogueEntry> ValidDialogue = GetDialogueForContext(Context, Category);

	if (ValidDialogue.Num() == 0)
	{
		return FDialogueEntry();
	}

	// Weight by priority
	TArray<FDialogueEntry> WeightedDialogue;
	for (const FDialogueEntry& Entry : ValidDialogue)
	{
		for (int32 i = 0; i < Entry.Priority; i++)
		{
			WeightedDialogue.Add(Entry);
		}
	}

	if (WeightedDialogue.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, WeightedDialogue.Num() - 1);
		return WeightedDialogue[RandomIndex];
	}

	return FDialogueEntry();
}

bool UDialogueContentDatabase::IsDialogueValidForContext(
	const FDialogueEntry& Dialogue,
	const FAIDialogueContext& Context) const
{
	// Check personality match if specified
	if (Dialogue.SpeakerPersonality != NAME_None && 
		Dialogue.SpeakerPersonality != Context.NPCPersonality)
	{
		return false;
	}

	// Check required tags
	for (const FName& RequiredTag : Dialogue.RequiredTags)
	{
		if (!Context.ActiveTags.Contains(RequiredTag))
		{
			return false;
		}
	}

	// Check excluded tags
	for (const FName& ExcludedTag : Dialogue.ExcludedTags)
	{
		if (Context.ActiveTags.Contains(ExcludedTag))
		{
			return false;
		}
	}

	// Check if can repeat
	if (!Dialogue.bCanRepeat && Context.TimesSpoken > 0)
	{
		return false;
	}

	return true;
}

FDialogueEntry UDialogueContentDatabase::FindDialogueByID(FName DialogueID) const
{
	for (const FDialogueEntry& Entry : AllDialogue)
	{
		if (Entry.DialogueID == DialogueID)
		{
			return Entry;
		}
	}

	return FDialogueEntry();
}

TArray<FName> UDialogueContentDatabase::GetAllDialogueIDs() const
{
	TArray<FName> IDs;

	for (const FDialogueEntry& Entry : AllDialogue)
	{
		IDs.Add(Entry.DialogueID);
	}

	return IDs;
}

TArray<FDialogueEntry> UDialogueContentDatabase::GetDialogueByTone(FName EmotionalTone) const
{
	TArray<FDialogueEntry> Result;

	for (const FDialogueEntry& Entry : AllDialogue)
	{
		if (Entry.EmotionalTone == EmotionalTone)
		{
			Result.Add(Entry);
		}
	}

	return Result;
}

void UDialogueContentDatabase::RebuildDialogueIndex()
{
	AllDialogue.Empty();

	// Add all greetings
	for (const auto& Pair : Greetings)
	{
		AllDialogue.Append(Pair.Value.Entries);
	}

	// Add all farewells
	for (const auto& Pair : Farewells)
	{
		AllDialogue.Append(Pair.Value.Entries);
	}

	// Add categorized dialogue
	AllDialogue.Append(MissionDialogue);
	AllDialogue.Append(TradingDialogue);
	AllDialogue.Append(CombatDialogue);
	AllDialogue.Append(InformationDialogue);
	AllDialogue.Append(AmbientDialogue);

	// Add all reactions
	for (const auto& Pair : ReactionDialogue)
	{
		AllDialogue.Append(Pair.Value.Entries);
	}

	UE_LOG(LogTemp, Log, TEXT("Rebuilt dialogue index: %d total entries"), AllDialogue.Num());
}
