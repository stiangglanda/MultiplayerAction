#include "BTDecorator_IsHealthLow.h"
#include "MultiplayerActionCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTDecorator_IsHealthLow::UBTDecorator_IsHealthLow()
{
	NodeName = "Is Health Low";
	BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsHealthLow, BlackboardKey));
}

bool UBTDecorator_IsHealthLow::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!BlackboardComp || !AIController)
	{
		return false;
	}

	const AMultiplayerActionCharacter* Character = Cast<AMultiplayerActionCharacter>(AIController->GetPawn());
	if (!Character)
	{
		return false;
	}

	const bool bIsHealthLow = Character->GetHeathPercent() <= HealthThreshold;

	OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), bIsHealthLow);

	return bIsHealthLow;
}

FString UBTDecorator_IsHealthLow::GetStaticDescription() const
{
	return FString::Printf(TEXT("Health Threshold: < %.0f%%"), HealthThreshold * 100);
}