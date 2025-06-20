// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "DefaultAIController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API ADefaultAIController : public AAIController
{
	GENERATED_BODY()
public:
	ADefaultAIController();
	virtual void Tick(float DeltaTime) override;
	bool IsDead() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Perception updated callback
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	UPROPERTY(EditAnywhere)
	float AcceptenceRadius = 200;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;

	// AI Perception components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};