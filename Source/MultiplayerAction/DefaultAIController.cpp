#include "DefaultAIController.h"
#include "MultiplayerAction/MultiplayerActionCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "AIGroupManager.h"
#include "Perception/AISenseConfig_Damage.h"

ADefaultAIController::ADefaultAIController()
{
    bSetControlRotationFromPawnOrientation = false;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1800.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADefaultAIController::OnTargetPerceptionUpdated);
}

void ADefaultAIController::BeginPlay()
{
    Super::BeginPlay();

    if (AIBehavior != nullptr)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        RunBehaviorTree(AIBehavior);
        GetBlackboardComponent()->SetValueAsVector(TEXT("Player Location"), PlayerPawn->GetActorLocation());
        GetBlackboardComponent()->SetValueAsVector(TEXT("Start Location"), GetPawn()->GetActorLocation());
    }
}

void ADefaultAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

bool ADefaultAIController::IsDead() const
{
    AMultiplayerActionCharacter* ControlledCharacter = Cast<AMultiplayerActionCharacter>(GetPawn());
    if (ControlledCharacter != nullptr)
    {
        return ControlledCharacter->IsDead();
    }

    return true;
}

void ADefaultAIController::OnGroupAlert_Implementation(AActor* AlertedAboutActor)
{
    if (GetBlackboardComponent()->GetValueAsObject(TEXT("Player")))
    {
        return;
    }

    if (AlertedAboutActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s received group alert about %s!"), *GetName(), *AlertedAboutActor->GetName());
        GetBlackboardComponent()->SetValueAsObject(TEXT("Player"), AlertedAboutActor);
    }
}

void ADefaultAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADefaultAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    APawn* SensedPawn = Cast<APawn>(Actor);
    if (!SensedPawn)
    {
        if (AController* InstigatorController = Cast<AController>(Actor))
        {
            SensedPawn = InstigatorController->GetPawn();
        }

        if (!SensedPawn)
        {
            return;
        }
    }

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    static const FName TargetActorKey = TEXT("Player");

    const FAISenseID SenseID = Stimulus.Type;

    if (SenseID == UAISense::GetSenseID<UAISense_Sight>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UE_LOG(LogTemp, Warning, TEXT("SIGHT stimulus: I see %s"), *SensedPawn->GetName());
            
            AMultiplayerActionCharacter* SensedCharacter = Cast<AMultiplayerActionCharacter>(SensedPawn);
            if (SensedCharacter)
            {
                AMultiplayerActionCharacter* ControlledCharacter = Cast<AMultiplayerActionCharacter>(GetPawn());
                if (ControlledCharacter)
                {
                    if (SensedCharacter->GetTeam() != ControlledCharacter->GetTeam())
                    {
                        BlackboardComp->SetValueAsObject(TargetActorKey, SensedCharacter);

                        if (ControlledCharacter->AIGroupManager)
                        {
                            ControlledCharacter->AIGroupManager->BroadcastAlert(ControlledCharacter, SensedCharacter);
                        }
                    }
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SIGHT stimulus: I lost sight of %s"), *SensedPawn->GetName());
            if (BlackboardComp->GetValueAsObject(TargetActorKey) == SensedPawn)
            {
                BlackboardComp->ClearValue(TargetActorKey);
            }
        }
    }
}