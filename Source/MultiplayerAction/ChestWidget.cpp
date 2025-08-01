#include "ChestWidget.h"

void UChestWidget::SetChestReference(AActor* InChestActor)
{
    ChestReference = InChestActor;
}

void UChestWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ChestReference)
    {
        // call BP implementable event here to update UI
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

void UChestWidget::Swap()
{
    if (ChestReference)
    {
        ChestReference->Swap();
    }
}
