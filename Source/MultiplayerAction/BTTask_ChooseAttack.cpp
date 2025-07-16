#include "BTTask_ChooseAttack.h"
#include "CombatDataTypes.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ChooseAttack::UBTTask_ChooseAttack()
{
	NodeName = "Choose Attack";
}

EBTNodeResult::Type UBTTask_ChooseAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	const float RandomRoll = FMath::FRand();

	EAttackType ChosenAttack = EAttackType::EAT_Regular;

	if (RandomRoll <= HeavyAttackChance)
	{
		ChosenAttack = EAttackType::EAT_Heavy;
	}
	else
	{
		ChosenAttack = EAttackType::EAT_Regular;
	}

	BlackboardComp->SetValueAsEnum(ChosenAttackTypeKey.SelectedKeyName, static_cast<uint8>(ChosenAttack));

	return EBTNodeResult::Succeeded;
}

FString UBTTask_ChooseAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Sets Attack Type (Heavy Chance: %.0f%%)"), HeavyAttackChance * 100);
}