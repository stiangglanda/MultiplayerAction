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
	TArray<TObjectPtr<class AMultiplayerActionCharacter>> GroupMembers;

	UPROPERTY(EditInstanceOnly, Category = "AI Group")
	TObjectPtr<class APatrolPath> PatrolPath;

	void BroadcastAlert(AActor* SentryPawn, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "AI Group")
	void SetGroupLeader(class AMultiplayerActionCharacter* NewLeaderPawn);

	UFUNCTION(BlueprintCallable, Category = "AI Group")
	void AllowCombat(class AMultiplayerActionCharacter* CallerPawn, bool bAllow);

	void UnregisterMember(class AMultiplayerActionCharacter* MemberToRemove);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "AI Group")
	TArray<FVector> FormationOffsets;

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName IsLeaderKeyName = "IsLeader";

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName PatrolPathKeyName = "PatrolPath";

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName CurrentPatrolIndexKeyName = "CurrentPatrolIndex";

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName PatrolLeaderKeyName = "PatrolLeader";

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName AllowCombatKeyName = "AllowCombat";

	UPROPERTY(EditDefaultsOnly, Category = "AI Group|Blackboard")
	FName FormationOffsetKeyName = "FormationOffset";

	void AttemptGroupSetup();

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<TObjectPtr<class AMultiplayerActionCharacter>> PendingSetupMembers;

	FTimerHandle SetupTimerHandle;
};
