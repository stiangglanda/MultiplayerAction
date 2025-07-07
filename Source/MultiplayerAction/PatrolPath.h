#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPath.generated.h"

UCLASS()
class MULTIPLAYERACTION_API APatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	APatrolPath();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI|Patrol", meta = (MakeEditWidget = true))
	TArray<FVector> PatrolPoints;

	FVector GetPatrolPoint(int32 Index) const;
	int32 GetNumPoints() const;

};
