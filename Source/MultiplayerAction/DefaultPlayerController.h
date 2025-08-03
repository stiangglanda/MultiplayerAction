#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DefaultGameState.h"
#include "PlayerHUDWidget.h"
#include "DefaultPlayerController.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ShowEndOfMatchUI(EMatchState MatchResult);

	UPlayerHUDWidget* GetHUD();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnWelcomeScreenDismissed();

	UFUNCTION(Client, Reliable)
	void Client_SetGameInputMode();

	UFUNCTION(Client, Reliable)
	void Client_ShowSpectatorUI();

	UFUNCTION(Client, Reliable)
	void Client_HideSpectatorUI();

	void ClearAllGameplayWidgets();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestBeginPlay();

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
