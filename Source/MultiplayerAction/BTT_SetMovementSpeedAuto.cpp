#include "BTT_SetMovementSpeedAuto.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_SetMovementSpeedAuto::UBTT_SetMovementSpeedAuto()
{
    NodeName = "Set Movement Speed Automatic";
}

EBTNodeResult::Type UBTT_SetMovementSpeedAuto::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    ACharacter* ControlledCharacter = Cast<ACharacter>(AIController->GetPawn());
    if (!ControlledCharacter)
    {
        return EBTNodeResult::Failed;
    }

    UCharacterMovementComponent* MovementComp = ControlledCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    AActor* PlayerPawn = Cast<AActor>(BlackboardComp->GetValueAsObject(PatrolLeaderKey.SelectedKeyName));
    if (!PlayerPawn)
    {
        return EBTNodeResult::Failed;
    }

    const FVector SelfLocation = ControlledCharacter->GetActorLocation();
    const FVector TargetLocation = PlayerPawn->GetActorLocation();
    const float CurrentDistance = FVector::Dist(SelfLocation, TargetLocation);

    if(CurrentDistance > thresholdDistance)
    {
        MovementComp->MaxWalkSpeed = MaxSpeed;
    }
    else
    {
        MovementComp->MaxWalkSpeed = MinSpeed;
	}

    return EBTNodeResult::Succeeded;
}
