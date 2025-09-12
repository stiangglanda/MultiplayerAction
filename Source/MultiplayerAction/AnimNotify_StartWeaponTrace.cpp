#include "AnimNotify_StartWeaponTrace.h"
#include "MultiplayerActionCharacter.h"

void UAnimNotify_StartWeaponTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMultiplayerActionCharacter* Character = Cast<AMultiplayerActionCharacter>(MeshComp->GetOwner()))
	{
		Character->StartWeaponTrace();
	}
}