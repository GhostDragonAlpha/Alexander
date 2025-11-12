// Copyright Epic Games, Inc. All Rights Reserved.

#include "SphereOfInfluenceManager.h"
#include "OrbitalBody.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USphereOfInfluenceManager::USphereOfInfluenceManager()
{
    bAutomaticTransitions = true;
    TransitionThreshold = 0.9f; // Transition at 90% of SOI radius
    MinTransitionTime = 1.0f; // Minimum 1 second between transitions
    TransitionCount = 0;
    TotalTransitionTime = 0.0f;
}

void USphereOfInfluenceManager::Initialize(AOrbitalBody* Root)
{
    if (!Root)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize SOI manager with null root body"));
        return;
    }

    RootBody = Root;
    RegisteredBodies.Empty();
    BodyNodeMap.Empty();
    
    // Register the root body
    RegisterBody(Root);
    
    // Build initial hierarchy
    BuildHierarchy();
    
    UE_LOG(LogTemp, Log, TEXT("SOI Manager initialized with root: %s"), *Root->GetName());
}

void USphereOfInfluenceManager::UpdateSOI(float DeltaTime)
{
    if (!RootBody.IsValid())
    {
        return;
    }

    // Update all sphere of influence radii
    UpdateAllSphereOfInfluences();

    // Check for SOI transitions if automatic transitions are enabled
    if (bAutomaticTransitions)
    {
        UpdateSphereOfInfluenceTransitions();
    }

    // Draw debug visualization if enabled
    if (GetWorld() && GetWorld()->WorldType == EWorldType::Game)
    {
        // Debug visualization would go here
    }
}

void USphereOfInfluenceManager::UpdateSphereOfInfluenceTransitions()
{
    // Check all registered bodies for potential SOI transitions
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            if (CheckSOITransition(Body))
            {
                HandleSOITransition(Body);
            }
        }
    }
}

bool USphereOfInfluenceManager::CheckSOITransition(AOrbitalBody* Body) const
{
    if (!Body || !Body->OrbitTarget.IsValid())
    {
        return false;
    }

    AOrbitalBody* Parent = Body->OrbitTarget.Get();
    if (!Parent)
    {
        return false;
    }

    // Get the current distance from body to parent
    float DistanceToParent = FVector::Dist(Body->GetActorLocation(), Parent->GetActorLocation());
    
    // Get the SOI radius of the parent
    float ParentSOIRadius = CalculateSphereOfInfluence(Parent);
    
    // Check if body is within transition threshold of parent's SOI
    return DistanceToParent >= (ParentSOIRadius * TransitionThreshold);
}

void USphereOfInfluenceManager::HandleSOITransition(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Find the new parent body (the one with the strongest gravitational influence)
    AOrbitalBody* NewParent = GetDominantBody(Body->GetActorLocation());
    
    if (NewParent && NewParent != Body->OrbitTarget.Get())
    {
        // Record transition
        FDateTime TransitionTime = FDateTime::UtcNow();
        TransitionCount++;
        
        // Calculate transition time (simplified)
        float TransitionDuration = FMath::RandRange(0.5f, 2.0f);
        TotalTransitionTime += TransitionDuration;
        
        // Update parent relationship
        Body->OrbitTarget = NewParent;
        
        // Rebuild hierarchy if needed
        BuildHierarchy();
        
        UE_LOG(LogTemp, Log, TEXT("SOI Transition: %s moved from %s to %s (Duration: %.2fs)"),
            *Body->GetName(),
            Body->OrbitTarget.IsValid() ? *Body->OrbitTarget->GetName() : TEXT("None"),
            NewParent ? *NewParent->GetName() : TEXT("None"),
            TransitionDuration);
    }
}

AOrbitalBody* USphereOfInfluenceManager::GetDominantBody(const FVector& Position) const
{
    AOrbitalBody* DominantBody = nullptr;
    float MaxInfluence = 0.0f;

    // Check all registered bodies
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        AOrbitalBody* Body = BodyPtr.Get();
        if (!Body || Body == RootBody.Get())
        {
            continue;
        }

        // Calculate gravitational influence at position
        FVector BodyPosition = Body->GetActorLocation();
        float Distance = FVector::Dist(Position, BodyPosition);
        
        if (Distance < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        // Gravitational influence ∝ Mass / Distance²
        float Influence = Body->Mass / (Distance * Distance);
        
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBody = Body;
        }
    }

    // If no dominant body found, return root
    if (!DominantBody && RootBody.IsValid())
    {
        DominantBody = RootBody.Get();
    }

    return DominantBody;
}

TArray<AOrbitalBody*> USphereOfInfluenceManager::GetInfluentialBodies(const FVector& Position) const
{
    TArray<AOrbitalBody*> InfluentialBodies;

    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        AOrbitalBody* Body = BodyPtr.Get();
        if (!Body)
        {
            continue;
        }

        if (IsInSphereOfInfluence(Position, Body))
        {
            InfluentialBodies.Add(Body);
        }
    }

    return InfluentialBodies;
}

float USphereOfInfluenceManager::CalculateSphereOfInfluence(AOrbitalBody* Body) const
{
    if (!Body || !Body->OrbitTarget.IsValid())
    {
        return 0.0f;
    }

    // Get distance to primary body
    FVector BodyPosition = Body->GetActorLocation();
    FVector PrimaryPosition = Body->OrbitTarget->GetActorLocation();
    float Distance = FVector::Dist(BodyPosition, PrimaryPosition);

    // Calculate mass ratio
    float PrimaryMass = Body->OrbitTarget->Mass;
    if (PrimaryMass < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    float MassRatio = Body->Mass / PrimaryMass;

    // SOI radius: r = a × (m/M)^(2/5)
    // Where a is the semi-major axis (approximated by current distance)
    float SOIRadius = Distance * FMath::Pow(MassRatio, 0.4f);

    return SOIRadius;
}

bool USphereOfInfluenceManager::IsInSphereOfInfluence(const FVector& Position, AOrbitalBody* Body) const
{
    if (!Body)
    {
        return false;
    }

    float SOIRadius = CalculateSphereOfInfluence(Body);
    if (SOIRadius <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    float Distance = FVector::Dist(Position, Body->GetActorLocation());
    return Distance <= SOIRadius;
}

void USphereOfInfluenceManager::RegisterBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    // Check if already registered
    for (const TWeakObjectPtr<AOrbitalBody>& RegisteredBody : RegisteredBodies)
    {
        if (RegisteredBody.Get() == Body)
        {
            return;
        }
    }

    RegisteredBodies.Add(Body);
    UE_LOG(LogTemp, Verbose, TEXT("Registered body %s with SOI Manager"), *Body->GetName());
}

void USphereOfInfluenceManager::UnregisterBody(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    RegisteredBodies.Remove(Body);
    BodyNodeMap.Remove(Body->GetUniqueID());
    UE_LOG(LogTemp, Verbose, TEXT("Unregistered body %s from SOI Manager"), *Body->GetName());
}

void USphereOfInfluenceManager::UpdateHierarchy()
{
    BuildHierarchy();
}

FSOINode USphereOfInfluenceManager::GetSOIHierarchy() const
{
    return HierarchyRoot;
}

TArray<AOrbitalBody*> USphereOfInfluenceManager::GetChildren(AOrbitalBody* Body) const
{
    TArray<AOrbitalBody*> Children;
    
    if (!Body)
    {
        return Children;
    }

    // Find body in hierarchy and get its children
    const FSOINode* Node = BodyNodeMap.FindRef(Body->GetUniqueID());
    if (Node)
    {
        for (const TWeakObjectPtr<AOrbitalBody>& ChildPtr : Node->Children)
        {
            if (AOrbitalBody* Child = ChildPtr.Get())
            {
                Children.Add(Child);
            }
        }
    }

    return Children;
}

AOrbitalBody* USphereOfInfluenceManager::GetParent(AOrbitalBody* Body) const
{
    if (!Body)
    {
        return nullptr;
    }

    const FSOINode* Node = BodyNodeMap.FindRef(Body->GetUniqueID());
    if (Node)
    {
        return Node->Parent.Get();
    }

    return nullptr;
}

int32 USphereOfInfluenceManager::GetHierarchyLevel(AOrbitalBody* Body) const
{
    if (!Body)
    {
        return -1;
    }

    const FSOINode* Node = BodyNodeMap.FindRef(Body->GetUniqueID());
    if (Node)
    {
        return Node->HierarchyLevel;
    }

    return -1;
}

void USphereOfInfluenceManager::BuildHierarchy()
{
    HierarchyRoot = FSOINode();
    BodyNodeMap.Empty();

    if (!RootBody.IsValid())
    {
        return;
    }

    // Set up root node
    HierarchyRoot.Body = RootBody.Get();
    HierarchyRoot.Parent = nullptr;
    HierarchyRoot.HierarchyLevel = 0;
    HierarchyRoot.SphereOfInfluenceRadius = CalculateSphereOfInfluence(RootBody.Get());
    BodyNodeMap.Add(RootBody->GetUniqueID(), &HierarchyRoot);

    // Build hierarchy recursively
    TArray<AOrbitalBody*> AllBodies = GetAllRegisteredBodies();
    for (AOrbitalBody* Body : AllBodies)
    {
        if (Body == RootBody.Get())
        {
            continue;
        }

        AOrbitalBody* Parent = FindAppropriateParent(Body);
        if (Parent)
        {
            // Add to parent's children
            FSOINode* ParentNode = BodyNodeMap.FindRef(Parent->GetUniqueID());
            if (ParentNode)
            {
                ParentNode->Children.Add(Body);
                
                // Create node for this body
                FSOINode* BodyNode = new FSOINode();
                BodyNode->Body = Body;
                BodyNode->Parent = Parent;
                BodyNode->HierarchyLevel = ParentNode->HierarchyLevel + 1;
                BodyNode->SphereOfInfluenceRadius = CalculateSphereOfInfluence(Body);
                
                BodyNodeMap.Add(Body->GetUniqueID(), BodyNode);
            }
        }
    }
}

AOrbitalBody* USphereOfInfluenceManager::FindAppropriateParent(AOrbitalBody* Body) const
{
    if (!Body || !RootBody.IsValid())
    {
        return nullptr;
    }

    AOrbitalBody* BestParent = RootBody.Get();
    float MinDistance = FVector::Dist(Body->GetActorLocation(), RootBody->GetActorLocation());

    // Check all other bodies for closer parents
    for (const TWeakObjectPtr<AOrbitalBody>& CandidatePtr : RegisteredBodies)
    {
        AOrbitalBody* Candidate = CandidatePtr.Get();
        if (!Candidate || Candidate == Body || Candidate == RootBody.Get())
        {
            continue;
        }

        float Distance = FVector::Dist(Body->GetActorLocation(), Candidate->GetActorLocation());
        float CandidateSOI = CalculateSphereOfInfluence(Candidate);

        // If body is within candidate's SOI, candidate is a better parent
        if (Distance < CandidateSOI && Distance < MinDistance)
        {
            MinDistance = Distance;
            BestParent = Candidate;
        }
    }

    return BestParent;
}

void USphereOfInfluenceManager::UpdateAllSphereOfInfluences()
{
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        AOrbitalBody* Body = BodyPtr.Get();
        if (!Body)
        {
            continue;
        }

        FSOINode* Node = BodyNodeMap.FindRef(Body->GetUniqueID());
        if (Node)
        {
            Node->SphereOfInfluenceRadius = CalculateSphereOfInfluence(Body);
        }
    }
}

float USphereOfInfluenceManager::CalculateGravitationalParameter(AOrbitalBody* Body) const
{
    if (!Body)
    {
        return 0.0f;
    }

    // μ = G × M
    const float GravitationalConstant = 6.67430e-11f; // m³/(kg·s²)
    return GravitationalConstant * Body->Mass;
}

TArray<AOrbitalBody*> USphereOfInfluenceManager::GetAllRegisteredBodies() const
{
    TArray<AOrbitalBody*> Bodies;
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        if (AOrbitalBody* Body = BodyPtr.Get())
        {
            Bodies.Add(Body);
        }
    }
    return Bodies;
}