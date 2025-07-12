#include "BTS_UpdateFollowerRotation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTS_UpdateFollowerRotation::UBTS_UpdateFollowerRotation()
{
    NodeName = "Update Follower Rotation";

    bNotifyTick = true;
    Interval = 3.0f;
}

void UBTS_UpdateFollowerRotation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    AActor* LeaderActor = Cast<AActor>(BlackboardComp->GetValueAsObject(PatrolLeaderKey));
    if (!LeaderActor)
    {
        return;
    }

    FRotator CurrentRotation = ControlledPawn->GetActorRotation();
    FRotator TargetRotation = LeaderActor->GetActorRotation();

    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, InterpSpeed);

    ControlledPawn->SetActorRotation(NewRotation);
}