// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGridManager.h"


// Sets default values
ABuildingGridManager::ABuildingGridManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABuildingGridManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildingGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

