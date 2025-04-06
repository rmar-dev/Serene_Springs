// BuildingController.cpp - Implementation of building placement controller
#include "BuildingController.h"
#include "BuildingGridManager.h"
#include "BuildingObjectAsset.h"
#include "BuildingObject.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ABuildingController::ABuildingController()
{
    // Set this controller to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Initialize member variables
    GridManager = nullptr;
    SelectedBuildingAsset = nullptr;
    bBuildingModeActive = false;
    CurrentRotation = 0;
    CurrentFloorLevel = 0;
    BuildingUIWidget = nullptr;
}

void ABuildingController::BeginPlay()
{
    Super::BeginPlay();

    // Find the grid manager in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingGridManager::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        GridManager = Cast<ABuildingGridManager>(FoundActors[0]);
    }
    
    // If we didn't find a grid manager, create one
    if (!GridManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        GridManager = GetWorld()->SpawnActor<ABuildingGridManager>(ABuildingGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
    
    // Create and add the building UI widget if specified
    if (BuildingUIWidgetClass)
    {
        BuildingUIWidget = CreateWidget<UUserWidget>(this, BuildingUIWidgetClass);
        if (BuildingUIWidget)
        {
            BuildingUIWidget->AddToViewport();
        }
    }
}

void ABuildingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only update placement preview if in building mode
    if (bBuildingModeActive && GridManager && SelectedBuildingAsset)
    {
        // Get hit location under cursor
        FHitResult HitResult;
        if (GetHitUnderCursor(HitResult))
        {
            // Update placement preview
            GridManager->UpdatePlacementPreview(SelectedBuildingAsset, HitResult.Location, CurrentRotation, CurrentFloorLevel);
        }
    }
}

void ABuildingController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind input actions for building placement
    InputComponent->BindAction("BuildingRotate", IE_Pressed, this, &ABuildingController::RotateBuilding);
    InputComponent->BindAction("BuildingConfirm", IE_Pressed, this, &ABuildingController::ConfirmPlacement);
    InputComponent->BindAction("BuildingCancel", IE_Pressed, this, &ABuildingController::CancelPlacement);
    InputComponent->BindAction("FloorLevelUp", IE_Pressed, this, &ABuildingController::IncrementFloorLevel);
    InputComponent->BindAction("FloorLevelDown", IE_Pressed, this, &ABuildingController::DecrementFloorLevel);
}

bool ABuildingController::GetHitUnderCursor(FHitResult& OutHit)
{
    // Get mouse screen position
    float MouseX;
    float MouseY;
    if (!GetMousePosition(MouseX, MouseY))
    {
        return false;
    }
    
    // Convert mouse position to world ray
    FVector WorldLocation;
    FVector WorldDirection;
    if (!DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
    {
        return false;
    }
    
    // Trace against the world
    const float TraceDistance = 100000.0f;
    FVector TraceEnd = WorldLocation + (WorldDirection * TraceDistance);
    
    // Setup trace parameters
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(GetPawn());
    
    // Perform the trace
    return GetWorld()->LineTraceSingleByChannel(OutHit, WorldLocation, TraceEnd, ECC_Visibility, TraceParams);
}

void ABuildingController::RotateBuilding()
{
    // Only rotate if in building mode
    if (bBuildingModeActive)
    {
        // Increment rotation and wrap around
        CurrentRotation = (CurrentRotation + 1) % 4;
    }
}

void ABuildingController::ConfirmPlacement()
{
    // Only confirm if in building mode
    if (bBuildingModeActive && GridManager && SelectedBuildingAsset)
    {
        // Get hit location under cursor
        FHitResult HitResult;
        if (GetHitUnderCursor(HitResult))
        {
            // Try to place the building
            ABuildingObject* PlacedBuilding = GridManager->PlaceBuilding(SelectedBuildingAsset, HitResult.Location, CurrentRotation, CurrentFloorLevel);
            
            // If building was placed successfully
            if (PlacedBuilding)
            {
                // You can add any post-placement logic here
                // For example, deduct resources, play sound, etc.
                
                // For now, just exit building mode
                ExitBuildingMode();
            }
            else
            {
                // Display placement error message
                // This would typically be handled by your UI system
                UE_LOG(LogTemp, Warning, TEXT("Cannot place building at this location"));
            }
        }
    }
}

void ABuildingController::CancelPlacement()
{
    // Exit building mode
    ExitBuildingMode();
}

void ABuildingController::IncrementFloorLevel()
{
    // Check if grid manager exists
    if (GridManager)
    {
        // Increment floor level within valid range
        CurrentFloorLevel = FMath::Min(CurrentFloorLevel + 1, GridManager->GetMaxFloors() - 1);
        
        // Update grid manager's active floor
        GridManager->SetActiveFloorLevel(CurrentFloorLevel);
    }
}

void ABuildingController::DecrementFloorLevel()
{
    // Check if grid manager exists
    if (GridManager && CurrentFloorLevel > 0)
    {
        // Decrement floor level (don't go below 0)
        CurrentFloorLevel = FMath::Max(0, CurrentFloorLevel - 1);
        
        // Update grid manager's active floor
        GridManager->SetActiveFloorLevel(CurrentFloorLevel);
    }
}

void ABuildingController::EnterBuildingMode(UBuildingObjectAsset* BuildingAsset)
{
    // Check if we have a valid asset and grid manager
    if (!BuildingAsset || !GridManager)
    {
        return;
    }
    
    // Set building mode active
    bBuildingModeActive = true;
    
    // Store the selected asset
    SelectedBuildingAsset = BuildingAsset;
    
    // Reset rotation
    CurrentRotation = 0;
    
    // Enable grid visualization
    GridManager->SetGridVisualizationEnabled(true);
}

void ABuildingController::ExitBuildingMode()
{
    // Clear building mode state
    bBuildingModeActive = false;
    SelectedBuildingAsset = nullptr;
    
    // Reset placement preview
    if (GridManager)
    {
        GridManager->ResetCellVisualStates();
        GridManager->SetGridVisualizationEnabled(false);
    }
}