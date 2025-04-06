// BuildingObjectAsset.cpp - Implementation of building asset definitions
#include "BuildingObjectAsset.h"

UBuildingObjectAsset::UBuildingObjectAsset()
{
	// Set default values
	BuildingName = FText::FromString("New Building");
	BuildingDescription = FText::FromString("A building that can be placed on the grid.");
	BuildingType = EBuildingType::None;
    
	// Default footprint (1x1)
	Footprint.Size = FIntPoint(1, 1);
    
	// Default icon and mesh are null (must be set in editor)
	BuildingIcon = nullptr;
	BuildingMesh = nullptr;
    
	// By default, we have no building class set
	BuildingClass = nullptr;
}