#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Weapon.h"
#include "ChestInterface.h"
#include "OutpostInteractable.h"
#include "ChestWidget.h"
#include "Chest.generated.h"

class UInteractionProgressBarWidget;

UCLASS()
class MULTIPLAYERACTION_API AChest : public AActor, public IChestInterface, public IOutpostInteractable
{
	GENERATED_BODY()
	
public:	
	AChest();

	virtual FWeaponData* GetChestContents() override;
	virtual FText GetChestName() const override;
	virtual void Swap() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeapon> Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Chest")
	FText ChestName;

protected:
	UPROPERTY(Category = Chest, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Chest")
	UAnimMontage* OpenCloseAnim;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TObjectPtr<UAnimMontage> UnlockMontage; // The player plays this

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestOpenSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestCloseSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestLockedSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> ChestUnlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UChestWidget> ChestMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UInteractionProgressBarWidget> UnlockingWidgetClass;

	UPROPERTY()
	UChestWidget* ChestMenuWidget;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float UnlockDuration = 5.0f;

	FTimerHandle UnlockTimerHandle;

	UPROPERTY()
	TObjectPtr<APawn> InteractingPlayer;

	bool bOpen = false;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Unlocked)
	bool bIsUnlocked = false;

public:	
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;

	virtual void OnStopInteract_Implementation(APawn* InstigatorPawn) override;

	virtual void OnEndFocus_Implementation(APawn* InstigatorPawn) override;

	virtual void OnClientStartInteract_Implementation(class AMultiplayerActionCharacter* InteractingCharacter) override;

	UFUNCTION()
	void OpenChest(APawn* InstigatorPawn);

	UFUNCTION()
	void CloseChest(APawn* InstigatorPawn);

	virtual void Tick(float DeltaTime) override;

	virtual bool IsCompleted() override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_BeginUnlock(APawn* InstigatorPawn);

	UFUNCTION(Server, Reliable)
	void Server_CancelUnlock();

	UFUNCTION()
	void OnUnlockComplete();

	UFUNCTION()
	void OnRep_Unlocked();
};
