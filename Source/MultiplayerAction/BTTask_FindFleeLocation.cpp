#include "BTTask_FindFleeLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"

UBTTask_FindFleeLocation::UBTTask_FindFleeLocation()
{
	NodeName = "Find Flee Location";
}

EBTNodeResult::Type UBTTask_FindFleeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp) { return EBTNodeResult::Failed; }

	APawn* SelfPawn = AIController->GetPawn();
	FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetToFleeFromKey.SelectedKeyName);
	if (!SelfPawn) { return EBTNodeResult::Failed; }

	const FVector SelfLocation = SelfPawn->GetActorLocation();
	const FVector FleeDirection = (SelfLocation - TargetLocation).GetSafeNormal();
	const FVector DesiredFleeLocation = SelfLocation + FleeDirection * FleeDistance;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (!NavSys) { return EBTNodeResult::Failed; }

	FNavLocation NavLocation;
	if (NavSys->GetRandomReachablePointInRadius(DesiredFleeLocation, 500.0f, NavLocation))
	{
		BlackboardComp->SetValueAsVector(FleeLocationKey.SelectedKeyName, NavLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}