#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIGroupManager.generated.h"

UCLASS()
class MULTIPLAYERACTION_API AAIGroupManager : public AActor
{
	GENERATED_BODY()
	
public:
	AAIGroupManager();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI|Group", meta = (MakeEditWidget = true))
	TArray<TObjectPtr<class AEnemyCharacter>> GroupMembers;

	UPROPERTY(EditInstanceOnly, Category = "AI Group")
	TObjectPtr<class APatrolPath> PatrolPath;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

};
