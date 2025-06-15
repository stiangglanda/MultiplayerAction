// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

UWeapon::UWeapon()
{
	ThumbnailImage = nullptr;
	WeaponName = FText::FromString(TEXT("Weapon"));
	WeaponDescription = FText::FromString(TEXT("A weapon description"));
}

void UWeapon::BeginPlay()
{
	Super::BeginPlay();
}
