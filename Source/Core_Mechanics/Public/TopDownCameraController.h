// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "TopDownCameraController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * A top-down camera controller pawn that supports panning, zooming, and movement
 */
UCLASS()
class CORE_MECHANICS_API ATopDownCameraController : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATopDownCameraController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Root component to attach everything to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* RootSceneComponent;

	/** Camera boom arm for controlling distance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* TopDownCamera;

	/** Movement component for panning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UPawnMovementComponent* MovementComponent;

	// ---------- Input Mapping Context ----------
	/** Input Mapping Context for the camera controller */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CameraControlsContext;

	// ---------- Input Actions ----------
	/** Input Action for camera panning */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PanAction;

	/** Input Action for mouse rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseRotateAction;

	/** Input Action for camera zooming */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomAction;

	/** Input Action for camera rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RotateAction;

	// ---------- Camera Configuration ----------
	/** Minimum allowed zoom distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MinZoomDistance = 500.0f;

	/** Maximum allowed zoom distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MaxZoomDistance = 3000.0f;

	/** Zoom speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float ZoomSpeed = 200.0f;

	/** Panning speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float PanSpeed = 1000.0f;

	/** Rotation speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float RotationSpeed = 10.0f;

	/** Current zoom level (affects boom arm length) */
	float CurrentZoomDistance;
	
	/** Target zoom level for smooth interpolation */
	float TargetZoomDistance;
	
	/** Current rotation (yaw) value */
	float CurrentRotationYaw;
	
	/** Target rotation (yaw) value for smooth interpolation */
	float TargetRotationYaw;
	
	/** Current pan velocity vector */
	FVector CurrentPanVelocity;
	
	/** Target pan velocity vector for smooth interpolation */
	FVector TargetPanVelocity;
	
	/** How quickly the camera zooms to target distance (higher = faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float ZoomInterpolationSpeed = 8.0f;
	
	/** How quickly the camera rotation changes (higher = faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float RotationInterpolationSpeed = 8.0f;
	
	/** How quickly the camera panning accelerates/decelerates (higher = faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float PanInterpolationSpeed = 10.0f;
	
	/** Maximum pan velocity (limits maximum speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MaxPanVelocity = 2000.0f;
	
	/** Deceleration rate when no input is given (higher = faster stop) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float PanDeceleration = 50.0f;
	
	/** Mouse rotation sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float MouseRotateSensitivity = 2.0f;

	// ---------- Input Callbacks ----------
	/** Handle camera panning input */
	void Pan(const FInputActionValue& Value);
	
	/** Handle camera panning release */
	void PanReleased(const FInputActionValue& Value);
	
	/** Handle mouse rotation input */
	void MouseRotate(const FInputActionValue& Value);
	
	/** Handle mouse rotation release */
	void MouseRotateReleased(const FInputActionValue& Value);

	/** Handle camera zooming input */
	void Zoom(const FInputActionValue& Value);

	/** Handle camera rotation input */
	void Rotate(const FInputActionValue& Value);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};