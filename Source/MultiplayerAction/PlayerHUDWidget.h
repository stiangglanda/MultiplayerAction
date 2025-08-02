#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
    void UpdateHealthBar(float NewMaxHealth);

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
    void PlayerGetsFollowers();
};
