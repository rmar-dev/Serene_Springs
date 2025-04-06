// BuildingGridManager.h - Manages the building grid system
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGridTypes.h"
#include "BuildingGridManager.generated.h"

class UBuildingObjectAsset;
class ABuildingObject;

/**
 * Manages the grid-based building system including cell occupancy, validation, and visualization
 */
UCLASS(BlueprintType, Blueprintable)
class GRID_API ABuildingGridManager : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABuildingGridManager();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // The number of cells in X dimension
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Setup")
    int32 GridSizeX = 50;

    // The number of cells in Y dimension
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Setup")
    int32 GridSizeY = 50;

    // The size of each grid cell in world units
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Setup")
    float CellSize = 100.0f;

    // Maximum number of floors supported
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Setup")
    int32 MaxFloors = 5;

    // The Z height of each floor in world units
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Setup")
    float FloorHeight = 400.0f;

    // Static mesh for grid cell visualization
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UStaticMesh* GridCellMesh;

    // Material for normal cells
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UMaterialInterface* NormalCellMaterial;

    // Material for valid placement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UMaterialInterface* ValidPlacementMaterial;

    // Material for invalid placement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UMaterialInterface* InvalidPlacementMaterial;

    // Material for selected cells
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UMaterialInterface* SelectedCellMaterial;

    // Material for highlighted cells
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visualization")
    UMaterialInterface* HighlightedCellMaterial;

    // Whether grid visualization is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    bool bGridVisualizationEnabled = false;

    // Instanced static mesh component for grid visualization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visualization")
    UInstancedStaticMeshComponent* GridMeshComponent;

    // The grid data - organized as floors containing rows of cells
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
    TArray<FGridFloor> GridData;

    // Currently active floor level for interaction
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 ActiveFloorLevel = 0;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /**
     * Initialize the grid with the specified dimensions
     * @param SizeX Number of cells in X direction
     * @param SizeY Number of cells in Y direction
     * @param CellSizeValue Size of each cell in world units
     * @param MaxFloorsValue Maximum number of floors supported
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void InitializeGrid(int32 SizeX = 50, int32 SizeY = 50, float CellSizeValue = 100.0f, int32 MaxFloorsValue = 5);

    /**
     * Get the cell data at the specified grid position and floor
     * @param GridPosition Grid position (not world position)
     * @param FloorLevel Floor level
     * @return Cell data or nullptr if out of bounds
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FGridCellData GetCellData(const FIntPoint& GridPosition, int32 FloorLevel = 0) const;

    /**
     * Convert world position to grid position
     * @param WorldPosition Position in world space
     * @param FloorLevel Optional output parameter for detected floor level
     * @return Grid position
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FIntPoint WorldToGrid(const FVector& WorldPosition, int32& OutFloorLevel) const;

    /**
     * Convert grid position to world position
     * @param GridPosition Grid coordinates
     * @param FloorLevel Floor level
     * @return World position (center of cell)
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GridToWorld(const FIntPoint& GridPosition, int32 FloorLevel = 0) const;

    /**
     * Check if grid position is within bounds
     * @param GridPosition Grid coordinates to check
     * @param FloorLevel Floor level
     * @return True if position is within bounds
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsValidGridPosition(const FIntPoint& GridPosition, int32 FloorLevel = 0) const;

    /**
     * Check if a building can be placed at the specified position
     * @param BuildingAsset Building asset to place
     * @param WorldLocation World location for placement
     * @param Rotation Rotation in quarters (0-3, representing 0, 90, 180, 270 degrees)
     * @param FloorLevel Floor level for placement
     * @return True if placement is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool CanPlaceBuilding(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation = 0, int32 FloorLevel = 0);

    /**
     * Place a building on the grid
     * @param BuildingAsset Building asset to place
     * @param WorldLocation World location for placement
     * @param Rotation Rotation in quarters (0-3, representing 0, 90, 180, 270 degrees)
     * @param FloorLevel Floor level for placement
     * @return Spawned building actor or nullptr if placement failed
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    ABuildingObject* PlaceBuilding(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation = 0, int32 FloorLevel = 0);

    /**
     * Remove a building from the grid
     * @param GridPosition Position of any cell occupied by the building
     * @param FloorLevel Floor level
     * @return True if building was removed
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool RemoveBuilding(const FIntPoint& GridPosition, int32 FloorLevel = 0);

    /**
     * Set whether grid visualization is enabled
     * @param bEnabled Whether visualization should be enabled
     */
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void SetGridVisualizationEnabled(bool bEnabled);

    /**
     * Update the visual state of cells for placement preview
     * @param BuildingAsset Building asset to preview
     * @param WorldLocation World location for preview
     * @param Rotation Rotation in quarters (0-3, representing 0, 90, 180, 270 degrees)
     * @param FloorLevel Floor level for preview
     */
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void UpdatePlacementPreview(UBuildingObjectAsset* BuildingAsset, const FVector& WorldLocation, int32 Rotation = 0, int32 FloorLevel = 0);

    /**
     * Reset all cell visual states to normal
     */
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ResetCellVisualStates();

    /**
     * Set the active floor level for interaction and visualization
     * @param FloorLevel Floor level to set active
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetActiveFloorLevel(int32 FloorLevel);

    /**
     * Get the maximum number of floors
     * @return Maximum number of floors
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    int32 GetMaxFloors() const { return MaxFloors; }

private:
    // Update the visual representation of a specific cell
    void UpdateCellVisual(const FIntPoint& GridPosition, int32 FloorLevel);

    // Update all grid cell visuals
    void UpdateAllCellVisuals();

    // Set cell data at specified position
    void SetCellData(const FIntPoint& GridPosition, int32 FloorLevel, const FGridCellData& CellData);

    // Checks if all cells a building would occupy are available
    bool AreCellsAvailableForBuilding(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel);

    // Marks cells as occupied by a building
    void MarkCellsAsOccupied(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel, AActor* Building);

    // Marks cells as unoccupied
    void MarkCellsAsUnoccupied(const FBuildingFootprint& Footprint, const FIntPoint& GridOrigin, int32 Rotation, int32 FloorLevel);

    // Check if placement meets adjacency requirements
    bool CheckAdjacencyRequirements(const TArray<FAdjacencyRequirement>& Requirements, const FIntPoint& GridOrigin, int32 FloorLevel);
};