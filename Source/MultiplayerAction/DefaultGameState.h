#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DefaultGameState.generated.h"

UENUM(BlueprintType)
enum class ECustomMatchState : uint8
{
	InProgress	UMETA(DisplayName = "In Progress"),
	Victory		UMETA(DisplayName = "Victory"),
	Defeat		UMETA(DisplayName = "Defeat")
};

UCLASS()
class MULTIPLAYERACTION_API ADefaultGameState : public AGameState
{
	GENERATED_BODY()
public:
	ADefaultGameState();

	UPROPERTY(ReplicatedUsing = OnRep_KingsKeyAcquired)
	bool bHasKingsKey = false;

	UFUNCTION()
	void OnRep_KingsKeyAcquired();

	void SetHasKingsKey(bool bNewState);

	UPROPERTY(ReplicatedUsing = OnRep_StartedUnlockingChest)
	bool bStartedUnlockingChest = false;

	UFUNCTION()
	void OnRep_StartedUnlockingChest();

	void SetStartedUnlockingChest(bool bNewState);

	void SetCustomMatchState(ECustomMatchState NewState);

	UPROPERTY(ReplicatedUsing = OnRep_CustomMatchState)
	ECustomMatchState  CustomMatchState = ECustomMatchState::InProgress;

	UFUNCTION()
	void OnRep_CustomMatchState();

	// Delegate that other actors (like the Chest) can subscribe to.
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKingsKeyAcquired);
	UPROPERTY(BlueprintAssignable)
	FOnKingsKeyAcquired OnKingsKeyAcquiredDelegate;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartedUnlockingChest);
	UPROPERTY(BlueprintAssignable)
	FOnStartedUnlockingChest OnStartedUnlockingChestDelegate;
};
