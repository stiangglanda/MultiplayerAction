// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AIGroupAlertInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAIGroupAlertInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIPLAYERACTION_API IAIGroupAlertInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Group Behavior")
	void OnGroupAlert(AActor* AlertedAboutActor);
};
