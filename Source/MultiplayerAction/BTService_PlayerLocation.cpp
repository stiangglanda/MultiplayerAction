#include "BTService_PlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_PlayerLocation::UBTService_PlayerLocation()
{
	NodeName = TEXT("Update Player Location");
}

void UBTService_PlayerLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) 
	{ 
		return; 
	}

	AActor* PlayerPawn = Cast<AActor>(BlackboardComp->GetValueAsObject(PlayerKey.SelectedKeyName));

	if (PlayerPawn == nullptr)
	{
		return;
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerPawn->GetActorLocation());
}
