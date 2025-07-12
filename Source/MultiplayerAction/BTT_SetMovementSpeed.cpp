#include "BTT_SetMovementSpeed.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTT_SetMovementSpeed::UBTT_SetMovementSpeed()
{
    NodeName = "Set Movement Speed";
}

EBTNodeResult::Type UBTT_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    MovementComp->MaxWalkSpeed = NewSpeed;

    return EBTNodeResult::Succeeded;
}