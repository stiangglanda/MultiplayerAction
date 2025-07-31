// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerActionGameMode.generated.h"

class AMultiplayerActionCharacter;
class ABossEnemyCharacter;

UCLASS(minimalapi)
class AMultiplayerActionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiplayerActionGameMode();

	void OnPlayerDied(AMultiplayerActionCharacter* DeadPlayer);

	/** Called by a Boss Character when it dies. */
	void OnBossDied(ABossEnemyCharacter* DeadBoss);

protected:
	// This function is called when a new player joins the game.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// --- TRACKING VARIABLES ---
	// These variables only exist on the server.

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> ActivePlayerControllers;

	UPROPERTY()
	TArray<TObjectPtr<ABossEnemyCharacter>> ActiveBosses;

	/** Total number of bosses that must be defeated to win. */
	int32 TotalBossesToDefeat = 0;

private:
	/** Checks the current game state to see if a win/loss condition has been met. */
	void CheckWinLossConditions();

	/** Handles the game over logic for a player victory. */
	void EndGame(bool bPlayersWon);

	APawn* FindNextSpectatorTarget(APlayerController* DeadPlayerController);

	void ReturnToMainMenu();
};



