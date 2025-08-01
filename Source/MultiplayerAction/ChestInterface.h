#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ChestInterface.generated.h"

UINTERFACE(MinimalAPI)
class UChestInterface : public UInterface
{
	GENERATED_BODY()
};

class MULTIPLAYERACTION_API IChestInterface
{
	GENERATED_BODY()

public:
    virtual struct FWeaponData* GetChestContents() = 0;
    virtual FText GetChestName() const = 0;
	virtual void Swap() = 0;
};
