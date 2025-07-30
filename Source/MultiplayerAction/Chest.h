// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Weapon.h"
#include "ChestInterface.h"
#include "OutpostInteractable.h"
#include "ChestWidget.h"
#include "Chest.generated.h"

UCLASS()
class MULTIPLAYERACTION_API AChest : public AActor, public IChestInterface, public IOutpostInteractable
{
	GENERATED_BODY()
	
public:	
	AChest();

	virtual FWeaponData* GetChestContents() override;
	virtual FText GetChestName() const override;
	virtual void Swap() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeapon> Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Chest")
	FText ChestName;

protected:
	UPROPERTY(Category = Chest, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Chest")
	UAnimMontage* OpenCloseAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestOpenSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestCloseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UChestWidget> ChestMenuWidgetClass;

	UPROPERTY()
	UChestWidget* ChestMenuWidget;

	bool bOpen;

	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	void OpenChest(APawn* InstigatorPawn);

	//UFUNCTION()
	//bool ToggleOpenClose(APawn* InstigatorPawn);//return true if opened, false if closed
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;

	UFUNCTION()
	void CloseChest(APawn* InstigatorPawn);

	virtual void Tick(float DeltaTime) override;

	bool IsCompleted() override;
};
