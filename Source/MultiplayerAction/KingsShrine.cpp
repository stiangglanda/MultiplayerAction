// Fill out your copyright notice in the Description page of Project Settings.


#include "KingsShrine.h"
#include "Components/WidgetComponent.h"
#include "InteractionProgressBarWidget.h"
#include <Net/UnrealNetwork.h>
#include "AIGroupManager.h"
#include "MultiplayerActionCharacter.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "DefaultGameState.h"

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
		//Client_ShowInteractionUI(PC);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnInteract (SERVER): Key is available. Telling client to show progress UI."));
		//Client_ShowInteractionUI(PC);
		StartInteraction(InstigatorPawn);
	}

	//if (!InstigatorPawn)
	//{
	//	return;
	//}

	// Get the widget instance first, as we'll need it in either case.
	//UInteractionProgressBarWidget* ProgressBarWidget = nullptr;
	//if (InteractionPromptWidget)
	//{
	//	// We cast here once to avoid repeating code.
	//	ProgressBarWidget = Cast<UInteractionProgressBarWidget>(InteractionPromptWidget->GetUserWidgetObject());
	//}

	//if (!ProgressBarWidget)
	//{
	//	// If we can't get the widget, we can't do anything.
	//	return;
	//}

	//// --- THE CORE LOGIC CHANGE ---
	//// First, check the state of the shrine.
	//if (bIsKeyTaken)
	//{
	//	// The key is already gone. Show the "completed" message.
	//	InteractionPromptWidget->SetVisibility(true); // Make sure the component is visible
	//	ProgressBarWidget->ShowCompletedMessage();
	//}
	//else
	//{
	//	// The key is available. Proceed with the normal interaction.
	//	InteractionPromptWidget->SetVisibility(true);
	//	ProgressBarWidget->StartProgress(InteractionDuration);

	//	// Tell the server to start the timer.
	//	Server_StartInteraction(InstigatorPawn);
	//}
}

void AKingsShrine::OnStopInteract_Implementation(APawn* InstigatorPawn)
{
	if (!InstigatorPawn) return;
    APlayerController* PC = InstigatorPawn->GetController<APlayerController>();
    if (!PC) return;
    
    // Tell this specific client to hide the UI.
    //Client_HideInteractionUI(PC);
    
    // Stop the server-side logic.
    StopInteraction();
	// When the player releases "E", they are stopping the interaction.
	// Call our server function to cancel the timer.
	//Server_StopInteraction();

	//// Handle hiding the UI on the client that released the key.
	//if (InstigatorPawn)
	//{
	//	if (InteractionPromptWidget)
	//	{
	//		UInteractionProgressBarWidget* ProgressBarWidget = Cast<UInteractionProgressBarWidget>(InteractionPromptWidget->GetUserWidgetObject());
	//		if (ProgressBarWidget)
	//		{
	//			ProgressBarWidget->StopProgress(); // Assuming you have a Stop function
	//		}
	//		InteractionPromptWidget->SetVisibility(false);
	//	}
	//}
}

//void AKingsShrine::Client_ShowInteractionUI_Implementation(APlayerController* PlayerToTell)
//{
//	// First, check if we are the correct client to be showing this UI.
//	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//	if (!PC || !PC->IsLocalController())
//	{
//		// This check is a safety net.
//		return;
//	}
//
//	// Now that we are on the client, this CreateWidget call is VALID.
//	if (InteractionProgressBarWidgetClass)
//	{
//		UInteractionProgressBarWidget* ProgressWidget = CreateWidget<UInteractionProgressBarWidget>(PC, InteractionProgressBarWidgetClass);
//		if (ProgressWidget)
//		{
//			if (bIsKeyTaken)
//			{
//				ProgressWidget->ShowCompletedMessage();
//			}
//			else
//			{
//				ProgressWidget->StartProgress(InteractionDuration);
//			}
//			ProgressWidget->AddToViewport();
//		}
//	}
//}

// This function is CALLED on the server, but EXECUTES on the client's machine.
//void AKingsShrine::Client_HideInteractionUI_Implementation(APlayerController* PlayerToTell)
//{
//	if (!PlayerToTell || !PlayerToTell->IsLocalController())
//	{
//		return;
//	}
//
//	// This is harder because we don't have a direct reference to the widget we created.
//	// This is a classic problem with this pattern.
//	// SOLUTION: We need to give the widget a name or find it by class.
//
//	TArray<UUserWidget*> FoundWidgets;
//	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
//		GetWorld(), FoundWidgets, UInteractionProgressBarWidget::StaticClass()
//	);
//
//	for (UUserWidget* Widget : FoundWidgets)
//	{
//		// This will remove ALL instances of this widget class. For a single player game this is fine.
//		Widget->RemoveFromParent();
//	}
//}

void AKingsShrine::OnBeginFocus_Implementation(APawn* InstigatorPawn)
{
	// This is where you would show a simple "Hold [E] to Activate" prompt.
	// For this shrine, we can leave this blank because our main progress bar
	// serves the same purpose and is shown via OnInteract.
	// If you had a separate, simpler prompt, you would show it here.
}

void AKingsShrine::OnEndFocus_Implementation(APawn* InstigatorPawn)
{
	// If the player looks away while interacting, we should cancel it.
	// This is an important piece of game feel.
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(false);
	}

	OnStopInteract_Implementation(InstigatorPawn);
}

FText AKingsShrine::GetInteractionText_Implementation() const
{
	// Provide the text for a generic UI prompt if you were using one.
	return FText::FromString(TEXT("Hold to Activate Shrine"));
}

void AKingsShrine::OnClientStartInteract_Implementation(AMultiplayerActionCharacter* InteractingCharacter)
{
	if (!InteractingCharacter) return;
	APlayerController* PC = InteractingCharacter->GetController<APlayerController>();
	if (!PC || !PC->IsLocalController()) return;

	// --- LOGIC MOVED FROM PLAYER ---
	if (bIsKeyTaken)
	{
		// Show the completed message
		if (InteractionProgressBarWidgetClass)
		{
			UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, InteractionProgressBarWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				Widget->ShowCompletedMessage();

				InteractingCharacter->SetActiveProgressBar(Widget);
				// We'd need a way to manage this temporary widget, maybe on the controller.
			}
		}
	}
	else
	{
		// Show the progress bar
		if (InteractionProgressBarWidgetClass)
		{
			UInteractionProgressBarWidget* Widget = CreateWidget<UInteractionProgressBarWidget>(PC, InteractionProgressBarWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				Widget->StartProgress(InteractionDuration);
				// The player controller should store this widget so it can be removed later.
				InteractingCharacter->SetActiveProgressBar(Widget);
			}
		}

		// --- TRIGGER THE SERVER RPC HOP ---
		// After showing the UI, the shrine tells the player character to notify the server.
		InteractingCharacter->Server_RequestStartInteract(this);
	}
}

bool AKingsShrine::IsCompleted()
{
	return bIsKeyTaken;
}


//// This function ONLY executes on the SERVER.
//void AKingsShrine::Server_StartInteraction_Implementation(APawn* InstigatorPawn)
//{
//
//}

// This function ONLY executes on the SERVER.
//void AKingsShrine::Server_StopInteraction_Implementation()
//{
//
//}

void AKingsShrine::StartInteraction(APawn* InstigatorPawn)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AKingsShrine"));
	if (bIsKeyTaken || InteractionTimerHandle.IsValid() || !InstigatorPawn)
	{
		return;
	}

	InteractingPlayer = InstigatorPawn;

	AMultiplayerActionCharacter* InteractingCharacter = Cast<AMultiplayerActionCharacter>(InteractingPlayer);
	if (InteractingCharacter)
	{
		// The shrine passes its own unique interaction montage
		InteractingCharacter->PlayInteractionMontage(ShrineChannelingMontage); // Assuming you add this UPROPERTY
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
			// Tell the character to stop its replicated animation
			InteractingCharacter->StopInteractionMontage();
		}

		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
		InteractingPlayer = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Server: Shrine interaction cancelled."));
	}
}

// This function ONLY executes on the SERVER.
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
		// Tell the group manager to assign the interacting player as the new leader.
		GroupManager->SetGroupLeader(InteractingCharacter);
	}

	ADefaultGameState* GameState = GetWorld()->GetGameState<ADefaultGameState>();
	if (GameState)
	{
		UE_LOG(LogTemp, Log, TEXT("Shrine reporting to GameState: King's Key has been acquired."));
		GameState->SetHasKingsKey(true);
	}

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