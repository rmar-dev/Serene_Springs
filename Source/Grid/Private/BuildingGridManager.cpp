// BuildingGridManager.cpp - Implementation of grid system manager
#include "BuildingGridManager.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "BuildingObjectAsset.h"
#include "BuildingObject.h"

// Sets default values
ABuildingGridManager::ABuildingGridManager()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create a root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create the instanced static mesh component for grid visualization
    GridMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridMeshComponent"));
    GridMeshComponent->SetupAttachment(RootComponent);
    GridMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GridMeshComponent->SetCastShadow(false);
}

// Called when the game starts or when spawned
void ABuildingGridManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the grid with default parameters
    InitializeGrid(GridSizeX, GridSizeY, CellSize, MaxFloors);
}

// Called every frame
void ABuildingGridManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABuildingGridManager::InitializeGrid(int32 SizeX, int32 SizeY, float CellSizeValue, int32 MaxFloorsValue)
{
    // Store grid properties
    GridSizeX = SizeX;
    GridSizeY = SizeY;
    CellSize = CellSizeValue;
    MaxFloors = MaxFloorsValue;
    
    // Initialize the grid data structure
    GridData.SetNum(MaxFloors);
    
    // For each floor
    for (int32 Floor = 0; Floor < MaxFloors; Floor++)
    {
        FGridFloor& CurrentFloor = GridData[Floor];
        CurrentFloor.SetNumRows(GridSizeY);
        
        // For each row
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            FGridRow& CurrentRow = CurrentFloor.GetRow(Y);
            CurrentRow.SetNumCells(GridSizeX);
            
            // For each column
            for (int32 X = 0; X < GridSizeX; X++)
            {
                // Initialize cell data
                FGridCellData& Cell = CurrentRow.GetCell(X);
                Cell.GridPosition = FIntPoint(X, Y);
                Cell.FloorLevel = Floor;
                Cell.bIsOccupied = false;
                Cell.bIsWalkable = true;
                Cell.ObjectOrigin = FIntPoint(X, Y);
                Cell.OccupyingObject = nullptr;
                Cell.PathCost = 1.0f;
                Cell.VisualState = EGridCellVisualState::Normal;
                
                // Ground floor automatically has utility connections for now
                if (Floor == 0)
                {
                    Cell.bHasWaterConnection = true;
                    Cell.bHasElectricalConnection = true;
                }
                else
                {
                    Cell.bHasWaterConnection = false;
                    Cell.bHasElectricalConnection = false;
                }
            }
        }
    }
    
    // Update the visual representation
    UpdateAllCellVisuals();
}

FGridCellData ABuildingGridManager::GetCellData(const FIntPoint& GridPosition, int32 FloorLevel) const
{
    // Check if position is valid
    if (IsValidGridPosition(GridPosition, FloorLevel))
    {
        return GridData[FloorLevel].GetRow(GridPosition.Y).GetCell(GridPosition.X);
    }
    
    // Return an empty cell if position is invalid
    return FGridCellData();
}

FIntPoint ABuildingGridManager::WorldToGrid(const FVector& WorldPosition, int32& OutFloorLevel) const
{
    // Calculate floor level first
    OutFloorLevel = FMath::Clamp(FMath::FloorToInt(WorldPosition.Z / FloorHeight), 0, MaxFloors - 1);
    
    // Convert world XY to grid XY
    // Add half cell size to ensure proper rounding (center of cell)
    float GridX = (WorldPosition.X - GetActorLocation().X + (CellSize * 0.5f)) / CellSize;
    float GridY = (WorldPosition.Y - GetActorLocation().Y + (CellSize * 0.5f)) / CellSize;
    
    // Convert to integers and clamp to grid bounds
    int32 X = FMath::Clamp(FMath::FloorToInt(GridX), 0, GridSizeX - 1);
    int32 Y = FMath::Clamp(FMath::FloorToInt(GridY), 0, GridSizeY - 1);
    
    return FIntPoint(X, Y);
}

FVector ABuildingGridManager::GridToWorld(const FIntPoint& GridPosition, int32 FloorLevel) const
{
    // Convert grid coordinates to world position
    float WorldX = GetActorLocation().X + (GridPosition.X * CellSize);
    float WorldY = GetActorLocation().Y + (GridPosition.Y * CellSize);
    float WorldZ = GetActorLocation().Z + (FloorLevel * FloorHeight);
    
    // Return the center of the cell
    return FVector(WorldX + (CellSize * 0.5f), WorldY + (CellSize * 0.5f), WorldZ);
}

bool ABuildingGridManager::IsValidGridPosition(const FIntPoint& GridPosition, int32 FloorLevel) const
{
    // Check if the position is within the grid bounds
    return FloorLevel >= 0 && 
           FloorLevel < MaxFloors && 
           GridPosition.X >= 0 && 
           GridPosition.X < GridSizeX && 
           GridPosition.Y >= 0 && 
           GridPosition.Y < GridSizeY;
}

bool ABuildingGridManager::CanPlaceBuilding(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation, int32 FloorLevel)
{
    // Validate input parameters
    if (!BuildingAsset)
    {
        return false;
    }
    
    // Convert world location to grid position
    int32 DetectedFloor;
    FIntPoint GridOrigin = WorldToGrid(WorldLocation, DetectedFloor);
    
    // Use detected floor if not specified
    if (FloorLevel < 0)
    {
        FloorLevel = DetectedFloor;
    }
    
    // Check if the building footprint is available
    const FBuildingFootprint& Footprint = BuildingAsset->GetFootprint();
    if (!AreCellsAvailableForBuilding(Footprint, GridOrigin, Rotation, FloorLevel))
    {
        return false;
    }
    
    // Check adjacency requirements
    if (!CheckAdjacencyRequirements(BuildingAsset->GetAdjacencyRequirements(), GridOrigin, FloorLevel))
    {
        return false;
    }
    
    // All checks passed
    return true;
}

ABuildingObject* ABuildingGridManager::PlaceBuilding(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation, int32 FloorLevel)
{
    // First check if placement is possible
    if (!CanPlaceBuilding(BuildingAsset, WorldLocation, Rotation, FloorLevel))
    {
        return nullptr;
    }
    
    // Convert world location to grid position
    int32 DetectedFloor;
    FIntPoint GridOrigin = WorldToGrid(WorldLocation, DetectedFloor);
    
    // Use detected floor if not specified
    if (FloorLevel < 0)
    {
        FloorLevel = DetectedFloor;
    }
    
    // Calculate world position for origin cell (center of cell)
    FVector Origin = GridToWorld(GridOrigin, FloorLevel);
    
    // Spawn the building object
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ABuildingObject* Building = GetWorld()->SpawnActor<ABuildingObject>(
        BuildingAsset->GetBuildingClass(), 
        Origin, 
        FRotator(0.0f, Rotation * 90.0f, 0.0f), 
        SpawnParams
    );
    
    if (Building)
    {
        // Initialize the building with its asset
        Building->InitializeFromAsset(BuildingAsset);
        
        // Set the building's grid properties
        Building->SetGridProperties(GridOrigin, FloorLevel, Rotation);
        
        // Mark cells as occupied
        MarkCellsAsOccupied(BuildingAsset->GetFootprint(), GridOrigin, Rotation, FloorLevel, Building);
        
        // Update visuals
        UpdateAllCellVisuals();
    }
    
    return Building;
}

bool ABuildingGridManager::RemoveBuilding(const FIntPoint& GridPosition, int32 FloorLevel)
{
    // Check if position is valid
    if (!IsValidGridPosition(GridPosition, FloorLevel))
    {
        return false;
    }
    
    // Get the cell data
    const FGridCellData& Cell = GridData[FloorLevel].GetRow(GridPosition.Y).GetCell(GridPosition.X);
    
    // Check if cell is occupied
    if (!Cell.bIsOccupied || !Cell.OccupyingObject)
    {
        return false;
    }
    
    // Get the building object
    ABuildingObject* Building = Cast<ABuildingObject>(Cell.OccupyingObject);
    if (!Building)
    {
        return false;
    }
    
    // Get the building's grid properties
    FIntPoint BuildingOrigin;
    int32 BuildingFloor;
    int32 BuildingRotation;
    Building->GetGridProperties(BuildingOrigin, BuildingFloor, BuildingRotation);
    
    // Get the building's footprint
    FBuildingFootprint Footprint = Building->GetFootprint();
    
    // Mark cells as unoccupied
    MarkCellsAsUnoccupied(Footprint, BuildingOrigin, BuildingRotation, BuildingFloor);
    
    // Destroy the building actor
    Building->Destroy();
    
    // Update visuals
    UpdateAllCellVisuals();
    
    return true;
}

void ABuildingGridManager::SetGridVisualizationEnabled(bool bEnabled)
{
    bGridVisualizationEnabled = bEnabled;
    
    // Show or hide the grid mesh
    GridMeshComponent->SetVisibility(bEnabled);
    
    // Update all cell visuals if enabling
    if (bEnabled)
    {
        UpdateAllCellVisuals();
    }
}

void ABuildingGridManager::UpdatePlacementPreview(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation, int32 FloorLevel)
{
    // Reset previous visualization
    ResetCellVisualStates();
    
    // Check if we have a valid asset
    if (!BuildingAsset)
    {
        return;
    }
    
    // Convert world location to grid position
    int32 DetectedFloor;
    FIntPoint GridOrigin = WorldToGrid(WorldLocation, DetectedFloor);
    
    // Use detected floor if not specified
    if (FloorLevel < 0)
    {
        FloorLevel = DetectedFloor;
    }
    
    // Get the building's footprint
    const FBuildingFootprint& Footprint = BuildingAsset->GetFootprint();
    
    // Get all occupied cells
    TArray<FIntPoint> OccupiedCells = Footprint.GetOccupiedCellPositions(GridOrigin, Rotation);
    
    // Check if placement is valid
    bool bIsValidPlacement = CanPlaceBuilding(BuildingAsset, WorldLocation, Rotation, FloorLevel);
    
    // Update visual state for each cell
    for (const FIntPoint& Cell : OccupiedCells)
    {
        if (IsValidGridPosition(Cell, FloorLevel))
        {
            // Set visual state based on placement validity
            EGridCellVisualState VisualState = bIsValidPlacement ? 
                EGridCellVisualState::Valid : 
                EGridCellVisualState::Invalid;
            
            // Update cell data
            FGridCellData& CellData = GridData[FloorLevel].GetRow(Cell.Y).GetCell(Cell.X);
            CellData.VisualState = VisualState;
            
            // Update the visual
            UpdateCellVisual(Cell, FloorLevel);
        }
    }
}

void ABuildingGridManager::ResetCellVisualStates()
{
    // For each floor
    for (int32 Floor = 0; Floor < MaxFloors; Floor++)
    {
        // For each row
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            // For each column
            for (int32 X = 0; X < GridSizeX; X++)
            {
                // Reset visual state to normal
                FGridCellData& Cell = GridData[Floor].GetRow(Y).GetCell(X);
                Cell.VisualState = EGridCellVisualState::Normal;
                
                // Update the visual
                UpdateCellVisual(FIntPoint(X, Y), Floor);
            }
        }
    }
}

void ABuildingGridManager::SetActiveFloorLevel(int32 FloorLevel)
{
    // Clamp to valid range
    ActiveFloorLevel = FMath::Clamp(FloorLevel, 0, MaxFloors - 1);
    
    // Update visuals to show only the active floor
    UpdateAllCellVisuals();
}

void ABuildingGridManager::UpdateCellVisual(const FIntPoint& GridPosition, int32 FloorLevel)
{
    // Only update if visualization is enabled
    if (!bGridVisualizationEnabled || !GridMeshComponent)
    {
        return;
    }
    
    // Only visualize cells on the active floor
    if (FloorLevel != ActiveFloorLevel)
    {
        return;
    }
    
    // Calculate instance index for this cell
    int32 InstanceIndex = GridPosition.Y * GridSizeX + GridPosition.X;
    
    // Check if instance already exists
    if (GridMeshComponent->GetInstanceCount() <= InstanceIndex)
    {
        // Add new instance
        FTransform CellTransform;
        CellTransform.SetLocation(GridToWorld(GridPosition, FloorLevel) - FVector(0, 0, CellSize * 0.5f)); // Place at bottom of cell
        CellTransform.SetScale3D(FVector(CellSize / 100.0f)); // Assuming mesh is 100x100 units
        
        GridMeshComponent->AddInstance(CellTransform);
    }
    else
    {
        // Update existing instance
        FTransform CellTransform;
        CellTransform.SetLocation(GridToWorld(GridPosition, FloorLevel) - FVector(0, 0, CellSize * 0.5f));
        CellTransform.SetScale3D(FVector(CellSize / 100.0f));
        
        GridMeshComponent->UpdateInstanceTransform(InstanceIndex, CellTransform, true);
    }
    
    // Set material based on cell state
    UMaterialInterface* CellMaterial = NormalCellMaterial;
    
    const FGridCellData& CellData = GridData[FloorLevel].GetRow(GridPosition.Y).GetCell(GridPosition.X);
    switch (CellData.VisualState)
    {
        case EGridCellVisualState::Valid:
            CellMaterial = ValidPlacementMaterial;
            break;
        case EGridCellVisualState::Invalid:
            CellMaterial = InvalidPlacementMaterial;
            break;
        case EGridCellVisualState::Selected:
            CellMaterial = SelectedCellMaterial;
            break;
        case EGridCellVisualState::Highlighted:
            CellMaterial = HighlightedCellMaterial;
            break;
        default:
            CellMaterial = NormalCellMaterial;
            break;
    }
    
    // Apply material to the instance
    if (CellMaterial)
    {
        GridMeshComponent->SetMaterial(InstanceIndex, CellMaterial);
    }
}

void ABuildingGridManager::UpdateAllCellVisuals()
{
    // Only update if visualization is enabled
    if (!bGridVisualizationEnabled || !GridMeshComponent)
    {
        return;
    }
    
    // Clear existing instances
    GridMeshComponent->ClearInstances();
    
    // Only visualize cells on the active floor
    int32 Floor = ActiveFloorLevel;
    
    // For each row
    for (int32 Y = 0; Y < GridSizeY; Y++)
    {
        // For each column
        for (int32 X = 0; X < GridSizeX; X++)
        {
            // Update the visual for this cell
            UpdateCellVisual(FIntPoint(X, Y), Floor);
        }
    }
}

void ABuildingGridManager::SetCellData(const FIntPoint& GridPosition, int32 FloorLevel, const FGridCellData& CellData)
{
    // Check if position is valid
    if (IsValidGridPosition(GridPosition, FloorLevel))
    {
        GridData[FloorLevel].GetRow(GridPosition.Y).GetCell(GridPosition.X) = CellData;
        
        // Update the visual
        UpdateCellVisual(GridPosition, FloorLevel);
    }
}

bool ABuildingGridManager::AreCellsAvailableForBuilding(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel)
{
    // Get all cells the building would occupy
    TArray<FIntPoint> OccupiedCells = Footprint.GetOccupiedCellPositions(GridOrigin, Rotation);
    
    // Check each cell
    for (const FIntPoint& Cell : OccupiedCells)
    {
        // Check if cell is within grid bounds
        if (!IsValidGridPosition(Cell, FloorLevel))
        {
            return false;
        }
        
        // Check if cell is already occupied
        const FGridCellData& CellData = GridData[FloorLevel].GetRow(Cell.Y).GetCell(Cell.X);
        if (CellData.bIsOccupied)
        {
            return false;
        }
        
        // For upper floors, check for structural support
        if (FloorLevel > 0)
        {
            // Check if there's a building below
            const FGridCellData& CellBelow = GridData[FloorLevel-1].GetRow(Cell.Y).GetCell(Cell.X);
            if (!CellBelow.bIsOccupied)
            {
                return false;
            }
        }
    }
    
    // All cells are available
    return true;
}

void ABuildingGridManager::MarkCellsAsOccupied(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel, AActor* Building)
{
    // Get all cells the building occupies
    TArray<FIntPoint> OccupiedCells = Footprint.GetOccupiedCellPositions(GridOrigin, Rotation);
    
    // Mark each cell
    for (const FIntPoint& Cell : OccupiedCells)
    {
        if (IsValidGridPosition(Cell, FloorLevel))
        {
            FGridCellData& CellData = GridData[FloorLevel].GetRow(Cell.Y).GetCell(Cell.X);
            CellData.bIsOccupied = true;
            CellData.OccupyingObject = Building;
            CellData.ObjectOrigin = GridOrigin;
            
            // Update visual
            UpdateCellVisual(Cell, FloorLevel);
        }
    }
}

void ABuildingGridManager::MarkCellsAsUnoccupied(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel)
{
    // Get all cells the building occupies
    TArray<FIntPoint> OccupiedCells = Footprint.GetOccupiedCellPositions(GridOrigin, Rotation);
    
    // Mark each cell
    for (const FIntPoint& Cell : OccupiedCells)
    {
        if (IsValidGridPosition(Cell, FloorLevel))
        {
            FGridCellData& CellData = GridData[FloorLevel].GetRow(Cell.Y).GetCell(Cell.X);
            CellData.bIsOccupied = false;
            CellData.OccupyingObject = nullptr;
            CellData.ObjectOrigin = Cell; // Reset to own position
            
            // Update visual
            UpdateCellVisual(Cell, FloorLevel);
        }
    }
}

bool ABuildingGridManager::CheckAdjacencyRequirements(const TArray<FAdjacencyRequirement>& Requirements, const FIntPoint& GridOrigin, int32 FloorLevel)
{
    // If no requirements, always valid
    if (Requirements.Num() == 0)
    {
        return true;
    }
    
    // Check each requirement
    for (const FAdjacencyRequirement& Requirement : Requirements)
    {
        bool bFoundMatch = false;
        
        // Skip if this is a "None" requirement
        if (Requirement.RequiredBuildingType == EBuildingType::None)
        {
            continue;
        }
        
        // Get adjacent cells to check based on direction
        TArray<FIntPoint> CellsToCheck;
        
        if (Requirement.Direction == EGridDirection::North || Requirement.Direction == EGridDirection::Any)
        {
            CellsToCheck.Add(FIntPoint(GridOrigin.X, GridOrigin.Y - 1));
        }
        
        if (Requirement.Direction == EGridDirection::East || Requirement.Direction == EGridDirection::Any)
        {
            CellsToCheck.Add(FIntPoint(GridOrigin.X + 1, GridOrigin.Y));
        }
        
        if (Requirement.Direction == EGridDirection::South || Requirement.Direction == EGridDirection::Any)
        {
            CellsToCheck.Add(FIntPoint(GridOrigin.X, GridOrigin.Y + 1));
        }
        
        if (Requirement.Direction == EGridDirection::West || Requirement.Direction == EGridDirection::Any)
        {
            CellsToCheck.Add(FIntPoint(GridOrigin.X - 1, GridOrigin.Y));
        }
        
        // Check each cell
        for (const FIntPoint& Cell : CellsToCheck)
        {
            // Skip if out of bounds
            if (!IsValidGridPosition(Cell, FloorLevel))
            {
                continue;
            }
            
            // Get the cell data
            const FGridCellData& CellData = GridData[FloorLevel].GetRow(Cell.Y).GetCell(Cell.X);
            
            // Skip if not occupied
            if (!CellData.bIsOccupied || !CellData.OccupyingObject)
            {
                continue;
            }
            
            // Get the building type
            ABuildingObject* Building = Cast<ABuildingObject>(CellData.OccupyingObject);
            if (!Building)
            {
                continue;
            }
            
            EBuildingType BuildingType = Building->GetBuildingType();
            
            // Check if it matches the requirement
            if (BuildingType == Requirement.RequiredBuildingType)
            {
                bFoundMatch = true;
                break;
            }
        }
        
        // Check if this requirement is satisfied
        if (Requirement.bIsNegativeRequirement)
        {
            // For negative requirements, we must NOT find a match
            if (bFoundMatch)
            {
                return false;
            }
        }
        else
        {
            // For positive requirements, we must find a match
            if (!bFoundMatch)
            {
                return false;
            }
        }
    }
    
    // All requirements satisfied
    return true;
}