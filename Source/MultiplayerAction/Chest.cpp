// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

// Sets default values
AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = true;

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
		Mesh->PlayAnimation(OpenCloseAnim, false);
		Mesh->SetPlayRate(1);
		Mesh->SetPosition(0);
	}
}

void AChest::CloseChest()
{
	if (OpenCloseAnim)
	{
		Mesh->PlayAnimation(OpenCloseAnim, false);
		Mesh->SetPlayRate(-1);
		Mesh->SetPosition(OpenCloseAnim->GetPlayLength());
	}
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

