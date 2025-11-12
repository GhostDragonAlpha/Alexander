// Copyright Epic Games, Inc. All Rights Reserved.

#include "SystemEvents.h"

// Initialize static instance
USystemEventHub* USystemEventHub::Instance = nullptr;

USystemEventHub* USystemEventHub::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<USystemEventHub>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}
