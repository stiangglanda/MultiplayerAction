#include "AnimNotify_StopMovementLoop.h"
#include "MultiplayerActionCharacter.h"

void UAnimNotify_StopMovementLoop::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMultiplayerActionCharacter* Character = Cast<AMultiplayerActionCharacter>(MeshComp->GetOwner()))
	{
		UAudioComponent* AudioComp = Character->GetMovementAudioComponent();
		if (AudioComp)
		{
			AudioComp->FadeOut(FadeOutDuration, 0.0f);
		}
	}
}