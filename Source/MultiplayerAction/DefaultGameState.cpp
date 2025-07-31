// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"
#include "DefaultPlayerController.h"
#include <Kismet/GameplayStatics.h>

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
    UE_LOG(LogTemp, Warning, TEXT("GAMESTATE (CLIENT): OnRep_MatchState FIRED!"));

    // Get the local player controller. This is a much more reliable object than the pawn.
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->IsLocalController())
    {
        // Cast to our specific Player Controller class
        ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
        if (MyPC)
        {
            // Tell the Player Controller to handle its own UI.
            MyPC->ShowEndOfMatchUI(MatchState);
        }
    }
}