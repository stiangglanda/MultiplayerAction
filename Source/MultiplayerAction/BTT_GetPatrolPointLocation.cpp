#include "BTT_GetPatrolPointLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrolPath.h"

UBTT_GetPatrolPointLocation::UBTT_GetPatrolPointLocation()
{
    NodeName = "Get Patrol Point Location";
}

EBTNodeResult::Type UBTT_GetPatrolPointLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APatrolPath* PathActor = Cast<APatrolPath>(BlackboardComp->GetValueAsObject(TEXT("PatrolPath")));
    if (!PathActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTT_GetPatrolPointLocation: No PatrolPath Actor found in Blackboard."));
        return EBTNodeResult::Failed;
    }

    int32 CurrentIndex = BlackboardComp->GetValueAsInt(TEXT("PatrolPointIndex"));

    FVector PointLocation = PathActor->GetPatrolPoint(CurrentIndex);

    BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), PointLocation);

    return EBTNodeResult::Succeeded;
}
