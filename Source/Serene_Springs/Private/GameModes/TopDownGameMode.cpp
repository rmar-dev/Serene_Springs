// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/TopDownGameMode.h"
#include "TopDownCameraController.h"

ATopDownGameMode::ATopDownGameMode()
{
	// Set the default pawn to our TopDownCameraController
	static ConstructorHelpers::FClassFinder<APawn> CameraControllerBPClass(TEXT("/Game/Blueprints/BP_TopDownCameraController"));
	if (CameraControllerBPClass.Class != NULL)
	{
		DefaultPawnClass = CameraControllerBPClass.Class;
	}
	else
	{
		DefaultPawnClass = ATopDownCameraController::StaticClass();
	}
}