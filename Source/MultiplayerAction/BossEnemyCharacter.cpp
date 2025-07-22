#include "BossEnemyCharacter.h"
#include "Engine/DamageEvents.h"

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