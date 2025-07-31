#include "Chest.h"
#include "MultiplayerActionCharacter.h"
#include "DefaultGameState.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "InteractionProgressBarWidget.h"
#include <Net/UnrealNetwork.h>

AChest::AChest()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	bOpen = false;

	ChestName = FText::FromString("Default Chest Name");

	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("Chest"));
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(RootComponent);
		static FName MeshCollisionProfileName(TEXT("Chest"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->SetGenerateOverlapEvents(true);
		Mesh->SetCanEverAffectNavigation(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

		Mesh->CanCharacterStepUpOn = ECB_Yes;
		Mesh->SetShouldUpdatePhysicsVolume(true);
	}


}

void AChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChest, bIsUnlocked);
}

void AChest::OpenChest(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;

	APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		// We only want to open the UI for the specific player who interacted.
		return;
	}

	if (OpenCloseAnim)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{
			if (ChestOpenSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ChestOpenSound, GetActorLocation());
			}
	
			AnimInstance->Montage_Play(OpenCloseAnim, 1, EMontagePlayReturnType::MontageLength, AnimInstance->Montage_GetPosition(OpenCloseAnim), true);
		}
	}

	// Create and show the chest's inventory widget.
	if (ChestMenuWidgetClass && !ChestMenuWidget)
	{
		ChestMenuWidget = CreateWidget<UChestWidget>(PC, ChestMenuWidgetClass);
		if (ChestMenuWidget)
		{
			ChestMenuWidget->SetChestReference(this);
			ChestMenuWidget->AddToViewport();

			// --- RESPONSIBILITY SHIFT ---
			// The CHEST is now responsible for changing the player's input mode.
			PC->SetInputMode(FInputModeGameAndUI());
			PC->bShowMouseCursor = true;
		}
	}

	bOpen = true;
}

void AChest::OnInteract_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;

	if (bIsUnlocked)
	{
		// STATE 3: UNLOCKED - Just open/close the chest
		if (bOpen)
		{
			CloseChest(InstigatorPawn);
		}
		else
		{
			OpenChest(InstigatorPawn);
		}
	}
	else
	{
		// STATE 1: LOCKED - Check for key and begin unlocking
		ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
		if (GameState && GameState->bHasKingsKey)
		{
			// Player HAS the key, begin the unlock process.
			Server_BeginUnlock(InstigatorPawn);
		}
		else
		{
			// Player does NOT have the key. Show feedback.
		}
	}
}

void AChest::OnStopInteract_Implementation(APawn* InstigatorPawn)
{
	Server_CancelUnlock();
}

void AChest::OnEndFocus_Implementation(APawn* InstigatorPawn)
{
	Server_CancelUnlock();
	CloseChest(InstigatorPawn);
}

void AChest::OnClientStartInteract_Implementation(AMultiplayerActionCharacter* InteractingCharacter)
{
	if (!InteractingCharacter) return;
	APlayerController* PC = InteractingCharacter->GetController<APlayerController>();
	if (!PC || !PC->IsLocalController()) return;

	// --- LOGIC MOVED FROM PLAYER ---
	ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();

	if (bIsUnlocked)
	{
		// Chest is unlocked, just tell the server to open it.
		OnInteract_Implementation(InteractingCharacter);
		//InteractingCharacter->Server_RequestStartInteract(this);
	}
	else if (GameState && GameState->bHasKingsKey)
	{
		// Chest is locked, but we have the key. Show progress bar.
		if (UnlockingWidgetClass)
		{
			UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, UnlockingWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				Widget->StartProgress(UnlockDuration);

				// --- HAND OFF THE WIDGET REFERENCE ---
				InteractingCharacter->SetActiveProgressBar(Widget);
			}
		}

		// Tell the server to start the unlock process.
		InteractingCharacter->Server_RequestStartInteract(this);
	}
	else
	{
		if (UnlockingWidgetClass)
		{
			UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, UnlockingWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				Widget->ShowCompletedMessage();

				// --- HAND OFF THE WIDGET REFERENCE ---
				InteractingCharacter->SetActiveProgressBar(Widget);
			}
		}

		if (ChestLockedSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ChestLockedSound, GetActorLocation());
		}
		// Chest is locked, no key. Show locked message.
		// ... create and show temporary "locked" widget ...
	}
}

void AChest::CloseChest(APawn* InstigatorPawn)
{
	if (!bOpen) return;
	if (!InstigatorPawn) return;

	APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());

	if (OpenCloseAnim)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{
			if (ChestCloseSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ChestCloseSound, GetActorLocation());
			}
	
			float CurrentPosition = AnimInstance->Montage_GetPosition(OpenCloseAnim) == 0 ? OpenCloseAnim->GetPlayLength() : AnimInstance->Montage_GetPosition(OpenCloseAnim);
			AnimInstance->Montage_Play(OpenCloseAnim,-1,EMontagePlayReturnType::MontageLength, CurrentPosition, true);
		}
	}

	// Remove the widget from the screen.
	if (ChestMenuWidget)
	{
		ChestMenuWidget->RemoveFromParent();
		ChestMenuWidget = nullptr;
	}

	// --- RESPONSIBILITY SHIFT ---
	// The CHEST tells the player's controller to return to game-only input.
	if (PC && PC->IsLocalController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	bOpen = false;
}

FWeaponData* AChest::GetChestContents()
{
	return &Weapon->WeaponData;
}

FText AChest::GetChestName() const
{
	return ChestName;
}

void AChest::Swap()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC && PC->IsLocalController())
	{
		AMultiplayerActionCharacter* PlayerPawn = Cast<AMultiplayerActionCharacter>(PC->GetPawn());
		if (PlayerPawn)
		{
			WeaponClass = PlayerPawn->SwapWeapon(WeaponClass);
			Weapon = nullptr;
			if (WeaponClass)
			{
				Weapon = NewObject<UWeapon>(this, WeaponClass);
			}
		}
	}
}

void AChest::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponClass && !Weapon)
	{
		Weapon = NewObject<UWeapon>(this, WeaponClass);
	}

}

void AChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AChest::IsCompleted()
{
	return bIsUnlocked;
}

void AChest::OnUnlockComplete()
{
	if (!InteractingPlayer) return;

	// Stop the player's animation
	if (AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InteractingPlayer))
	{
		Char->Client_OnInteractionSuccess();
		Char->StopInteractionMontage();
	}

	// Set the replicated state
	bIsUnlocked = true;
	OnRep_Unlocked(); // Call for server

	InteractingPlayer = nullptr;
}

void AChest::OnRep_Unlocked()
{
	if (ChestUnlockSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChestUnlockSound, GetActorLocation());
	}
}

void AChest::Server_CancelUnlock_Implementation()
{
	if (UnlockTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(UnlockTimerHandle);

		if (InteractingPlayer)
		{
			if (AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InteractingPlayer))
			{
				Char->StopInteractionMontage();
			}
		}
		InteractingPlayer = nullptr;
	}
}

void AChest::Server_BeginUnlock_Implementation(APawn* InstigatorPawn)
{
	if (bIsUnlocked || UnlockTimerHandle.IsValid() || !InstigatorPawn)
	{
		return;
	}

	InteractingPlayer = InstigatorPawn;

	ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
	if (GameState)
	{
		GameState->SetStartedUnlockingChest(true);
		UE_LOG(LogTemp, Log, TEXT("Chest (Server): Reporting to GameState that unlocking has started."));
	}

	// Start the server-side timer
	GetWorld()->GetTimerManager().SetTimer(UnlockTimerHandle, this, &AChest::OnUnlockComplete, UnlockDuration, false);

	// Tell the player to start their unlock animation
	if (AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InstigatorPawn))
	{
		// Now we pass our specific UnlockMontage to the generic function
		Char->PlayInteractionMontage(UnlockMontage);
	}

}

