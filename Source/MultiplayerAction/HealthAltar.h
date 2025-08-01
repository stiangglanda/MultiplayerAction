#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutpostInteractable.h"
#include "HealthAltar.generated.h"

class UInteractionProgressBarWidget;

UCLASS()
class MULTIPLAYERACTION_API AHealthAltar : public AActor, public IOutpostInteractable
{
	GENERATED_BODY()
	
public:	
	AHealthAltar();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> AltarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GobletMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float NewMaxHealth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDuration = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	TObjectPtr<UAnimMontage> InteractionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInteractionProgressBarWidget> InteractionProgressBarWidgetClass;

	FTimerHandle InteractionTimerHandle;

	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

public:	
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnStopInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnClientStartInteract_Implementation(class AMultiplayerActionCharacter* InteractingCharacter) override;
	virtual void OnEndFocus_Implementation(APawn* InstigatorPawn) override;

protected:
	void BeginInteraction(APawn* InstigatorPawn);
	void CancelInteraction();
	bool IsCompleted() override;

	UFUNCTION()
	void OnInteractionComplete();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Used)
	bool bWasUsed = false;

	UFUNCTION()
	void OnRep_Used();

};
