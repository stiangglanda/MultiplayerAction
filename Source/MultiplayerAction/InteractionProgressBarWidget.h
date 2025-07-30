// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionProgressBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API UInteractionProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    // This function can be implemented in Blueprints but is callable from C++.
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void StartProgress(float Duration);

    // You can also add a function to stop/reset it.
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void StopProgress();
};
