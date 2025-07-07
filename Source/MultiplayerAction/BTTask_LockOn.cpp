#include "BTTask_LockOn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MultiplayerActionCharacter.h"

UBTTask_LockOn::UBTTask_LockOn()
{
	NodeName = TEXT("LockOn");
}

EBTNodeResult::Type UBTTask_LockOn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!BlackboardComp)
		return EBTNodeResult::Failed;

	AMultiplayerActionCharacter* TargetActor = Cast<AMultiplayerActionCharacter>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!TargetActor)
		return EBTNodeResult::Failed;

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AMultiplayerActionCharacter* charackter = Cast<AMultiplayerActionCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (charackter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	charackter->SetLockedOnTarget(TargetActor);

	return EBTNodeResult::Succeeded;
}

