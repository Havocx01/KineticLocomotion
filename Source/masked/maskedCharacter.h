// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "maskedCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EMaskedMovementGait : uint8
{
	Walking,
	Jogging,
	Sprinting
};

UENUM(BlueprintType)
enum class EMaskedLandingSeverity : uint8
{
	None,
	Light,
	Medium,
	Heavy
};

USTRUCT(BlueprintType)
struct FMaskedGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0", Units="cm/s"))
	float MaxSpeed = 380.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0", Units="cm/s^2"))
	float MaxAcceleration = 850.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0", Units="cm/s^2"))
	float BrakingDeceleration = 950.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(ClampMin="0.0", Units="deg/s"))
	float RotationRate = 360.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMaskedMovementGaitChanged,
	EMaskedMovementGait, PreviousGait,
	EMaskedMovementGait, NewGait);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FMaskedCharacterLanded,
	EMaskedLandingSeverity, Severity,
	float, ImpactSpeed);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AmaskedCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Hold Input Action used to request sprinting. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	/** Toggle Input Action used to enter or leave crouch. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Hold Input Action used to enter aim-down-sights. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;

	/** Movement values used during ordinary free locomotion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait")
	FMaskedGaitSettings JogSettings;

	/** Movement values used while sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait")
	FMaskedGaitSettings SprintSettings;

	/** Movement values used while crouched. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait")
	FMaskedGaitSettings CrouchSettings;

	/** Speeds at or below this value are classified as walking for animation and gameplay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait", meta=(ClampMin="0.0", Units="cm/s"))
	float WalkSpeedThreshold = 200.0f;

	/** Forward-biased sprint input above this point receives deliberate lateral steering reduction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SprintForwardInputThreshold = 0.55f;

	/** Reduces lateral input while sprinting so sprint turns stay broad and deliberate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SprintLateralInputScale = 0.45f;

	/** Below this speed, crouch side/back input remains controller-facing for its directional loops. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Rotation", meta=(ClampMin="0.0", Units="cm/s"))
	float LowSpeedStrafeMaxSpeed = 230.0f;

	/** Responsive low-speed rotation, blended down toward the active gait rate as speed rises. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Rotation", meta=(ClampMin="0.0", Units="deg/s"))
	float LowSpeedRotationRate = 540.0f;

	/** How quickly ordinary gait values blend between jogging and sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Gait", meta=(ClampMin="0.0"))
	float GaitTransitionInterpSpeed = 6.0f;

	/** Direction change begins to plant and brake beyond this velocity/input angle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float DirectionChangeStartAngle = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float DirectionChangeFullAngle = 165.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DirectionChangeAccelerationScale = 0.38f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="1.0"))
	float DirectionChangeBrakingScale = 2.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.1", ClampMax="1.0"))
	float DirectionChangeSpeedScale = 0.72f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float SprintSharpTurnExitAngle = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0"))
	float DirectionChangeInterpSpeed = 10.0f;

	/** Idle camera yaw required before a deliberate planted turn begins. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float TurnInPlaceStartAngle = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", Units="cm/s"))
	float TurnInPlaceMaxSpeed = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.0", Units="s"))
	float TurnInPlaceDelay = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Turning", meta=(ClampMin="0.1"))
	float TurnInPlacePlayRate = 1.50f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JogAirControl = 0.16f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SprintAirControl = 0.08f;

	/** Temporary master switch for player-triggered jumping. Ledge falls remain unaffected. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Air")
	bool bJumpingEnabled = false;

	/** Steepest surface treated as normal walkable ground. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.0", ClampMax="89.0", Units="deg"))
	float MaxWalkableSlopeAngle = 46.0f;

	/** Maximum ledge or stair riser the capsule can step onto without jumping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.0", Units="cm"))
	float TerrainMaxStepHeight = 42.0f;

	/** Keeps the capsule from balancing on a very thin sliver at a ledge. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.0", Units="cm"))
	float TerrainPerchRadiusThreshold = 14.0f;

	/** Slopes below this angle are treated as flat to avoid noisy response at mesh seams. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.0", ClampMax="45.0", Units="deg"))
	float MinimumSlopeResponseAngle = 4.0f;

	/** Speed retained when travelling directly up the steepest walkable slope. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.1", ClampMax="1.0"))
	float UphillSpeedScaleAtLimit = 0.72f;

	/** Small controlled speed gain when travelling directly down a steep slope. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="1.0", ClampMax="1.5"))
	float DownhillSpeedScaleAtLimit = 1.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.1", ClampMax="1.0"))
	float UphillAccelerationScaleAtLimit = 0.82f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="1.0", ClampMax="1.5"))
	float DownhillAccelerationScaleAtLimit = 1.04f;

	/** Extra stopping authority prevents downhill movement from feeling slippery. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="1.0", ClampMax="2.0"))
	float DownhillBrakingScaleAtLimit = 1.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain", meta=(ClampMin="0.0"))
	float TerrainResponseInterpSpeed = 7.0f;

	/** Minimum one-frame height change considered an actual stair rather than floor noise. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm"))
	float MinimumDetectedStepHeight = 4.0f;

	/** Hard upper bound; the live tread-depth cadence normally selects a lower value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm/s"))
	float AscendingStairMaxSpeed = 155.0f;

	/** Descending stays a walk: faster than climbing, never a jog down the flight. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm/s"))
	float DescendingStairMaxSpeed = 175.0f;

	/** Controlled stair run: deliberately below the 620 cm/s flat-ground sprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="0.0", Units="cm/s"))
	float AscendingStairSprintMaxSpeed = 290.0f;

	/** Descending receives a small gravity-assisted gain without becoming reckless. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="0.0", Units="cm/s"))
	float DescendingStairSprintMaxSpeed = 320.0f;

	/** Lowest allowed stair speed on unusually narrow treads. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm/s"))
	float MinimumAdaptiveStairSpeed = 90.0f;

	/** Used until two consecutive risers provide an actual tread-depth measurement. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="1.0", Units="cm"))
	float DefaultStairTreadDepth = 38.0f;

	/** Target foot contacts per second; speed is tread depth multiplied by this cadence. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.5", ClampMax="5.0"))
	float AscendingStairFootfallCadence = 4.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.5", ClampMax="5.0"))
	float DescendingStairFootfallCadence = 4.60f;

	/** Faster physical cadence used only while sprint is held on a flight. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="1.0", ClampMax="12.0"))
	float AscendingStairSprintFootfallCadence = 7.40f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="1.0", ClampMax="12.0"))
	float DescendingStairSprintFootfallCadence = 8.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="0.0", Units="cm/s"))
	float MinimumAdaptiveStairSprintSpeed = 220.0f;

	/** Detects the next riser early enough to decelerate before the capsule steps up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="10.0", Units="cm"))
	float StairLookAheadDistance = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.1", ClampMax="1.0"))
	float StairAccelerationScale = 0.68f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="1.0", ClampMax="2.0"))
	float StairBrakingScale = 1.40f;

	/** Keeps stair sprint responsive without reproducing flat-ground launch acceleration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="0.1", ClampMax="1.0"))
	float StairSprintAccelerationScale = 0.58f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs|Sprint", meta=(ClampMin="1.0", ClampMax="2.0"))
	float StairSprintBrakingScale = 1.25f;

	/** Bridges the gap between treads so a whole flight reads as one traversal. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="s"))
	float StairResponseHoldDuration = 0.42f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0"))
	float StairResponseInterpSpeed = 12.0f;

	/** Slower exit keeps visual smoothing alive through the last tread and landing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0"))
	float StairResponseExitInterpSpeed = 4.0f;

	/** Critically damped time for the mesh and camera to absorb an upward capsule step. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.01", Units="s"))
	float StairVisualSmoothTime = 0.14f;

	/** Downward steps settle faster so the body does not hover above the lower tread. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.01", Units="s"))
	float StairVisualDescendSmoothTime = 0.12f;

	/** Re-sync speed used on flat ground after leaving a staircase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.01", Units="s"))
	float StairVisualCatchupTime = 0.14f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm"))
	float MaxStairVisualOffset = 40.0f;

	/** Extra perch while on treads prevents one-frame ledge drops between steps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm"))
	float StairGroundPerchRadius = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm"))
	float StairGroundPerchAdditionalHeight = 16.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0"))
	float StairCameraLagSpeed = 6.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Terrain|Stairs", meta=(ClampMin="0.0", Units="cm"))
	float StairCameraLagMaxDistance = 50.0f;

	/** Starts the live movement diagnostics automatically; F6 can always toggle it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Debug")
	bool bMovementDebugEnabledByDefault = false;

	/** Length multiplier for the velocity and acceleration arrows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Debug", meta=(ClampMin="0.0"))
	float MovementDebugVectorScale = 0.30f;

	/** Downward speed that begins a visible light landing response. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float LightLandingImpactSpeed = 350.0f;

	/** Downward speed that begins a movement-affecting medium landing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float MediumLandingImpactSpeed = 700.0f;

	/** Downward speed that begins a committed heavy landing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float HeavyLandingImpactSpeed = 950.0f;

	/** Intentional jumps landing below this speed return directly to locomotion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float RoutineJumpLandingMaxImpactSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="s"))
	float LightLandingRecoveryDuration = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="s"))
	float MediumLandingRecoveryDuration = 0.32f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", Units="s"))
	float HeavyLandingRecoveryDuration = 0.65f;

	/** Initial movement authority retained after each landing category. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LightLandingMovementScale = 0.90f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MediumLandingMovementScale = 0.68f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HeavyLandingMovementScale = 0.38f;

	/** Horizontal approach speed that switches recovery from a planted landing to a moving landing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", Units="cm/s"))
	float MovingLandingRecoverySpeedThreshold = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", Units="s"))
	float MovingLightLandingRecoveryDuration = 0.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", Units="s"))
	float MovingMediumLandingRecoveryDuration = 0.16f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", Units="s"))
	float MovingHeavyLandingRecoveryDuration = 0.34f;

	/** Moving landings retain momentum instead of planting the capsule beneath an advancing animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MovingLightLandingMovementScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MovingMediumLandingMovementScale = 0.92f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Landing|Moving", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MovingHeavyLandingMovementScale = 0.72f;

	/** Maximum visual roll applied to the mesh while changing direction at speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Lean", meta=(ClampMin="0.0", Units="deg"))
	float MaxTurnLeanAngle = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Lean", meta=(ClampMin="0.0"))
	float TurnLeanInterpSpeed = 7.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="5.0", ClampMax="170.0", Units="deg"))
	float DefaultFieldOfView = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="5.0", ClampMax="170.0", Units="deg"))
	float SprintFieldOfView = 96.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0"))
	float FieldOfViewInterpSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0"))
	float StandardCameraLagSpeed = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0"))
	float SprintCameraLagSpeed = 8.0f;

	/** Base third-person follow distance, held close like GTA/RDR2. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera", meta=(ClampMin="0.0", Units="cm"))
	float DefaultCameraTargetArmLength = 300.0f;

	/** Speed used to visually absorb the capsule height change when crouching or standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Crouch", meta=(ClampMin="0.0"))
	float CrouchTransitionInterpSpeed = 10.0f;

	/** Collision capsule half-height while crouched. Standing remains ceiling-safe through CharacterMovement. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Crouch", meta=(ClampMin="0.0", Units="cm"))
	float CrouchedCapsuleHalfHeight = 65.0f;

	/** Compensates vertical pelvis translation in retargeted crouch clips while preserving collision height. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Crouch", meta=(Units="cm"))
	float CrouchVisualMeshHeightOffset = 0.0f;

	/** Braking used when all movement input is released at sprint speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Stopping", meta=(ClampMin="0.0", Units="cm/s^2"))
	float SprintStopBrakingDeceleration = 1600.0f;

	/** Extra friction used only during a sprint stop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Stopping", meta=(ClampMin="0.0"))
	float SprintStopBrakingFriction = 5.0f;

	/** Normal braking friction restored after the sprint-stop phase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Stopping", meta=(ClampMin="0.0"))
	float StandardBrakingFriction = 3.0f;

	/** Speed cap while aiming; aiming is a deliberate walk, never a run or sprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Aim", meta=(ClampMin="0.0", Units="cm/s"))
	float AimMaxWalkSpeed = 200.0f;

	/** Acceleration while aiming; a deliberate ramp rather than a jog launch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Aim", meta=(ClampMin="0.0", Units="cm/s^2"))
	float AimMaxAcceleration = 500.0f;

	/** Braking while aiming; snappier stops keep strafing precise. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Aim", meta=(ClampMin="0.0", Units="cm/s^2"))
	float AimBrakingDeceleration = 1100.0f;

	/** Responsive facing rate while aiming so the body tracks the camera direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Aim", meta=(ClampMin="0.0", Units="deg/s"))
	float AimRotationRate = 540.0f;

	/** How quickly the aim stance and its movement/camera effects blend in and out. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Aim", meta=(ClampMin="0.0"))
	float AimTransitionInterpSpeed = 12.0f;

	/** Over-the-shoulder camera distance while aiming. Kept reasonably far back. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim", meta=(ClampMin="0.0", Units="cm"))
	float AimCameraTargetArmLength = 250.0f;

	/** Lateral over-the-shoulder camera offset while aiming (right shoulder, positive Y). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim")
	FVector AimCameraSocketOffset = FVector(0.0f, 40.0f, 15.0f);

	/** Narrower field of view applied while aiming. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim", meta=(ClampMin="5.0", ClampMax="170.0", Units="deg"))
	float AimFieldOfView = 70.0f;

	/** Gait derived from physical speed rather than directly from the pressed key. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Movement|Gait")
	EMaskedMovementGait CurrentGait = EMaskedMovementGait::Walking;

	/** True while the owning player is holding the sprint input. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Movement|Gait")
	bool bSprintRequested = false;

	/** True during the short physical plant that follows an abrupt sprint stop. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Movement|Stopping")
	bool bBrakingFromSprint = false;

	/** True while the owning player is holding the aim input. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Movement|Aim")
	bool bAimRequested = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Movement|Turning")
	bool bTurningInPlace = false;

	bool bSprintTurnSuppressed = false;
	float CurrentDirectionChangeAlpha = 0.0f;
	float CurrentDirectionChangeAngle = 0.0f;
	float CurrentSignedDirectionChangeAngle = 0.0f;
	float TurnInPlaceRequestedAngle = 0.0f;
	float TurnInPlaceStartYaw = 0.0f;
	float TurnInPlaceElapsedTime = 0.0f;
	float TurnInPlaceDuration = 0.0f;
	float TurnInPlaceIntentTime = 0.0f;

	EMaskedMovementGait AppliedMovementGait = EMaskedMovementGait::Jogging;
	FVector2D MovementInput = FVector2D::ZeroVector;
	/** Unscaled gait values; terrain and direction modifiers compose from these each frame. */
	float CurrentBaseMaxWalkSpeed = 380.0f;
	float CurrentBaseMaxAcceleration = 850.0f;
	float CurrentBaseBrakingDeceleration = 950.0f;
	FRotator MeshBaseRelativeRotation = FRotator::ZeroRotator;
	FVector MeshBaseRelativeLocation = FVector::ZeroVector;
	FVector CameraBoomBaseRelativeLocation = FVector::ZeroVector;
	float CameraBoomBaseLagMaxDistance = 25.0f;
	float CurrentTurnLeanAngle = 0.0f;
	float PreviousCapsuleHalfHeight = 0.0f;
	float CameraHeightCompensation = 0.0f;
	float CurrentCrouchAlpha = 0.0f;
	float AppliedCrouchVisualMeshOffset = 0.0f;
	float CurrentAimAlpha = 0.0f;
	float CameraBoomBaseTargetArmLength = 400.0f;
	FVector CameraBoomBaseSocketOffset = FVector::ZeroVector;
	float CurrentTerrainSpeedScale = 1.0f;
	float CurrentTerrainAccelerationScale = 1.0f;
	float CurrentTerrainBrakingScale = 1.0f;
	float CurrentSignedGroundSlopeAngle = 0.0f;
	FVector PreviousTerrainActorLocation = FVector::ZeroVector;
	float CurrentStairTraversalAlpha = 0.0f;
	float StairResponseTimeRemaining = 0.0f;
	float LastDetectedStepHeight = 0.0f;
	float HorizontalDistanceSinceStairStep = 0.0f;
	float DetectedStairTreadDepth = 38.0f;
	float CurrentAdaptiveStairSpeedLimit = 0.0f;
	bool bHasDetectedStairStep = false;
	int32 StairStepSerial = 0;
	bool bLastStairStepLeftFoot = false;
	bool bNextStairStepLeftFoot = false;
	float SmoothedVisualWorldZ = 0.0f;
	float StairVisualZVelocity = 0.0f;
	float StairVisualHeightOffset = 0.0f;
	bool bStairVisualHeightInitialized = false;
	bool bAscendingStairs = false;
	float PeakDownwardSpeed = 0.0f;
	float LandingRecoveryTimeRemaining = 0.0f;
	float ActiveLandingRecoveryDuration = 0.0f;
	float ActiveLandingMovementScale = 1.0f;
	float LastLandingImpactSpeed = 0.0f;
	float LastLandingHorizontalSpeed = 0.0f;
	float LastAirborneHorizontalSpeed = 0.0f;
	float LandingRecoveryAlpha = 0.0f;
	EMaskedLandingSeverity LastLandingSeverity = EMaskedLandingSeverity::None;
	bool bAirborneFromSprint = false;
	bool bJumpInitiatedByInput = false;
	bool bMovementDebugEnabled = false;

public:

	/** Constructor */
	AmaskedCharacter();	

protected:
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void Landed(const FHitResult& Hit) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Keeps movement policy synchronized with input, terrain state, and physical speed. */
	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	void StopMove();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	bool CanSprint() const;
	FVector GetDesiredWorldMovementDirection() const;
	void UpdateMovementPolicy(float DeltaSeconds);
	void UpdateDirectionChangePolicy(float DeltaSeconds);
	void ConfigureTerrainMovement();
	void UpdateTerrainResponse(float DeltaSeconds);
	void UpdateStairTraversal(float DeltaSeconds);
	void UpdateMovementDebug() const;
	void UpdateCurrentGait();
	void UpdateRotationPolicy(float DeltaSeconds);
	void UpdateTurnLean(float DeltaSeconds);
	void UpdateCameraResponse(float DeltaSeconds);
	void UpdateCrouchResponse(float DeltaSeconds);
	void UpdateAimResponse(float DeltaSeconds);
	void UpdateLandingRecovery(float DeltaSeconds);
	void ApplyLandingRecoverySettings();
	void BeginLandingRecovery(float ImpactSpeed);
	EMaskedLandingSeverity ClassifyLanding(float ImpactSpeed) const;
	void ApplyGaitSettings(EMaskedMovementGait NewAppliedGait, const FMaskedGaitSettings& Settings);
	void ApplyGaitSettingsSmoothed(EMaskedMovementGait NewAppliedGait, const FMaskedGaitSettings& Settings, float DeltaSeconds);
	void ApplySprintStopSettings();
	void ApplyLegacyStairSettingMigration();

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Begins requesting a sprint. Eligibility is still decided by the movement policy. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	/** Stops requesting a sprint. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

	/** Toggles crouch. Native standing clearance checks prevent uncrouching into a ceiling. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchToggle();

	/** Begins aiming down sights. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimStart();

	/** Stops aiming down sights. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimEnd();

	/** Toggles the live movement diagnostics overlay and world-space vectors. */
	UFUNCTION(BlueprintCallable, Category="Movement|Debug")
	void ToggleMovementDebug();

	/** Broadcast when physical speed crosses a gait boundary. */
	UPROPERTY(BlueprintAssignable, Category="Movement|Gait")
	FMaskedMovementGaitChanged OnMovementGaitChanged;

	/** Broadcast once per physical landing with the pre-impact downward speed. */
	UPROPERTY(BlueprintAssignable, Category="Movement|Landing")
	FMaskedCharacterLanded OnCharacterLanded;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns the gait derived from the character's physical velocity. */
	UFUNCTION(BlueprintPure, Category="Movement|Gait")
	EMaskedMovementGait GetCurrentMovementGait() const { return CurrentGait; }

	/** True while the character is physically planting after an abrupt sprint stop. */
	UFUNCTION(BlueprintPure, Category="Movement|Stopping")
	bool IsBrakingFromSprint() const { return bBrakingFromSprint; }

	UFUNCTION(BlueprintPure, Category="Movement|Turning")
	bool IsTurningInPlace() const { return bTurningInPlace; }

	UFUNCTION(BlueprintPure, Category="Movement|Turning")
	float GetTurnInPlaceRequestedAngle() const { return TurnInPlaceRequestedAngle; }

	UFUNCTION(BlueprintPure, Category="Movement|Turning")
	float GetDirectionChangeAlpha() const { return CurrentDirectionChangeAlpha; }

	UFUNCTION(BlueprintPure, Category="Movement|Turning")
	float GetSignedDirectionChangeAngle() const { return CurrentSignedDirectionChangeAngle; }

	/** Signed visual turn lean: negative left, positive right. */
	UFUNCTION(BlueprintPure, Category="Movement|Lean")
	float GetTurnLeanAngle() const { return CurrentTurnLeanAngle; }

	UFUNCTION(BlueprintPure, Category="Movement|Crouch")
	bool IsCharacterCrouched() const { return bIsCrouched; }

	/** Smoothed 0-1 value for crouch animation blending. */
	UFUNCTION(BlueprintPure, Category="Movement|Crouch")
	float GetCrouchAlpha() const { return CurrentCrouchAlpha; }

	/** True while the owning player is holding the aim input. */
	UFUNCTION(BlueprintPure, Category="Movement|Aim")
	bool IsAiming() const { return bAimRequested; }

	/** Smoothed 0-1 value for aim animation blending and camera/movement response. */
	UFUNCTION(BlueprintPure, Category="Movement|Aim")
	float GetAimAlpha() const { return CurrentAimAlpha; }

	UFUNCTION(BlueprintPure, Category="Movement|Gait")
	bool IsSprintRequested() const { return bSprintRequested; }

	UFUNCTION(BlueprintPure, Category="Movement|Landing")
	EMaskedLandingSeverity GetLastLandingSeverity() const { return LastLandingSeverity; }

	UFUNCTION(BlueprintPure, Category="Movement|Landing")
	float GetLastLandingImpactSpeed() const { return LastLandingImpactSpeed; }

	/** Horizontal capsule speed immediately before contact, used to choose the moving landing variant. */
	UFUNCTION(BlueprintPure, Category="Movement|Landing")
	float GetLastLandingHorizontalSpeed() const { return LastLandingHorizontalSpeed; }

	/** One at initial contact and smoothly approaches zero through physical recovery. */
	UFUNCTION(BlueprintPure, Category="Movement|Landing")
	float GetLandingRecoveryAlpha() const { return LandingRecoveryAlpha; }

	UFUNCTION(BlueprintPure, Category="Movement|Landing")
	bool IsRecoveringFromLanding() const { return LandingRecoveryTimeRemaining > 0.0f; }

	/** True only for airborne movement created by the player's jump input, not a ledge drop. */
	UFUNCTION(BlueprintPure, Category="Movement|Air")
	bool IsAirborneFromJumpInput() const { return bJumpInitiatedByInput; }

	/** Positive uphill, negative downhill, measured along the current travel direction. */
	UFUNCTION(BlueprintPure, Category="Movement|Terrain")
	float GetSignedGroundSlopeAngle() const { return CurrentSignedGroundSlopeAngle; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain")
	float GetTerrainSpeedScale() const { return CurrentTerrainSpeedScale; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	bool IsTraversingStairs() const { return CurrentStairTraversalAlpha > 0.05f; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	bool IsAscendingStairs() const { return IsTraversingStairs() && bAscendingStairs; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	float GetStairTraversalAlpha() const { return CurrentStairTraversalAlpha; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	float GetLastDetectedStepHeight() const { return LastDetectedStepHeight; }

	/** Monotonic physical riser count used to synchronize authored foot contacts. */
	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	int32 GetStairStepSerial() const { return StairStepSerial; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	bool WasLastStairStepLeftFoot() const { return bLastStairStepLeftFoot; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	float GetDetectedStairTreadDepth() const { return DetectedStairTreadDepth; }

	UFUNCTION(BlueprintPure, Category="Movement|Terrain|Stairs")
	float GetAdaptiveStairSpeedLimit() const { return CurrentAdaptiveStairSpeedLimit; }
};

