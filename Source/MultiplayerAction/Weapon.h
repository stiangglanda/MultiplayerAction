// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<UTexture2D> ThumbnailImage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FText WeaponName = FText::GetEmpty();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FText WeaponDescription = FText::GetEmpty();

    FWeaponData() = default;

    FWeaponData(UTexture2D* InThumbnail, const FText& InName, const FText& InDescription)
        : ThumbnailImage(InThumbnail)
        , WeaponName(InName)
        , WeaponDescription(InDescription)
    {
    }
};


UCLASS()
class MULTIPLAYERACTION_API UWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	UWeapon();

    // Replace individual properties with the struct
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    FWeaponData WeaponData;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
