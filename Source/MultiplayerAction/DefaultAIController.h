#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "PatrolPath.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AIGroupAlertInterface.h"
#include "DefaultAIController.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ADefaultAIController : public AAIController, public IAIGroupAlertInterface
{
	GENERATED_BODY()
public:
	ADefaultAIController();
	virtual void Tick(float DeltaTime) override;
	bool IsDead() const;

	virtual void OnGroupAlert_Implementation(AActor* AlertedAboutActor) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;

	UPROPERTY(EditAnywhere)
	float AcceptenceRadius = 200;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;// Damage sense configuration doesnt quite work
};