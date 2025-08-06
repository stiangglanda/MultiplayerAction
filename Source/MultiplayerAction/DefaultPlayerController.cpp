#include "DefaultPlayerController.h"
#include <Blueprint/WidgetLayoutLibrary.h>
#include "GameFramework/Character.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void ADefaultPlayerController::ShowEndOfMatchUI(ECustomMatchState  MatchResult)
{
	//ClearAllGameplayWidgets();

	ACharacter* MyCharacter = GetCharacter();
	if (MyCharacter && MyCharacter->GetCharacterMovement())
	{
		MyCharacter->GetCharacterMovement()->StopMovementImmediately();
	}

	if (MyCharacter)
	{
		MyCharacter->DisableInput(this);
	}

	Client_HideSpectatorUI();

	TSubclassOf<UUserWidget> WidgetToShowClass = nullptr;
	if (MatchResult == ECustomMatchState::Victory)
	{
		WidgetToShowClass = VictoryWidgetClass;
	}
	else if (MatchResult == ECustomMatchState::Defeat)
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

void ADefaultPlayerController::Client_ShowWelcomeScreen_Implementation()
{
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

	if (IsLocalController() && CrossHairHUD)
	{
		HUD = CreateWidget<UPlayerHUDWidget>(this, CrossHairHUD);
		if (HUD != nullptr)
		{
			HUD->AddToViewport();
		}
	}

	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Log, TEXT("DefaultMappingContext added to subsystem."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is NOT SET on the PlayerController!"));
			}
		}
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

