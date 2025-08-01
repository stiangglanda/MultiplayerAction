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

    UPROPERTY(EditAnywhere)
    float WeaponDamage = 20;

    FWeaponData() = default;

    FWeaponData(UTexture2D* InThumbnail, const FText& InName, const FText& InDescription, float WeaponDamage)
        : ThumbnailImage(InThumbnail)
        , WeaponName(InName)
        , WeaponDescription(InDescription)
		, WeaponDamage(WeaponDamage)
    {
    }
};


UCLASS()
class MULTIPLAYERACTION_API UWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	UWeapon();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    FWeaponData WeaponData;

protected:
	virtual void BeginPlay() override;
	
};
