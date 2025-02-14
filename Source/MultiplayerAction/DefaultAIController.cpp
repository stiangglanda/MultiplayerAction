// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultAIController.h"
#include "MultiplayerAction/MultiplayerActionCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

ADefaultAIController::ADefaultAIController()
{

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