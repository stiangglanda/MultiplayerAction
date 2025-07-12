#include "AIGroupManager.h"
#include "EnemyCharacter.h"

AAIGroupManager::AAIGroupManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAIGroupManager::BeginPlay()
{
	Super::BeginPlay();

    for(int32 i=0; i<GroupMembers.Num(); ++i)
    {
        if (GroupMembers[i])
        {
            GroupMembers[i]->InitializeGroupMembership(this);
        }
	}
	
}

void AAIGroupManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
