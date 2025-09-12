#include "AnimNotify_StopWeaponTrace.h"
#include "MultiplayerActionCharacter.h"

void UAnimNotify_StopWeaponTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMultiplayerActionCharacter* Character = Cast<AMultiplayerActionCharacter>(MeshComp->GetOwner()))
	{
		Character->StopWeaponTrace();
	}
}