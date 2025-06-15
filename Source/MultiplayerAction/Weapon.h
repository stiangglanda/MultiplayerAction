// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API UWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	UWeapon();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTexture2D> ThumbnailImage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    FText WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    FText WeaponDescription;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
