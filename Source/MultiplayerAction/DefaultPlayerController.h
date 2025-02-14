// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DefaultPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CrossHairHUD;

	UPROPERTY(VisibleAnywhere)
	UUserWidget* HUD;
};
