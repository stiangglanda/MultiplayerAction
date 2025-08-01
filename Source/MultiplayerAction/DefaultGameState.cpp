#include "DefaultGameState.h"
#include "Net/UnrealNetwork.h"
#include "DefaultPlayerController.h"
#include <Kismet/GameplayStatics.h>

ADefaultGameState::ADefaultGameState()
{
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

void ADefaultGameState::SetHasKingsKey(bool bNewState)
{
    if (HasAuthority())
    {
        bHasKingsKey = bNewState;

        if (bHasKingsKey)
        {
            OnRep_KingsKeyAcquired();
        }
    }
}

void ADefaultGameState::OnRep_KingsKeyAcquired()
{
    if (bHasKingsKey)
    {
        OnKingsKeyAcquiredDelegate.Broadcast();
    }
}

void ADefaultGameState::SetStartedUnlockingChest(bool bNewState)
{
    if (HasAuthority())
    {
        bStartedUnlockingChest = bNewState;
        if (bStartedUnlockingChest)
        {
            OnRep_StartedUnlockingChest();
        }
    }
}

void ADefaultGameState::OnRep_StartedUnlockingChest()
{
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
        if (MatchState != NewState)
        {
            MatchState = NewState;
            OnRep_MatchState();
        }
    }
}

void ADefaultGameState::OnRep_MatchState()
{
    UE_LOG(LogTemp, Warning, TEXT("GAMESTATE (CLIENT): OnRep_MatchState FIRED!"));

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->IsLocalController())
    {
        ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
        if (MyPC)
        {
            MyPC->ShowEndOfMatchUI(MatchState);
        }
    }
}