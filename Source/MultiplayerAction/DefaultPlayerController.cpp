// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

void ADefaultPlayerController::ShowEndOfMatchUI(EMatchState MatchResult)
{
	TSubclassOf<UUserWidget> WidgetToShowClass = nullptr;
	if (MatchResult == EMatchState::Victory)
	{
		WidgetToShowClass = VictoryWidgetClass;
	}
	else if (MatchResult == EMatchState::Defeat)
	{
		WidgetToShowClass = DefeatWidgetClass;
	}

	if (WidgetToShowClass)
	{
		UUserWidget* EndOfMatchWidget = CreateWidget<UUserWidget>(this, WidgetToShowClass);
		if (EndOfMatchWidget)
		{
			EndOfMatchWidget->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
		}
	}
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();
    HUD = CreateWidget(this, CrossHairHUD);
    if (HUD != nullptr)
    {
        HUD->AddToViewport();
    }
}

