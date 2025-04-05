// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TopDownGameMode.generated.h"

/**
 * Game mode that uses the TopDownCameraController as default pawn
 */
UCLASS()
class SERENE_SPRINGS_API ATopDownGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATopDownGameMode();
};