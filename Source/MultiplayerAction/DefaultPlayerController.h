#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DefaultGameState.h"
#include "PlayerHUDWidget.h"
#include "InputMappingContext.h"
#include "DefaultPlayerController.generated.h"

class UCameraShakeBase;

UCLASS()
class MULTIPLAYERACTION_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ShowEndOfMatchUI(ECustomMatchState  MatchResult);

	UPlayerHUDWidget* GetHUD();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnWelcomeScreenDismissed();

	UFUNCTION(Client, Reliable)
	void Client_ShowWelcomeScreen();

	UFUNCTION(Client, Reliable)
	void Client_SetGameInputMode();

	UFUNCTION(Client, Reliable)
	void Client_ShowSpectatorUI();

	UFUNCTION(Client, Reliable)
	void Client_HideSpectatorUI();

	void ClearAllGameplayWidgets();

	UFUNCTION(Client, Reliable)
	void Client_PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale);

	UFUNCTION(Client, Reliable)
	void Client_PlayDamageTakenShake(float Scale);

	UPROPERTY(EditDefaultsOnly, Category = "Effects|CameraShake")
	TSubclassOf<UCameraShakeBase> DamageTakenCameraShakeClass;

	float GetMouseSensitivity() const { return CurrentMouseSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetSensitivity(float NewSensitivity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestBeginPlay();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	float CurrentMouseSensitivity = 1.0f;

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> WelcomeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SpectatorWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> SpectatorWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> WelcomeWidgetInstance;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UPlayerHUDWidget> CrossHairHUD;

	UPROPERTY(VisibleAnywhere)
	UPlayerHUDWidget* HUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DefeatWidgetClass;
};
