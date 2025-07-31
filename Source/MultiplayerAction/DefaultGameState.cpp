// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"

ADefaultGameState::ADefaultGameState()
{
    // Default state
    bHasKingsKey = false;
}

void ADefaultGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADefaultGameState, bHasKingsKey);
}

// This runs on the server when the state is changed.
void ADefaultGameState::SetHasKingsKey(bool bNewState)
{
    if (HasAuthority()) // Only the server can change this state
    {
        bHasKingsKey = bNewState;

        // If the key was just acquired, call the OnRep manually for the server and broadcast the delegate.
        if (bHasKingsKey)
        {
            OnRep_KingsKeyAcquired();
        }
    }
}

// This runs on all clients when bHasKingsKey changes.
void ADefaultGameState::OnRep_KingsKeyAcquired()
{
    // When clients learn that the key has been acquired, broadcast the delegate locally.
    if (bHasKingsKey)
    {
        OnKingsKeyAcquiredDelegate.Broadcast();
    }
}
