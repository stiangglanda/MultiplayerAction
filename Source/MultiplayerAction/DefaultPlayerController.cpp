#include "DefaultPlayerController.h"
#include <Blueprint/WidgetLayoutLibrary.h>

void ADefaultPlayerController::ShowEndOfMatchUI(EMatchState MatchResult)
{
	//ClearAllGameplayWidgets();
	Client_HideSpectatorUI();

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

UPlayerHUDWidget* ADefaultPlayerController::GetHUD()
{
	return HUD;
}

void ADefaultPlayerController::OnWelcomeScreenDismissed()
{
	if (WelcomeWidgetInstance)
	{
		WelcomeWidgetInstance->RemoveFromParent();
		WelcomeWidgetInstance = nullptr;
	}

	Server_RequestBeginPlay();
}

void ADefaultPlayerController::Client_ShowSpectatorUI_Implementation()
{
	if (IsLocalController() && SpectatorWidgetClass && !SpectatorWidgetInstance)
	{
		SpectatorWidgetInstance = CreateWidget<UUserWidget>(this, SpectatorWidgetClass);
		SpectatorWidgetInstance->AddToViewport();

		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void ADefaultPlayerController::Client_SetGameInputMode_Implementation()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	UE_LOG(LogTemp, Warning, TEXT("Client has been commanded to switch to GameOnly input mode."));
}

void ADefaultPlayerController::Client_HideSpectatorUI_Implementation()
{
	if (SpectatorWidgetInstance)
	{
		SpectatorWidgetInstance->RemoveFromParent();
		SpectatorWidgetInstance = nullptr;
	}
}

void ADefaultPlayerController::ClearAllGameplayWidgets()
{
	if (!IsLocalController()) return;

	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
}

void ADefaultPlayerController::BeginPlay()
{
    Super::BeginPlay();

	if (IsLocalController() && WelcomeWidgetClass)
	{
		WelcomeWidgetInstance = CreateWidget<UUserWidget>(this, WelcomeWidgetClass);
		if (WelcomeWidgetInstance)
		{
			WelcomeWidgetInstance->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(WelcomeWidgetInstance->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}

    HUD = CreateWidget<UPlayerHUDWidget>(this, CrossHairHUD);
    if (HUD != nullptr)
    {
        HUD->AddToViewport();
    }
}

void ADefaultPlayerController::Server_RequestBeginPlay_Implementation()
{
	if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
	{
		GM->RestartPlayer(this);
	}

	Client_SetGameInputMode();
}

