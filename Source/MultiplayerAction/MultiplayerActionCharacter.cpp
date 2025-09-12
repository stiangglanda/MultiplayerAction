#include "MultiplayerActionCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Chest.h"
#include <Kismet/KismetMathLibrary.h>
#include "AIGroupManager.h"
#include <Perception/AISense_Damage.h>
#include <AIController.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "KingsShrine.h"
#include "MultiplayerActionGameMode.h"
#include "BossEnemyCharacter.h"
#include "DefaultPlayerController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMultiplayerActionCharacter::AMultiplayerActionCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;

	Health = MaxHealth;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Chest Collider"));
	SphereCollider->InitSphereRadius(SphereColliderRadius);

	SphereCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SphereCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

	SphereCollider->CanCharacterStepUpOn = ECB_Yes;
	SphereCollider->SetShouldUpdatePhysicsVolume(true);
	SphereCollider->SetCanEverAffectNavigation(false);
	SphereCollider->SetupAttachment(RootComponent);

	CurrentInteractable = nullptr;
	LockedOnTarget = nullptr;

	MovementAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MovementAudioComponent"));
	MovementAudioComponent->SetupAttachment(GetRootComponent());
	MovementAudioComponent->bAutoActivate = false;

	ShieldMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(GetMesh(), TEXT("ShieldSocket"));
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

AMultiplayerActionCharacter::~AMultiplayerActionCharacter()
{
}

void AMultiplayerActionCharacter::BeginPlay()
{
	Super::BeginPlay();

	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AMultiplayerActionCharacter::OnOverlapBegin);
	SphereCollider->OnComponentEndOverlap.AddDynamic(this, &AMultiplayerActionCharacter::OnOverlapEnd);

	if (WeaponClass && !Weapon)
	{
		Weapon = NewObject<UWeapon>(this, WeaponClass);
		if (Weapon)
		{
			Weapon->RegisterComponent();
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
		}
	}


	if (GetMesh())
	{
		const int32 MaterialCount = GetMesh()->GetNumMaterials();

		for (int32 i = 0; i < MaterialCount; ++i)
		{
			UMaterialInstanceDynamic* DMI = GetMesh()->CreateAndSetMaterialInstanceDynamic(i);
			if (DMI)
			{
				BodyMaterialInstances.Add(DMI);
			}
		}
	}
}

void AMultiplayerActionCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Error, TEXT("'%s' is calling EndPlay. Cleaning up timers and delegates."), *GetName());
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.Clear();
	}

	Super::EndPlay(EndPlayReason);
}

void AMultiplayerActionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMultiplayerActionCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::AttackInputMapping);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Block);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Lock);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Roll);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMultiplayerActionCharacter::Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AMultiplayerActionCharacter::StopInteract);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::HeavyAttack);
		EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Escape);
		EnhancedInputComponent->BindAction(GroupControlAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::GroupControl);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMultiplayerActionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	if (bIsRolling)
	{
		FVector ForwardVector = GetCharacterMovement()->GetLastInputVector();
		if (ForwardVector == FVector::ZeroVector)
		{
			ForwardVector = GetActorForwardVector();
		}

		AddMovementInput(ForwardVector, 1.0f);
	}

	if (bIsLockedOn && (IsLocallyControlled() || HasAuthority()))
	{
		if (IsValid(LockedOnTarget) && !LockedOnTarget->IsDead())
		{
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(FollowCamera->GetComponentLocation(), LockedOnTarget->GetActorLocation());

			if (!bIsRolling)
			{
				FRotator TargetYawRotation = FRotator(0, LookAtRotation.Yaw, 0);
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetYawRotation, DeltaTime, 15.0f));
			}

			if (IsLocallyControlled())
			{
				GetController()->SetControlRotation(FMath::RInterpTo(GetController()->GetControlRotation(), LookAtRotation, DeltaTime, 15.0f));
			}
		}
		else
		{
			if (HasAuthority())
			{
				bIsLockedOn = false;
				LockedOnTarget = nullptr;
			}
		}
	}

	if (IsLocallyControlled() && CameraBoom)
	{
		FVector TargetSocketOffset = DefaultSocketOffset;

		if (bIsLockedOn)
		{
			const float MoveRightInput = MoveRightAxisValue;
			TargetSocketOffset.Y = MoveRightInput * LockOnCameraHorizontalOffset;
		}

		CameraBoom->SocketOffset = FMath::VInterpTo(
			CameraBoom->SocketOffset,
			TargetSocketOffset,
			DeltaTime,
			LockOnCameraShiftSpeed
		);
	}
}

void AMultiplayerActionCharacter::OnRep_MaxHealth()
{
	if (HealthShrineComplete)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HealthShrineComplete, GetActorLocation());
	}

	if (ADefaultPlayerController* PC = GetController<ADefaultPlayerController>())
	{
		if (UPlayerHUDWidget* HUD = PC->GetHUD())
		{
			HUD->UpdateHealthBar(MaxHealth);
		}
	}
}

void AMultiplayerActionCharacter::OnRep_CurrentHealth()
{
	if (IsLocallyControlled())
	{
		//FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), Health);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (Health <= 0)
		{
			//FString deathMessage = FString::Printf(TEXT("You have been killed."));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		//FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), Health);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
}

void AMultiplayerActionCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		MoveRightAxisValue = MovementVector.X;//for locking system
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMultiplayerActionCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMultiplayerActionCharacter::Roll(const FInputActionValue& Value)
{
	if (IsBusy())
	{
		return;
	}

	if (!bIsRolling && RollMontage)
	{
		bIsRolling = true;
		DisableInput(Cast<APlayerController>(GetController()));

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!RollMontageEndedDelegate.IsBound())
			{
				RollMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnRollMontageEnded);
			}

			if (RollSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, RollSound, GetActorLocation());
			}

			AnimInstance->Montage_Play(RollMontage);
			AnimInstance->Montage_SetEndDelegate(RollMontageEndedDelegate, RollMontage);
		}
	}
	Server_RequestRoll();
}

void AMultiplayerActionCharacter::Server_RequestRoll_Implementation()
{
	if (IsBusy())
	{
		return;
	}

	if (!bIsRolling && RollMontage)
	{
		bIsRolling = true;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!RollMontageEndedDelegate.IsBound())
			{
				RollMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnRollMontageEnded);
			}
			AnimInstance->Montage_Play(RollMontage);
			AnimInstance->Montage_SetEndDelegate(RollMontageEndedDelegate, RollMontage);
		}
	}
}

void AMultiplayerActionCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsRolling = false;

	if (IsLocallyControlled())
	{
		EnableInput(Cast<APlayerController>(GetController()));
	}
}

void AMultiplayerActionCharacter::Lock(const FInputActionValue& Value)
{
	Server_RequestLockOn();
}

void AMultiplayerActionCharacter::Block(const FInputActionValue& Value)
{
	Server_RequestBlock();
}

void AMultiplayerActionCharacter::HeavyAttack(const FInputActionValue& Value)
{
	Server_RequestHeavyAttack();
}

void AMultiplayerActionCharacter::Server_RequestHeavyAttack_Implementation()
{
	if (IsBusy())
	{
		return;
	}

	if (!bIsAttacking && HeavyAttackMontage)
	{
		bIsAttacking = true;

		RotationBeforeAttack = GetActorRotation();

		CurrentAttackType = EAttackType::EAT_Heavy;

		Multicast_PlayHeavyAttackEffects();

		StartWeaponTrace();
	}
}

void AMultiplayerActionCharacter::Multicast_PlayHeavyAttackEffects_Implementation()
{
	if (HeavyAttackGruntSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HeavyAttackGruntSound, GetActorLocation());
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HeavyAttackMontage)
	{
		AnimInstance->Montage_Play(HeavyAttackMontage);

		if (HasAuthority())
		{
			if (!HeavyAttackMontageEndedDelegate.IsBound())
			{
				HeavyAttackMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnHeavyAttackMontageEnded);
			}
			AnimInstance->Montage_SetEndDelegate(HeavyAttackMontageEndedDelegate, HeavyAttackMontage);
		}
	}
}

void AMultiplayerActionCharacter::Interact(const FInputActionValue& Value)
{
	if (CurrentInteractable)
	{
		if (InteractionWidget)
		{
			InteractionWidget->RemoveFromParent();
			InteractionWidget = nullptr;
		}

		IOutpostInteractable::Execute_OnClientStartInteract(CurrentInteractable.GetObject(), this);
	}
}

void AMultiplayerActionCharacter::StopInteract(const FInputActionValue& Value)
{
	if (ActiveProgressBarWidget)
	{
		ActiveProgressBarWidget->RemoveFromParent();
		ActiveProgressBarWidget = nullptr;
	}

	if (CurrentInteractable)
	{
		AActor* InteractableActor = Cast<AActor>(CurrentInteractable.GetObject());
		if (InteractableActor)
		{
			Server_RequestStopInteract(InteractableActor);
		}
	}
}

void AMultiplayerActionCharacter::Escape(const FInputActionValue& Value)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (!EscapeWidgetClass)
	{
		return;
	}

	if (!EscapeWidget)
	{
		EscapeWidget = CreateWidget<UUserWidget>(PC, EscapeWidgetClass);
		if (EscapeWidget)
		{
			EscapeWidget->AddToViewport();
		}
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		return;
	}

	if (EscapeWidget->IsInViewport())
	{
		EscapeWidget->RemoveFromParent();
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		return;
	}
	else
	{
		EscapeWidget->AddToViewport();
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
		return;
	}
}

void AMultiplayerActionCharacter::GroupControl(const FInputActionValue& Value)
{
	Server_RequestToggleGroupCombat();
}

void AMultiplayerActionCharacter::Jump()
{
	Super::Jump();
	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
	}
}

void AMultiplayerActionCharacter::SetTeam(int NewTeam)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Team was updated"));
		Team = NewTeam;
	}
}

int AMultiplayerActionCharacter::GetTeam()
{
	return Team;
}

float AMultiplayerActionCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead())
	{
		return 0.f;
	}

	float DamageApplied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (bIsBlocking)
	{
		Multicast_PlayBlockSound();
		return 0;
	}

	DamageApplied = FMath::Min(Health, DamageApplied);
	Health -= DamageApplied;
	UE_LOG(LogTemp, Display, TEXT("Health: %f"), Health);
	OnRep_CurrentHealth();

	if (EventInstigator != nullptr && DamageCauser != nullptr)
	{
		AAIController* MyController = Cast<AAIController>(GetController());
		if (MyController)
		{
			UBlackboardComponent* MyBlackboard = MyController->GetBlackboardComponent();
			if (MyBlackboard)
			{
				MyBlackboard->SetValueAsObject(TEXT("Player"), EventInstigator->GetPawn());
				UE_LOG(LogTemp, Warning, TEXT("Direct Damage React: Setting target to %s"), *EventInstigator->GetPawn()->GetName());
			}
		}
	}

	Multicast_PlayDamageEffects();

	if (IsDead())
	{
		GetWorld()->GetTimerManager().ClearTimer(WeaponTraceTimer);

		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.Clear();
		}

		if (HasAuthority())
		{
			AMultiplayerActionGameMode* GameMode = GetWorld()->GetAuthGameMode<AMultiplayerActionGameMode>();
			if (GameMode)
			{
				if (ABossEnemyCharacter* Boss = Cast<ABossEnemyCharacter>(this))
				{
					GameMode->OnBossDied(Boss);
				}
				else if (IsPlayerControlled())
				{
					GameMode->OnPlayerDied(this);
				}
			}
		}

		Multicast_PlayDeathEffects();

		GetCharacterMovement()->DisableMovement();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetLifeSpan(10.0f);
	}

	return DamageApplied;
}

void AMultiplayerActionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiplayerActionCharacter, Health);
	DOREPLIFETIME(AMultiplayerActionCharacter, WeaponClass);
	DOREPLIFETIME(AMultiplayerActionCharacter, CurrentInteractionMontage);
	DOREPLIFETIME(AMultiplayerActionCharacter, MaxHealth);
	DOREPLIFETIME(AMultiplayerActionCharacter, Team);
	DOREPLIFETIME(AMultiplayerActionCharacter, AIGroupManager);
	DOREPLIFETIME(AMultiplayerActionCharacter, bfollowMode);
	DOREPLIFETIME(AMultiplayerActionCharacter, bIsAttacking);
	DOREPLIFETIME(AMultiplayerActionCharacter, AttackAnim);
	DOREPLIFETIME(AMultiplayerActionCharacter, bIsBlocking);
	DOREPLIFETIME_CONDITION(AMultiplayerActionCharacter, bIsRolling, COND_SkipOwner);
	DOREPLIFETIME(AMultiplayerActionCharacter, bIsLockedOn);
	DOREPLIFETIME(AMultiplayerActionCharacter, LockedOnTarget);
}

bool AMultiplayerActionCharacter::IsDead()
{
	return Health<=0;
}

bool AMultiplayerActionCharacter::GetFollowMode()
{
	return bfollowMode;
}

float AMultiplayerActionCharacter::GetHeathPercent() const
{
	return Health / MaxHealth;
}

void AMultiplayerActionCharacter::OnRep_WeaponClass()
{
	if (Weapon)
	{
		Weapon->DestroyComponent();
		Weapon = nullptr;
	}

	if (WeaponClass)
	{
		Weapon = NewObject<UWeapon>(this, WeaponClass);
		if (Weapon)
		{
			Weapon->RegisterComponent();
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
		}
	}
}

TSubclassOf<UWeapon> AMultiplayerActionCharacter::SwapWeapon(TSubclassOf<UWeapon> NewWeaponClass)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerReliableRPC_SwapWeapon(NewWeaponClass);
		return WeaponClass; // Return current weapon class since the actual swap will happen via RPC
	}

	TSubclassOf<UWeapon> OldWeaponClass = WeaponClass;
	if (WeaponClass == NewWeaponClass || !NewWeaponClass)
	{
		return OldWeaponClass;
	}

	NetMulticastReliableRPC_SwapWeapon(NewWeaponClass);
	return OldWeaponClass;
}

void AMultiplayerActionCharacter::Server_RequestLockOn_Implementation()
{
	if (bIsLockedOn)
	{
		bIsLockedOn = false;
		LockedOnTarget = nullptr;
		return;
	}

	TArray<FHitResult> hits;
	TArray<AActor*> ignore;
	ignore.Add(this);

	bool bSuccess = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), GetActorLocation(), SphereTraceRadiusLockOn,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1), false, ignore,
		EDrawDebugTrace::None, hits, true);

	if (bSuccess)
	{
		for (int i = hits.Num() - 1; i >= 0; i--)
		{
			if (hits[i].GetActor() != nullptr)
			{
				AMultiplayerActionCharacter* unit = Cast<AMultiplayerActionCharacter>(hits[i].GetActor());
				if (unit && unit->GetTeam() != GetTeam())
				{
					bIsLockedOn = true;
					LockedOnTarget = unit;
					break;
				}
			}
		}
	}
}

void AMultiplayerActionCharacter::ServerReliableRPC_SwapWeapon_Implementation(TSubclassOf<UWeapon> NewWeaponClass)
{
	SwapWeapon(NewWeaponClass);
}

void AMultiplayerActionCharacter::NetMulticastReliableRPC_SwapWeapon_Implementation(TSubclassOf<UWeapon> NewWeaponClass)
{
	if (WeaponSwapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponSwapSound, GetActorLocation());
	}
	WeaponClass = NewWeaponClass;
	OnRep_WeaponClass();
}

void AMultiplayerActionCharacter::Server_RequestStartInteract_Implementation(AActor* InteractableActor)
{
	if (InteractableActor && InteractableActor->Implements<UOutpostInteractable>())
	{
		IOutpostInteractable::Execute_OnInteract(InteractableActor, this);
	}
}

void AMultiplayerActionCharacter::Server_RequestStopInteract_Implementation(AActor* InteractableActor)
{
	if (InteractableActor && InteractableActor->Implements<UOutpostInteractable>())
	{
		IOutpostInteractable::Execute_OnStopInteract(InteractableActor, this);
	}
}

void AMultiplayerActionCharacter::Multicast_ConcludeHeavyAttack_Implementation(const FRotator& NewRotation)
{
	if (HeavyAttackMontage)
	{
		StopAnimMontage(HeavyAttackMontage);
	}

	SetActorRotation(NewRotation);
}

void AMultiplayerActionCharacter::PerformWeaponTrace()
{
	if (!IsValid(Weapon))
	{
		return;
	}

	TArray<FHitResult> hits;
	TArray<AActor*> ignore = ActorsHit;
	ignore.Add(this);
	FVector WeaponStart = Weapon->GetSocketLocation("start");
	FVector WeaponEnd = Weapon->GetSocketLocation("end");

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
					if (!ActorsHit.Contains(unit))
					{
						Multicast_PlayImpactSound(hits[i].ImpactPoint);

						float Damage = 0.0f;

						switch (CurrentAttackType)	
						{
						case EAttackType::EAT_None:
							Damage = 0.0f;
							break;
						case EAttackType::EAT_Regular:
							Damage = Weapon->WeaponData.WeaponDamage;
							break;
						case EAttackType::EAT_Heavy:
							Damage = Weapon->WeaponData.WeaponHeavyDamage;
							break;
						default:
							break;
						}

						FPointDamageEvent DamageEvent(Damage, hits[i], -GetActorLocation(), nullptr);
						unit->TakeDamage(Damage, DamageEvent, GetController(), this);
						ActorsHit.Add(unit);
					}
				}
			}
		}
	}
}

void AMultiplayerActionCharacter::StartWeaponTrace()
{
	//if (HasAuthority())
	//{
	//	ActorsHit.Empty();
	//	FTimerDelegate TimerDelegate;

	//	TimerDelegate.BindLambda([this]()
	//	{
	//		this->PerformWeaponTrace();
	//	});

	//	GetWorld()->GetTimerManager().SetTimer(WeaponTraceTimer, TimerDelegate, WeaponTraceInterval, true);
	//}

	if (HasAuthority())
	{
		ActorsHit.Empty();

		FTimerDelegate TimerDelegate;
		TWeakObjectPtr<AMultiplayerActionCharacter> WeakThis(this);

		TimerDelegate.BindLambda([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->PerformWeaponTrace();
			}
		});

		GetWorld()->GetTimerManager().SetTimer(
			WeaponTraceTimer,
			TimerDelegate,
			WeaponTraceInterval,
			true
		);
	}
}

void AMultiplayerActionCharacter::StopWeaponTrace()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(WeaponTraceTimer);
		ActorsHit.Empty();
	}
}

void AMultiplayerActionCharacter::AttackInputMapping(const FInputActionValue& Value)
{
	Server_RequestAttack();
}

void AMultiplayerActionCharacter::Server_RequestAttack_Implementation()
{
	if (IsBusy())
	{
		return;
	}

	if (!bIsAttacking && CombatMontage && CombatMontageAlt)
	{
		bIsAttacking = true;

		UAnimMontage* MontageToPlay = AttackAnim ? CombatMontage : CombatMontageAlt;
		AttackAnim = !AttackAnim;

		CurrentAttackType = EAttackType::EAT_Regular;

		Multicast_PlayAttackEffects(MontageToPlay);

		StartWeaponTrace();
	}
}

void AMultiplayerActionCharacter::Multicast_PlayAttackEffects_Implementation(UAnimMontage* MontageToPlay)
{
	if (AttackGruntSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackGruntSound, GetActorLocation());
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MontageToPlay)
	{
		AnimInstance->Montage_Play(MontageToPlay);

		if (HasAuthority())
		{
			if (!AttackMontageEndedDelegate.IsBound())
			{
				AttackMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnAttackMontageEnded);
			}
			AnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, MontageToPlay);
		}
	}
}

void AMultiplayerActionCharacter::OnRep_IsRolling()
{
	if (bIsRolling)
	{
		PlayAnimMontage(RollMontage);
	}
}

void AMultiplayerActionCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		CurrentAttackType = EAttackType::EAT_None;
		bIsAttacking = false;
		StopWeaponTrace();
	}
}

void AMultiplayerActionCharacter::OnHeavyAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		OnAttackMontageEnded(Montage, bInterrupted);

		Multicast_ConcludeHeavyAttack(RotationBeforeAttack);
	}
}

void AMultiplayerActionCharacter::Server_RequestBlock_Implementation()
{
	if (IsBusy())
	{
		return;
	}

	if (!bIsBlocking && BlockMontage)
	{
		bIsBlocking = true;

		Multicast_PlayBlockEffects();
	}
}

void AMultiplayerActionCharacter::Multicast_PlayBlockEffects_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && BlockMontage)
	{
		AnimInstance->Montage_Play(BlockMontage);

		if (HasAuthority())
		{
			if (!BlockMontageEndedDelegate.IsBound())
			{
				BlockMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnBlockMontageEnded);
			}
			AnimInstance->Montage_SetEndDelegate(BlockMontageEndedDelegate, BlockMontage);
		}
	}
}

void AMultiplayerActionCharacter::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		bIsBlocking = false;
	}
}

void AMultiplayerActionCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	if (OtherActor->Implements<UOutpostInteractable>())
	{
		if (InteractionWidgetClass && !InteractionWidget)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				InteractionWidget = CreateWidget<UUserWidget>(PC, InteractionWidgetClass);
				if (InteractionWidget)
				{
					InteractionWidget->AddToViewport();
				}
			}
		}

		IOutpostInteractable::Execute_OnBeginFocus(OtherActor, this);
		CurrentInteractable = OtherActor;
	}
}

void AMultiplayerActionCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	if (OtherActor == CurrentInteractable.GetObject())
	{
		if (InteractionWidget)
		{
			InteractionWidget->RemoveFromParent();
			InteractionWidget = nullptr;
		}

		if (ActiveProgressBarWidget)
		{
			ActiveProgressBarWidget->RemoveFromParent();
			ActiveProgressBarWidget = nullptr;
		}

		IOutpostInteractable::Execute_OnEndFocus(OtherActor, this);
		CurrentInteractable = nullptr;

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}

void AMultiplayerActionCharacter::PlayInteractionMontage(UAnimMontage* MontageToPlay)
{
	if (HasAuthority() && MontageToPlay)
	{
		CurrentInteractionMontage = MontageToPlay;
		Multicast_PlayInteractionMontage(MontageToPlay);
	}
}

void AMultiplayerActionCharacter::StopInteractionMontage()
{
	if (HasAuthority())
	{
		Multicast_StopInteractionMontage();

		CurrentInteractionMontage = nullptr;
	}
}

void AMultiplayerActionCharacter::SetActiveProgressBar(UInteractionProgressBarWidget* Widget)
{
	if (ActiveProgressBarWidget)
	{
		ActiveProgressBarWidget->RemoveFromParent();
		ActiveProgressBarWidget = nullptr;
	}

	ActiveProgressBarWidget = Widget;
}

void AMultiplayerActionCharacter::SetMaxHealth(float NewValue)
{
	if (HasAuthority())
	{
		MaxHealth = NewValue;
		OnRep_MaxHealth();
	}
}

void AMultiplayerActionCharacter::SetHealth(float NewValue)
{
	if (HasAuthority())
	{
		Health = FMath::Clamp(NewValue, 0.f, MaxHealth);
		OnRep_CurrentHealth();
	}
}

void AMultiplayerActionCharacter::Multicast_PlayInteractionMontage_Implementation(UAnimMontage* MontageToPlay)
{
	if (Weapon) { Weapon->SetVisibility(false); }
	if (ShieldMesh) { ShieldMesh->SetVisibility(false); }

	if (MontageToPlay)
	{
		CurrentInteractionMontage = MontageToPlay;
		PlayAnimMontage(MontageToPlay);
	}
}

void AMultiplayerActionCharacter::Multicast_StopInteractionMontage_Implementation()
{
	if (Weapon) { Weapon->SetVisibility(true); }
	if (ShieldMesh) { ShieldMesh->SetVisibility(true); }

	if (CurrentInteractionMontage)
	{
		StopAnimMontage(CurrentInteractionMontage);
	}
}

void AMultiplayerActionCharacter::Client_OnInteractionSuccess_Implementation()
{
	if (ActiveProgressBarWidget)
	{
		ActiveProgressBarWidget->RemoveFromParent();
		ActiveProgressBarWidget = nullptr;
	}
}

void AMultiplayerActionCharacter::Client_OnBecameGroupLeader_Implementation()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("CLIENT received OnBecameGroupLeader notification."));

	if (KeyShrineComplete)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KeyShrineComplete, GetActorLocation());
	}

	if (ADefaultPlayerController* PC = GetController<ADefaultPlayerController>())
	{
		if (PC->IsLocalController())
		{
			if (UPlayerHUDWidget* HUD = PC->GetHUD())
			{
				HUD->PlayerGetsFollowers();
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Warning, TEXT("Client_OnBecameGroupLeader: Could not get HUD from local PlayerController."));
			}
		}
	}
}

void AMultiplayerActionCharacter::Server_RequestToggleGroupCombat_Implementation()
{
	if (AIGroupManager)
	{
		bfollowMode = !bfollowMode;
		AIGroupManager->AllowCombat(this, bfollowMode);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s tried to toggle group combat but has no Group Manager."), *GetName());
	}
}

void AMultiplayerActionCharacter::Multicast_PlayImpactSound_Implementation(FVector ImpactLocation)
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			AttackSound,
			ImpactLocation
		);
	}
}

void AMultiplayerActionCharacter::Multicast_PlayBlockSound_Implementation()
{
	if (BlockSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
	}
}

void AMultiplayerActionCharacter::Multicast_PlayDamageEffects_Implementation()
{
	if (DamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageSound, GetActorLocation());
	}
	if (ImpactMontage)
	{
		PlayAnimMontage(ImpactMontage);
	}

	PlayHitFlash();
}

void AMultiplayerActionCharacter::Multicast_PlayDeathEffects_Implementation()
{
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	GetMesh()->SetSimulatePhysics(true);
}

void AMultiplayerActionCharacter::PlayHitFlash()
{
	if (BodyMaterialInstances.Num() > 0)
	{
		for (UMaterialInstanceDynamic* DMI : BodyMaterialInstances)
		{
			if (DMI)
			{
				DMI->SetScalarParameterValue(FlashIntensityParameterName, 1.0f);
			}
		}

		GetWorld()->GetTimerManager().SetTimer(
			HitFlashTimer,
			this,
			&AMultiplayerActionCharacter::StopHitFlash,
			0.1f,
			false
		);
	}
}

void AMultiplayerActionCharacter::StopHitFlash()
{
	for (UMaterialInstanceDynamic* DMI : BodyMaterialInstances)
	{
		if (DMI)
		{
			DMI->SetScalarParameterValue(FlashIntensityParameterName, 0.0f);
		}
	}
}

void AMultiplayerActionCharacter::InitializeGroupMembership(TObjectPtr<class AAIGroupManager> GroupManager)
{
	if (!HasAuthority())
	{
		return;
	}

	if (GroupManager)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("GroupManager initialized successfull"));
		AIGroupManager = GroupManager;

		Client_OnBecameGroupLeader();
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("GroupManager is null"));
	}
}

bool AMultiplayerActionCharacter::IsBusy() const
{
	return bIsAttacking || bIsRolling;
}