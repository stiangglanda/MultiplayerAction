#include "EnemyCharacter.h"
#include "AIGroupManager.h"

void AEnemyCharacter::InitializeGroupMembership(TObjectPtr<class AAIGroupManager> GroupManager)
{
	if (GroupManager)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("GroupManager initialized successfull"));
		AIGroupManager = GroupManager;
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("GroupManager is null"));
	}
}
