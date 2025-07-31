// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerActionGameMode.h"
#include "MultiplayerActionCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "DefaultGameState.h"

AMultiplayerActionGameMode::AMultiplayerActionGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/dynamic/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ADefaultGameState::StaticClass();
}
