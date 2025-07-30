// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KingsShrine.generated.h"

class UWidgetComponent;
class UInteractionProgressBarWidget;

UCLASS()
class MULTIPLAYERACTION_API AKingsShrine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKingsShrine();

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

	/** [CLIENT] Called by the player to request starting the interaction. */
	UFUNCTION(Server, Reliable)
	void Server_StartInteraction(APawn* InstigatorPawn);

	/** [CLIENT] Called by the player when they stop interacting (e.g., release key or move away). */
	UFUNCTION(Server, Reliable)
	void Server_StopInteraction();

	/** [SERVER] Called by the timer when the interaction is successfully completed. */
	UFUNCTION()
	void OnInteractionComplete();

public:
	// Public functions that the player character will call.
	void StartInteraction(APawn* InstigatorPawn);
	void StopInteraction();

};
