// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultAIController.h"
#include "MultiplayerAction/MultiplayerActionCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"

ADefaultAIController::ADefaultAIController()
{
    // Create and configure perception component
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

// Called when the game starts or when spawned
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

bool ADefaultAIController::IsDead() const
{
    AMultiplayerActionCharacter* ControlledCharacter = Cast<AMultiplayerActionCharacter>(GetPawn());
    if (ControlledCharacter != nullptr)
    {
        return ControlledCharacter->IsDead();
    }

    return true;
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

    // Only react to player-controlled pawns
    APawn* SensedPawn = Cast<APawn>(Actor);
    if (!SensedPawn || !SensedPawn->IsPlayerControlled())
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    static const FName PlayerActorKey = TEXT("Player");

    if (Stimulus.WasSuccessfullySensed())
    {
        // Player seen: set blackboard key
        BlackboardComp->SetValueAsObject(PlayerActorKey, Actor);
    }
    else
    {
        // Player lost: clear blackboard key
        BlackboardComp->ClearValue(PlayerActorKey);
    }
}