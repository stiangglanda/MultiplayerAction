// Copyright Epic Games, Inc. All Rights Reserved.

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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMyLevelPlaygroundCharacter

AMultiplayerActionCharacter::AMultiplayerActionCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMultiplayerActionCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Look);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::AttackInputMapping);

		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Triggered, this, &AMultiplayerActionCharacter::Block);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMultiplayerActionCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AMultiplayerActionCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMultiplayerActionCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMultiplayerActionCharacter::Block(const FInputActionValue& Value)
{
	ServerReliableRPC_Block();
}

void AMultiplayerActionCharacter::AttackInputMapping(const FInputActionValue& Value)
{
	ServerReliableRPC_Attack();
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
		return 0;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("TakeDamage"));
	DamageApplied = FMath::Min(Health, DamageApplied);

	Health -= DamageApplied;

	UE_LOG(LogTemp, Display, TEXT("Health: %f"), Health);

	PlayImpactAnimation();

	if (IsDead())
	{
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return DamageApplied;
}

void AMultiplayerActionCharacter::PlayImpactAnimation()
{
	if (ImpactMontage)
	{
		PlayAnimMontage(ImpactMontage);
	}
}

void AMultiplayerActionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AMultiplayerActionCharacter, Health);
}

void AMultiplayerActionCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), Health);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (Health <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), Health);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}


	PlayImpactAnimation();

	//Functions that occur on all machines.
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

bool AMultiplayerActionCharacter::IsDead()
{
	return Health<=0;
}

float AMultiplayerActionCharacter::GetHeathPercent() const
{
	return Health / MaxHealth;
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
		}


		TArray<FHitResult> hits;
		TArray<AActor*> ignore;
		ignore.Add(this);

		bool bSuccess = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * SphareTraceLength,
			SphareTraceRadius, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel1), false, ignore,
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
					}
				}
			}
		}
	}
}

void AMultiplayerActionCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
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