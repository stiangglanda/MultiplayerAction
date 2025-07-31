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
		// Get the current GameState.
		ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
		if (GameState)
		{
			// Bind our local function 'OnChestUnlockingStarted' to the delegate.
			// Now, whenever the GameState broadcasts this delegate, our function will be called.
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
					FPointDamageEvent DamageEvent(WeaponDamage, hits[i], -GetActorLocation(), nullptr);
					unit->TakeDamage(WeaponDamage, DamageEvent, GetController(), this);
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
	// This code will execute on the server when the delegate is broadcast.
	UE_LOG(LogTemp, Warning, TEXT("BOSS ENEMY: Alerted! Abandoning patrol and moving to start location."));

	// Get the AI Controller that is possessing this boss character.
	AAIController* MyAIController = GetController<AAIController>();
	if (!MyAIController)
	{
		UE_LOG(LogTemp, Error, TEXT("BossEnemyCharacter::OnChestUnlockingStarted - Could not get AI Controller."));
		return;
	}

	// Get the Blackboard component from the AI Controller.
	UBlackboardComponent* MyBlackboard = MyAIController->GetBlackboardComponent();
	if (!MyBlackboard)
	{
		UE_LOG(LogTemp, Error, TEXT("BossEnemyCharacter::OnChestUnlockingStarted - Could not get Blackboard Component."));
		return;
	}

	// --- BLACKBOARD MANIPULATION ---

	// 1. Clear the "PatrolPath" key.
	// This will cause any Behavior Tree branch that depends on this key (e.g., the "Patrol" sequence) to fail.
	// We use the key's name as an FName. Make sure this matches the name in your Blackboard asset EXACTLY.
	MyBlackboard->ClearValue(TEXT("PatrolPath"));
	UE_LOG(LogTemp, Log, TEXT("Boss AI: Cleared 'PatrolPath' key."));

	// 2. Get the "StartLocation" value and set it as the new "MoveToLocation".
	// We assume you have a key named "StartLocation" of type Vector and a key named "MoveToLocation" of type Vector.
	// If you have a single "TargetLocation" key, you would use that instead.
	const FVector StartLocation = MyBlackboard->GetValueAsVector(TEXT("Start Location"));

	// Check if the StartLocation is valid (not zero).
	if (StartLocation != FVector::ZeroVector)
	{
		// Set the "MoveToLocation" key. This will trigger a "Move To" task in your Behavior Tree.
		MyBlackboard->SetValueAsVector(TEXT("BossReturnLocation"), StartLocation);
		UE_LOG(LogTemp, Log, TEXT("Boss AI: Setting 'MoveToLocation' to StartLocation: %s"), *StartLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: 'StartLocation' key is not set on the Blackboard. Cannot move to start."));
	}

	// Any other logic, like setting an "bIsEnraged" boolean, would go here.
	// MyBlackboard->SetValueAsBool(TEXT("bIsEnraged"), true);
}
