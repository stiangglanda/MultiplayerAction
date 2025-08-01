#include "HealthAltar.h"
#include "MultiplayerActionCharacter.h"
#include <Net/UnrealNetwork.h>

AHealthAltar::AHealthAltar()
{
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);
	AltarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh"));
	AltarMesh->SetupAttachment(Root);
	GobletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GobletMesh"));
	GobletMesh->SetupAttachment(AltarMesh);
}

void AHealthAltar::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHealthAltar::OnInteract_Implementation(APawn* InstigatorPawn)
{
	BeginInteraction(InstigatorPawn);
}

void AHealthAltar::OnStopInteract_Implementation(APawn* InstigatorPawn)
{
	CancelInteraction();
}

void AHealthAltar::BeginInteraction(APawn* InstigatorPawn)
{
	if (InteractionTimerHandle.IsValid() || !InstigatorPawn)
	{
		return;
	}

	InteractingPlayer = InstigatorPawn;

	GetWorld()->GetTimerManager().SetTimer(InteractionTimerHandle, this, &AHealthAltar::OnInteractionComplete, InteractionDuration, false);

	if (AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InstigatorPawn))
	{
		Char->PlayInteractionMontage(InteractionMontage);
	}
}

void AHealthAltar::CancelInteraction()
{
	if (InteractionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);

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

bool AHealthAltar::IsCompleted()
{
	return bWasUsed;
}

void AHealthAltar::OnInteractionComplete()
{
	if (!InteractingPlayer) return;

	AMultiplayerActionCharacter* Char = Cast<AMultiplayerActionCharacter>(InteractingPlayer);
	if (Char)
	{
		Char->SetMaxHealth(NewMaxHealth);
		Char->SetHealth(NewMaxHealth);

		Char->StopInteractionMontage();
		Char->Client_OnInteractionSuccess();

		bWasUsed = true;
		OnRep_Used();
	}

	InteractingPlayer = nullptr;
}

void AHealthAltar::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHealthAltar, bWasUsed);
}

void AHealthAltar::OnRep_Used()
{
	UE_LOG(LogTemp, Warning, TEXT("Client: Replicating AHealthAltar was used"));

	if (GobletMesh)
	{
		GobletMesh->SetVisibility(false);
		GobletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AHealthAltar::OnClientStartInteract_Implementation(AMultiplayerActionCharacter* InteractingCharacter)
{
	if (!InteractingCharacter) return;
	APlayerController* PC = InteractingCharacter->GetController<APlayerController>();
	if (!PC || !PC->IsLocalController()) return;


	if (InteractionProgressBarWidgetClass)
	{
		UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, InteractionProgressBarWidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
			Widget->StartProgress(InteractionDuration);

			InteractingCharacter->SetActiveProgressBar(Widget);
		}
	}

	InteractingCharacter->Server_RequestStartInteract(this);
}

void AHealthAltar::OnEndFocus_Implementation(APawn* InstigatorPawn)
{
	OnStopInteract_Implementation(InstigatorPawn);
}
