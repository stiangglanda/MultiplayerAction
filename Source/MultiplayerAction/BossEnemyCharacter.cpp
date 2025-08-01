#include "BossEnemyCharacter.h"
#include "Engine/DamageEvents.h"
#include "DefaultGameState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void ABossEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
		if (GameState)
		{
			GameState->OnStartedUnlockingChestDelegate.AddDynamic(this, &ABossEnemyCharacter::OnChestUnlockingStarted);
			UE_LOG(LogTemp, Log, TEXT("BossEnemyCharacter has subscribed to OnStartedUnlockingChestDelegate."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BossEnemyCharacter could not find ADefaultGameState to subscribe to delegate."));
		}
	}
}

void ABossEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
		if (GameState)
		{
			GameState->OnStartedUnlockingChestDelegate.RemoveDynamic(this, &ABossEnemyCharacter::OnChestUnlockingStarted);
			UE_LOG(LogTemp, Log, TEXT("BossEnemyCharacter has unsubscribed from OnStartedUnlockingChestDelegate."));
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ABossEnemyCharacter::PerformWeaponTrace()
{
	TArray<FHitResult> hits;
	TArray<AActor*> ignore = ActorsHit;
	ignore.Add(this);
	FVector WeaponStart = GetMesh()->GetBoneLocation("lowerarm_r");
	FVector WeaponEnd = GetMesh()->GetBoneLocation("index_03_r");

	bool bSuccess = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), WeaponStart, WeaponEnd, SphereTraceRadiusWeapon,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1), false, ignore,
		EDrawDebugTrace::None, hits, true, FLinearColor::Red, FLinearColor::Blue, 10.0f);

	if (bSuccess)
	{
		for (int i = hits.Num() - 1; i >= 0; i--)
		{
			if (hits[i].GetActor() != nullptr)
			{
				AMultiplayerActionCharacter* unit = Cast<AMultiplayerActionCharacter>(hits[i].GetActor());
				if (unit && unit->GetTeam() != GetTeam())
				{
					FPointDamageEvent DamageEvent(BossDamage, hits[i], -GetActorLocation(), nullptr);
					unit->TakeDamage(BossDamage, DamageEvent, GetController(), this);
					ActorsHit.Add(unit);
					ApplyKnockbackToPlayer(unit, hits[i]);
				}
			}
		}
	}
}

void ABossEnemyCharacter::ApplyKnockbackToPlayer(ACharacter* PlayerToLaunch, const FHitResult& HitResult)
{
	if (!PlayerToLaunch)
	{
		return;
	}

	FVector KnockbackDirection = -HitResult.ImpactNormal;
	KnockbackDirection.Z = 0;
	KnockbackDirection.Normalize();

	FVector LaunchVelocity = KnockbackDirection * KnockbackStrength;
	LaunchVelocity.Z = KnockbackUpwardForce;

	PlayerToLaunch->LaunchCharacter(LaunchVelocity, true, true);
}

void ABossEnemyCharacter::OnChestUnlockingStarted()
{
	UE_LOG(LogTemp, Warning, TEXT("BOSS ENEMY: Alerted! Abandoning patrol and moving to start location."));

	AAIController* MyAIController = GetController<AAIController>();
	if (!MyAIController)
	{
		UE_LOG(LogTemp, Error, TEXT("BossEnemyCharacter::OnChestUnlockingStarted - Could not get AI Controller."));
		return;
	}

	UBlackboardComponent* MyBlackboard = MyAIController->GetBlackboardComponent();
	if (!MyBlackboard)
	{
		UE_LOG(LogTemp, Error, TEXT("BossEnemyCharacter::OnChestUnlockingStarted - Could not get Blackboard Component."));
		return;
	}

	MyBlackboard->ClearValue(TEXT("PatrolPath"));
	UE_LOG(LogTemp, Log, TEXT("Boss AI: Cleared 'PatrolPath' key."));

	const FVector StartLocation = MyBlackboard->GetValueAsVector(TEXT("Start Location"));

	if (StartLocation != FVector::ZeroVector)
	{
		MyBlackboard->SetValueAsVector(TEXT("BossReturnLocation"), StartLocation);
		UE_LOG(LogTemp, Log, TEXT("Boss AI: Setting 'MoveToLocation' to StartLocation: %s"), *StartLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: 'StartLocation' key is not set on the Blackboard. Cannot move to start."));
	}
}
