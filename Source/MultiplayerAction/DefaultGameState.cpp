// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerActionCharacter.h"

ADefaultGameState::ADefaultGameState()
{
    // Default state
    bHasKingsKey = false;
    bStartedUnlockingChest = false;
}

void ADefaultGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADefaultGameState, bHasKingsKey);
    DOREPLIFETIME(ADefaultGameState, bStartedUnlockingChest);
    DOREPLIFETIME(ADefaultGameState, MatchState);
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

void ADefaultGameState::SetStartedUnlockingChest(bool bNewState)
{
    // This function must only be called on the server.
    if (HasAuthority())
    {
        bStartedUnlockingChest = bNewState;

        // If the state was just set to true, call the OnRep for the server
        // and broadcast the delegate.
        if (bStartedUnlockingChest)
        {
            OnRep_StartedUnlockingChest();
        }
    }
}

void ADefaultGameState::OnRep_StartedUnlockingChest()
{
    // When clients learn that the unlocking has started, broadcast the delegate locally.
    if (bStartedUnlockingChest)
    {
        OnStartedUnlockingChestDelegate.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("GameState (Client): Replicated that chest unlocking has started."));
    }
}

void ADefaultGameState::SetMatchState(EMatchState NewState)
{
    if (HasAuthority())
    {
        // Only change the state if it's different
        if (MatchState != NewState)
        {
            MatchState = NewState;
            // Call the OnRep manually for the server
            OnRep_MatchState();
        }
    }
}

// This function is automatically called on CLIENTS when MatchState changes
void ADefaultGameState::OnRep_MatchState()
{
    // This is where the magic happens on the client.
    // We will create the UI here.

    // Get the local player controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->IsLocalController())
    {
        // You'll need a way to get the widget class, e.g., from a GameInstance or PlayerController property.
        // For now, let's assume the PlayerController has the class reference.
        AMultiplayerActionCharacter* PlayerChar = PC->GetPawn<AMultiplayerActionCharacter>();
        if (PlayerChar)
        {
            PlayerChar->ShowEndOfMatchUI(MatchState);
        }
    }
}