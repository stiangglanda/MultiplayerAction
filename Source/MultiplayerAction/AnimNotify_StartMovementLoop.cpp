#include "AnimNotify_StartMovementLoop.h"
#include "MultiplayerActionCharacter.h"

void UAnimNotify_StartMovementLoop::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMultiplayerActionCharacter* Character = Cast<AMultiplayerActionCharacter>(MeshComp->GetOwner()))
	{
		UAudioComponent* AudioComp = Character->GetMovementAudioComponent();

		USoundBase* SoundToPlay = Character->GetMovementLoopSound();

		if (AudioComp && SoundToPlay)
		{
			AudioComp->SetSound(SoundToPlay);
			AudioComp->FadeIn(FadeInDuration);
		}
	}
}