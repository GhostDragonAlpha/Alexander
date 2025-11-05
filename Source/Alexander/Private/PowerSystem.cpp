// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================================
// APowerNetworkNode
// ============================================================================

APowerNetworkNode::APowerNetworkNode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	// Create node mesh
	NodeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NodeMesh"));
	NodeMesh->SetupAttachment(RootComp);

	// Create connection range
	ConnectionRange = CreateDefaultSubobject<UBoxComponent>(TEXT("ConnectionRange"));
	ConnectionRange->SetupAttachment(RootComp);
	ConnectionRange->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f)); // 20m x 20m x 10m

	// Default properties
	ConnectionRangeCm = 2000.0f; // 20 meters
	UpdateInterval = 1.0f; // Update every second
	TimeSinceLastUpdate = 0.0f;
	NetworkID = -1;
}

void APowerNetworkNode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize network connections
	UpdateNetworkConnections();
}

void APowerNetworkNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastUpdate += DeltaTime;

	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		UpdateNetworkConnections();
		DistributePower();
		TimeSinceLastUpdate = 0.0f;
	}
}

void APowerNetworkNode::UpdateNetworkConnections()
{
	// Find power actors in range
	FindPowerActorsInRange();

	// Find and connect to nearby nodes
	TArray<APowerNetworkNode*> NearbyNodes = FindNearbyNodes();
	for (APowerNetworkNode* Node : NearbyNodes)
	{
		if (Node && !ConnectedNodes.Contains(Node))
		{
			ConnectToNode(Node);
		}
	}

	// Remove disconnected nodes
	for (int32 i = ConnectedNodes.Num() - 1; i >= 0; --i)
	{
		APowerNetworkNode* Node = ConnectedNodes[i];
		if (!Node || FVector::Dist(GetActorLocation(), Node->GetActorLocation()) > ConnectionRangeCm)
		{
			DisconnectFromNode(Node);
		}
	}

	// Assign network ID if not set
	if (NetworkID == -1)
	{
		NetworkID = GetUniqueID();
		PropagateNetworkID(NetworkID);
	}
}

float APowerNetworkNode::GetTotalAvailablePower() const
{
	float TotalPower = 0.0f;

	for (const TScriptInterface<IPowerProvider>& Provider : ConnectedProviders)
	{
		if (Provider.GetObject())
		{
			TotalPower += IPowerProvider::Execute_GetAvailablePower(Provider.GetObject());
		}
	}

	return TotalPower;
}

float APowerNetworkNode::GetTotalPowerDemand() const
{
	float TotalDemand = 0.0f;

	for (const TScriptInterface<IPowerConsumer>& Consumer : ConnectedConsumers)
	{
		if (Consumer.GetObject())
		{
			TotalDemand += IPowerConsumer::Execute_GetPowerRequirement(Consumer.GetObject());
		}
	}

	return TotalDemand;
}

bool APowerNetworkNode::HasSufficientPower() const
{
	return GetTotalAvailablePower() >= GetTotalPowerDemand();
}

void APowerNetworkNode::DistributePower()
{
	float AvailablePower = GetTotalAvailablePower();
	float TotalDemand = GetTotalPowerDemand();

	bool bHasSufficientPower = AvailablePower >= TotalDemand;

	// Update all consumers
	for (const TScriptInterface<IPowerConsumer>& Consumer : ConnectedConsumers)
	{
		if (Consumer.GetObject())
		{
			IPowerConsumer::Execute_SetPowered(Consumer.GetObject(), bHasSufficientPower);
		}
	}
}

void APowerNetworkNode::FindPowerActorsInRange()
{
	if (!GetWorld())
	{
		return;
	}

	// Clear existing connections
	ConnectedProviders.Empty();
	ConnectedConsumers.Empty();

	// Find all actors in range
	TArray<AActor*> OverlappingActors;
	ConnectionRange->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == this)
		{
			continue;
		}

		// Check if actor implements power provider interface
		if (Actor->GetClass()->ImplementsInterface(UPowerProvider::StaticClass()))
		{
			TScriptInterface<IPowerProvider> Provider;
			Provider.SetObject(Actor);
			Provider.SetInterface(Cast<IPowerProvider>(Actor));
			ConnectedProviders.Add(Provider);
		}

		// Check if actor implements power consumer interface
		if (Actor->GetClass()->ImplementsInterface(UPowerConsumer::StaticClass()))
		{
			TScriptInterface<IPowerConsumer> Consumer;
			Consumer.SetObject(Actor);
			Consumer.SetInterface(Cast<IPowerConsumer>(Actor));
			ConnectedConsumers.Add(Consumer);
		}
	}
}

void APowerNetworkNode::ConnectToNode(APowerNetworkNode* OtherNode)
{
	if (!OtherNode || OtherNode == this)
	{
		return;
	}

	if (!ConnectedNodes.Contains(OtherNode))
	{
		ConnectedNodes.Add(OtherNode);
	}

	if (!OtherNode->ConnectedNodes.Contains(this))
	{
		OtherNode->ConnectedNodes.Add(this);
	}

	// Merge network IDs
	if (NetworkID != OtherNode->NetworkID)
	{
		int32 NewNetworkID = FMath::Min(NetworkID, OtherNode->NetworkID);
		PropagateNetworkID(NewNetworkID);
		OtherNode->PropagateNetworkID(NewNetworkID);
	}
}

void APowerNetworkNode::DisconnectFromNode(APowerNetworkNode* OtherNode)
{
	if (!OtherNode)
	{
		return;
	}

	ConnectedNodes.Remove(OtherNode);
	OtherNode->ConnectedNodes.Remove(this);
}

TArray<APowerNetworkNode*> APowerNetworkNode::FindNearbyNodes() const
{
	TArray<APowerNetworkNode*> NearbyNodes;

	if (!GetWorld())
	{
		return NearbyNodes;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APowerNetworkNode::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		APowerNetworkNode* Node = Cast<APowerNetworkNode>(Actor);
		if (Node && Node != this)
		{
			float Distance = FVector::Dist(GetActorLocation(), Node->GetActorLocation());
			if (Distance <= ConnectionRangeCm)
			{
				NearbyNodes.Add(Node);
			}
		}
	}

	return NearbyNodes;
}

void APowerNetworkNode::PropagateNetworkID(int32 NewNetworkID)
{
	if (NetworkID == NewNetworkID)
	{
		return;
	}

	NetworkID = NewNetworkID;

	// Propagate to connected nodes
	for (APowerNetworkNode* Node : ConnectedNodes)
	{
		if (Node && Node->NetworkID != NewNetworkID)
		{
			Node->PropagateNetworkID(NewNetworkID);
		}
	}
}

// ============================================================================
// Interface Default Implementations
// ============================================================================

float IPowerConsumer::Execute_GetPowerRequirement(const UObject* O)
{
	return 0.0f;
}

void IPowerConsumer::Execute_SetPowered(UObject* O, bool bIsPowered)
{
	// Default implementation does nothing
}

bool IPowerConsumer::Execute_IsPowered(const UObject* O)
{
	return false;
}

float IPowerProvider::Execute_GetAvailablePower(const UObject* O)
{
	return 0.0f;
}

float IPowerProvider::Execute_GetMaxPowerCapacity(const UObject* O)
{
	return 0.0f;
}

EPowerSourceType IPowerProvider::Execute_GetPowerSourceType(const UObject* O)
{
	return EPowerSourceType::Grid;
}
