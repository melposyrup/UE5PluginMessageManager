// Copyright Epic Games, Inc. All Rights Reserved.

#include "PluginEditorGameMode.h"
#include "PluginEditorCharacter.h"
#include "UObject/ConstructorHelpers.h"

APluginEditorGameMode::APluginEditorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
