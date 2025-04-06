// BuildingObject.cpp - Implementation of placed building actor
#include "BuildingObject.h"
#include "BuildingObjectAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

// Sets default values
ABuildingObject::ABuildingObject()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create a root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create the static mesh component
    BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    BuildingMesh->SetupAttachment(RootComponent);
    
    // Set default values
    BuildingType = EBuildingType::None;
    GridOrigin = FIntPoint::ZeroValue;
    FloorLevel = 0;
    GridRotation = 0;
    BuildingState = 0; // Default state (planning/construction)
    OperationalEfficiency = 100.0f; // Start at 100% efficiency
}

// Called when the game starts or when spawned
void ABuildingObject::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ABuildingObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABuildingObject::InitializeFromAsset(UBuildingObjectAsset* Asset)
{
    if (!Asset)
    {
        return;
    }
    
    // Store reference to asset
    BuildingAsset = Asset;
    
    // Set building type
    BuildingType = Asset->BuildingType;
    
    // Set static mesh if available
    if (Asset->BuildingMesh)
    {
        BuildingMesh->SetStaticMesh(Asset->BuildingMesh);
    }
    
    // Initialize other properties from asset
    OperationalEfficiency = 100.0f;
    BuildingState = 0; // Construction state
    
    // Clear staff and guests
    AssignedStaff.Empty();
    CurrentGuests.Empty();
}

FBuildingFootprint ABuildingObject::GetFootprint() const
{
    // Return from asset if available
    if (BuildingAsset)
    {
        return BuildingAsset->GetFootprint();
    }
    
    // Otherwise return default 1x1 footprint
    FBuildingFootprint DefaultFootprint;
    DefaultFootprint.Size = FIntPoint(1, 1);
    return DefaultFootprint;
}

void ABuildingObject::SetGridProperties(const FIntPoint& Origin, int32 Floor, int32 Rotation)
{
    GridOrigin = Origin;
    FloorLevel = Floor;
    GridRotation = Rotation;
}

void ABuildingObject::GetGridProperties(FIntPoint& OutOrigin, int32& OutFloor, int32& OutRotation) const
{
    OutOrigin = GridOrigin;
    OutFloor = FloorLevel;
    OutRotation = GridRotation;
}

bool ABuildingObject::AssignStaffMember(AActor* StaffMember)
{
    if (!StaffMember)
    {
        return false;
    }
    
    // Check if already assigned
    if (AssignedStaff.Contains(StaffMember))
    {
        return true; // Already assigned
    }
    
    // Add to assigned staff
    AssignedStaff.Add(StaffMember);
    
    // Update efficiency based on staff
    UpdateEfficiency();
    
    return true;
}

bool ABuildingObject::RemoveStaffMember(AActor* StaffMember)
{
    if (!StaffMember)
    {
        return false;
    }
    
    // Remove from assigned staff
    int32 NumRemoved = AssignedStaff.Remove(StaffMember);
    
    // Update efficiency if staff was removed
    if (NumRemoved > 0)
    {
        UpdateEfficiency();
        return true;
    }
    
    return false;
}

bool ABuildingObject::HasAvailableCapacity() const
{
    // Get max capacity from asset
    int32 MaxCapacity = BuildingAsset ? BuildingAsset->MaxGuests : 1;
    
    // Check current guests
    return CurrentGuests.Num() < MaxCapacity;
}

bool ABuildingObject::RegisterGuest(AActor* Guest)
{
    if (!Guest)
    {
        return false;
    }
    
    // Check if already registered
    if (CurrentGuests.Contains(Guest))
    {
        return true; // Already registered
    }
    
    // Check capacity
    if (!HasAvailableCapacity())
    {
        return false; // No capacity
    }
    
    // Add to current guests
    CurrentGuests.Add(Guest);
    
    return true;
}

bool ABuildingObject::RemoveGuest(AActor* Guest)
{
    if (!Guest)
    {
        return false;
    }
    
    // Remove from current guests
    int32 NumRemoved = CurrentGuests.Remove(Guest);
    
    return NumRemoved > 0;
}

bool ABuildingObject::SupportsTreatment(const FName& TreatmentType) const
{
    // Check if building asset is valid
    if (!BuildingAsset)
    {
        return false;
    }
    
    // Check if treatment is supported
    return BuildingAsset->SupportedTreatments.Contains(TreatmentType);
}

bool ABuildingObject::IsOperational() const
{
    // Check if building is in operational state
    // For now, we'll consider state 1 as operational
    if (BuildingState != 1)
    {
        return false;
    }
    
    // Check if we have required staff
    if (BuildingAsset)
    {
        // For each required staff type
        for (const auto& StaffRequirement : BuildingAsset->RequiredStaffTypes)
        {
            // Count assigned staff of this type
            // Note: This is simplified, in a real implementation we would check the staff type
            if (AssignedStaff.Num() < StaffRequirement.Value)
            {
                return false; // Not enough staff
            }
        }
    }
    
    return true;
}

int32 ABuildingObject::CalculateMaintenanceCost() const
{
    if (!BuildingAsset)
    {
        return 0;
    }
    
    // Base maintenance cost from asset
    int32 Cost = BuildingAsset->MaintenanceCost;
    
    // Modify based on efficiency and other factors if needed
    // For now, we'll just return the base cost
    
    return Cost;
}

void ABuildingObject::OnDailyUpdate()
{
    // Update building state if needed
    // For example, progress from construction to operational
    if (BuildingState == 0)
    {
        // This would normally be based on construction progress
        // For now, we'll just set it to operational
        BuildingState = 1;
    }
    
    // Update efficiency
    UpdateEfficiency();
}

// Private helper function to update operational efficiency
void ABuildingObject::UpdateEfficiency()
{
    // Start with base efficiency
    float NewEfficiency = 100.0f;
    
    // Check if we have required staff
    if (BuildingAsset)
    {
        // Calculate staff efficiency
        float StaffEfficiency = 100.0f;
        
        // For each required staff type
        for (const auto& StaffRequirement : BuildingAsset->RequiredStaffTypes)
        {
            // Count assigned staff of this type
            // Note: This is simplified, in a real implementation we would check the staff type
            float StaffRatio = FMath::Min(1.0f, (float)AssignedStaff.Num() / StaffRequirement.Value);
            
            // Staff efficiency is the minimum ratio across all required types
            StaffEfficiency = FMath::Min(StaffEfficiency, StaffRatio * 100.0f);
        }
        
        // Apply staff efficiency
        NewEfficiency = StaffEfficiency;
    }
    
    // Apply additional factors (could be maintenance state, etc.)
    // For now, we'll just use the staff efficiency
    
    // Update operational efficiency
    OperationalEfficiency = NewEfficiency;
}