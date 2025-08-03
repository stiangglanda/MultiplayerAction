#include "BTTask_Block.h"
#include "MultiplayerActionCharacter.h"
#include "AIController.h"

UBTTask_Block::UBTTask_Block()
{
	NodeName = TEXT("Block");
}

EBTNodeResult::Type UBTTask_Block::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	charackter->Server_RequestBlock();

	return EBTNodeResult::Succeeded;
}
