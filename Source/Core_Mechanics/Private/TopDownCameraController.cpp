#include "TopDownCameraController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
ATopDownCameraController::ATopDownCameraController()
{
 	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootSceneComponent);

	// Create movement component for panning
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	
	// Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeRotation(FRotator(-50.0f, 0.0f, 0.0f)); // Default to looking down at an angle
	CameraBoom->TargetArmLength = 1500.0f; // Default zoom distance
	CameraBoom->bDoCollisionTest = false; // Don't let the camera boom collide with the world
	
	// Initialize camera control variables
	CurrentZoomDistance = CameraBoom->TargetArmLength;
	TargetZoomDistance = CurrentZoomDistance;
	CurrentRotationYaw = 0.0f;
	TargetRotationYaw = 0.0f;
	CurrentPanVelocity = FVector::ZeroVector;
	TargetPanVelocity = FVector::ZeroVector;

	// Create camera
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false; // Don't rotate camera relative to arm
}

// Called when the game starts or when spawned
void ATopDownCameraController::BeginPlay()
{
	Super::BeginPlay();
	
	// Set up enhanced input
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// In UE5, we can just add the mapping context
			// The AddMappingContext replaces existing mappings with the same priority
			Subsystem->AddMappingContext(CameraControlsContext, 0);
		}
	}
}

// Called every frame
void ATopDownCameraController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ======= Smooth Zoom Interpolation =======
	if (!FMath::IsNearlyEqual(CurrentZoomDistance, TargetZoomDistance, 1.0f))
	{
		// Use FInterpTo for smooth movement toward target
		CurrentZoomDistance = FMath::FInterpTo(
			CurrentZoomDistance, 
			TargetZoomDistance, 
			DeltaTime, 
			ZoomInterpolationSpeed);
		
		// Apply the interpolated zoom distance to the camera boom
		CameraBoom->TargetArmLength = CurrentZoomDistance;
	}
	
	// ======= Smooth Rotation Interpolation =======
	FRotator CurrentRotation = CameraBoom->GetRelativeRotation();
	if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, TargetRotationYaw, 0.1f))
	{
		// Smoothly interpolate camera rotation
		CurrentRotationYaw = FMath::FInterpTo(
			CurrentRotation.Yaw,
			TargetRotationYaw,
			DeltaTime,
			RotationInterpolationSpeed);
		
		// Apply the new rotation
		FRotator NewRotation = CurrentRotation;
		NewRotation.Yaw = CurrentRotationYaw;
		CameraBoom->SetRelativeRotation(NewRotation);
	}
	
	// ======= Smooth Pan Interpolation =======
	// Different interpolation when stopping vs starting (much faster stopping)
	float CurrentInterpolationSpeed = TargetPanVelocity.IsZero() ? 
		PanDeceleration : PanInterpolationSpeed;
	
	// Interpolate current velocity toward target velocity
	CurrentPanVelocity = FMath::VInterpTo(
		CurrentPanVelocity,
		TargetPanVelocity,
		DeltaTime,
		CurrentInterpolationSpeed);
	
	// Force velocity to zero if it's very small (prevents continued tiny movement)
	if (CurrentPanVelocity.SizeSquared() < 5.0f && TargetPanVelocity.IsZero())
	{
		CurrentPanVelocity = FVector::ZeroVector;
	}
	
	// Apply movement only if velocity is significant
	if (!CurrentPanVelocity.IsNearlyZero(0.1f))
	{
		// Calculate displacement this frame
		FVector Displacement = CurrentPanVelocity * DeltaTime;
		
		// Move the camera
		AddActorWorldOffset(Displacement);
	}
}

// Called to bind functionality to input
void ATopDownCameraController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Get the Enhanced Input Component
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind the actions
		if (PanAction)
		{
			EnhancedInputComponent->BindAction(PanAction, ETriggerEvent::Triggered, this, &ATopDownCameraController::Pan);
			EnhancedInputComponent->BindAction(PanAction, ETriggerEvent::Completed, this, &ATopDownCameraController::PanReleased);
		}
		
		if (MouseRotateAction)
		{
			EnhancedInputComponent->BindAction(MouseRotateAction, ETriggerEvent::Triggered, this, &ATopDownCameraController::MouseRotate);
			EnhancedInputComponent->BindAction(MouseRotateAction, ETriggerEvent::Completed, this, &ATopDownCameraController::MouseRotateReleased);
		}

		if (ZoomAction)
		{
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATopDownCameraController::Zoom);
		}

		if (RotateAction)
		{
			EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATopDownCameraController::Rotate);
		}
	}
}

void ATopDownCameraController::Pan(const FInputActionValue& Value)
{
	FVector2D PanValue = Value.Get<FVector2D>();
	
	if (!PanValue.IsZero())
	{
		// Get the camera's world-space forward and right vectors
		FVector CameraForward = CameraBoom->GetForwardVector();
		FVector CameraRight = CameraBoom->GetRightVector();
		
		// Project these vectors to the horizontal plane (ignore Z component)
		CameraForward.Z = 0;
		CameraRight.Z = 0;
		
		// Normalize to ensure consistent movement speed regardless of camera angle
		if (!CameraForward.IsNearlyZero())
		{
			CameraForward.Normalize();
		}
		if (!CameraRight.IsNearlyZero())
		{
			CameraRight.Normalize();
		}

		// Calculate target pan velocity from input
		TargetPanVelocity = (CameraForward * PanValue.Y + CameraRight * PanValue.X) * PanSpeed;
		
		// Clamp maximum velocity
		if (TargetPanVelocity.SizeSquared() > FMath::Square(MaxPanVelocity))
		{
			TargetPanVelocity = TargetPanVelocity.GetSafeNormal() * MaxPanVelocity;
		}
	}
}

void ATopDownCameraController::PanReleased(const FInputActionValue& Value)
{
	// When input is explicitly released, immediately begin deceleration
	TargetPanVelocity = FVector::ZeroVector;
	
	// Optional: For even quicker stopping, we could directly reduce current velocity here
	CurrentPanVelocity *= 0.5f; // Immediately cut velocity by half on release
}

void ATopDownCameraController::MouseRotate(const FInputActionValue& Value)
{
	FVector2D MouseDelta = Value.Get<FVector2D>();
	
	if (!MouseDelta.IsZero())
	{
		// Only use the X component for rotation (horizontal mouse movement)
		float RotateValue = MouseDelta.X * MouseRotateSensitivity;
		
		// Update target rotation - accumulate the rotation value
		TargetRotationYaw += RotateValue;
	}
}

void ATopDownCameraController::MouseRotateReleased(const FInputActionValue& Value)
{
	// When middle mouse is released, we don't need to do anything special for rotation
	// The current target rotation will be maintained
}

void ATopDownCameraController::Zoom(const FInputActionValue& Value)
{
	float ZoomValue = Value.Get<float>();
	
	if (!FMath::IsNearlyZero(ZoomValue))
	{
		// Adjust zoom speed based on current distance for more natural feeling
		float AdjustedZoomSpeed = ZoomSpeed * (TargetZoomDistance / 1000.0f);
		
		// Ensure minimum zoom speed
		AdjustedZoomSpeed = FMath::Max(AdjustedZoomSpeed, ZoomSpeed * 0.5f);
		
		// Update target zoom distance - note the direction may need to be flipped
		// depending on how your mouse wheel input is configured
		TargetZoomDistance -= ZoomValue * AdjustedZoomSpeed;
		
		// Clamp to min/max range
		TargetZoomDistance = FMath::Clamp(TargetZoomDistance, MinZoomDistance, MaxZoomDistance);
	}
}

void ATopDownCameraController::Rotate(const FInputActionValue& Value)
{
	float RotateValue = Value.Get<float>();
	if (!FMath::IsNearlyZero(RotateValue))
	{
		// Update target rotation - accumulate the rotation value
		TargetRotationYaw += RotateValue * RotationSpeed;
	}
}