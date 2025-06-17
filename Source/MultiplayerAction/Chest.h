// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Chest.generated.h"

UCLASS()
class MULTIPLAYERACTION_API AChest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChest();

protected:
	UPROPERTY(Category = Chest, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Chest")
	UAnimMontage* OpenCloseAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ChestMenuWidgetClass;

	UPROPERTY()
	UUserWidget* ChestMenuWidget;

	bool bOpen;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	void OpenChest();

	UFUNCTION()
	bool ToggleOpenClose();//return true if opened, false if closed

	UFUNCTION()
	void CloseChest();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
