#include "KingsShrine.h"
#include "Components/WidgetComponent.h"
#include "InteractionProgressBarWidget.h"
#include <Net/UnrealNetwork.h>
#include "AIGroupManager.h"
#include "MultiplayerActionCharacter.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "DefaultGameState.h"

AKingsShrine::AKingsShrine()
{
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);
	AltarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh"));
	AltarMesh->SetupAttachment(Root);
	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(AltarMesh);

	InteractionPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(Root);
	InteractionPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidget->SetVisibility(false);
}

void AKingsShrine::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionProgressBarWidgetClass)
	{
		InteractionPromptWidget->SetWidgetClass(InteractionProgressBarWidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("KingsShrine: InteractionProgressBarWidgetClass is NOT set in the Blueprint!"));
	}
	
}

void AKingsShrine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKingsShrine, bIsKeyTaken);
}

void AKingsShrine::OnInteract_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;

	APlayerController* PC = InstigatorPawn->GetController<APlayerController>();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("OnInteract (SERVER): FAILED to get PlayerController from InstigatorPawn!"));
		return;
	}

	if (bIsKeyTaken)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInteract (SERVER): Key is taken. Telling client to show 'Completed' message."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInteract (SERVER): Key is available. Telling client to show progress UI."));
		StartInteraction(InstigatorPawn);
	}
}

void AKingsShrine::OnStopInteract_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;
    APlayerController* PC = InstigatorPawn->GetController<APlayerController>();
    if (!PC) return;
    
    StopInteraction();
}

void AKingsShrine::OnBeginFocus_Implementation(APawn* InstigatorPawn)
{

}

void AKingsShrine::OnEndFocus_Implementation(APawn* InstigatorPawn)
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(false);
	}

	OnStopInteract_Implementation(InstigatorPawn);
}

FText AKingsShrine::GetInteractionText_Implementation() const
{
	return FText::FromString(TEXT("Hold to Activate Shrine"));
}

void AKingsShrine::OnClientStartInteract_Implementation(AMultiplayerActionCharacter* InteractingCharacter)
{
	if (!InteractingCharacter) return;
	APlayerController* PC = InteractingCharacter->GetController<APlayerController>();
	if (!PC || !PC->IsLocalController()) return;

	if (bIsKeyTaken)
	{
		if (InteractionProgressBarWidgetClass)
		{
			UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, InteractionProgressBarWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				Widget->ShowCompletedMessage();

				InteractingCharacter->SetActiveProgressBar(Widget);
			}
		}
	}
	else
	{
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
}

bool AKingsShrine::IsCompleted()
{
	return bIsKeyTaken;
}

void AKingsShrine::StartInteraction(APawn* InstigatorPawn)
{
	if (bIsKeyTaken || InteractionTimerHandle.IsValid() || !InstigatorPawn)
	{
		return;
	}

	InteractingPlayer = InstigatorPawn;

	AMultiplayerActionCharacter* InteractingCharacter = Cast<AMultiplayerActionCharacter>(InteractingPlayer);
	if (InteractingCharacter)
	{
		InteractingCharacter->PlayInteractionMontage(ShrineChannelingMontage);
	}

	GetWorld()->GetTimerManager().SetTimer(
		InteractionTimerHandle,
		this,
		&AKingsShrine::OnInteractionComplete,
		InteractionDuration,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("Server: Shrine interaction started by %s."), *InstigatorPawn->GetName());
}

void AKingsShrine::StopInteraction()
{
	if (InteractionTimerHandle.IsValid())
	{
		AMultiplayerActionCharacter* InteractingCharacter = Cast<AMultiplayerActionCharacter>(InteractingPlayer);
		if (InteractingCharacter)
		{
			InteractingCharacter->StopInteractionMontage();
		}

		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
		InteractingPlayer = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Server: Shrine interaction cancelled."));
	}
}

void AKingsShrine::OnInteractionComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: Interaction COMPLETE. Key is taken."));

	AMultiplayerActionCharacter* InteractingCharacter = Cast<AMultiplayerActionCharacter>(InteractingPlayer);
	if (InteractingCharacter)
	{
		InteractingCharacter->Client_OnInteractionSuccess();
		InteractingCharacter->StopInteractionMontage();
	}

	if (GroupManager)
	{
		GroupManager->SetGroupLeader(InteractingCharacter);
	}

	ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
	if (GameState)
	{
		UE_LOG(LogTemp, Log, TEXT("Shrine reporting to GameState: King's Key has been acquired."));
		GameState->SetHasKingsKey(true);
	}

	bIsKeyTaken = true;

	OnRep_KeyTaken();

	InteractingPlayer = nullptr;
}

void AKingsShrine::OnRep_KeyTaken()
{
	UE_LOG(LogTemp, Warning, TEXT("Client: Replicating key taken state. Hiding key mesh."));

	if (KeyMesh)
	{
		KeyMesh->SetVisibility(false);
		KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(false);
	}
}