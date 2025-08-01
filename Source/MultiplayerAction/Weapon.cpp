#include "Weapon.h"

UWeapon::UWeapon()
{
	WeaponData = FWeaponData(nullptr, FText::FromString(TEXT("Weapon")), FText::FromString(TEXT("A weapon description")), 20);
}

void UWeapon::BeginPlay()
{
	Super::BeginPlay();
}
