// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "MultiplayerActionCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AMultiplayerActionCharacter* charackter = Cast<AMultiplayerActionCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (charackter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	charackter->ServerReliableRPC_Attack();

	return EBTNodeResult::Succeeded;
}
