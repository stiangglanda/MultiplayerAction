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
//void AChest::OpenChest()
//{
//	if (OpenCloseAnim)
//	{
//		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
//		if (AnimInstance)
//		{
//			if (ChestOpenSound)
//			{
//				UGameplayStatics::PlaySoundAtLocation(this, ChestOpenSound, GetActorLocation());
//			}
//
//			AnimInstance->Montage_Play(OpenCloseAnim, 1, EMontagePlayReturnType::MontageLength, AnimInstance->Montage_GetPosition(OpenCloseAnim), true);
//		}
//	}
//
//	if (ChestMenuWidgetClass && !ChestMenuWidget)
//	{
//		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//
//		if (PC && PC->IsLocalController())
//		{
//			ChestMenuWidget = CreateWidget<UChestWidget>(PC, ChestMenuWidgetClass);
//			if (ChestMenuWidget)
//			{
//				ChestMenuWidget->SetChestReference(this);
//				ChestMenuWidget->AddToViewport();
//			}
//		}
//	}
//
//	bOpen = !bOpen;
//}

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
			Client_ShowFeedback(true, false);
		}
	}
}

void AChest::OnStopInteract_Implementation(APawn* InstigatorPawn)
{
	Server_CancelUnlock();
}

void AChest::OnEndFocus_Implementation(APawn* InstigatorPawn)
{
	CloseChest(InstigatorPawn);
}

//bool AChest::ToggleOpenClose()
//{
//	if(bOpen)
//	{
//		CloseChest();
//	}
//	else
//	{
//		OpenChest();
//	}
//	return bOpen;
//}

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

//void AChest::CloseChest()
//{
//	if (!bOpen)
//		return;
//	
//	if (OpenCloseAnim)
//	{
//		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
//		if (AnimInstance)
//		{
//			if (ChestCloseSound)
//			{
//				UGameplayStatics::PlaySoundAtLocation(this, ChestCloseSound, GetActorLocation());
//			}
//
//			float CurrentPosition = AnimInstance->Montage_GetPosition(OpenCloseAnim) == 0 ? OpenCloseAnim->GetPlayLength() : AnimInstance->Montage_GetPosition(OpenCloseAnim);
//			AnimInstance->Montage_Play(OpenCloseAnim,-1,EMontagePlayReturnType::MontageLength, CurrentPosition, true);
//		}
//	}
//
//	if (ChestMenuWidget)
//	{
//		ChestMenuWidget->RemoveFromParent();
//		ChestMenuWidget = nullptr;
//	}
//
//	bOpen = !bOpen;
//}

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
		Char->StopInteractionMontage();
	}

	// Tell the client to hide the UI
	Client_HideUnlockUI();

	// Set the replicated state
	bIsUnlocked = true;
	OnRep_Unlocked(); // Call for server

	InteractingPlayer = nullptr;
}

void AChest::Client_ShowUnlockUI_Implementation()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->IsLocalController() && UnlockingWidgetClass)
	{
		UInteractionProgressBarWidget* UnlockWidget = CreateWidget<UInteractionProgressBarWidget>(PC, UnlockingWidgetClass);
		if (UnlockWidget)
		{
			UnlockWidget->AddToViewport();
			UnlockWidget->StartProgress(UnlockDuration);
		}
	}
}

void AChest::Client_ShowFeedback_Implementation(bool bIsLocked, bool bHasKey)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->IsLocalController())
	{
		if (bIsLocked && !bHasKey)
		{
			// Play locked sound, show "Requires King's Key" widget.
			if (ChestLockedSound) UGameplayStatics::PlaySoundAtLocation(this, ChestLockedSound, GetActorLocation());
			// Create and show a temporary "Locked" widget...
		}
	}
}

void AChest::Client_HideUnlockUI_Implementation()
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		GetWorld(), FoundWidgets, UnlockingWidgetClass
	);
	for (UUserWidget* Widget : FoundWidgets)
	{
		Widget->RemoveFromParent();
	}
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

		// Tell the client to hide the progress bar
		Client_HideUnlockUI();
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

	// Start the server-side timer
	GetWorld()->GetTimerManager().SetTimer(UnlockTimerHandle, this, &AChest::OnUnlockComplete, UnlockDuration, false);

	// Tell the player to start their unlock animation
	if (AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InstigatorPawn))
	{
		// Now we pass our specific UnlockMontage to the generic function
		Char->PlayInteractionMontage(UnlockMontage);
	}

	// Tell the client to show the progress bar
	Client_ShowUnlockUI();
}

