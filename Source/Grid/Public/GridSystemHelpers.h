// GridSystem.h - Module header for grid system components
#pragma once

// Include all grid system headers
#include "GridTypes.h"
#include "BuildingGridManager.h"
#include "BuildingObjectAsset.h"
#include "BuildingObject.h"

// Additional helper functions for working with the grid system
namespace GridSystemHelpers
{
    /**
     * Convert a rotation in degrees to quarters (0-3)
     * @param Degrees Rotation in degrees
     * @return Rotation in quarters (0=0°, 1=90°, 2=180°, 3=270°)
     */
    inline int32 DegreesToQuarters(float Degrees)
    {
        // Normalize to 0-360
        Degrees = FMath::Fmod(Degrees, 360.0f);
        if (Degrees < 0.0f)
        {
            Degrees += 360.0f;
        }
        
        // Convert to quarters (rounding to nearest)
        int32 Quarters = FMath::RoundToInt(Degrees / 90.0f) % 4;
        
        return Quarters;
    }
    
    /**
     * Convert quarters (0-3) to rotation in degrees
     * @param Quarters Rotation in quarters
     * @return Rotation in degrees
     */
    inline float QuartersToDegrees(int32 Quarters)
    {
        // Normalize to 0-3
        Quarters = Quarters % 4;
        if (Quarters < 0)
        {
            Quarters += 4;
        }
        
        // Convert to degrees
        return Quarters * 90.0f;
    }
    
    /**
     * Get the direction vector for a grid direction
     * @param Direction Grid direction enum
     * @return 2D vector representing the direction
     */
    inline FIntPoint GetDirectionVector(EGridDirection Direction)
    {
        switch (Direction)
        {
            case EGridDirection::North: return FIntPoint(0, -1);
            case EGridDirection::East: return FIntPoint(1, 0);
            case EGridDirection::South: return FIntPoint(0, 1);
            case EGridDirection::West: return FIntPoint(-1, 0);
            default: return FIntPoint(0, 0);
        }
    }
    
    /**
     * Get the opposite direction
     * @param Direction Grid direction enum
     * @return Opposite direction
     */
    inline EGridDirection GetOppositeDirection(EGridDirection Direction)
    {
        switch (Direction)
        {
            case EGridDirection::North: return EGridDirection::South;
            case EGridDirection::East: return EGridDirection::West;
            case EGridDirection::South: return EGridDirection::North;
            case EGridDirection::West: return EGridDirection::East;
            default: return Direction;
        }
    }
}