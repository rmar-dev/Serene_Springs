// BuildingObjectAsset.h - Asset definition for placeable building objects
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EGridTypes.h"
#include "BuildingObjectAsset.generated.h"

class ABuildingObject;

/**
 * Data asset that defines properties for a building object that can be placed on the grid
 */
UCLASS(BlueprintType)
class GRID_API UBuildingObjectAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UBuildingObjectAsset();
    
    // Name of the building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    FText BuildingName;
    
    // Description of the building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    FText BuildingDescription;
    
    // Icon for UI representation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    UTexture2D* BuildingIcon;
    
    // The type of building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EBuildingType BuildingType;
    
    // Size and shape of the building on the grid
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    FBuildingFootprint Footprint;
    
    // Actor class to spawn when placing this building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    TSubclassOf<ABuildingObject> BuildingClass;
    
    // Static mesh for the building's appearance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    UStaticMesh* BuildingMesh;
    
    // Construction cost
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 ConstructionCost = 1000;
    
    // Maintenance cost per day
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 MaintenanceCost = 50;
    
    // Base revenue per use
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 BaseRevenue = 100;
    
    // Utility requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    bool bRequiresWater = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    bool bRequiresElectricity = false;
    
    // Required staff types and counts
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TMap<FName, int32> RequiredStaffTypes;
    
    // Adjacency requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    TArray<FAdjacencyRequirement> AdjacencyRequirements;
    
    // Guest capacity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guests")
    int32 MaxGuests = 1;
    
    // Treatment types supported by this building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Treatments")
    TArray<FName> SupportedTreatments;
    
    // Get the building's footprint
    UFUNCTION(BlueprintCallable, Category = "Building")
    const FBuildingFootprint& GetFootprint() const { return Footprint; }
    
    // Get the building's class
    UFUNCTION(BlueprintCallable, Category = "Building")
    TSubclassOf<ABuildingObject> GetBuildingClass() const { return BuildingClass; }
    
    // Get the building's adjacency requirements
    UFUNCTION(BlueprintCallable, Category = "Building")
    const TArray<FAdjacencyRequirement>& GetAdjacencyRequirements() const { return AdjacencyRequirements; }
    
    // Asset ID for saving/loading
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset")
    FPrimaryAssetId PrimaryAssetId;
    
    // Override to set asset ID based on file name
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return PrimaryAssetId.IsValid() ? PrimaryAssetId : FPrimaryAssetId(GetFName(), FPrimaryAssetType("BuildingAsset"));
    }
};