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

	if (IsRolling)
	{
		FVector ForwardVector = GetCharacterMovement()->GetLastInputVector();
		if (ForwardVector == FVector::ZeroVector)
		{
			ForwardVector = GetActorForwardVector();
		}

		AddMovementInput(ForwardVector, 1.0f);
	}

	if (IsLockedOn)
	{
		if (LockedOnTarget == nullptr || LockedOnTarget->IsDead())
		{
			IsLockedOn = false;
			GetController()->ResetIgnoreLookInput();
		}
		else
		{
			FVector LookAtVector = LockedOnTarget->GetActorLocation();
			LookAtVector.Z -= 90;
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookAtVector);
			FRotator LookAtYawRotation = FRotator(0, LookAtRotation.Yaw, 0);

			if (!IsRolling)
			{
				SetActorRotation(LookAtYawRotation);
			}
			GetController()->SetControlRotation(LookAtRotation);
		}
	}
}

void AMultiplayerActionCharacter::OnRep_MaxHealth()
{
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
	ServerReliableRPC_Roll();
}

void AMultiplayerActionCharacter::ServerReliableRPC_Roll_Implementation()
{
	NetMulticastReliableRPC_Roll();
}

void AMultiplayerActionCharacter::NetMulticastReliableRPC_Roll_Implementation()
{
	if (RollMontage)
	{
		IsBlocking = true;
		IsRolling = true;

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
}

void AMultiplayerActionCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	IsBlocking = false;
	IsRolling = false;
	EnableInput(Cast<APlayerController>(GetController()));
}

void AMultiplayerActionCharacter::Lock(const FInputActionValue& Value)
{
	if (IsLockedOn == true)
	{
		GetController()->ResetIgnoreLookInput();
		IsLockedOn = false;
		LockedOnTarget = nullptr;
		return;
	}

	TArray<FHitResult> hits;
	TArray<AActor*> ignore;
	ignore.Add(this);

	bool bSuccess = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), GetActorLocation(), SphereTraceRadiusLockOn,
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
					SetLockedOnTarget(unit);
				}
			}
		}
	}
}

void AMultiplayerActionCharacter::SetLockedOnTarget(AMultiplayerActionCharacter* unit)
{
	IsLockedOn = true;
	GetController()->SetIgnoreLookInput(true);
	LockedOnTarget = unit;
}

void AMultiplayerActionCharacter::Block(const FInputActionValue& Value)
{
	ServerReliableRPC_Block();
}

void AMultiplayerActionCharacter::HeavyAttack(const FInputActionValue& Value)
{
	ServerReliableRPC_HeavyAttack();
}

void AMultiplayerActionCharacter::ServerReliableRPC_HeavyAttack_Implementation()
{
	NetMulticastReliableRPC_HeavyAttack();
}

void AMultiplayerActionCharacter::NetMulticastReliableRPC_HeavyAttack_Implementation()
{
	if (!bIsAttacking && HeavyAttackMontage)
	{
		bIsAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!HeavyAttackMontageEndedDelegate.IsBound())
			{
				HeavyAttackMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnHeavyAttackMontageEnded);
			}
			RotationBeforeAttack = GetActorRotation();

			if (HeavyAttackGruntSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HeavyAttackGruntSound, GetActorLocation());
			}

			AnimInstance->Montage_Play(HeavyAttackMontage);
			AnimInstance->Montage_SetEndDelegate(HeavyAttackMontageEndedDelegate, HeavyAttackMontage);

			StartWeaponTrace();
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
		PC->SetInputMode(FInputModeGameAndUI());
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
		PC->SetInputMode(FInputModeGameAndUI());
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

	if (IsBlocking)
	{
		if (BlockSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
		}
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

	if (DamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageSound, GetActorLocation());
	}

	if (ImpactMontage)
	{
		PlayAnimMontage(ImpactMontage);
	}

	if (IsDead())
	{
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

		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}

		GetCharacterMovement()->DisableMovement();
		GetMesh()->SetSimulatePhysics(true);// Ragdoll
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
	DOREPLIFETIME(AMultiplayerActionCharacter, allowCombat);
}

bool AMultiplayerActionCharacter::IsDead()
{
	return Health<=0;
}

bool AMultiplayerActionCharacter::GetAllowCombat()
{
	return allowCombat;
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
					if (AttackSound)
					{
						UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
					}
					FPointDamageEvent DamageEvent(Weapon->WeaponData.WeaponDamage, hits[i], -GetActorLocation(), nullptr);
					unit->TakeDamage(Weapon->WeaponData.WeaponDamage, DamageEvent, GetController(), this);
					ActorsHit.Add(unit);
				}
			}
		}
	}
}

void AMultiplayerActionCharacter::StartWeaponTrace()
{
	ActorsHit.Empty();
	GetWorld()->GetTimerManager().SetTimer(WeaponTraceTimer, this, &AMultiplayerActionCharacter::PerformWeaponTrace, WeaponTraceInterval, true);
}

void AMultiplayerActionCharacter::StopWeaponTrace()
{
	GetWorld()->GetTimerManager().ClearTimer(WeaponTraceTimer);
	ActorsHit.Empty();
}

void AMultiplayerActionCharacter::AttackInputMapping(const FInputActionValue& Value)
{
	ServerReliableRPC_Attack();
}

void AMultiplayerActionCharacter::ServerReliableRPC_Attack_Implementation()
{
	NetMulticastReliableRPC_Attack();
}

void AMultiplayerActionCharacter::NetMulticastReliableRPC_Attack_Implementation()
{
	if (!bIsAttacking && CombatMontage && CombatMontageAlt)
	{
		bIsAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!AttackMontageEndedDelegate.IsBound())
			{
				AttackMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnAttackMontageEnded);
			}

			if (AttackGruntSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, AttackGruntSound, GetActorLocation());
			}

			if (AttackAnim)
			{
				AnimInstance->Montage_Play(CombatMontage);
				AnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, CombatMontage);
			}
			else 
			{
				AnimInstance->Montage_Play(CombatMontageAlt);
				AnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, CombatMontageAlt);
			}

			AttackAnim = !AttackAnim;
			StartWeaponTrace();
		}
	}
}

void AMultiplayerActionCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	StopWeaponTrace();
}

void AMultiplayerActionCharacter::OnHeavyAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnAttackMontageEnded(Montage, bInterrupted);
	SetActorRotation(RotationBeforeAttack);
}

void AMultiplayerActionCharacter::ServerReliableRPC_Block_Implementation()
{
	NetMulticastReliableRPC_Block();
}

void AMultiplayerActionCharacter::NetMulticastReliableRPC_Block_Implementation()
{
	if (!IsBlocking && BlockMontage)
	{
		IsBlocking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!BlockMontageEndedDelegate.IsBound())
			{
				BlockMontageEndedDelegate.BindUObject(this, &AMultiplayerActionCharacter::OnBlockMontageEnded);
			}

			AnimInstance->Montage_Play(BlockMontage);
			AnimInstance->Montage_SetEndDelegate(BlockMontageEndedDelegate, BlockMontage);
		}
	}
}

void AMultiplayerActionCharacter::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	IsBlocking = false;
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

	// Now that we are on the client, we can safely access the local HUD.
	if (ADefaultPlayerController* PC = GetController<ADefaultPlayerController>())
	{
		// This check is an extra layer of safety.
		if (PC->IsLocalController())
		{
			if (UPlayerHUDWidget* HUD = PC->GetHUD())
			{
				// This will now work correctly!
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
		// Toggle the state
		allowCombat = !allowCombat;

		// Call the manager's function with the new state
		AIGroupManager->AllowCombat(this, allowCombat);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s tried to toggle group combat but has no Group Manager."), *GetName());
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

		//if (ADefaultPlayerController* PC = GetController<ADefaultPlayerController>())
		//{
		//	if (UPlayerHUDWidget* HUD = PC->GetHUD())
		//	{
		//		HUD->PlayerGetsFollowers();
		//	}
		//}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("GroupManager is null"));
	}
}