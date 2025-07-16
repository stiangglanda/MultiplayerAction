#include "BTTask_Attack.h"
#include "AIController.h"
#include "MultiplayerActionCharacter.h"
#include "CombatDataTypes.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_Attack: Blackboard Component not found!"));
		return EBTNodeResult::Failed;
	}

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AMultiplayerActionCharacter* charackter = Cast<AMultiplayerActionCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (charackter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	EAttackType AttackType = static_cast<EAttackType>(BlackboardComp->GetValueAsEnum(ChosenAttackTypeKey.SelectedKeyName));

	bool bAttackStarted = false;

	switch (AttackType)
	{
	case EAttackType::EAT_Regular:
		charackter->ServerReliableRPC_Attack();
		bAttackStarted = true;
		break;
	case EAttackType::EAT_Heavy:
		charackter->ServerReliableRPC_HeavyAttack();
		bAttackStarted = true;
		break;
	case EAttackType::EAT_None:
	default:
		return EBTNodeResult::Failed;
	}

	if (!bAttackStarted)
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}
