// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

UWeapon::UWeapon()
{
	WeaponData = FWeaponData(nullptr, FText::FromString(TEXT("Weapon")), FText::FromString(TEXT("A weapon description")));
}

void UWeapon::BeginPlay()
{
	Super::BeginPlay();
}
