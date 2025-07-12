#include "EnemyCharacter.h"
#include "AIGroupManager.h"

void AEnemyCharacter::InitializeGroupMembership(TObjectPtr<class AAIGroupManager> GroupManager)
{
	if (GroupManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GroupManager initialized successfull"));
		AIGroupManager = GroupManager;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GroupManager is null"));
	}
}
