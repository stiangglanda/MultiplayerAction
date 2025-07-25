#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_StartMovementLoop.generated.h"

UCLASS()
class MULTIPLAYERACTION_API UAnimNotify_StartMovementLoop : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	float FadeInDuration = 0.2f;
};
