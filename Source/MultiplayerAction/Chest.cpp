// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

// Sets default values
AChest::AChest()
{
	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Chest Collider"));
	SphereCollider->InitSphereRadius(SphereColliderRadius);

	SphereCollider->CanCharacterStepUpOn = ECB_Yes;
	SphereCollider->SetShouldUpdatePhysicsVolume(true);
	SphereCollider->SetCanEverAffectNavigation(false);
	RootComponent = SphereCollider;

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
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}


}

// Called when the game starts or when spawned
void AChest::BeginPlay()
{
	Super::BeginPlay();
	Mesh->SetPlayRate(-1);
	if (Animation)
	{
		Mesh->PlayAnimation(Animation, true);
	}
	
}

// Called every frame
void AChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

