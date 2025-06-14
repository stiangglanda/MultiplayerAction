// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

// Sets default values
AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = true;
	bOpen = false;

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

void AChest::OpenChest()
{
	if (OpenCloseAnim)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(OpenCloseAnim, 1, EMontagePlayReturnType::MontageLength, AnimInstance->Montage_GetPosition(OpenCloseAnim), true);
		}
	}

	// Create and show widget
	if (ChestMenuWidgetClass && !ChestMenuWidget)
	{
		// Get the first local player controller
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (PC && PC->IsLocalController())
		{
			ChestMenuWidget = CreateWidget<UUserWidget>(PC, ChestMenuWidgetClass);
			if (ChestMenuWidget)
			{
				ChestMenuWidget->AddToViewport();
			}
		}
	}

	bOpen = !bOpen;
}

void AChest::ToggleOpenClose()
{
	if(bOpen)
	{
		CloseChest();
	}
	else
	{
		OpenChest();
	}
}

void AChest::CloseChest()
{
	if (!bOpen)
		return;
	
	if (OpenCloseAnim)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{
			float CurrentPosition = AnimInstance->Montage_GetPosition(OpenCloseAnim) == 0 ? OpenCloseAnim->GetPlayLength() : AnimInstance->Montage_GetPosition(OpenCloseAnim);
			AnimInstance->Montage_Play(OpenCloseAnim,-1,EMontagePlayReturnType::MontageLength, CurrentPosition, true);
		}
	}

	if (ChestMenuWidget)
	{
		ChestMenuWidget->RemoveFromParent();
		ChestMenuWidget = nullptr;
	}

	bOpen = !bOpen;
}

// Called when the game starts or when spawned
void AChest::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

