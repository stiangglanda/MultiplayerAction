#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionProgressBarWidget.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UInteractionProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void StartProgress(float Duration);

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void StopProgress();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
    void ShowCompletedMessage();
};
