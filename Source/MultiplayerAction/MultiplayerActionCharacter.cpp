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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMyLevelPlaygroundCharacter

AMultiplayerActionCharacter::AMultiplayerActionCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;


	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	bReplicates = true;

	Health = MaxHealth;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Chest Collider"));
	SphereCollider->InitSphereRadius(SphereColliderRadius);

	// Add these lines to ensure proper collision setup
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
	// Call the base class  
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

//////////////////////////////////////////////////////////////////////////
// Input

void AMultiplayerActionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMultiplayerActionCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Look);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::AttackInputMapping);

		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Block);

		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Lock);

		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Roll);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMultiplayerActionCharacter::Interact);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AMultiplayerActionCharacter::StopInteract);

		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::HeavyAttack);

		EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Escape);
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

AMultiplayerActionCharacter::~AMultiplayerActionCharacter()
{

}

void AMultiplayerActionCharacter::OnRep_CurrentHealth()
{
	//Client-specific functionality
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

	//Server-specific functionality
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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HeavyAttack"));
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
		// 2. Call the generic OnInteract function on the object.
		// We pass 'this' as the InstigatorPawn.
		IOutpostInteractable::Execute_OnInteract(CurrentInteractable.GetObject(), this);
	}

	//if (InteractionWidget)
	//{
	//	InteractionWidget->RemoveFromParent();
	//	InteractionWidget = nullptr;
	//}

	//if (!OverlappingChest)
	//{
	//	return;
	//}

	//APlayerController* PC = Cast<APlayerController>(GetController());
	//if (!PC)
	//{
	//	return;
	//}

	//if (OverlappingChest->ToggleOpenClose())
	//{
	//	PC->SetInputMode(FInputModeGameAndUI());
	//	PC->bShowMouseCursor = true;
	//}
	//else
	//{
	//	PC->SetInputMode(FInputModeGameOnly());
	//	PC->bShowMouseCursor = false;
	//}
}

void AMultiplayerActionCharacter::StopInteract(const FInputActionValue& Value)
{
	if (CurrentInteractable)
	{
		// This will be used by the Shrine when the player releases the key.
		IOutpostInteractable::Execute_OnStopInteract(CurrentInteractable.GetObject(), this);
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

void AMultiplayerActionCharacter::Jump()
{
	Super::Jump();
	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
	}
}

int AMultiplayerActionCharacter::GetTeam()
{
	return Team;
}

float AMultiplayerActionCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (IsBlocking)
	{
		if (BlockSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
		}
		return 0;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("TakeDamage"));
	DamageApplied = FMath::Min(Health, DamageApplied);

	Health -= DamageApplied;

	UE_LOG(LogTemp, Display, TEXT("Health: %f"), Health);
	OnRep_CurrentHealth();

	if (EventInstigator != nullptr && DamageCauser != nullptr)// this is for the AIPerceptionComponent to persive the damage event
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
		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}
		GetMesh()->SetSimulatePhysics(true);
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return DamageApplied;
}

void AMultiplayerActionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AMultiplayerActionCharacter, Health);
	DOREPLIFETIME(AMultiplayerActionCharacter, WeaponClass);
}

bool AMultiplayerActionCharacter::IsDead()
{
	return Health<=0;
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
					FPointDamageEvent DamageEvent(WeaponDamage, hits[i], -GetActorLocation(), nullptr);
					unit->TakeDamage(WeaponDamage, DamageEvent, GetController(), this);
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

	// 1. Check if the overlapped actor implements our interface.
	// This replaces "IsA(AChest::StaticClass())".
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
		// 2. Tell the object that we are now "focused" on it.
		// This lets the object decide whether to show a prompt.
		// We use Execute_ to safely call interface functions.
		IOutpostInteractable::Execute_OnBeginFocus(OtherActor, this);

		// 3. Store a reference to this interactable object.
		CurrentInteractable = OtherActor;
	}

	//if (OtherActor && OtherActor->IsA(AChest::StaticClass()))
	//{
	//	// Create and show widget
	//	if (InteractionWidgetClass && !InteractionWidget)
	//	{
	//		APlayerController* PC = Cast<APlayerController>(GetController());
	//		if (PC)
	//		{
	//			InteractionWidget = CreateWidget<UUserWidget>(PC, InteractionWidgetClass);
	//			if (InteractionWidget)
	//			{
	//				InteractionWidget->AddToViewport();
	//			}
	//		}
	//	}

	//	AChest* chest = Cast<AChest>(OtherActor);
	//	OverlappingChest = chest;
	//}
}

void AMultiplayerActionCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	// 1. Check if the actor we are no longer overlapping is the one we were interacting with.
	if (OtherActor == CurrentInteractable.GetObject())
	{
		if (InteractionWidget)
		{
			InteractionWidget->RemoveFromParent();
			InteractionWidget = nullptr;
		}

		// 2. Tell the object that we are no longer focused on it.
		// This lets the object hide its own UI prompt.
		IOutpostInteractable::Execute_OnEndFocus(OtherActor, this);

		// 3. Clear our reference.
		CurrentInteractable = nullptr;
	}

	//if (OtherActor && OtherActor->IsA(AChest::StaticClass()))
	//{
	//	// Remove widget from viewport
	//	if (InteractionWidget)
	//	{
	//		InteractionWidget->RemoveFromParent();
	//		InteractionWidget = nullptr;
	//	}

	//	APlayerController* PC = Cast<APlayerController>(GetController());
	//	if (PC)
	//	{
	//		PC->SetInputMode(FInputModeGameOnly());
	//		PC->bShowMouseCursor = false;
	//	}

	//	OverlappingChest->CloseChest(); // Close the chest if it was open
	//	OverlappingChest = nullptr;
	//}
}

void AMultiplayerActionCharacter::PlayInteractionMontage()
{
	// Only the server should be able to initiate this replicated animation
	if (HasAuthority())
	{
		Multicast_PlayInteractionMontage();
	}
}

// This function is called BY THE SERVER (e.g., from the Shrine)
void AMultiplayerActionCharacter::StopInteractionMontage()
{
	if (HasAuthority())
	{
		Multicast_StopInteractionMontage();
	}
}


// This function executes on the SERVER and then REPLICATES to ALL CLIENTS
void AMultiplayerActionCharacter::Multicast_PlayInteractionMontage_Implementation()
{
	if (Weapon) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		Weapon->SetVisibility(false);
	}

	if (ShieldMesh) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		ShieldMesh->SetVisibility(false);
	}

	if (InteractionMontage)
	{
		// Play the looping section of the montage
		PlayAnimMontage(InteractionMontage, 1.0f);
	}
}

// This function executes on the SERVER and then REPLICATES to ALL CLIENTS
void AMultiplayerActionCharacter::Multicast_StopInteractionMontage_Implementation()
{
	if (Weapon) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		Weapon->SetVisibility(true);
	}

	if (ShieldMesh) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		ShieldMesh->SetVisibility(true);
	}

	if (InteractionMontage)
	{
		// Stop any instance of this montage that is currently playing.
		StopAnimMontage(InteractionMontage);
	}
}

void AMultiplayerActionCharacter::PlayPrayMontage()
{
	// Only the server should be able to initiate this replicated animation
	if (HasAuthority())
	{
		Multicast_PlayPrayMontage();
	}
}

// This function is called BY THE SERVER (e.g., from the Shrine)
void AMultiplayerActionCharacter::StopPrayMontage()
{
	if (HasAuthority())
	{
		Multicast_StopPrayMontage();
	}
}


// This function executes on the SERVER and then REPLICATES to ALL CLIENTS
void AMultiplayerActionCharacter::Multicast_PlayPrayMontage_Implementation()
{
	if (Weapon) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		Weapon->SetVisibility(false);
	}

	if (ShieldMesh) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		ShieldMesh->SetVisibility(false);
	}

	if (PrayMontage)
	{
		// Play the looping section of the montage
		PlayAnimMontage(PrayMontage, 1.0f, FName("Loop"));
	}
}

// This function executes on the SERVER and then REPLICATES to ALL CLIENTS
void AMultiplayerActionCharacter::Multicast_StopPrayMontage_Implementation()
{
	if (Weapon) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		Weapon->SetVisibility(true);
	}

	if (ShieldMesh) // Assuming CurrentWeapon is your TObjectPtr<AWeapon>
	{
		ShieldMesh->SetVisibility(true);
	}

	if (PrayMontage)
	{
		// Stop any instance of this montage that is currently playing.
		StopAnimMontage(PrayMontage);
	}
}

void AMultiplayerActionCharacter::InitializeGroupMembership(TObjectPtr<class AAIGroupManager> GroupManager)
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