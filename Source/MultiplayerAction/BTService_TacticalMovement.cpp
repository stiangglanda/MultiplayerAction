#include "BTService_TacticalMovement.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_TacticalMovement::UBTService_TacticalMovement()
{
    NodeName = TEXT("Tactical Movement");
    Interval = 0.1f;
}

void UBTService_TacticalMovement::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
        return;

    APawn* AIPawn = AIController->GetPawn();
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

    if (!AIPawn || !TargetActor)
        return;

    FVector AILocation = AIPawn->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();
    float DistanceToTarget = FVector::Distance(AILocation, TargetLocation);

    // Update circling angle with some randomization
    float SpeedMultiplier = FMath::RandRange(0.8f, 1.2f);
    CurrentCirclingAngle += DeltaSeconds * CirclingSpeed * SpeedMultiplier;
    if (CurrentCirclingAngle > 360.0f)
        CurrentCirclingAngle -= 360.0f;

    FVector DesiredLocation;

    // Calculate base circling position
    float AngleRadians = FMath::DegreesToRadians(CurrentCirclingAngle);
    FVector CircleOffset(
        FMath::Cos(AngleRadians) * CirclingRadius,
        FMath::Sin(AngleRadians) * CirclingRadius,
        0.0f
    );

    if (DistanceToTarget > MaxApproachDistance)
    {
        // Blend between direct approach and circling when too far
        float BlendAlpha = FMath::Min((DistanceToTarget - MaxApproachDistance) / 200.0f, 1.0f);
        FVector DirectApproach = FMath::VInterpTo(AILocation, TargetLocation, DeltaSeconds, 1.0f);
        FVector CirclingPosition = TargetLocation + CircleOffset;
        DesiredLocation = FMath::Lerp(CirclingPosition, DirectApproach, BlendAlpha);
    }
    else if (DistanceToTarget < MinApproachDistance)
    {
        // Back away while maintaining the circular pattern
        FVector DirectionFromTarget = (AILocation - TargetLocation).GetSafeNormal();
        FVector BackoffPosition = TargetLocation + DirectionFromTarget * CirclingRadius;
        DesiredLocation = FMath::VInterpTo(AILocation, BackoffPosition, DeltaSeconds, 2.0f);
    }
    else
    {
        // Normal circling behavior with some variation
        float RadiusVariation = FMath::Sin(CurrentCirclingAngle * 0.5f) * 50.0f;
        CircleOffset *= (1.0f + RadiusVariation / CirclingRadius);
        DesiredLocation = TargetLocation + CircleOffset;

        // Add slight random variation to movement
        DesiredLocation += FVector(
            FMath::RandRange(-20.0f, 20.0f),
            FMath::RandRange(-20.0f, 20.0f),
            0.0f
        );
    }

    // Update movement speed based on distance
    if (ACharacter* AICharacter = Cast<ACharacter>(AIPawn))
    {
        if (UCharacterMovementComponent* MovementComp = AICharacter->GetCharacterMovement())
        {
            float SpeedFactor = FMath::GetMappedRangeValueClamped(
                FVector2D(MinApproachDistance, MaxApproachDistance),
                FVector2D(0.8f, 1.2f),
                DistanceToTarget
            );
            MovementComp->MaxWalkSpeed = 300.0f * SpeedFactor;
        }
    }

    // Move to the calculated position
    AIController->MoveToLocation(DesiredLocation, -1.0f, true, true, true, true);

    // Smoothly rotate to face the target
    FRotator CurrentRotation = AIPawn->GetActorRotation();
    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(AILocation, TargetLocation);
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, FRotator(0.0f, TargetRotation.Yaw, 0.0f), DeltaSeconds, 5.0f);
    AIPawn->SetActorRotation(NewRotation);
}