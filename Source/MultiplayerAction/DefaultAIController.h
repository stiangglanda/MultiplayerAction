// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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
private:
	UPROPERTY(EditAnywhere)
	float AcceptenceRadius = 200;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehavior;
};