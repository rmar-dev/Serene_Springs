// BuildingController.h - Controller for building placement interaction
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EGridTypes.h"
#include "BuildingController.generated.h"

class ABuildingGridManager;
class UBuildingObjectAsset;
class UUserWidget;

/**
 * Player controller with building placement functionality
 */
UCLASS()
class GRID_API ABuildingController : public APlayerController
{
    GENERATED_BODY()

public:
    ABuildingController();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Reference to the grid manager in the level
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    ABuildingGridManager* GridManager;

    // Currently selected building asset
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    UBuildingObjectAsset* SelectedBuildingAsset;

    // Whether we are in building placement mode
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    bool bBuildingModeActive;

    // Current rotation of building preview (0-3)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    int32 CurrentRotation;

    // Current floor level for building placement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
    int32 CurrentFloorLevel;

    // Building UI widget class
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> BuildingUIWidgetClass;

    // Building UI widget instance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UUserWidget* BuildingUIWidget;

    // Setup input binding
    virtual void SetupInputComponent() override;

    // Trace from screen position to world
    bool GetHitUnderCursor(FHitResult& OutHit);

    // Handle building rotation input
    void RotateBuilding();

    // Handle building placement confirmation
    void ConfirmPlacement();

    // Handle building placement cancellation
    void CancelPlacement();

    // Handle changing floor level up
    void IncrementFloorLevel();

    // Handle changing floor level down
    void DecrementFloorLevel();

public:
    /**
     * Enter building placement mode with the specified asset
     * @param BuildingAsset The building asset to place
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    void EnterBuildingMode(UBuildingObjectAsset* BuildingAsset);

    /**
     * Exit building placement mode
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    void ExitBuildingMode();

    /**
     * Get the currently selected building asset
     * @return The selected building asset
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    UBuildingObjectAsset* GetSelectedBuildingAsset() const { return SelectedBuildingAsset; }

    /**
     * Get whether building mode is active
     * @return True if building mode is active
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool IsBuildingModeActive() const { return bBuildingModeActive; }

    /**
     * Get the current floor level
     * @return The current floor level
     */
    UFUNCTION(BlueprintCallable, Category = "Building")
    int32 GetCurrentFloorLevel() const { return CurrentFloorLevel; }
};