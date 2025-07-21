#include "BossEnemyCharacter.h"
#include "Engine/DamageEvents.h"
#include "BrainComponent.h"

void ABossEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &ABossEnemyCharacter::OnOverlapBegin);

	if (!bIsAwake)
	{
		// If the AI Controller somehow possessed this pawn already, unpossess it.
		AController* CurrentController = GetController();
		if (CurrentController)
		{
			// Stop any running logic (like a behavior tree)
			AAIController* AICont = Cast<AAIController>(CurrentController);
			if (AICont && AICont->GetBrainComponent())
			{
				AICont->GetBrainComponent()->StopLogic("Golem is sleeping.");
			}
			CurrentController->UnPossess();
		}

		// Play the wake-up montage but pause it immediately.
		if (WakeUpMontage)
		{
			PlayAnimMontage(WakeUpMontage, 1.0f, NAME_None); // Start from the beginning
			GetMesh()->GetAnimInstance()->Montage_Pause(WakeUpMontage);
		}
	}
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

void ABossEnemyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if we are already awake or if the overlapping actor is not the player
	if (bIsAwake || !OtherActor || !Cast<ACharacter>(OtherActor)->IsPlayerControlled())
	{
		return;
	}

	// --- START THE WAKE-UP SEQUENCE ---
	UE_LOG(LogTemp, Warning, TEXT("Player detected! Waking up golem."));

	bIsAwake = true;

	// No longer need to detect overlaps
	SphereCollider->OnComponentBeginOverlap.RemoveAll(this);

	// 1. Possess the golem with its AI Controller
	ADefaultAIController* MyAIController = GetController<ADefaultAIController>();
	if (!MyAIController)
	{
		// Spawn a new controller if one doesn't exist
		MyAIController = GetWorld()->SpawnActor<ADefaultAIController>(DefaultAIControllerClass, GetActorLocation(), GetActorRotation());
		if (MyAIController)
		{
			MyAIController->Possess(this);
		}
	}

	// 2. Un-pause the animation
	if (WakeUpMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Resume(WakeUpMontage);
	}

	// 3. Start the Behavior Tree after the animation finishes
	const float MontageLength = WakeUpMontage ? WakeUpMontage->CalculateSequenceLength() : 0.f;

	FTimerHandle TimerHandle_StartBT;
	FTimerDelegate TimerDelegate;

	// Use a lambda to capture the controller
	TimerDelegate.BindLambda([MyAIController]()
		{
			if (MyAIController)
			{
				// The animation is done, tell the AI to start thinking.
				MyAIController->RunBehaviorTree(MyAIController->AIBehavior);
			}
		});

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_StartBT, TimerDelegate, MontageLength, false);
}
