// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DefaultGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERACTION_API ADefaultGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	ADefaultGameState();

	// We'll use an OnRep function so other actors can react when the key is acquired.
	UPROPERTY(ReplicatedUsing = OnRep_KingsKeyAcquired)
	bool bHasKingsKey = false;

	UFUNCTION()
	void OnRep_KingsKeyAcquired();

	// A public server-only function to set the state
	void SetHasKingsKey(bool bNewState);

	// Delegate that other actors (like the Chest) can subscribe to.
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKingsKeyAcquired);
	UPROPERTY(BlueprintAssignable)
	FOnKingsKeyAcquired OnKingsKeyAcquiredDelegate;
};
