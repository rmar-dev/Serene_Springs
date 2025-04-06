// BuildingObject.h - Actor that represents a placed building
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EGridTypes.h"
#include "BuildingObject.generated.h"

class UBuildingObjectAsset;
class UStaticMeshComponent;

/**
 * Actor that represents a building placed on the grid
 */
UCLASS(BlueprintType, Blueprintable)
class GRID_API ABuildingObject : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ABuildingObject();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    
    // Static mesh component for the building appearance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    UStaticMeshComponent* BuildingMesh;
    
    // Asset that defines this building's properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    UBuildingObjectAsset* BuildingAsset;
    
    // Type of building
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
    EBuildingType BuildingType;
    
    // Grid origin position
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    FIntPoint GridOrigin;
    
    // Grid floor level
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int32 FloorLevel;
    
    // Rotation in quarters (0-3, representing 0, 90, 180, 270 degrees)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
    int32 GridRotation;
    
    // Current building state (construction, operational, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    uint8 BuildingState;
    
    // Current operational efficiency (0-100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float OperationalEfficiency;
    
    // Assigned staff members
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff")
    TArray<AActor*> AssignedStaff;
    
    // Current guests using this building
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guests")
    TArray<AActor*> CurrentGuests;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    
    /**
     * Initialize the building from an asset definition
     * @param Asset The asset to initialize from
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    virtual void InitializeFromAsset(UBuildingObjectAsset* Asset);
    
    /**
     * Get the building's footprint
     * @return Footprint structure
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    FBuildingFootprint GetFootprint() const;
    
    /**
     * Get the building's type
     * @return Building type enum
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    EBuildingType GetBuildingType() const { return BuildingType; }
    
    /**
     * Set the building's grid properties
     * @param Origin Grid origin position
     * @param Floor Floor level
     * @param Rotation Rotation in quarters (0-3)
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetGridProperties(const FIntPoint& Origin, int32 Floor, int32 Rotation);
    
    /**
     * Get the building's grid properties
     * @param OutOrigin Output for grid origin position
     * @param OutFloor Output for floor level
     * @param OutRotation Output for rotation in quarters
     */
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void GetGridProperties(FIntPoint& OutOrigin, int32& OutFloor, int32& OutRotation) const;
    
    /**
     * Assign a staff member to this building
     * @param StaffMember The staff actor to assign
     * @return True if successfully assigned
     */
    UFUNCTION(BlueprintCallable, Category = "Staff")
    bool AssignStaffMember(AActor* StaffMember);
    
    /**
     * Remove a staff member from this building
     * @param StaffMember The staff actor to remove
     * @return True if successfully removed
     */
    UFUNCTION(BlueprintCallable, Category = "Staff")
    bool RemoveStaffMember(AActor* StaffMember);
    
    /**
     * Check if building has available capacity for guests
     * @return True if has available capacity
     */
    UFUNCTION(BlueprintCallable, Category = "Guests")
    bool HasAvailableCapacity() const;
    
    /**
     * Register a guest using this building
     * @param Guest The guest actor
     * @return True if successfully registered
     */
    UFUNCTION(BlueprintCallable, Category = "Guests")
    bool RegisterGuest(AActor* Guest);
    
    /**
     * Remove a guest from this building
     * @param Guest The guest actor to remove
     * @return True if successfully removed
     */
    UFUNCTION(BlueprintCallable, Category = "Guests")
    bool RemoveGuest(AActor* Guest);
    
    /**
     * Check if this building supports a specific treatment type
     * @param TreatmentType The treatment type name
     * @return True if treatment is supported
     */
    UFUNCTION(BlueprintCallable, Category = "Treatments")
    bool SupportsTreatment(const FName& TreatmentType) const;
    
    /**
     * Get the building's current operational status
     * @return True if building is operational
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsOperational() const;
    
    /**
     * Get the building's current efficiency
     * @return Efficiency as a percentage (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    float GetEfficiency() const { return OperationalEfficiency; }
    
    /**
     * Calculate daily maintenance cost
     * @return Current maintenance cost
     */
    UFUNCTION(BlueprintCallable, Category = "Economy")
    int32 CalculateMaintenanceCost() const;
    
    /**
     * Handle daily update (called by game system)
     * Updates efficiency, condition, etc.
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    virtual void OnDailyUpdate();
    
private:
    /**
     * Update operational efficiency based on staff, maintenance, etc.
     */
    void UpdateEfficiency();
};