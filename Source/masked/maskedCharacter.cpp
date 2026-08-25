// Copyright Epic Games, Inc. All Rights Reserved.

#include "maskedCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Animation/MaskedCharacterAnimInstance.h"
#include "masked.h"

namespace MaskedMovement
{
	float SmoothDamp(
		float Current,
		float Target,
		float& Velocity,
		float SmoothTime,
		float DeltaTime)
	{
		SmoothTime = FMath::Max(0.0001f, SmoothTime);
		DeltaTime = FMath::Max(DeltaTime, 0.0001f);

		const float Omega = 2.0f / SmoothTime;
		const float X = Omega * DeltaTime;
		const float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);
		const float OriginalTo = Target;
		float Change = Current - Target;
		Target = Current - Change;

		const float Temp = (Velocity + Omega * Change) * DeltaTime;
		Velocity = (Velocity - Omega * Temp) * Exp;
		float Result = Target + (Change + Temp) * Exp;

		if ((OriginalTo - Current > 0.0f) == (Result > OriginalTo))
		{
			Result = OriginalTo;
			Velocity = 0.0f;
		}

		return Result;
	}
}

AmaskedCharacter::AmaskedCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->bUseControllerDesiredRotation = false;
	MovementComponent->GroundFriction = 5.0f;
	MovementComponent->bUseSeparateBrakingFriction = true;
	MovementComponent->BrakingFriction = StandardBrakingFriction;
	MovementComponent->BrakingFrictionFactor = 1.0f;
	MovementComponent->JumpZVelocity = 450.0f;
	MovementComponent->AirControl = JogAirControl;
	MovementComponent->AirControlBoostMultiplier = 1.25f;
	MovementComponent->AirControlBoostVelocityThreshold = 120.0f;
	MovementComponent->FallingLateralFriction = 0.15f;
	MovementComponent->MinAnalogWalkSpeed = 10.0f;
	MovementComponent->MaxWalkSpeedCrouched = 180.0f;
	MovementComponent->SetCrouchedHalfHeight(CrouchedCapsuleHalfHeight);
	MovementComponent->bCanWalkOffLedgesWhenCrouching = true;
	MovementComponent->BrakingDecelerationFalling = 0.0f;
	MovementComponent->GetNavAgentPropertiesRef().bCanCrouch = true;
	ConfigureTerrainMovement();

	SprintSettings.MaxSpeed = 620.0f;
	SprintSettings.MaxAcceleration = 1100.0f;
	SprintSettings.BrakingDeceleration = 700.0f;
	SprintSettings.RotationRate = 240.0f;

	CrouchSettings.MaxSpeed = 180.0f;
	CrouchSettings.MaxAcceleration = 650.0f;
	CrouchSettings.BrakingDeceleration = 1100.0f;
	CrouchSettings.RotationRate = 420.0f;

	ApplyGaitSettings(EMaskedMovementGait::Jogging, JogSettings);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultCameraTargetArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = StandardCameraLagSpeed;
	CameraBoom->CameraLagMaxDistance = 25.0f;
	CameraBoom->bUseCameraLagSubstepping = true;
	CameraBoom->CameraLagMaxTimeStep = 1.0f / 60.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 16.0f;
	CameraBoom->PrimaryComponentTick.AddPrerequisite(this, PrimaryActorTick);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = DefaultFieldOfView;

	PrimaryActorTick.AddPrerequisite(MovementComponent, MovementComponent->PrimaryComponentTick);
	GetMesh()->PrimaryComponentTick.AddPrerequisite(this, PrimaryActorTick);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AmaskedCharacter::PostLoad()
{
	Super::PostLoad();
	ApplyLegacyStairSettingMigration();
}

void AmaskedCharacter::BeginPlay()
{
	Super::BeginPlay();

	MeshBaseRelativeRotation = GetMesh()->GetRelativeRotation();
	MeshBaseRelativeLocation = GetMesh()->GetRelativeLocation();
	CameraBoomBaseRelativeLocation = CameraBoom->GetRelativeLocation();
	CameraBoomBaseLagMaxDistance = CameraBoom->CameraLagMaxDistance;
	CameraBoom->TargetArmLength = DefaultCameraTargetArmLength;
	CameraBoomBaseTargetArmLength = CameraBoom->TargetArmLength;
	CameraBoomBaseSocketOffset = CameraBoom->SocketOffset;
	PreviousTerrainActorLocation = GetActorLocation();
	SmoothedVisualWorldZ = GetActorLocation().Z;
	DetectedStairTreadDepth = DefaultStairTreadDepth;
	bStairVisualHeightInitialized = true;
	PreviousCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	FollowCamera->SetFieldOfView(DefaultFieldOfView);
	CameraBoom->CameraLagSpeed = StandardCameraLagSpeed;
	ConfigureTerrainMovement();
	bMovementDebugEnabled = bMovementDebugEnabledByDefault;
}

void AmaskedCharacter::OnMovementModeChanged(
	EMovementMode PrevMovementMode,
	uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent->IsFalling())
	{
		bAirborneFromSprint = AppliedMovementGait == EMaskedMovementGait::Sprinting;
		LastAirborneHorizontalSpeed = GetVelocity().Size2D();
		if (GetVelocity().Z <= 0.0f)
		{
			// Walking from a ledge is a genuine fall, even if jump was pressed earlier.
			bJumpInitiatedByInput = false;
		}
		PeakDownwardSpeed = FMath::Max(0.0f, -GetVelocity().Z);
		LandingRecoveryTimeRemaining = 0.0f;
		ActiveLandingRecoveryDuration = 0.0f;
		LandingRecoveryAlpha = 0.0f;

		// The movement-mode callback is the authoritative ledge boundary. Clearing
		// crouch here cannot miss a short fall the way a once-per-frame floor poll can.
		MovementComponent->bWantsToCrouch = false;
		bBrakingFromSprint = false;
		if (bIsCrouched)
		{
			UnCrouch(false);
		}
	}
}

void AmaskedCharacter::Landed(const FHitResult& Hit)
{
	// Landed can run after CharacterMovement has already started resolving the
	// contact, so retain the last airborne horizontal speed as well as the fastest
	// downward velocity observed across the fall.
	const float ImpactSpeed = FMath::Max(PeakDownwardSpeed, FMath::Max(0.0f, -GetVelocity().Z));
	LastLandingHorizontalSpeed = FMath::Max(LastAirborneHorizontalSpeed, GetVelocity().Size2D());
	BeginLandingRecovery(ImpactSpeed);
	Super::Landed(Hit);
	bAirborneFromSprint = false;
	PeakDownwardSpeed = 0.0f;

	// Defensive recovery for low ledges where falling and landing can occur inside
	// one rendered frame, or where capsule expansion was temporarily obstructed.
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->bWantsToCrouch = false;
	if (bIsCrouched)
	{
		UnCrouch(false);
	}

	OnCharacterLanded.Broadcast(LastLandingSeverity, LastLandingImpactSpeed);
	bJumpInitiatedByInput = false;
}

void AmaskedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AmaskedCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AmaskedCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AmaskedCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AmaskedCharacter::StopMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AmaskedCharacter::StopMove);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AmaskedCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AmaskedCharacter::Look);

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AmaskedCharacter::DoSprintStart);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AmaskedCharacter::DoSprintEnd);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AmaskedCharacter::DoSprintEnd);
		}
		else
		{
			// Temporary native fallback keeps this first movement checkpoint testable before
			// the project-specific Enhanced Input asset is assigned in the Blueprint child.
			PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AmaskedCharacter::DoSprintStart);
			PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AmaskedCharacter::DoSprintEnd);
			PlayerInputComponent->BindKey(EKeys::Gamepad_LeftThumbstick, IE_Pressed, this, &AmaskedCharacter::DoSprintStart);
			PlayerInputComponent->BindKey(EKeys::Gamepad_LeftThumbstick, IE_Released, this, &AmaskedCharacter::DoSprintEnd);
		}

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AmaskedCharacter::DoCrouchToggle);
		}
		else
		{
			// Asset-independent fallback until IA_Crouch is added to the mapping context.
			PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AmaskedCharacter::DoCrouchToggle);
			PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &AmaskedCharacter::DoCrouchToggle);
			PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AmaskedCharacter::DoCrouchToggle);
		}

		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AmaskedCharacter::DoAimStart);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AmaskedCharacter::DoAimEnd);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &AmaskedCharacter::DoAimEnd);
		}
		else
		{
			// Asset-independent fallback until IA_Aim is added to the mapping context.
			PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AmaskedCharacter::DoAimStart);
			PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AmaskedCharacter::DoAimEnd);
		}
	}
	else
	{
		UE_LOG(Logmasked, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Keep diagnostics available while project input assets are being changed.
	PlayerInputComponent->BindKey(EKeys::F6, IE_Pressed, this, &AmaskedCharacter::ToggleMovementDebug);
}

void AmaskedCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetCharacterMovement()->IsFalling())
	{
		PeakDownwardSpeed = FMath::Max(PeakDownwardSpeed, FMath::Max(0.0f, -GetVelocity().Z));
		LastAirborneHorizontalSpeed = GetVelocity().Size2D();
	}
	UpdateLandingRecovery(DeltaSeconds);
	UpdateMovementPolicy(DeltaSeconds);
	UpdateDirectionChangePolicy(DeltaSeconds);
	UpdateTerrainResponse(DeltaSeconds);
	UpdateStairTraversal(DeltaSeconds);
	UpdateAimResponse(DeltaSeconds);
	UpdateCurrentGait();
	UpdateRotationPolicy(DeltaSeconds);
	UpdateTurnLean(DeltaSeconds);
	UpdateCameraResponse(DeltaSeconds);
	UpdateCrouchResponse(DeltaSeconds);
	UpdateMovementDebug();
}

void AmaskedCharacter::ToggleMovementDebug()
{
	bMovementDebugEnabled = !bMovementDebugEnabled;

	if (!bMovementDebugEnabled && GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(0x4D4F5645);
	}
}

void AmaskedCharacter::UpdateMovementDebug() const
{
	if (!bMovementDebugEnabled)
	{
		return;
	}

	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const FFindFloorResult& Floor = MovementComponent->CurrentFloor;
	const FVector Velocity = GetVelocity();
	const FVector Acceleration = MovementComponent->GetCurrentAcceleration();
	const FString GaitName = StaticEnum<EMaskedMovementGait>()->GetNameStringByValue(
		static_cast<int64>(CurrentGait));
	const FString AppliedGaitName = StaticEnum<EMaskedMovementGait>()->GetNameStringByValue(
		static_cast<int64>(AppliedMovementGait));
	const FString LandingName = StaticEnum<EMaskedLandingSeverity>()->GetNameStringByValue(
		static_cast<int64>(LastLandingSeverity));
	const UMaskedCharacterAnimInstance* MaskedAnimInstance =
		Cast<UMaskedCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	const TCHAR* FootIKState = MaskedAnimInstance && MaskedAnimInstance->IsFootIKTracingEnabled()
		? TEXT("ACTIVE (flat)")
		: TEXT("RELAXED (ramp/stairs/air)");
	const FString GroundTransitionName = MaskedAnimInstance
		? MaskedAnimInstance->GetGroundTransitionDebugName()
		: TEXT("N/A");
	const FString LandingLocomotionName = MaskedAnimInstance
		? MaskedAnimInstance->GetLandingLocomotionDebugName()
		: TEXT("N/A");
	const TCHAR* StairDirection = CurrentStairTraversalAlpha > 0.01f
		? (AppliedMovementGait == EMaskedMovementGait::Sprinting
			? (bAscendingStairs ? TEXT("UP RUN") : TEXT("DN RUN"))
			: (bAscendingStairs ? TEXT("UP") : TEXT("DOWN")))
		: TEXT("OFF");

	const FString DebugText = FString::Printf(
		TEXT("MOVEMENT DEBUG  [F6 hides]\n")
		TEXT("Mode: %-8s  Gait: %s (applied %s)  Crouch: %s  Sprint: %s\n")
		TEXT("Speed: %6.1f  Vertical: %6.1f  Max: %6.1f  Input: X %+.2f  Y %+.2f\n")
		TEXT("Accel: %6.1f / %6.1f  Brake: %6.1f  Friction: %.2f\n")
		TEXT("Floor: %s  Walkable: %s  Normal Z: %.3f  Slope: %+.1f deg\n")
		TEXT("Terrain scales  Speed %.2f  Accel %.2f  Brake %.2f\n")
		TEXT("Turning: angle %5.1f  plant %.2f  idle %s (%+.0f deg)  transition %s\n")
		TEXT("Stairs: %-6s  Alpha %.2f  Step %.1f  Tread %.1f  Target %.0f  Foot %s #%d  Visual %+5.1f\n")
		TEXT("Landing: %-6s  Variant %-6s  Impact %.1f  Move %.1f  Recovery %.2f\n")
		TEXT("Foot IK: %s\n")
		TEXT("Vectors: GREEN velocity  BLUE accel  YELLOW floor  MAGENTA/ORANGE feet"),
		*MovementComponent->GetMovementName(),
		*GaitName,
		*AppliedGaitName,
		bIsCrouched ? TEXT("YES") : TEXT("NO"),
		bSprintRequested ? TEXT("YES") : TEXT("NO"),
		Velocity.Size2D(),
		Velocity.Z,
		MovementComponent->GetMaxSpeed(),
		MovementInput.X,
		MovementInput.Y,
		Acceleration.Size(),
		MovementComponent->GetMaxAcceleration(),
		MovementComponent->BrakingDecelerationWalking,
		MovementComponent->BrakingFriction,
		Floor.bBlockingHit ? TEXT("HIT") : TEXT("NONE"),
		Floor.IsWalkableFloor() ? TEXT("YES") : TEXT("NO"),
		Floor.bBlockingHit ? Floor.HitResult.ImpactNormal.Z : 0.0f,
		CurrentSignedGroundSlopeAngle,
		CurrentTerrainSpeedScale,
		CurrentTerrainAccelerationScale,
		CurrentTerrainBrakingScale,
		CurrentDirectionChangeAngle,
		CurrentDirectionChangeAlpha,
		bTurningInPlace ? TEXT("TURN") : TEXT("OFF"),
		TurnInPlaceRequestedAngle,
		*GroundTransitionName,
		StairDirection,
		CurrentStairTraversalAlpha,
		LastDetectedStepHeight,
		DetectedStairTreadDepth,
		CurrentAdaptiveStairSpeedLimit,
		StairStepSerial > 0 ? (bLastStairStepLeftFoot ? TEXT("L") : TEXT("R")) : TEXT("-"),
		StairStepSerial,
		StairVisualHeightOffset,
		*LandingName,
		*LandingLocomotionName,
		LastLandingImpactSpeed,
		LastLandingHorizontalSpeed,
		LandingRecoveryAlpha,
		FootIKState);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			0x4D4F5645,
			0.0f,
			FColor::Cyan,
			DebugText,
			false,
			FVector2D(0.9f, 0.9f));
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
	DrawDebugDirectionalArrow(
		World,
		Origin,
		Origin + Velocity * MovementDebugVectorScale,
		18.0f,
		FColor::Green,
		false,
		0.0f,
		0,
		2.5f);
	DrawDebugDirectionalArrow(
		World,
		Origin,
		Origin + Acceleration * MovementDebugVectorScale * 0.18f,
		18.0f,
		FColor::Blue,
		false,
		0.0f,
		0,
		2.5f);

	if (Floor.bBlockingHit)
	{
		const FVector FloorPoint = Floor.HitResult.ImpactPoint;
		DrawDebugLine(World, Origin, FloorPoint, FColor::Yellow, false, 0.0f, 0, 1.5f);
		DrawDebugDirectionalArrow(
			World,
			FloorPoint,
			FloorPoint + Floor.HitResult.ImpactNormal * 65.0f,
			12.0f,
			FColor::Yellow,
			false,
			0.0f,
			0,
			2.5f);
	}

	const USkeletalMeshComponent* MeshComponent = GetMesh();
	FCollisionQueryParams FootProbeParams(SCENE_QUERY_STAT(MovementDebugFootProbe), false, this);
	auto DrawFootProbe = [World, MeshComponent, &FootProbeParams](
		const FName BoneName,
		const FColor Color,
		const TCHAR* Label)
	{
		if (!MeshComponent || MeshComponent->GetBoneIndex(BoneName) == INDEX_NONE)
		{
			return;
		}

		const FVector FootLocation = MeshComponent->GetBoneLocation(BoneName);
		const FVector TraceStart = FootLocation + FVector(0.0f, 0.0f, 25.0f);
		const FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, 55.0f);
		FHitResult FootHit;
		const bool bHit = World->LineTraceSingleByChannel(
			FootHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			FootProbeParams);
		const FVector ProbeEnd = bHit ? FootHit.ImpactPoint : TraceEnd;
		const float GroundGap = bHit ? FootLocation.Z - FootHit.ImpactPoint.Z : -1.0f;

		DrawDebugSphere(World, FootLocation, 4.5f, 8, Color, false, 0.0f, 0, 1.5f);
		DrawDebugLine(World, TraceStart, ProbeEnd, Color, false, 0.0f, 0, 1.5f);
		if (bHit)
		{
			DrawDebugPoint(World, FootHit.ImpactPoint, 8.0f, Color, false, 0.0f, 0);
		}
		DrawDebugString(
			World,
			FootLocation + FVector(0.0f, 0.0f, 12.0f),
			bHit
				? FString::Printf(TEXT("%s %.1fcm"), Label, GroundGap)
				: FString::Printf(TEXT("%s NO HIT"), Label),
			nullptr,
			Color,
			0.0f,
			true,
			0.9f);
	};

	DrawFootProbe(TEXT("foot_l"), FColor::Magenta, TEXT("L"));
	DrawFootProbe(TEXT("foot_r"), FColor::Orange, TEXT("R"));
}

void AmaskedCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AmaskedCharacter::StopMove()
{
	MovementInput = FVector2D::ZeroVector;
}

void AmaskedCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AmaskedCharacter::DoMove(float Right, float Forward)
{
	MovementInput = FVector2D(Right, Forward).GetClampedToMaxSize(1.0f);
	if (bSprintRequested && MovementInput.Y >= SprintForwardInputThreshold)
	{
		MovementInput.X *= SprintLateralInputScale;
		MovementInput.Y = FMath::Max(0.0f, MovementInput.Y);
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementInput.Y);
		AddMovementInput(RightDirection, MovementInput.X);
	}
}

void AmaskedCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AmaskedCharacter::DoJumpStart()
{
	if (!bJumpingEnabled)
	{
		bJumpInitiatedByInput = false;
		return;
	}

	const bool bCommittedLandingRecovery = LandingRecoveryTimeRemaining > 0.0f
		&& (LastLandingSeverity == EMaskedLandingSeverity::Medium
			|| LastLandingSeverity == EMaskedLandingSeverity::Heavy);
	if (bCommittedLandingRecovery)
	{
		return;
	}

	if (bIsCrouched || GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
		return;
	}

	bJumpInitiatedByInput = CanJump();
	Jump();
}

void AmaskedCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AmaskedCharacter::DoSprintStart()
{
	if (bIsCrouched || GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}

	bSprintRequested = true;
}

void AmaskedCharacter::DoSprintEnd()
{
	bSprintRequested = false;
	bSprintTurnSuppressed = false;
}

void AmaskedCharacter::DoCrouchToggle()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (bIsCrouched || MovementComponent->bWantsToCrouch)
	{
		// ACharacter::UnCrouch performs the standing overlap test. If the space is
		// blocked, the capsule safely remains crouched.
		UnCrouch();
		return;
	}

	if (!MovementComponent->IsMovingOnGround())
	{
		return;
	}

	bSprintRequested = false;
	bBrakingFromSprint = false;
	Crouch();
}

void AmaskedCharacter::DoAimStart()
{
	bAimRequested = true;
}

void AmaskedCharacter::DoAimEnd()
{
	bAimRequested = false;
}

bool AmaskedCharacter::CanSprint() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const bool bCommittedLandingRecovery = LandingRecoveryTimeRemaining > 0.0f
		&& LastLandingHorizontalSpeed < MovingLandingRecoverySpeedThreshold
		&& (LastLandingSeverity == EMaskedLandingSeverity::Medium
			|| LastLandingSeverity == EMaskedLandingSeverity::Heavy);

	// Sprint is a forward drive. Backward stick/key input while sprint is held
	// drops the character to the ordinary jog so full-speed reverse locomotion
	// (which reads as a sprint-speed moonwalk) never occurs. Side input remains
	// valid because free locomotion rotates the body into its travel direction.
	const bool bBackwardIntent = MovementInput.Y < 0.0f;
	if (!bSprintRequested
		|| MovementInput.IsNearlyZero()
		|| bBackwardIntent
		|| bAimRequested
		|| bSprintTurnSuppressed
		|| bIsCrouched
		|| bCommittedLandingRecovery
		|| !MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	// Free locomotion rotates the character into its travel direction. Sprint is
	// therefore camera-relative and valid from forward or side input; sharp moving
	// reversals are still temporarily governed by the pivot policy.
	return true;
}

FVector AmaskedCharacter::GetDesiredWorldMovementDirection() const
{
	if (MovementInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FRotator ReferenceRotation = GetController()
		? GetController()->GetControlRotation()
		: GetActorRotation();
	const FRotator YawRotation(0.0f, ReferenceRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	return (ForwardDirection * MovementInput.Y + RightDirection * MovementInput.X).GetSafeNormal2D();
}

void AmaskedCharacter::UpdateMovementPolicy(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const bool bHasMovementInput = !MovementInput.IsNearlyZero();
	const float HorizontalSpeed = GetVelocity().Size2D();
	bool bCrouchRequestedOrActive = bIsCrouched || MovementComponent->bWantsToCrouch;

	// Crouching is a grounded stance, not an airborne movement mode. Clear it as
	// soon as the floor is lost so the normal fall graph, capsule and air control
	// take over instead of snapping downward in a frozen crouch pose.
	if (bCrouchRequestedOrActive && MovementComponent->IsFalling())
	{
		MovementComponent->bWantsToCrouch = false;
		UnCrouch();
		bCrouchRequestedOrActive = bIsCrouched || MovementComponent->bWantsToCrouch;
	}

	if (bCrouchRequestedOrActive && MovementComponent->IsMovingOnGround())
	{
		bSprintRequested = false;
		bBrakingFromSprint = false;
		ApplyGaitSettings(EMaskedMovementGait::Jogging, JogSettings);
		MovementComponent->MaxWalkSpeedCrouched = CrouchSettings.MaxSpeed;
		MovementComponent->MaxAcceleration = CrouchSettings.MaxAcceleration;
		MovementComponent->BrakingDecelerationWalking = CrouchSettings.BrakingDeceleration;
		MovementComponent->RotationRate = FRotator(0.0f, CrouchSettings.RotationRate, 0.0f);
		ApplyLandingRecoverySettings();
		return;
	}

	if (bBrakingFromSprint)
	{
		if (bHasMovementInput || !MovementComponent->IsMovingOnGround() || HorizontalSpeed <= 15.0f)
		{
			bBrakingFromSprint = false;
		}
		else
		{
			ApplySprintStopSettings();
			ApplyLandingRecoverySettings();
			return;
		}
	}

	if (AppliedMovementGait == EMaskedMovementGait::Sprinting
		&& !bHasMovementInput
		&& MovementComponent->IsMovingOnGround()
		&& HorizontalSpeed > WalkSpeedThreshold)
	{
		bBrakingFromSprint = true;
		ApplySprintStopSettings();
		ApplyLandingRecoverySettings();
		return;
	}

	if (CanSprint())
	{
		ApplyGaitSettingsSmoothed(EMaskedMovementGait::Sprinting, SprintSettings, DeltaSeconds);
	}
	else
	{
		ApplyGaitSettingsSmoothed(EMaskedMovementGait::Jogging, JogSettings, DeltaSeconds);
	}

	ApplyLandingRecoverySettings();
}

void AmaskedCharacter::UpdateDirectionChangePolicy(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const FVector HorizontalVelocity = FVector(GetVelocity().X, GetVelocity().Y, 0.0f);
	const FVector DesiredDirection = GetDesiredWorldMovementDirection();
	const float HorizontalSpeed = HorizontalVelocity.Size();

	float TargetAngle = 0.0f;
	float TargetSignedAngle = 0.0f;
	float TargetAlpha = 0.0f;
	if (MovementComponent->IsMovingOnGround()
		&& HorizontalSpeed > 40.0f
		&& !DesiredDirection.IsNearlyZero())
	{
		TargetAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(
			FVector::DotProduct(HorizontalVelocity.GetSafeNormal(), DesiredDirection),
			-1.0f,
			1.0f)));
		const float TurnSign = FVector::CrossProduct(
			HorizontalVelocity.GetSafeNormal(),
			DesiredDirection).Z >= 0.0f ? 1.0f : -1.0f;
		TargetSignedAngle = TargetAngle * TurnSign;
		const float AngleAlpha = FMath::GetMappedRangeValueClamped(
			FVector2D(DirectionChangeStartAngle, DirectionChangeFullAngle),
			FVector2D(0.0f, 1.0f),
			TargetAngle);
		const float SpeedAlpha = FMath::GetMappedRangeValueClamped(
			FVector2D(80.0f, SprintSettings.MaxSpeed),
			FVector2D(0.25f, 1.0f),
			HorizontalSpeed);
		TargetAlpha = AngleAlpha * SpeedAlpha;
	}

	CurrentDirectionChangeAngle = FMath::FInterpTo(
		CurrentDirectionChangeAngle,
		TargetAngle,
		DeltaSeconds,
		DirectionChangeInterpSpeed);
	CurrentSignedDirectionChangeAngle = FMath::FInterpTo(
		CurrentSignedDirectionChangeAngle,
		TargetSignedAngle,
		DeltaSeconds,
		DirectionChangeInterpSpeed);
	CurrentDirectionChangeAlpha = FMath::FInterpTo(
		CurrentDirectionChangeAlpha,
		TargetAlpha,
		DeltaSeconds,
		DirectionChangeInterpSpeed);

	// A sharp sprint steer first becomes a planted jog turn. Sprint can resume
	// automatically once velocity and intent agree again.
	if ((AppliedMovementGait == EMaskedMovementGait::Sprinting || bSprintTurnSuppressed)
		&& TargetAngle >= SprintSharpTurnExitAngle)
	{
		bSprintTurnSuppressed = true;
		ApplyGaitSettingsSmoothed(EMaskedMovementGait::Jogging, JogSettings, DeltaSeconds);
	}
	else if (bSprintTurnSuppressed && (TargetAngle < 25.0f || HorizontalSpeed < 80.0f))
	{
		bSprintTurnSuppressed = false;
	}

	if (CurrentDirectionChangeAlpha > KINDA_SMALL_NUMBER
		&& MovementComponent->IsMovingOnGround())
	{
		MovementComponent->MaxWalkSpeed *= FMath::Lerp(
			1.0f,
			DirectionChangeSpeedScale,
			CurrentDirectionChangeAlpha);
		MovementComponent->MaxAcceleration *= FMath::Lerp(
			1.0f,
			DirectionChangeAccelerationScale,
			CurrentDirectionChangeAlpha);
		MovementComponent->BrakingDecelerationWalking *= FMath::Lerp(
			1.0f,
			DirectionChangeBrakingScale,
			CurrentDirectionChangeAlpha);
	}
}

void AmaskedCharacter::ConfigureTerrainMovement()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->SetWalkableFloorAngle(MaxWalkableSlopeAngle);
	MovementComponent->MaxStepHeight = TerrainMaxStepHeight;
	MovementComponent->PerchRadiusThreshold = TerrainPerchRadiusThreshold;
	MovementComponent->PerchAdditionalHeight = 10.0f;
	MovementComponent->bAlwaysCheckFloor = true;
	MovementComponent->bUseFlatBaseForFloorChecks = true;
	MovementComponent->bMaintainHorizontalGroundVelocity = false;
	MovementComponent->BrakingSubStepTime = 1.0f / 60.0f;
}

void AmaskedCharacter::UpdateTerrainResponse(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	float TargetSpeedScale = 1.0f;
	float TargetAccelerationScale = 1.0f;
	float TargetBrakingScale = 1.0f;
	float TargetSignedSlopeAngle = 0.0f;

	const FFindFloorResult& Floor = MovementComponent->CurrentFloor;
	if (MovementComponent->IsMovingOnGround()
		&& Floor.bBlockingHit
		&& Floor.IsWalkableFloor())
	{
		FVector TravelDirection = GetDesiredWorldMovementDirection();
		if (TravelDirection.IsNearlyZero() && GetVelocity().SizeSquared2D() > 25.0f)
		{
			TravelDirection = GetVelocity().GetSafeNormal2D();
		}

		if (!TravelDirection.IsNearlyZero())
		{
			const FVector FloorNormal = Floor.HitResult.ImpactNormal.GetSafeNormal();
			const FVector SurfaceDirection = FVector::VectorPlaneProject(
				TravelDirection,
				FloorNormal).GetSafeNormal();
			TargetSignedSlopeAngle = FMath::RadiansToDegrees(FMath::Asin(
				FMath::Clamp(SurfaceDirection.Z, -1.0f, 1.0f)));

			const float SlopeAlpha = FMath::GetMappedRangeValueClamped(
				FVector2D(MinimumSlopeResponseAngle, MaxWalkableSlopeAngle),
				FVector2D(0.0f, 1.0f),
				FMath::Abs(TargetSignedSlopeAngle));
			if (TargetSignedSlopeAngle > 0.0f)
			{
				TargetSpeedScale = FMath::Lerp(1.0f, UphillSpeedScaleAtLimit, SlopeAlpha);
				TargetAccelerationScale = FMath::Lerp(
					1.0f,
					UphillAccelerationScaleAtLimit,
					SlopeAlpha);
			}
			else if (TargetSignedSlopeAngle < 0.0f)
			{
				TargetSpeedScale = FMath::Lerp(1.0f, DownhillSpeedScaleAtLimit, SlopeAlpha);
				TargetAccelerationScale = FMath::Lerp(
					1.0f,
					DownhillAccelerationScaleAtLimit,
					SlopeAlpha);
				TargetBrakingScale = FMath::Lerp(
					1.0f,
					DownhillBrakingScaleAtLimit,
					SlopeAlpha);
			}
		}
	}

	CurrentTerrainSpeedScale = FMath::FInterpTo(
		CurrentTerrainSpeedScale,
		TargetSpeedScale,
		DeltaSeconds,
		TerrainResponseInterpSpeed);
	CurrentTerrainAccelerationScale = FMath::FInterpTo(
		CurrentTerrainAccelerationScale,
		TargetAccelerationScale,
		DeltaSeconds,
		TerrainResponseInterpSpeed);
	CurrentTerrainBrakingScale = FMath::FInterpTo(
		CurrentTerrainBrakingScale,
		TargetBrakingScale,
		DeltaSeconds,
		TerrainResponseInterpSpeed);
	CurrentSignedGroundSlopeAngle = FMath::FInterpTo(
		CurrentSignedGroundSlopeAngle,
		TargetSignedSlopeAngle,
		DeltaSeconds,
		TerrainResponseInterpSpeed);

	if (MovementComponent->IsMovingOnGround())
	{
		MovementComponent->MaxWalkSpeed *= CurrentTerrainSpeedScale;
		MovementComponent->MaxWalkSpeedCrouched *= CurrentTerrainSpeedScale;
		MovementComponent->MaxAcceleration *= CurrentTerrainAccelerationScale;
		MovementComponent->BrakingDecelerationWalking *= CurrentTerrainBrakingScale;
	}
}

void AmaskedCharacter::UpdateStairTraversal(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const FVector CurrentLocation = GetActorLocation();
	const FVector TravelDelta = CurrentLocation - PreviousTerrainActorLocation;
	PreviousTerrainActorLocation = CurrentLocation;

	bool bDetectedStep = false;
	bool bUpcomingStair = false;
	const FFindFloorResult& Floor = MovementComponent->CurrentFloor;
	const float HorizontalTravel = TravelDelta.Size2D();
	const float VerticalTravel = TravelDelta.Z;
	const float AbsoluteVerticalTravel = FMath::Abs(VerticalTravel);
	const bool bFlatTreadContact = Floor.bBlockingHit
		&& Floor.IsWalkableFloor()
		&& Floor.HitResult.ImpactNormal.Z >= 0.98f;
	const bool bStairWasActive = StairResponseTimeRemaining > 0.0f
		|| CurrentStairTraversalAlpha > 0.05f;

	if (bHasDetectedStairStep && bStairWasActive && HorizontalTravel <= StairLookAheadDistance)
	{
		HorizontalDistanceSinceStairStep += HorizontalTravel;
	}

	// Anticipate the next flat-topped riser or tread drop. Detection after StepUp is
	// one frame too late when the player approaches at jog speed and can skip the
	// first tread before the stair cap takes effect.
	if (MovementComponent->IsMovingOnGround() && bFlatTreadContact)
	{
		FVector TravelDirection = GetDesiredWorldMovementDirection();
		if (TravelDirection.IsNearlyZero() && MovementComponent->Velocity.SizeSquared2D() > 25.0f)
		{
			TravelDirection = MovementComponent->Velocity.GetSafeNormal2D();
		}

		if (!TravelDirection.IsNearlyZero())
		{
			const float FloorZ = Floor.HitResult.ImpactPoint.Z;
			const float DynamicLookAhead = FMath::Clamp(
				MovementComponent->Velocity.Size2D() * 0.24f,
				40.0f,
				StairLookAheadDistance);
			FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(MaskedStairProbe), false, this);

			const FVector LowStart(CurrentLocation.X, CurrentLocation.Y, FloorZ + MinimumDetectedStepHeight);
			const FVector LowEnd = LowStart + TravelDirection * DynamicLookAhead;
			FHitResult RiserHit;
			const bool bHitRiser = GetWorld()->LineTraceSingleByChannel(
				RiserHit,
				LowStart,
				LowEnd,
				ECC_Visibility,
				ProbeParams)
				&& RiserHit.ImpactNormal.Z < 0.35f;

			if (bHitRiser)
			{
				const FVector TopProbeLocation = RiserHit.ImpactPoint + TravelDirection * 8.0f;
				const FVector TopStart(
					TopProbeLocation.X,
					TopProbeLocation.Y,
					FloorZ + TerrainMaxStepHeight + 8.0f);
				const FVector TopEnd(
					TopProbeLocation.X,
					TopProbeLocation.Y,
					FloorZ + MinimumDetectedStepHeight);
				FHitResult TopHit;
				if (GetWorld()->LineTraceSingleByChannel(
					TopHit,
					TopStart,
					TopEnd,
					ECC_Visibility,
					ProbeParams))
				{
					const float UpcomingRise = TopHit.ImpactPoint.Z - FloorZ;
					bUpcomingStair = TopHit.ImpactNormal.Z >= 0.95f
						&& UpcomingRise >= MinimumDetectedStepHeight
						&& UpcomingRise <= TerrainMaxStepHeight + 2.0f;
					if (bUpcomingStair)
					{
						bAscendingStairs = true;
					}
				}
			}

			if (!bUpcomingStair)
			{
				const FVector Ahead = CurrentLocation + TravelDirection * FMath::Min(DynamicLookAhead, 55.0f);
				const FVector DownStart(Ahead.X, Ahead.Y, FloorZ + 10.0f);
				const FVector DownEnd(Ahead.X, Ahead.Y, FloorZ - TerrainMaxStepHeight - 4.0f);
				FHitResult LowerTreadHit;
				if (GetWorld()->LineTraceSingleByChannel(
					LowerTreadHit,
					DownStart,
					DownEnd,
					ECC_Visibility,
					ProbeParams))
				{
					const float UpcomingDrop = FloorZ - LowerTreadHit.ImpactPoint.Z;
					bUpcomingStair = LowerTreadHit.ImpactNormal.Z >= 0.95f
						&& UpcomingDrop >= MinimumDetectedStepHeight
						&& UpcomingDrop <= TerrainMaxStepHeight + 2.0f;
					if (bUpcomingStair)
					{
						bAscendingStairs = false;
					}
				}
			}
		}
	}

	if (MovementComponent->IsMovingOnGround()
		&& bFlatTreadContact
		&& HorizontalTravel > 0.5f
		&& AbsoluteVerticalTravel >= MinimumDetectedStepHeight
		&& AbsoluteVerticalTravel <= TerrainMaxStepHeight + 2.0f)
	{
		bDetectedStep = true;
		bAscendingStairs = VerticalTravel > 0.0f;
		LastDetectedStepHeight = AbsoluteVerticalTravel;
		StairResponseTimeRemaining = StairResponseHoldDuration;

		if (bHasDetectedStairStep
			&& HorizontalDistanceSinceStairStep >= 12.0f
			&& HorizontalDistanceSinceStairStep <= StairLookAheadDistance + 20.0f)
		{
			DetectedStairTreadDepth = FMath::Lerp(
				DetectedStairTreadDepth,
				HorizontalDistanceSinceStairStep,
				0.65f);
		}
		else if (!bHasDetectedStairStep)
		{
			DetectedStairTreadDepth = DefaultStairTreadDepth;
		}
		HorizontalDistanceSinceStairStep = 0.0f;
		bHasDetectedStairStep = true;
		bLastStairStepLeftFoot = bNextStairStepLeftFoot;
		bNextStairStepLeftFoot = !bNextStairStepLeftFoot;
		++StairStepSerial;

		// StepUp teleports the capsule onto the next tread and can leave a residual
		// vertical velocity that reads as a hop. Kill it so the next frame stays planted.
		FVector GroundVelocity = MovementComponent->Velocity;
		if (!FMath::IsNearlyZero(GroundVelocity.Z))
		{
			GroundVelocity.Z = 0.0f;
			MovementComponent->Velocity = GroundVelocity;
		}
	}
	else if (!bUpcomingStair)
	{
		StairResponseTimeRemaining = FMath::Max(
			0.0f,
			StairResponseTimeRemaining - DeltaSeconds);
	}
	else
	{
		StairResponseTimeRemaining = StairResponseHoldDuration;
	}

	const float TargetStairAlpha = bDetectedStep || bUpcomingStair || StairResponseTimeRemaining > 0.0f
		? 1.0f
		: 0.0f;
	const float AlphaInterpSpeed = TargetStairAlpha > CurrentStairTraversalAlpha
		? StairResponseInterpSpeed
		: StairResponseExitInterpSpeed;
	CurrentStairTraversalAlpha = FMath::FInterpTo(
		CurrentStairTraversalAlpha,
		TargetStairAlpha,
		DeltaSeconds,
		AlphaInterpSpeed);

	if (TargetStairAlpha <= 0.0f && CurrentStairTraversalAlpha <= 0.01f)
	{
		CurrentStairTraversalAlpha = 0.0f;
		LastDetectedStepHeight = 0.0f;
		HorizontalDistanceSinceStairStep = 0.0f;
		DetectedStairTreadDepth = DefaultStairTreadDepth;
		CurrentAdaptiveStairSpeedLimit = 0.0f;
		bHasDetectedStairStep = false;
	}

	if (MovementComponent->IsMovingOnGround() && CurrentStairTraversalAlpha > 0.0f)
	{
		const bool bStairSprinting = AppliedMovementGait == EMaskedMovementGait::Sprinting
			&& bSprintRequested;
		const float TargetCadence = bStairSprinting
			? (bAscendingStairs
				? AscendingStairSprintFootfallCadence
				: DescendingStairSprintFootfallCadence)
			: (bAscendingStairs
				? AscendingStairFootfallCadence
				: DescendingStairFootfallCadence);
		const float HardSpeedLimit = bStairSprinting
			? (bAscendingStairs
				? AscendingStairSprintMaxSpeed
				: DescendingStairSprintMaxSpeed)
			: (bAscendingStairs
				? AscendingStairMaxSpeed
				: DescendingStairMaxSpeed);
		const float MinimumSpeed = bStairSprinting
			? MinimumAdaptiveStairSprintSpeed
			: MinimumAdaptiveStairSpeed;
		const float StairSpeedLimit = FMath::Clamp(
			DetectedStairTreadDepth * TargetCadence,
			MinimumSpeed,
			HardSpeedLimit);
		CurrentAdaptiveStairSpeedLimit = StairSpeedLimit;
		const float LimitedWalkSpeed = FMath::Min(
			MovementComponent->MaxWalkSpeed,
			StairSpeedLimit);
		MovementComponent->MaxWalkSpeed = FMath::Lerp(
			MovementComponent->MaxWalkSpeed,
			LimitedWalkSpeed,
			CurrentStairTraversalAlpha);
		const float LimitedCrouchSpeed = FMath::Min(
			MovementComponent->MaxWalkSpeedCrouched,
			StairSpeedLimit);
		MovementComponent->MaxWalkSpeedCrouched = FMath::Lerp(
			MovementComponent->MaxWalkSpeedCrouched,
			LimitedCrouchSpeed,
			CurrentStairTraversalAlpha);
		MovementComponent->MaxAcceleration *= FMath::Lerp(
			1.0f,
			bStairSprinting ? StairSprintAccelerationScale : StairAccelerationScale,
			CurrentStairTraversalAlpha);
		MovementComponent->BrakingDecelerationWalking *= FMath::Lerp(
			1.0f,
			bStairSprinting ? StairSprintBrakingScale : StairBrakingScale,
			CurrentStairTraversalAlpha);
	}

	MovementComponent->PerchRadiusThreshold = FMath::Lerp(
		TerrainPerchRadiusThreshold,
		StairGroundPerchRadius,
		CurrentStairTraversalAlpha);
	MovementComponent->PerchAdditionalHeight = FMath::Lerp(
		10.0f,
		StairGroundPerchAdditionalHeight,
		CurrentStairTraversalAlpha);

	const float CapsuleZ = CurrentLocation.Z;
	if (!bStairVisualHeightInitialized)
	{
		SmoothedVisualWorldZ = CapsuleZ;
		bStairVisualHeightInitialized = true;
	}

	const bool bLargeRelocation = TravelDelta.SizeSquared() > FMath::Square(250.0f);
	if (!MovementComponent->IsMovingOnGround() || (bLargeRelocation && !bDetectedStep))
	{
		SmoothedVisualWorldZ = CapsuleZ;
		StairVisualZVelocity = 0.0f;
		StairVisualHeightOffset = 0.0f;
	}
	else
	{
		const float ActiveSmoothTime = bAscendingStairs
			? StairVisualSmoothTime
			: StairVisualDescendSmoothTime;
		const float SmoothTime = FMath::Lerp(
			StairVisualCatchupTime,
			ActiveSmoothTime,
			CurrentStairTraversalAlpha);
		SmoothedVisualWorldZ = MaskedMovement::SmoothDamp(
			SmoothedVisualWorldZ,
			CapsuleZ,
			StairVisualZVelocity,
			SmoothTime,
			DeltaSeconds);
		const float PerStepVisualLimit = FMath::Min(
			MaxStairVisualOffset,
			FMath::Max(LastDetectedStepHeight * 1.45f, 12.0f));
		StairVisualHeightOffset = FMath::Clamp(
			SmoothedVisualWorldZ - CapsuleZ,
			-PerStepVisualLimit,
			PerStepVisualLimit);
		SmoothedVisualWorldZ = CapsuleZ + StairVisualHeightOffset;
	}
}

void AmaskedCharacter::UpdateCurrentGait()
{
	const float HorizontalSpeed = GetVelocity().Size2D();

	// Dead-band below each boundary stops the gait label (and the animation/FOV
	// states that consume it) from flickering when speed hovers at a threshold.
	// Entering a faster gait uses the authored threshold; leaving it requires
	// dropping this far below, so ordinary speed wobble cannot oscillate the label.
	constexpr float GaitHysteresisBand = 25.0f;

	EMaskedMovementGait NewGait = EMaskedMovementGait::Walking;

	const float SprintThreshold = JogSettings.MaxSpeed
		- (CurrentGait == EMaskedMovementGait::Sprinting ? GaitHysteresisBand : 0.0f);
	const float JogThreshold = WalkSpeedThreshold
		- (CurrentGait == EMaskedMovementGait::Jogging ? GaitHysteresisBand : 0.0f);

	if (AppliedMovementGait == EMaskedMovementGait::Sprinting && HorizontalSpeed > SprintThreshold)
	{
		NewGait = EMaskedMovementGait::Sprinting;
	}
	else if (HorizontalSpeed > JogThreshold)
	{
		NewGait = EMaskedMovementGait::Jogging;
	}

	if (CurrentGait != NewGait)
	{
		const EMaskedMovementGait PreviousGait = CurrentGait;
		CurrentGait = NewGait;
		OnMovementGaitChanged.Broadcast(PreviousGait, CurrentGait);
	}
}

void AmaskedCharacter::UpdateRotationPolicy(float DeltaSeconds)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	const float HorizontalSpeed = GetVelocity().Size2D();
	const bool bHasMovementInput = !MovementInput.IsNearlyZero();
	const bool bCanTurnInPlace = MovementComponent->IsMovingOnGround()
		&& !bHasMovementInput
		&& HorizontalSpeed <= TurnInPlaceMaxSpeed
		&& !bIsCrouched
		&& CurrentStairTraversalAlpha < 0.05f
		&& GetController();

	if (bTurningInPlace)
	{
		if (!bCanTurnInPlace)
		{
			bTurningInPlace = false;
			TurnInPlaceRequestedAngle = 0.0f;
			TurnInPlaceElapsedTime = 0.0f;
		}
		else
		{
			TurnInPlaceElapsedTime += DeltaSeconds;
			const float LinearAlpha = FMath::Clamp(
				TurnInPlaceElapsedTime / FMath::Max(TurnInPlaceDuration, KINDA_SMALL_NUMBER),
				0.0f,
				1.0f);
			const float SmoothAlpha = LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);
			FRotator NewRotation = GetActorRotation();
			NewRotation.Yaw = FMath::UnwindDegrees(
				TurnInPlaceStartYaw + TurnInPlaceRequestedAngle * SmoothAlpha);
			SetActorRotation(NewRotation);
			MovementComponent->bOrientRotationToMovement = false;
			MovementComponent->bUseControllerDesiredRotation = false;

			if (LinearAlpha >= 1.0f)
			{
				bTurningInPlace = false;
				TurnInPlaceRequestedAngle = 0.0f;
				TurnInPlaceIntentTime = 0.0f;
			}
			return;
		}
	}

	if (bCanTurnInPlace)
	{
		const float DesiredYawDelta = FMath::FindDeltaAngleDegrees(
			GetActorRotation().Yaw,
			GetController()->GetControlRotation().Yaw);
		if (FMath::Abs(DesiredYawDelta) >= TurnInPlaceStartAngle)
		{
			TurnInPlaceIntentTime += DeltaSeconds;
			// Aiming fires the planted turn immediately so continuous camera spins
			// still produce in-place foot steps instead of a rigid idle rotation.
			const float EffectiveTurnInPlaceDelay = CurrentAimAlpha > 0.05f ? 0.0f : TurnInPlaceDelay;
			if (TurnInPlaceIntentTime >= EffectiveTurnInPlaceDelay)
			{
				const float AbsoluteAngle = FMath::Abs(DesiredYawDelta);
				const float AuthoredLength = AbsoluteAngle <= 67.5f
					? 1.6667f
					: (AbsoluteAngle <= 157.5f ? 2.0f : 2.1667f);
				bTurningInPlace = true;
				TurnInPlaceRequestedAngle = FMath::Clamp(DesiredYawDelta, -180.0f, 180.0f);
				TurnInPlaceStartYaw = GetActorRotation().Yaw;
				TurnInPlaceElapsedTime = 0.0f;
				TurnInPlaceDuration = AuthoredLength / FMath::Max(TurnInPlacePlayRate, 0.1f);
				MovementComponent->bOrientRotationToMovement = false;
				MovementComponent->bUseControllerDesiredRotation = false;
				return;
			}
		}
		else
		{
			TurnInPlaceIntentTime = 0.0f;
		}
	}
	else
	{
		TurnInPlaceIntentTime = 0.0f;
	}

	const bool bSideOrBackwardIntent = MovementInput.Y < 0.65f || FMath::Abs(MovementInput.X) > 0.35f;
	// Free locomotion faces the travel direction, including side and backward
	// starts. Crouch keeps controller-facing strafing because it has a full
	// authored eight-way animation set; aiming also faces the controller so the
	// body tracks the camera while strafing.
	const bool bUseLowSpeedStrafe = bIsCrouched
		&& MovementComponent->IsMovingOnGround()
		&& AppliedMovementGait != EMaskedMovementGait::Sprinting
		&& bHasMovementInput
		&& HorizontalSpeed <= LowSpeedStrafeMaxSpeed
		&& bSideOrBackwardIntent;
	const bool bAimingStance = CurrentAimAlpha > 0.05f;
	const bool bUseControllerFacing = bUseLowSpeedStrafe || bAimingStance;

	MovementComponent->bOrientRotationToMovement = !bUseControllerFacing;
	MovementComponent->bUseControllerDesiredRotation = bUseControllerFacing;

	const FMaskedGaitSettings& ActiveSettings = bIsCrouched
		? CrouchSettings
		: (AppliedMovementGait == EMaskedMovementGait::Sprinting ? SprintSettings : JogSettings);
	const float SpeedAlpha = FMath::Clamp(
		HorizontalSpeed / FMath::Max(ActiveSettings.MaxSpeed, 1.0f),
		0.0f,
		1.0f);
	const float DynamicRotationRate = FMath::Lerp(LowSpeedRotationRate, ActiveSettings.RotationRate, SpeedAlpha);
	const float StairRotationScale = FMath::Lerp(1.0f, 0.72f, CurrentStairTraversalAlpha);
	const float BaseRotationRate = DynamicRotationRate * StairRotationScale;
	MovementComponent->RotationRate = FRotator(
		0.0f,
		FMath::Lerp(BaseRotationRate, AimRotationRate, CurrentAimAlpha),
		0.0f);
	MovementComponent->AirControl = (MovementComponent->IsFalling() && bAirborneFromSprint)
		|| AppliedMovementGait == EMaskedMovementGait::Sprinting
		? SprintAirControl
		: JogAirControl;
}

void AmaskedCharacter::UpdateLandingRecovery(float DeltaSeconds)
{
	if (LandingRecoveryTimeRemaining <= 0.0f || ActiveLandingRecoveryDuration <= KINDA_SMALL_NUMBER)
	{
		LandingRecoveryTimeRemaining = 0.0f;
		LandingRecoveryAlpha = 0.0f;
		return;
	}

	LandingRecoveryTimeRemaining = FMath::Max(0.0f, LandingRecoveryTimeRemaining - DeltaSeconds);
	LandingRecoveryAlpha = FMath::Clamp(
		LandingRecoveryTimeRemaining / ActiveLandingRecoveryDuration,
		0.0f,
		1.0f);
}

void AmaskedCharacter::ApplyLandingRecoverySettings()
{
	if (LandingRecoveryAlpha <= 0.0f)
	{
		return;
	}

	// SmoothStep gives the initial contact visible weight without leaving a hard
	// speed discontinuity at the end of recovery.
	const float RecoveryProgress = 1.0f - LandingRecoveryAlpha;
	const float SmoothedProgress = RecoveryProgress * RecoveryProgress * (3.0f - 2.0f * RecoveryProgress);
	const float MovementScale = FMath::Lerp(ActiveLandingMovementScale, 1.0f, SmoothedProgress);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed *= MovementScale;
	MovementComponent->MaxAcceleration *= MovementScale;
}

void AmaskedCharacter::BeginLandingRecovery(float ImpactSpeed)
{
	LastLandingImpactSpeed = ImpactSpeed;
	const bool bRoutineJumpLanding = bJumpInitiatedByInput
		&& ImpactSpeed <= RoutineJumpLandingMaxImpactSpeed;
	LastLandingSeverity = bRoutineJumpLanding
		? EMaskedLandingSeverity::None
		: ClassifyLanding(ImpactSpeed);

	const bool bMovingLanding = LastLandingHorizontalSpeed >= MovingLandingRecoverySpeedThreshold;
	switch (LastLandingSeverity)
	{
	case EMaskedLandingSeverity::Light:
		ActiveLandingRecoveryDuration = bMovingLanding
			? MovingLightLandingRecoveryDuration
			: LightLandingRecoveryDuration;
		ActiveLandingMovementScale = bMovingLanding
			? MovingLightLandingMovementScale
			: LightLandingMovementScale;
		break;
	case EMaskedLandingSeverity::Medium:
		ActiveLandingRecoveryDuration = bMovingLanding
			? MovingMediumLandingRecoveryDuration
			: MediumLandingRecoveryDuration;
		ActiveLandingMovementScale = bMovingLanding
			? MovingMediumLandingMovementScale
			: MediumLandingMovementScale;
		break;
	case EMaskedLandingSeverity::Heavy:
		ActiveLandingRecoveryDuration = bMovingLanding
			? MovingHeavyLandingRecoveryDuration
			: HeavyLandingRecoveryDuration;
		ActiveLandingMovementScale = bMovingLanding
			? MovingHeavyLandingMovementScale
			: HeavyLandingMovementScale;
		break;
	default:
		ActiveLandingRecoveryDuration = 0.0f;
		ActiveLandingMovementScale = 1.0f;
		break;
	}

	LandingRecoveryTimeRemaining = ActiveLandingRecoveryDuration;
	LandingRecoveryAlpha = ActiveLandingRecoveryDuration > KINDA_SMALL_NUMBER ? 1.0f : 0.0f;
}

EMaskedLandingSeverity AmaskedCharacter::ClassifyLanding(float ImpactSpeed) const
{
	if (ImpactSpeed >= HeavyLandingImpactSpeed)
	{
		return EMaskedLandingSeverity::Heavy;
	}
	if (ImpactSpeed >= MediumLandingImpactSpeed)
	{
		return EMaskedLandingSeverity::Medium;
	}
	if (ImpactSpeed >= LightLandingImpactSpeed)
	{
		return EMaskedLandingSeverity::Light;
	}
	return EMaskedLandingSeverity::None;
}

void AmaskedCharacter::UpdateTurnLean(float DeltaSeconds)
{
	const float HorizontalSpeed = GetVelocity().Size2D();
	float DesiredLeanAngle = 0.0f;

	if (!bIsCrouched
		&& GetCharacterMovement()->IsMovingOnGround()
		&& HorizontalSpeed > 80.0f
		&& !MovementInput.IsNearlyZero())
	{
		const FVector VelocityDirection = GetVelocity().GetSafeNormal2D();
		const FVector DesiredDirection = GetDesiredWorldMovementDirection();
		const float TurnDirection = FVector::CrossProduct(VelocityDirection, DesiredDirection).Z;
		const float SpeedAlpha = FMath::Clamp(HorizontalSpeed / FMath::Max(SprintSettings.MaxSpeed, 1.0f), 0.0f, 1.0f);
		const float StairLeanScale = 1.0f - 0.7f * CurrentStairTraversalAlpha;
		DesiredLeanAngle = TurnDirection * MaxTurnLeanAngle * SpeedAlpha * StairLeanScale;
	}

	CurrentTurnLeanAngle = FMath::FInterpTo(
		CurrentTurnLeanAngle,
		DesiredLeanAngle,
		DeltaSeconds,
		TurnLeanInterpSpeed);

	FRotator LeanedMeshRotation = MeshBaseRelativeRotation;
	LeanedMeshRotation.Roll += CurrentTurnLeanAngle;
	GetMesh()->SetRelativeRotation(LeanedMeshRotation);
}

void AmaskedCharacter::UpdateCameraResponse(float DeltaSeconds)
{
	const bool bUseSprintCamera = CurrentGait == EMaskedMovementGait::Sprinting && !bBrakingFromSprint;
	const float BaseTargetFieldOfView = bUseSprintCamera ? SprintFieldOfView : DefaultFieldOfView;
	// Aim narrows the field of view and pulls the camera in over the shoulder.
	const float TargetFieldOfView = FMath::Lerp(BaseTargetFieldOfView, AimFieldOfView, CurrentAimAlpha);
	const float GroundCameraLagSpeed = bUseSprintCamera ? SprintCameraLagSpeed : StandardCameraLagSpeed;
	const float TargetCameraLagSpeed = FMath::Lerp(
		GroundCameraLagSpeed,
		StairCameraLagSpeed,
		CurrentStairTraversalAlpha);

	FollowCamera->SetFieldOfView(FMath::FInterpTo(
		FollowCamera->FieldOfView,
		TargetFieldOfView,
		DeltaSeconds,
		FieldOfViewInterpSpeed));
	const float TargetArmLength = FMath::Lerp(
		CameraBoomBaseTargetArmLength,
		AimCameraTargetArmLength,
		CurrentAimAlpha);
	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetArmLength,
		DeltaSeconds,
		AimTransitionInterpSpeed);
	const FVector TargetSocketOffset = FMath::Lerp(
		CameraBoomBaseSocketOffset,
		AimCameraSocketOffset,
		CurrentAimAlpha);
	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset,
		TargetSocketOffset,
		DeltaSeconds,
		AimTransitionInterpSpeed);
	CameraBoom->CameraLagSpeed = FMath::FInterpTo(
		CameraBoom->CameraLagSpeed,
		TargetCameraLagSpeed,
		DeltaSeconds,
		FieldOfViewInterpSpeed);
	CameraBoom->CameraLagMaxDistance = FMath::Lerp(
		CameraBoomBaseLagMaxDistance,
		StairCameraLagMaxDistance,
		CurrentStairTraversalAlpha);
}

void AmaskedCharacter::UpdateAimResponse(float DeltaSeconds)
{
	CurrentAimAlpha = FMath::FInterpTo(
		CurrentAimAlpha,
		bAimRequested ? 1.0f : 0.0f,
		DeltaSeconds,
		AimTransitionInterpSpeed);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent->IsMovingOnGround() && CurrentAimAlpha > KINDA_SMALL_NUMBER)
	{
		// Aiming caps forward speed and acceleration to a controlled walk while
		// still allowing terrain and stair modifiers to slow it further (Min, not set).
		const float AimSpeedLimit = FMath::Lerp(JogSettings.MaxSpeed, AimMaxWalkSpeed, CurrentAimAlpha);
		MovementComponent->MaxWalkSpeed = FMath::Min(MovementComponent->MaxWalkSpeed, AimSpeedLimit);
		MovementComponent->MaxWalkSpeedCrouched = FMath::Min(MovementComponent->MaxWalkSpeedCrouched, AimSpeedLimit);

		const float AimAccelerationLimit = FMath::Lerp(JogSettings.MaxAcceleration, AimMaxAcceleration, CurrentAimAlpha);
		MovementComponent->MaxAcceleration = FMath::Min(MovementComponent->MaxAcceleration, AimAccelerationLimit);

		const float AimBrakingLimit = FMath::Lerp(JogSettings.BrakingDeceleration, AimBrakingDeceleration, CurrentAimAlpha);
		MovementComponent->BrakingDecelerationWalking = FMath::Min(MovementComponent->BrakingDecelerationWalking, AimBrakingLimit);
	}
}

void AmaskedCharacter::UpdateCrouchResponse(float DeltaSeconds)
{
	const float CurrentCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	if (PreviousCapsuleHalfHeight <= 0.0f)
	{
		PreviousCapsuleHalfHeight = CurrentCapsuleHalfHeight;
	}

	// Crouch changes capsule height immediately for correct collision. Counter that world-space
	// camera jump, then smoothly release the compensation to the new physical height.
	CameraHeightCompensation += PreviousCapsuleHalfHeight - CurrentCapsuleHalfHeight;
	PreviousCapsuleHalfHeight = CurrentCapsuleHalfHeight;
	CameraHeightCompensation = FMath::FInterpTo(
		CameraHeightCompensation,
		0.0f,
		DeltaSeconds,
		CrouchTransitionInterpSpeed);

	FVector SmoothedBoomLocation = CameraBoomBaseRelativeLocation;
	SmoothedBoomLocation.Z += CameraHeightCompensation + StairVisualHeightOffset;
	CameraBoom->SetRelativeLocation(SmoothedBoomLocation);

	CurrentCrouchAlpha = FMath::FInterpTo(
		CurrentCrouchAlpha,
		bIsCrouched ? 1.0f : 0.0f,
		DeltaSeconds,
		CrouchTransitionInterpSpeed);

	// Anchor the visual correction to the authored standing transform. CharacterMovement
	// adjusts the mesh during capsule changes, so subtracting last frame's correction can
	// retain that transient adjustment and leave the character below the floor on stand-up.
	FVector MeshLocation = GetMesh()->GetRelativeLocation();
	AppliedCrouchVisualMeshOffset = FMath::FInterpTo(
		AppliedCrouchVisualMeshOffset,
		bIsCrouched ? CrouchVisualMeshHeightOffset : 0.0f,
		DeltaSeconds,
		CrouchTransitionInterpSpeed);
	MeshLocation.Z = MeshBaseRelativeLocation.Z + AppliedCrouchVisualMeshOffset + StairVisualHeightOffset;
	GetMesh()->SetRelativeLocation(MeshLocation);
}

void AmaskedCharacter::ApplyGaitSettings(
	EMaskedMovementGait NewAppliedGait,
	const FMaskedGaitSettings& Settings)
{
	AppliedMovementGait = NewAppliedGait;
	CurrentBaseMaxWalkSpeed = Settings.MaxSpeed;
	CurrentBaseMaxAcceleration = Settings.MaxAcceleration;
	CurrentBaseBrakingDeceleration = Settings.BrakingDeceleration;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = CurrentBaseMaxWalkSpeed;
	MovementComponent->MaxWalkSpeedCrouched = CrouchSettings.MaxSpeed;
	MovementComponent->MaxAcceleration = CurrentBaseMaxAcceleration;
	MovementComponent->BrakingDecelerationWalking = CurrentBaseBrakingDeceleration;
	MovementComponent->BrakingFriction = StandardBrakingFriction;
	MovementComponent->RotationRate = FRotator(0.0f, Settings.RotationRate, 0.0f);
}

void AmaskedCharacter::ApplySprintStopSettings()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = JogSettings.MaxSpeed;
	MovementComponent->MaxAcceleration = JogSettings.MaxAcceleration;
	MovementComponent->BrakingDecelerationWalking = SprintStopBrakingDeceleration;
	MovementComponent->BrakingFriction = SprintStopBrakingFriction;
	MovementComponent->RotationRate = FRotator(0.0f, JogSettings.RotationRate, 0.0f);
}

void AmaskedCharacter::ApplyLegacyStairSettingMigration()
{
	// BP_ThirdPersonCharacter serialized the first stair pass. Replace those exact
	// legacy values so a stale Blueprint CDO cannot keep the jagged climb.
	if (FMath::IsNearlyEqual(AscendingStairMaxSpeed, 210.0f)
		|| FMath::IsNearlyEqual(AscendingStairMaxSpeed, 165.0f)
		|| FMath::IsNearlyEqual(AscendingStairMaxSpeed, 120.0f))
	{
		AscendingStairMaxSpeed = 155.0f;
	}
	if (FMath::IsNearlyEqual(DescendingStairMaxSpeed, 240.0f)
		|| FMath::IsNearlyEqual(DescendingStairMaxSpeed, 185.0f)
		|| FMath::IsNearlyEqual(DescendingStairMaxSpeed, 135.0f))
	{
		DescendingStairMaxSpeed = 175.0f;
	}
	if (FMath::IsNearlyEqual(MinimumAdaptiveStairSpeed, 65.0f))
	{
		MinimumAdaptiveStairSpeed = 90.0f;
	}
	if (FMath::IsNearlyEqual(AscendingStairFootfallCadence, 2.40f, 0.001f))
	{
		AscendingStairFootfallCadence = 4.25f;
	}
	if (FMath::IsNearlyEqual(DescendingStairFootfallCadence, 2.60f, 0.001f))
	{
		DescendingStairFootfallCadence = 4.60f;
	}
	if (FMath::IsNearlyEqual(StairAccelerationScale, 0.65f, 0.001f)
		|| FMath::IsNearlyEqual(StairAccelerationScale, 0.52f, 0.001f))
	{
		StairAccelerationScale = 0.68f;
	}
	if (FMath::IsNearlyEqual(StairBrakingScale, 1.35f, 0.001f)
		|| FMath::IsNearlyEqual(StairBrakingScale, 1.50f, 0.001f))
	{
		StairBrakingScale = 1.40f;
	}
	if (FMath::IsNearlyEqual(StairResponseHoldDuration, 0.16f, 0.01f)
		|| FMath::IsNearlyEqual(StairResponseHoldDuration, 0.20f, 0.01f))
	{
		StairResponseHoldDuration = 0.42f;
	}
	if (FMath::IsNearlyEqual(StairResponseInterpSpeed, 14.0f))
	{
		StairResponseInterpSpeed = 12.0f;
	}
	if (FMath::IsNearlyEqual(StairVisualSmoothTime, 0.24f, 0.001f))
	{
		StairVisualSmoothTime = 0.14f;
	}
	if (FMath::IsNearlyEqual(StairVisualDescendSmoothTime, 0.16f, 0.001f))
	{
		StairVisualDescendSmoothTime = 0.12f;
	}
	if (FMath::IsNearlyEqual(StairVisualCatchupTime, 0.05f, 0.001f))
	{
		StairVisualCatchupTime = 0.14f;
	}
}

void AmaskedCharacter::ApplyGaitSettingsSmoothed(
	EMaskedMovementGait NewAppliedGait,
	const FMaskedGaitSettings& Settings,
	float DeltaSeconds)
{
	AppliedMovementGait = NewAppliedGait;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	CurrentBaseMaxWalkSpeed = FMath::FInterpTo(
		CurrentBaseMaxWalkSpeed,
		Settings.MaxSpeed,
		DeltaSeconds,
		GaitTransitionInterpSpeed);
	MovementComponent->MaxWalkSpeed = CurrentBaseMaxWalkSpeed;
	MovementComponent->MaxWalkSpeedCrouched = CrouchSettings.MaxSpeed;
	CurrentBaseMaxAcceleration = FMath::FInterpTo(
		CurrentBaseMaxAcceleration,
		Settings.MaxAcceleration,
		DeltaSeconds,
		GaitTransitionInterpSpeed);
	MovementComponent->MaxAcceleration = CurrentBaseMaxAcceleration;
	CurrentBaseBrakingDeceleration = FMath::FInterpTo(
		CurrentBaseBrakingDeceleration,
		Settings.BrakingDeceleration,
		DeltaSeconds,
		GaitTransitionInterpSpeed);
	MovementComponent->BrakingDecelerationWalking = CurrentBaseBrakingDeceleration;
	MovementComponent->BrakingFriction = StandardBrakingFriction;
}
