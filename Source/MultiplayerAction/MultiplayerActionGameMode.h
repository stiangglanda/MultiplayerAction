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

	void OnBossDied(ABossEnemyCharacter* DeadBoss);

protected:
	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> ActivePlayerControllers;

	UPROPERTY()
	TArray<TObjectPtr<ABossEnemyCharacter>> ActiveBosses;

	int32 InitialBossCount = 0;

	bool bMatchHasEnded = false;

private:
	void CheckWinLossConditions();

	void EndGame(bool bPlayersWon);

	APawn* FindNextSpectatorTarget(APlayerController* DeadPlayerController);

	void ReturnToMainMenu();
};



