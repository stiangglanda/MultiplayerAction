// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LockOn.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API UBTTask_LockOn : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_LockOn();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetKey; // Key for the target actor (player)
};
