// Fill out your copyright notice in the Description page of Project Settings.


#include "KingsShrine.h"
#include "Components/WidgetComponent.h"
#include "InteractionProgressBarWidget.h"
#include <Net/UnrealNetwork.h>

// Sets default values
AKingsShrine::AKingsShrine()
{
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);
	AltarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh"));
	AltarMesh->SetupAttachment(Root);
	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(AltarMesh);

	// Create the component, but we will set its class later.
	InteractionPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(Root);
	InteractionPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidget->SetVisibility(false);
}

// Called when the game starts or when spawned
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

	// Tell the engine to replicate the bIsKeyTaken variable to all clients.
	DOREPLIFETIME(AKingsShrine, bIsKeyTaken);
}

void AKingsShrine::StartInteraction(APawn* InstigatorPawn)
{
	// Ask the server to start the interaction process.
	Server_StartInteraction(InstigatorPawn);

	// --- CLIENT-SIDE UI ---
	if (InteractionPromptWidget)
	{
		// 1. Get the actual UUserWidget instance from the component.
		UUserWidget* GenericWidget = InteractionPromptWidget->GetUserWidgetObject();
		if (GenericWidget)
		{
			// 2. Cast the generic UUserWidget to your specific C++ class.
			// This is the "safe" way to access your custom functions.
			UInteractionProgressBarWidget* ProgressBarWidget = Cast<UInteractionProgressBarWidget>(GenericWidget);
			if (InteractionPromptWidget)
			{
				// 3. Now you can call the function!
				ProgressBarWidget->StartProgress(InteractionDuration);
			}
		}

		// Show the component itself, which makes the widget visible in the world.
		InteractionPromptWidget->SetVisibility(true);
	}
}

void AKingsShrine::StopInteraction()
{
	// Ask the server to stop the interaction.
	Server_StopInteraction();

	// --- CLIENT-SIDE UI ---
	if (InteractionPromptWidget)
	{
		UUserWidget* GenericWidget = InteractionPromptWidget->GetUserWidgetObject();
		if (GenericWidget)
		{
			UInteractionProgressBarWidget* ProgressBarWidget = Cast<UInteractionProgressBarWidget>(GenericWidget);
			if (ProgressBarWidget)
			{
				// Call the corresponding "Stop" or "Reset" function
				ProgressBarWidget->StopProgress();
			}
		}

		InteractionPromptWidget->SetVisibility(false);
	}
}

// This function ONLY executes on the SERVER.
void AKingsShrine::Server_StartInteraction_Implementation(APawn* InstigatorPawn)
{
	// Can't interact if key is gone or someone else is already interacting.
	if (bIsKeyTaken || InteractionTimerHandle.IsValid())
	{
		return;
	}

	InteractingPlayer = InstigatorPawn;

	// Start a timer that will call OnInteractionComplete after InteractionDuration seconds.
	GetWorld()->GetTimerManager().SetTimer(
		InteractionTimerHandle,
		this,
		&AKingsShrine::OnInteractionComplete,
		InteractionDuration,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("Server: Interaction started by %s."), *InstigatorPawn->GetName());
}

// This function ONLY executes on the SERVER.
void AKingsShrine::Server_StopInteraction_Implementation()
{
	// If the timer is active, clear it.
	if (InteractionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
		InteractingPlayer = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Server: Interaction cancelled."));
	}
}

// This function ONLY executes on the SERVER.
void AKingsShrine::OnInteractionComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: Interaction COMPLETE. Key is taken."));

	// Set the replicated variable. This will trigger OnRep_KeyTaken on all clients.
	bIsKeyTaken = true;

	// We need to manually call the OnRep for the server itself, as it doesn't happen automatically.
	OnRep_KeyTaken();

	// Optionally give the key to the player who completed the interaction
	if (InteractingPlayer)
	{
		// ... Add logic to give item to InteractingPlayer ...
	}

	InteractingPlayer = nullptr;
}


// This function executes on ALL CLIENTS when the server changes bIsKeyTaken.
void AKingsShrine::OnRep_KeyTaken()
{
	UE_LOG(LogTemp, Warning, TEXT("Client: Replicating key taken state. Hiding key mesh."));

	// Hide the key mesh to show it has been taken.
	if (KeyMesh)
	{
		KeyMesh->SetVisibility(false);
		KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Also hide the interaction prompt permanently.
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(false);
	}
}