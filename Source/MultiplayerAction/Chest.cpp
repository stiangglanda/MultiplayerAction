#include "Chest.h"
#include "MultiplayerActionCharacter.h"

AChest::AChest()
{
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
		// ... your animation and sound logic remains the same ...
		UGameplayStatics::PlaySoundAtLocation(this, ChestOpenSound, GetActorLocation());
		Mesh->GetAnimInstance()->Montage_Play(OpenCloseAnim, 1.0f);
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
	// The logic from your old ToggleOpenClose function moves here.
	if (bOpen)
	{
		CloseChest(InstigatorPawn); // Pass the pawn along
	}
	else
	{
		OpenChest(InstigatorPawn); // Pass the pawn along
	}
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
		// ... your animation and sound logic remains the same ...
		UGameplayStatics::PlaySoundAtLocation(this, ChestCloseSound, GetActorLocation());
		Mesh->GetAnimInstance()->Montage_Play(OpenCloseAnim, -1.0f, EMontagePlayReturnType::MontageLength, OpenCloseAnim->GetPlayLength());
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
	return false;
}

