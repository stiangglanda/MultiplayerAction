#pragma once

#include "CoreMinimal.h"
#include "MultiplayerActionCharacter.h"
#include "PatrolPath.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class MULTIPLAYERACTION_API AEnemyCharacter : public AMultiplayerActionCharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI|Patrol")
	APatrolPath* PatrolPath;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class AAIGroupManager> AIGroupManager;

	void InitializeGroupMembership(TObjectPtr<class AAIGroupManager> AIGroupManager);
};
