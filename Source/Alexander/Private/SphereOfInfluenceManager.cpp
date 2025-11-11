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

bool USphereOfInfluenceManager::CheckSOITransition(AOrbitalBody* Body) const
{
    if (!Body || !Body->OrbitTarget.IsValid())
    {
        return false;
    }

    // Get current distance from primary
    FVector CurrentPosition = Body->GetActorLocation();
    FVector PrimaryPosition = Body->OrbitTarget->GetActorLocation();
    float CurrentDistance = FVector::Dist(CurrentPosition, PrimaryPosition);

    // Calculate SOI radius of current primary
    float CurrentSOI = CalculateSphereOfInfluence(Body->OrbitTarget.Get());

    // Check if we're near the edge of current SOI
    float DistanceRatio = CurrentDistance / CurrentSOI;

    // If we're beyond the transition threshold, check for new primary
    if (DistanceRatio > TransitionThreshold)
    {
        // Find the dominant body at current position
        AOrbitalBody* DominantBody = GetDominantBody(CurrentPosition);
        
        // If dominant body is different from current primary, transition needed
        if (DominantBody && DominantBody != Body->OrbitTarget.Get())
        {
            return true;
        }
    }

    return false;
}

void USphereOfInfluenceManager::HandleSOITransition(AOrbitalBody* Body)
{
    if (!Body)
    {
        return;
    }

    float StartTime = FPlatformTime::Seconds();

    // Find the dominant body at the body's current position
    AOrbitalBody* NewPrimary = GetDominantBody(Body->GetActorLocation());
    
    if (!NewPrimary || NewPrimary == Body->OrbitTarget.Get())
    {
        return;
    }

    // Perform the transition
    PerformSOITransition(Body, NewPrimary);

    // Update statistics
    TransitionCount++;
    TotalTransitionTime += (FPlatformTime::Seconds() - StartTime);

    // Broadcast transition event
    FSOITransitionEvent TransitionEvent;
    TransitionEvent.Body = Body;
    TransitionEvent.PreviousPrimary = Body->OrbitTarget.Get();
    TransitionEvent.NewPrimary = NewPrimary;
    TransitionEvent.TransitionPosition = Body->GetActorLocation();
    TransitionEvent.TransitionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    OnSOITransition.Broadcast(TransitionEvent);
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

void USphereOfInfluenceManager::UpdateSphereOfInfluenceTransitions()
{
    for (const TWeakObjectPtr<AOrbitalBody>& BodyPtr : RegisteredBodies)
    {
        AOrbitalBody* Body = BodyPtr.Get();
        if (!Body || !Body->OrbitTarget.IsValid())
        {
            continue;
        }

        // Check if this body needs to transition
        if (CheckSOITransition(Body))
        {
            HandleSOITransition(Body);
        }
    }
}

void USphereOfInfluenceManager::PerformSOITransition(AOrbitalBody* Body, AOrbitalBody* NewPrimary)
{
    if (!Body || !NewPrimary)
    {
        return;
    }

    AOrbitalBody* OldPrimary = Body->OrbitTarget.Get();
    if (!OldPrimary)
    {
        return;
    }

    // Calculate current velocity relative to old primary
    FVector CurrentVelocity = Body->Velocity;
    
    // Get old primary's velocity
    FVector OldPrimaryVelocity = OldPrimary->Velocity;
    
    // Calculate absolute velocity
    FVector AbsoluteVelocity = CurrentVelocity + OldPrimaryVelocity;

    // Update orbit target
    Body->OrbitTarget = NewPrimary;

    // Calculate new velocity relative to new primary
    FVector NewPrimaryVelocity = NewPrimary->Velocity;
    Body->Velocity = AbsoluteVelocity - NewPrimaryVelocity;

    // Update orbital mechanics component if present
    if (Body->OrbitalMechanics)
    {
        Body->OrbitalMechanics->SetPrimaryBody(NewPrimary);
        Body->OrbitalMechanics->UpdateOrbitalElements();
    }

    UE_LOG(LogTemp, Log, TEXT("SOI Transition: %s moved from %s to %s"), 
           *Body->GetName(), *OldPrimary->GetName(), *NewPrimary->GetName());
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