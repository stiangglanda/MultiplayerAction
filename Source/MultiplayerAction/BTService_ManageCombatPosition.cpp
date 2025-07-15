#include "BTService_ManageCombatPosition.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Navigation/PathFollowingComponent.h"

UBTService_ManageCombatPosition::UBTService_ManageCombatPosition()
{
	NodeName = "Manage Combat Position";

	Interval = 0.5f;
	RandomDeviation = 0.1f;

	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}

uint16 UBTService_ManageCombatPosition::GetInstanceMemorySize() const
{
	return sizeof(FCombatPositionServiceMemory);
}

FString UBTService_ManageCombatPosition::GetStaticDescription() const
{
	return FString::Printf(TEXT("Manages Focus, Positioning, and Movement\nOptimal Dist: %.0f, Strafe Dist: %.0f"), OptimalDistance, StrafeDistance);
}

void UBTService_ManageCombatPosition::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	FCombatPositionServiceMemory* MyMemory = reinterpret_cast<FCombatPositionServiceMemory*>(NodeMemory);

	if (!AIController || !BlackboardComp || !MyMemory)
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < MyMemory->WaitEndTime)
	{
		if (AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)))
		{
			AIController->SetFocus(TargetActor);
		}
		return;
	}

	APawn* SelfPawn = AIController->GetPawn();
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!SelfPawn || !TargetActor)
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		AIController->StopMovement();
		return;
	}

	AIController->SetFocus(TargetActor);

	const FVector SelfLocation = SelfPawn->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const float CurrentDistance = FVector::Dist(SelfLocation, TargetLocation);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (!NavSys)
	{
		return;
	}

	FNavLocation NavLocation;
	FVector NewTargetLocation = FVector::ZeroVector;

	if (CurrentDistance > OptimalDistance + DistanceBuffer)
	{
		NewTargetLocation = TargetLocation;
	}
	else if (CurrentDistance < OptimalDistance - DistanceBuffer)
	{
		const FVector DirectionAwayFromTarget = (SelfLocation - TargetLocation).GetSafeNormal();
		const FVector BackupPoint = SelfLocation + DirectionAwayFromTarget * BackupDistance;
		if (NavSys->GetRandomReachablePointInRadius(BackupPoint, 500.0f, NavLocation))
		{
			NewTargetLocation = NavLocation.Location;
		}
	}
	else //perfect range (Strafe)
	{
		const float StrafeDirection = FMath::RandBool() ? 1.0f : -1.0f;
		const FVector RightVector = SelfPawn->GetActorRightVector();
		const FVector StrafePoint = SelfLocation + (RightVector * StrafeDirection * StrafeDistance);
		if (NavSys->GetRandomReachablePointInRadius(StrafePoint, 500.0f, NavLocation))
		{
			NewTargetLocation = NavLocation.Location;
		}
	}

	if (NewTargetLocation != FVector::ZeroVector &&
		!NewTargetLocation.Equals(MyMemory->LastMovedToLocation, 100.f))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, NewTargetLocation);
		MyMemory->LastMovedToLocation = NewTargetLocation;
		MyMemory->WaitEndTime = CurrentTime + WaitDuration;
	}
}
