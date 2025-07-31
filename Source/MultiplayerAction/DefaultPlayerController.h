// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DefaultGameState.h"
#include "DefaultPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ShowEndOfMatchUI(EMatchState MatchResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CrossHairHUD;

	UPROPERTY(VisibleAnywhere)
	UUserWidget* HUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	/** The widget class to show on Defeat. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DefeatWidgetClass;
};
