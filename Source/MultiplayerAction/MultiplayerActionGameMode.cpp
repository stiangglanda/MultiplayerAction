#include "MultiplayerActionGameMode.h"
#include "MultiplayerActionCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "DefaultGameState.h"
#include "BossEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "DefaultPlayerController.h"

AMultiplayerActionGameMode::AMultiplayerActionGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/dynamic/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ADefaultGameState::StaticClass();
}

bool AMultiplayerActionGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return true;
}

void AMultiplayerActionGameMode::StartPlay()
{
	Super::StartPlay();

	ActiveBosses.Empty();

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
	//Super::PostLogin(NewPlayer);
	ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(NewPlayer);
	if (MyPC)
	{
		MyPC->Client_ShowWelcomeScreen();
	}

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
		if (PC && PC != DeadPlayerController && PC->GetPawn() != nullptr)
		{
			return PC->GetPawn();
		}
	}

	return nullptr;
}

void AMultiplayerActionGameMode::OnPlayerDied(AMultiplayerActionCharacter* DeadPlayer)
{
	if (!DeadPlayer) return;

	APlayerController* PC = DeadPlayer->GetController<APlayerController>();
	if (!PC) return;

	APawn* SpectatorTarget = FindNextSpectatorTarget(PC);
	if (SpectatorTarget)
	{
		PC->PlayerState->SetIsSpectator(true);
		PC->ChangeState(NAME_Spectating);

		PC->SetViewTargetWithBlend(SpectatorTarget, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
	else
	{
		PC->PlayerState->SetIsSpectator(true);
		PC->ChangeState(NAME_Spectating);
	}

	ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
	if (MyPC)
	{
		MyPC->Client_ShowSpectatorUI();
	}

	ActivePlayerControllers.Remove(PC);
	UE_LOG(LogTemp, Log, TEXT("Player died and is now spectating. Players remaining: %d"), ActivePlayerControllers.Num());

	CheckWinLossConditions();
}

void AMultiplayerActionGameMode::OnBossDied(ABossEnemyCharacter* DeadBoss)
{
	if (DeadBoss)
	{
		ActiveBosses.Remove(DeadBoss);
		UE_LOG(LogTemp, Log, TEXT("Boss died. Bosses remaining: %d"), ActiveBosses.Num());
	}

	CheckWinLossConditions();
}

void AMultiplayerActionGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void AMultiplayerActionGameMode::CheckWinLossConditions()
{
	if (bMatchHasEnded) return;

	if (ActivePlayerControllers.Num() <= 0)
	{
		bMatchHasEnded = true;
		EndGame(false);
		return;
	}

	if (InitialBossCount > 0 && ActiveBosses.Num() <= 0)
	{
		bMatchHasEnded = true;
		EndGame(true);
	}
}

void AMultiplayerActionGameMode::EndGame(bool bPlayersWon)
{
	EndMatch();

	ADefaultGameState* MyGameState = GetGameState<ADefaultGameState>();
	if (MyGameState)
	{
		ECustomMatchState  NewCustomState = bPlayersWon ? ECustomMatchState::Victory : ECustomMatchState::Defeat;
		MyGameState->SetCustomMatchState(NewCustomState);
	}
}