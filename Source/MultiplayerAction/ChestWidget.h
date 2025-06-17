#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChestInterface.h"
#include "Weapon.h"
#include "ChestWidget.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UChestWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void SetChestReference(AActor* InChestActor);

    UFUNCTION(BlueprintPure, Category = "Chest")
    FWeaponData GetChestContents();

    UFUNCTION(BlueprintPure, Category = "Chest")
    FText GetChestName() const;

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY()
    TScriptInterface<IChestInterface> ChestReference;
};
