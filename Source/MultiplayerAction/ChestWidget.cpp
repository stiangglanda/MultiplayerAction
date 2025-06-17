#include "ChestWidget.h"

void UChestWidget::SetChestReference(AActor* InChestActor)
{
    ChestReference = InChestActor;
}

void UChestWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize widget with chest data
    if (ChestReference)
    {
        // You can call BP implementable event here to update UI
        // or directly update C++ widgets
    }
}

FWeaponData UChestWidget::GetChestContents()
{
    if (ChestReference)
    {
        return *ChestReference->GetChestContents();
    }
    return FWeaponData();
}

FText UChestWidget::GetChestName() const
{
    if (ChestReference)
    {
        return ChestReference->GetChestName();
    }
    return FText::GetEmpty();
}
