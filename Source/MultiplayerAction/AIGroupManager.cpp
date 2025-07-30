#include "AIGroupManager.h"
#include "MultiplayerActionCharacter.h"
#include "PatrolPath.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h"
#include "DefaultAIController.h"
#include "AIGroupAlertInterface.h"

AAIGroupManager::AAIGroupManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAIGroupManager::BroadcastAlert(AActor* SentryPawn, AActor* TargetActor)
{
    if (!SentryPawn || !TargetActor)
    {
        return;
    }

    // Iterate through all members of the group
    for (AMultiplayerActionCharacter* CurrentMember : GroupMembers)
    {
        // Don't re-alert the original sentry or null members
        if (CurrentMember && CurrentMember != SentryPawn)
        {
            // Check if the member implements the interface
            if (CurrentMember->GetController()->Implements<UAIGroupAlertInterface>())
            {
                // Call the interface function. This is the broadcast!
                IAIGroupAlertInterface::Execute_OnGroupAlert(CurrentMember->GetController(), TargetActor);
            }
        }
    }
}

void AAIGroupManager::BeginPlay()
{
	Super::BeginPlay();

    if (GroupMembers.Num() > 0)// delayed intialization for group members because of the BlackboardComponent
    {
        PendingSetupMembers = GroupMembers;

        GetWorld()->GetTimerManager().SetTimer(
            SetupTimerHandle,
            this,
            &AAIGroupManager::AttemptGroupSetup,
            0.1f,
            true  // Make the timer loop
        );
    }
}

void AAIGroupManager::AttemptGroupSetup()
{
    if (PendingSetupMembers.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(SetupTimerHandle);
        return;
    }

    for (int32 i = PendingSetupMembers.Num() - 1; i >= 0; --i)
    {
        AMultiplayerActionCharacter* CurrentMember = PendingSetupMembers[i];

        if (!CurrentMember)
        {
            PendingSetupMembers.RemoveAt(i);
            continue;
        }

        AAIController* AIController = Cast<AAIController>(CurrentMember->GetController());

        if (AIController && AIController->GetBlackboardComponent())
        {
            UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

            CurrentMember->InitializeGroupMembership(this);

            int32 OriginalIndex = GroupMembers.Find(CurrentMember);

            
            if (OriginalIndex == 0)// This is the Leader
            {
                BlackboardComp->SetValueAsBool(IsLeaderKeyName, true);
                BlackboardComp->SetValueAsObject(PatrolPathKeyName, PatrolPath);
                BlackboardComp->SetValueAsInt(CurrentPatrolIndexKeyName, 0);
            }
            else if (OriginalIndex != INDEX_NONE)// This is a Follower
            {
                AMultiplayerActionCharacter* LeaderCharacter = GroupMembers[0];
                BlackboardComp->SetValueAsBool(IsLeaderKeyName, false);
                BlackboardComp->SetValueAsObject(PatrolLeaderKeyName, LeaderCharacter);

                if (FormationOffsets.IsValidIndex(OriginalIndex - 1))
                {
                    BlackboardComp->SetValueAsVector(FormationOffsetKeyName, FormationOffsets[OriginalIndex - 1]);
                }
            }

            PendingSetupMembers.RemoveAt(i);
        }
    }

    if (PendingSetupMembers.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("All group members successfully initialized for %s."), *this->GetName());
        GetWorld()->GetTimerManager().ClearTimer(SetupTimerHandle);
    }
}

void AAIGroupManager::SetGroupLeader(class AMultiplayerActionCharacter* NewLeaderPawn)
{
    // This function should ONLY ever run on the server.
    if (!HasAuthority())
    {
        return;
    }

    if (!NewLeaderPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetGroupLeader called with null Pawn!"));
        return;
	}

    NewLeaderPawn->InitializeGroupMembership(this);


    // Now, command every group member to update their target.
    for (AMultiplayerActionCharacter* CurrentMember : GroupMembers)
    {
        if(!CurrentMember)
        {
            continue; // Skip null members
		}

		ADefaultAIController* MemberController = Cast<ADefaultAIController>(CurrentMember->GetController());

        if (MemberController && MemberController->GetBlackboardComponent())
        {
            int32 OriginalIndex = GroupMembers.Find(CurrentMember);
            // Set the "Leader" key on each AI's blackboard.
            // This will cause their Behavior Tree to switch to the "Follow" state.
            // If the NewLeaderPawn is null, it will clear the value, making them stop following.
            MemberController->GetBlackboardComponent()->SetValueAsBool(IsLeaderKeyName, false);
            MemberController->GetBlackboardComponent()->SetValueAsObject(PatrolLeaderKeyName, NewLeaderPawn);

            if (FormationOffsets.IsValidIndex(OriginalIndex))
            {
                MemberController->GetBlackboardComponent()->SetValueAsVector(FormationOffsetKeyName, FormationOffsets[OriginalIndex]);
            }
        }
    }
}

void AAIGroupManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
