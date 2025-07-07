#include "BTT_IncrementPatrolIndex.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrolPath.h"

UBTT_IncrementPatrolIndex::UBTT_IncrementPatrolIndex()
{
    NodeName = "Increment Patrol Index";
}

EBTNodeResult::Type UBTT_IncrementPatrolIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return EBTNodeResult::Failed;

    APatrolPath* PathActor = Cast<APatrolPath>(BlackboardComp->GetValueAsObject(TEXT("PatrolPath")));
    if (!PathActor || PathActor->GetNumPoints() == 0)
    {
        return EBTNodeResult::Failed;
    }

    int32 CurrentIndex = BlackboardComp->GetValueAsInt(TEXT("PatrolPointIndex"));
    int32 NumPoints = PathActor->GetNumPoints();

    // Increment index and loop back to 0 if it goes past the end
    int32 NextIndex = (CurrentIndex + 1) % NumPoints;

    BlackboardComp->SetValueAsInt(TEXT("PatrolPointIndex"), NextIndex);

    return EBTNodeResult::Succeeded;
}
