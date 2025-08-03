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
	AKingsShrine();

	UPROPERTY(EditInstanceOnly, Category = "AI Group")
	TObjectPtr<class AAIGroupManager> GroupManager;

	UFUNCTION(BlueprintPure, Category = "Shrine")
	TSubclassOf<UInteractionProgressBarWidget> GetProgressBarWidgetClass() const { return InteractionProgressBarWidgetClass; }

	UFUNCTION(BlueprintPure, Category = "Shrine")
	float GetInteractionDuration() const { return InteractionDuration; }

	UFUNCTION(BlueprintPure, Category = "Shrine")
	bool IsKeyAlreadyTaken() const { return bIsKeyTaken; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AltarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> InteractionPromptWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInteractionProgressBarWidget> InteractionProgressBarWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TObjectPtr<UAnimMontage> ShrineChannelingMontage; // The player plays this

	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

	FTimerHandle InteractionTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDuration = 20.0f;

	UPROPERTY(ReplicatedUsing = OnRep_KeyTaken)
	bool bIsKeyTaken = false;

	UFUNCTION()
	void OnRep_KeyTaken();

protected:
	UFUNCTION()
	void StartInteraction(APawn* InstigatorPawn);

	UFUNCTION()
	void StopInteraction();

	UFUNCTION()
	void OnInteractionComplete();

public:
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnStopInteract_Implementation(APawn* InstigatorPawn) override;
	virtual void OnBeginFocus_Implementation(APawn* InstigatorPawn) override;
	virtual void OnEndFocus_Implementation(APawn* InstigatorPawn) override;
	virtual FText GetInteractionText_Implementation() const override;
	virtual void OnClientStartInteract_Implementation(class AMultiplayerActionCharacter* InteractingCharacter) override;

	bool IsCompleted() override;
};
