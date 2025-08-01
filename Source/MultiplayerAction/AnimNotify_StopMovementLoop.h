#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_StopMovementLoop.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UAnimNotify_StopMovementLoop : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	float FadeOutDuration = 0.5f;
};
