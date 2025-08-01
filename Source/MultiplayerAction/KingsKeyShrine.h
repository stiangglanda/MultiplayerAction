#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KingsKeyShrine.generated.h"

UCLASS()
class MULTIPLAYERACTION_API AKingsKeyShrine : public AActor
{
	GENERATED_BODY()
	
public:	
	AKingsKeyShrine();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
