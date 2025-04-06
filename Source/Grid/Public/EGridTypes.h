// GridTypes.h - Type definitions for the grid system
#pragma once

#include "CoreMinimal.h"
#include "EGridTypes.generated.h"

// Forward declarations
class UBuildingObjectAsset;
class ABuildingObject;

/**
 * Visual state for grid cells during building mode
 */
UENUM(BlueprintType)
enum class EGridCellVisualState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Valid       UMETA(DisplayName = "Valid Placement"),
    Invalid     UMETA(DisplayName = "Invalid Placement"),
    Selected    UMETA(DisplayName = "Selected"),
    Highlighted UMETA(DisplayName = "Highlighted")
};

/**
 * Types of building objects that can be placed on the grid
 */
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    None                   UMETA(DisplayName = "None"),
    StandardRoom           UMETA(DisplayName = "Standard Room"),
    Suite                  UMETA(DisplayName = "Suite"),
    Villa                  UMETA(DisplayName = "Villa"),
    MassageRoom            UMETA(DisplayName = "Massage Room"),
    YogaStudio             UMETA(DisplayName = "Yoga Studio"),
    MeditationRoom         UMETA(DisplayName = "Meditation Room"),
    Restaurant             UMETA(DisplayName = "Restaurant"),
    JuiceBar               UMETA(DisplayName = "Juice Bar"),
    Garden                 UMETA(DisplayName = "Garden"),
    StaffRoom              UMETA(DisplayName = "Staff Room"),
    Office                 UMETA(DisplayName = "Office"),
    Utility                UMETA(DisplayName = "Utility")
};

/**
 * Direction enum for adjacency rules
 */
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North      UMETA(DisplayName = "North"),
    East       UMETA(DisplayName = "East"),
    South      UMETA(DisplayName = "South"),
    West       UMETA(DisplayName = "West"),
    Any        UMETA(DisplayName = "Any Direction")
};

/**
 * Data structure for a single grid cell
 */
USTRUCT(BlueprintType)
struct GRID_API FGridCellData
{
    GENERATED_BODY()

    // Cell coordinates in the grid (not world space)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    FIntPoint GridPosition;

    // Is this cell currently occupied by a building
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bIsOccupied = false;

    // Can characters walk through this cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bIsWalkable = true;

    // Which floor level this cell belongs to (0 = ground)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 FloorLevel = 0;
    
    // If this cell is part of a multi-cell object, where is the origin cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    FIntPoint ObjectOrigin;
    
    // Reference to the actor occupying this cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    AActor* OccupyingObject = nullptr;
    
    // Pathfinding cost to move through this cell
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float PathCost = 1.0f;
    
    // Visual state for building mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    EGridCellVisualState VisualState = EGridCellVisualState::Normal;
    
    // Utility connections
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bHasWaterConnection = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bHasElectricalConnection = false;

    // Constructor
    FGridCellData()
        : GridPosition(FIntPoint::ZeroValue)
        , ObjectOrigin(FIntPoint::ZeroValue)
    {}

    FGridCellData(FIntPoint InPosition, int32 InFloor = 0)
        : GridPosition(InPosition)
        , FloorLevel(InFloor)
        , ObjectOrigin(InPosition)
    {}
};

/**
 * Structure defining a building's size and shape on the grid
 */
USTRUCT(BlueprintType)
struct GRID_API FBuildingFootprint
{
    GENERATED_BODY()
    
    // Size in grid cells (x, y)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FIntPoint Size = FIntPoint(1, 1);
    
    // Optional array for non-rectangular shapes
    // If empty, a rectangular shape of Size is assumed
    // If populated, each point represents an occupied cell relative to origin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<FIntPoint> OccupiedCells;
    
    // Get all occupied cells based on an origin point and rotation
    TArray<FIntPoint> GetOccupiedCellPositions(const FIntPoint& Origin, int32 RotationQuarters = 0) const
    {
        TArray<FIntPoint> Result;
        
        // If custom shape is defined
        if (OccupiedCells.Num() > 0)
        {
            for (const FIntPoint& Cell : OccupiedCells)
            {
                FIntPoint RotatedCell = RotatePoint(Cell, RotationQuarters);
                Result.Add(FIntPoint(Origin.X + RotatedCell.X, Origin.Y + RotatedCell.Y));
            }
            return Result;
        }
        
        // Default rectangular shape
        FIntPoint RotatedSize = RotateSize(Size, RotationQuarters);
        
        for (int32 X = 0; X < RotatedSize.X; X++)
        {
            for (int32 Y = 0; Y < RotatedSize.Y; Y++)
            {
                FIntPoint LocalPoint(X, Y);
                FIntPoint RotatedPoint = RotatePoint(LocalPoint, RotationQuarters);
                Result.Add(FIntPoint(Origin.X + RotatedPoint.X, Origin.Y + RotatedPoint.Y));
            }
        }
        
        return Result;
    }
    
private:
    // Helper to rotate a point 90 degrees * Quarters
    FIntPoint RotatePoint(const FIntPoint& Point, int32 Quarters) const
    {
        // Ensure rotation is in range 0-3
        Quarters = Quarters % 4;
        
        switch (Quarters)
        {
            case 0: return Point;
            case 1: return FIntPoint(-Point.Y, Point.X);
            case 2: return FIntPoint(-Point.X, -Point.Y);
            case 3: return FIntPoint(Point.Y, -Point.X);
            default: return Point;
        }
    }
    
    // Helper to get rotated size
    FIntPoint RotateSize(const FIntPoint& InSize, int32 Quarters) const
    {
        // Ensure rotation is in range 0-3
        Quarters = Quarters % 4;
        
        // For 90 and 270 degrees, swap X and Y
        if (Quarters % 2 == 1)
        {
            return FIntPoint(InSize.Y, InSize.X);
        }
        
        return InSize;
    }
};

/**
 * Structure defining adjacency requirements for buildings
 */
USTRUCT(BlueprintType)
struct GRID_API FAdjacencyRequirement
{
    GENERATED_BODY()
    
    // The type of building required to be adjacent
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EBuildingType RequiredBuildingType = EBuildingType::None;
    
    // The direction where adjacency is required (or Any)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    EGridDirection Direction = EGridDirection::Any;
    
    // True if this is a negative requirement (must NOT be adjacent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIsNegativeRequirement = false;
    
    // Constructor
    FAdjacencyRequirement()
    {}
    
    FAdjacencyRequirement(EBuildingType InType, EGridDirection InDir = EGridDirection::Any, bool bNegative = false)
        : RequiredBuildingType(InType)
        , Direction(InDir)
        , bIsNegativeRequirement(bNegative)
    {}
};

/**
 * Row of grid cells
 */
USTRUCT(BlueprintType)
struct GRID_API FGridRow
{
    GENERATED_BODY()
    
    // Array of cells in this row
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TArray<FGridCellData> Cells;
    
    // Constructor
    FGridRow()
    {
    }
    
    // Get cell at specified index
    FGridCellData& GetCell(int32 Index)
    {
        check(Cells.IsValidIndex(Index));
        return Cells[Index];
    }
    
    // Get cell at specified index (const version)
    const FGridCellData& GetCell(int32 Index) const
    {
        check(Cells.IsValidIndex(Index));
        return Cells[Index];
    }
    
    // Set number of cells in this row
    void SetNumCells(int32 NumCells)
    {
        Cells.SetNum(NumCells);
    }
    
    // Get number of cells in this row
    int32 Num() const
    {
        return Cells.Num();
    }
};

/**
 * Floor of grid cells
 */
USTRUCT(BlueprintType)
struct GRID_API FGridFloor
{
    GENERATED_BODY()
    
    // Array of rows in this floor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TArray<FGridRow> Rows;
    
    // Constructor
    FGridFloor()
    {
    }
    
    // Get row at specified index
    FGridRow& GetRow(int32 Index)
    {
        check(Rows.IsValidIndex(Index));
        return Rows[Index];
    }
    
    // Get row at specified index (const version)
    const FGridRow& GetRow(int32 Index) const
    {
        check(Rows.IsValidIndex(Index));
        return Rows[Index];
    }
    
    // Set number of rows in this floor
    void SetNumRows(int32 NumRows)
    {
        Rows.SetNum(NumRows);
    }
    
    // Get number of rows in this floor
    int32 Num() const
    {
        return Rows.Num();
    }
};