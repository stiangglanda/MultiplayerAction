#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AIGroupAlertInterface.generated.h"

UINTERFACE(MinimalAPI)
class UAIGroupAlertInterface : public UInterface
{
	GENERATED_BODY()
};

class MULTIPLAYERACTION_API IAIGroupAlertInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Group Behavior")
	void OnGroupAlert(AActor* AlertedAboutActor);
};
