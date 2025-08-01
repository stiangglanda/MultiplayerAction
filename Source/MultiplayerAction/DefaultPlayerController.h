#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DefaultGameState.h"
#include "DefaultPlayerController.generated.h"

UCLASS()
class MULTIPLAYERACTION_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ShowEndOfMatchUI(EMatchState MatchResult);

protected:
	virtual void BeginPlay() override;
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CrossHairHUD;

	UPROPERTY(VisibleAnywhere)
	UUserWidget* HUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DefeatWidgetClass;
};
