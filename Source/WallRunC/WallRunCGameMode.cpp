// Copyright Epic Games, Inc. All Rights Reserved.

#include "WallRunCGameMode.h"
#include "WallRunCHUD.h"
#include "WallRunCCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWallRunCGameMode::AWallRunCGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWallRunCHUD::StaticClass();
}
