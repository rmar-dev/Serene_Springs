// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownCameraController.h"


// Sets default values
ATopDownCameraController::ATopDownCameraController()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATopDownCameraController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATopDownCameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATopDownCameraController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

