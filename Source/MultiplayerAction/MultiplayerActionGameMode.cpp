// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerActionGameMode.h"
#include "MultiplayerActionCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "DefaultGameState.h"
#include "BossEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"

AMultiplayerActionGameMode::AMultiplayerActionGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/dynamic/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ADefaultGameState::StaticClass();
}

void AMultiplayerActionGameMode::StartPlay()
{
	Super::StartPlay();

	// --- INITIALIZE ACTOR TRACKING ---
	// Clear any existing data first to be safe.
	ActiveBosses.Empty();

	// Find all instances of the Boss character class in the level.
	TArray<AActor*> FoundBosses;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABossEnemyCharacter::StaticClass(), FoundBosses);

	for (AActor* BossActor : FoundBosses)
	{
		if (ABossEnemyCharacter* Boss = Cast<ABossEnemyCharacter>(BossActor))
		{
			ActiveBosses.Add(Boss);
		}
	}

	InitialBossCount = ActiveBosses.Num(); // Store the starting number of bosses.
	bMatchHasEnded = false;
	UE_LOG(LogTemp, Log, TEXT("GameMode Initialized: Found %d players and %d bosses."), ActivePlayerControllers.Num(), ActiveBosses.Num());
}

void AMultiplayerActionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Add the new player to our tracking list.
	if (NewPlayer)
	{
		ActivePlayerControllers.Add(NewPlayer);
		UE_LOG(LogTemp, Log, TEXT("Player logged in. Total players: %d"), ActivePlayerControllers.Num());
	}
}

APawn* AMultiplayerActionGameMode::FindNextSpectatorTarget(APlayerController* DeadPlayerController)
{
	for (APlayerController* PC : ActivePlayerControllers)
	{
		// Make sure it's not the controller of the player who just died,
		// and that they have a valid pawn to watch.
		if (PC && PC != DeadPlayerController && PC->GetPawn() != nullptr)
		{
			// Found a valid target.
			return PC->GetPawn();
		}
	}

	// No other living players were found.
	return nullptr;
}

// This function will be called by your player character's death logic.
void AMultiplayerActionGameMode::OnPlayerDied(AMultiplayerActionCharacter* DeadPlayer)
{
	if (!DeadPlayer) return;

	APlayerController* PC = DeadPlayer->GetController<APlayerController>();
	if (!PC) return;

	// --- SPECTATING LOGIC ---
	// This must run before we remove the player from the active list.
	APawn* SpectatorTarget = FindNextSpectatorTarget(PC);
	if (SpectatorTarget)
	{
		// 1. Tell the PlayerController to begin spectating.
		// This will unpossess the dead pawn and may spawn a SpectatorPawn.
		PC->PlayerState->SetIsSpectator(true);
		PC->ChangeState(NAME_Spectating);

		// 2. Set the camera to view the new target.
		// The blend time makes for a smooth transition.
		PC->SetViewTargetWithBlend(SpectatorTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
	else
	{
		// No one left to spectate. Handle game over or final state.
		// For now, they will just be a free-roaming spectator.
		PC->PlayerState->SetIsSpectator(true);
		PC->ChangeState(NAME_Spectating);
	}
	// --- END SPECTATING LOGIC ---

	// Now remove the player's controller from the active list for win/loss checks.
	ActivePlayerControllers.Remove(PC);
	UE_LOG(LogTemp, Log, TEXT("Player died and is now spectating. Players remaining: %d"), ActivePlayerControllers.Num());

	CheckWinLossConditions();
}

// This function will be called by your boss character's death logic.
void AMultiplayerActionGameMode::OnBossDied(ABossEnemyCharacter* DeadBoss)
{
	if (DeadBoss)
	{
		ActiveBosses.Remove(DeadBoss);
		UE_LOG(LogTemp, Log, TEXT("Boss died. Bosses remaining: %d"), ActiveBosses.Num());
	}

	CheckWinLossConditions();
}

void AMultiplayerActionGameMode::CheckWinLossConditions()
{
	if (bMatchHasEnded) return;

	// LOSS Condition
	if (ActivePlayerControllers.Num() <= 0)
	{
		bMatchHasEnded = true;
		EndGame(false);
		return;
	}

	// WIN Condition
	// Only check for a win if there were bosses to defeat in the first place.
	if (InitialBossCount > 0 && ActiveBosses.Num() <= 0)
	{
		bMatchHasEnded = true;
		EndGame(true);
	}
}

void AMultiplayerActionGameMode::EndGame(bool bPlayersWon)
{
	// Get the GameState and tell it to update the match state for all clients.
	ADefaultGameState* MyGameState = GetGameState<ADefaultGameState>();
	if (MyGameState)
	{
		EMatchState NewState = bPlayersWon ? EMatchState::Victory : EMatchState::Defeat;
		UE_LOG(LogTemp, Error, TEXT("GAMEMODE (SERVER): Ending game. Setting MatchState to: %s"), *UEnum::GetValueAsString(NewState));
		MyGameState->SetMatchState(NewState);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GAMEMODE (SERVER): FAILED to get GameState in EndGame!"));
	}

	// You might also want to disable player input here.
	for (APlayerController* PC : ActivePlayerControllers)
	{
		PC->DisableInput(nullptr);
	}

	FTimerHandle TimerHandle_ReturnToMenu;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ReturnToMenu, this, &AMultiplayerActionGameMode::ReturnToMainMenu, 5.0f, false);
}

void AMultiplayerActionGameMode::ReturnToMainMenu()
{
	//UGameplayStatics::OpenLevel(GetWorld(), FName("Minimal_Default"), true);
}