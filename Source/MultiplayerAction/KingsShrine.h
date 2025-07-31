// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutpostInteractable.h"
#include "KingsShrine.generated.h"

class UWidgetComponent;
class UInteractionProgressBarWidget;

UCLASS()
class MULTIPLAYERACTION_API AKingsShrine : public AActor, public IOutpostInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKingsShrine();

	UPROPERTY(EditInstanceOnly, Category = "AI Group")
	TObjectPtr<class AAIGroupManager> GroupManager;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- COMPONENTS ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AltarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	// We still create the component, but we will configure it later.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> InteractionPromptWidget;

	// --- NEW: THE CLASS REFERENCE PROPERTY ---
	/** The Blueprint version of our progress bar widget to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInteractionProgressBarWidget> InteractionProgressBarWidgetClass;

	// --- INTERACTION LOGIC ---

	/** The player currently interacting with the shrine. Only one at a time. */
	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

	/** Timer handle for the "hold to activate" progress. */
	FTimerHandle InteractionTimerHandle;

	/** How long the player needs to hold the interaction key. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDuration = 30.0f;

	// --- STATE ---

	/** Replicated variable to track if the key has been taken. */
	UPROPERTY(ReplicatedUsing = OnRep_KeyTaken)
	bool bIsKeyTaken = false;

	/** This function is automatically called on all clients when bIsKeyTaken changes. */
	UFUNCTION()
	void OnRep_KeyTaken();

	// --- NETWORKING FUNCTIONS ---
protected:
	/** [CLIENT] Called by the player to request starting the interaction. */
	UFUNCTION()
	void StartInteraction(APawn* InstigatorPawn);

	/** [CLIENT] Called by the player when they stop interacting (e.g., release key or move away). */
	UFUNCTION()
	void StopInteraction();

	/** [SERVER] Called by the timer when the interaction is successfully completed. */
	UFUNCTION()
	void OnInteractionComplete();

	UFUNCTION(Client, Reliable)
	void Client_ShowInteractionUI(APlayerController* PlayerToTell);

	/** [CLIENT] Tells a specific client to hide the progress bar UI. */
	UFUNCTION(Client, Reliable)
	void Client_HideInteractionUI(APlayerController* PlayerToTell);

public:
	// --- INTERFACE IMPLEMENTATION ---
	// We add 'virtual' and 'override' for good C++ practice.
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnStopInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnBeginFocus_Implementation(APawn* InstigatorPawn) override;
	virtual void OnEndFocus_Implementation(APawn* InstigatorPawn) override;
	virtual FText GetInteractionText_Implementation() const override;


	// Inherited via IOutpostInteractable
	bool IsCompleted() override;

};
