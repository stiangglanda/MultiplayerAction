#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_ManageCombatPosition.generated.h"

USTRUCT()
struct FCombatPositionServiceMemory
{
	GENERATED_BODY()

	UPROPERTY()
	FVector LastMovedToLocation = FVector::ZeroVector;

	UPROPERTY()
	float WaitEndTime = 0.f;
};

UCLASS()
class MULTIPLAYERACTION_API UBTService_ManageCombatPosition : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_ManageCombatPosition();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual uint16 GetInstanceMemorySize() const override;

	virtual FString GetStaticDescription() const override;

	UPROPERTY(Category = "AI|Combat", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float OptimalDistance = 400.0f;

	UPROPERTY(Category = "AI|Combat", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float DistanceBuffer = 100.0f;

	UPROPERTY(Category = "AI|Combat", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float StrafeDistance = 500.0f;

	UPROPERTY(Category = "AI|Combat", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float BackupDistance = 300.0f;

	UPROPERTY(Category = "Blackboard", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(Category = "AI|Combat", EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float WaitDuration = 1.5f;
};
