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
    DOREPLIFETIME(ADefaultGameState, CustomMatchState);
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

void ADefaultGameState::SetCustomMatchState(ECustomMatchState  NewState)
{
    if (HasAuthority())
    {
        CustomMatchState = NewState;
        OnRep_CustomMatchState();
    }
}

void ADefaultGameState::OnRep_CustomMatchState()
{
    ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (MyPC && MyPC->IsLocalController())
    {
        MyPC->ShowEndOfMatchUI(CustomMatchState);
    }
}