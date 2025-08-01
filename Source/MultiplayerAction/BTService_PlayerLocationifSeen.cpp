#include "BTService_PlayerLocationifSeen.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "MultiplayerActionCharacter.h"

UBTService_PlayerLocationifSeen::UBTService_PlayerLocationifSeen()
{
    NodeName = TEXT("Update Player Location if Seen");
}

void UBTService_PlayerLocationifSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return;
    }

    AMultiplayerActionCharacter* AIPawn = Cast<AMultiplayerActionCharacter>(AIController->GetPawn());
    if (!AIPawn)
    {
        return;
    }

    UAIPerceptionComponent* PerceptionComp = AIController->GetAIPerceptionComponent();
    if (!PerceptionComp)
    {
        PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception"));
        AIController->SetPerceptionComponent(*PerceptionComp);

        // Configure sight sense
        UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(AIController);
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        SightConfig->SetMaxAge(5.0f);

        PerceptionComp->ConfigureSense(*SightConfig);
        PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    TArray<AActor*> PerceivedActors;
    PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    float ClosestDistance = MAX_FLT;
    AActor* ClosestActor = nullptr;

    for (AActor* Actor : PerceivedActors)
    {
        if (AMultiplayerActionCharacter* PawnActor = Cast<AMultiplayerActionCharacter>(Actor))
        {
            if (PawnActor->GetTeam() != AIPawn->GetTeam())
            {
                float Distance = FVector::Distance(AIPawn->GetActorLocation(),
                                                 PawnActor->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestActor = PawnActor;
                }
            }
        }
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(GetSelectedBlackboardKey(), ClosestActor);
    }
}
