// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../maskedCharacter.h"
#include "MaskedCharacterAnimInstance.generated.h"

class AmaskedCharacter;
class UAnimMontage;
class UAnimSequence;

enum class EMaskedCrouchAnimationMode : uint8
{
	None,
	Entering,
	Idle,
	Walking,
	Exiting,
	Sprinting
};

enum class EMaskedCrouchDirection : uint8
{
	None,
	Forward,
	ForwardRight,
	Right,
	BackwardRight,
	Backward,
	BackwardLeft,
	Left,
	ForwardLeft
};

enum class EMaskedAirAnimationMode : uint8
{
	None,
	JumpStart,
	Falling,
	Landing
};

enum class EMaskedGroundTransitionMode : uint8
{
	None,
	Starting,
	Stopping,
	Pivoting
};

enum class EMaskedLandingLocomotionMode : uint8
{
	Standing,
	Walking,
	Running,
	Sprinting
};

/**
 * Project-owned animation data bridge for the player locomotion graph.
 * Animation decisions stay in the Animation Blueprint; authoritative movement
 * state and physically derived values are gathered here in one place.
 */
UCLASS(Blueprintable, BlueprintType)
class MASKED_API UMaskedCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMaskedCharacterAnimInstance();
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** True when the full independent foot/pelvis placement solver is being applied. */
	UFUNCTION(BlueprintPure, Category="Locomotion|Terrain|Foot IK")
	bool IsFootIKTracingEnabled() const { return bMaskedFootIKTracingEnabled; }

	/** Compact state label consumed by the F6 movement diagnostic overlay. */
	FString GetGroundTransitionDebugName() const;
	FString GetLandingLocomotionDebugName() const;

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|References", meta=(DisplayName="Character Owner"))
	TObjectPtr<AmaskedCharacter> MaskedCharacterOwner;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="World Velocity"))
	FVector MaskedWorldVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="World Acceleration"))
	FVector MaskedWorldAcceleration = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="Ground Speed", Units="cm/s"))
	float MaskedGroundSpeed = 0.0f;

	/** Horizontal speed captured at contact, before ground braking can alter the landing decision. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Landing", meta=(DisplayName="Landing Horizontal Speed", Units="cm/s"))
	float MaskedLandingHorizontalSpeed = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="Vertical Speed", Units="cm/s"))
	float MaskedVerticalSpeed = 0.0f;

	/** Signed local-space travel direction: forward 0, right 90, left -90, backward 180. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="Movement Direction", Units="deg"))
	float MaskedMovementDirection = 0.0f;

	/** Local-space input/acceleration direction used before velocity has built up. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Motion", meta=(DisplayName="Acceleration Direction", Units="deg"))
	float MaskedAccelerationDirection = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Movement Gait"))
	EMaskedMovementGait MaskedMovementGait = EMaskedMovementGait::Walking;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Should Move"))
	bool bMaskedShouldMove = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Accelerating"))
	bool bMaskedIsAccelerating = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Braking"))
	bool bMaskedIsBraking = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Braking From Sprint"))
	bool bMaskedIsBrakingFromSprint = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Falling"))
	bool bMaskedIsFalling = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Is Traversing Stairs"))
	bool bMaskedIsTraversingStairs = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Is Ascending Stairs"))
	bool bMaskedIsAscendingStairs = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Stair Traversal Alpha", ClampMin="0.0", ClampMax="1.0"))
	float MaskedStairTraversalAlpha = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Detected Step Height", Units="cm"))
	float MaskedDetectedStepHeight = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Stair Step Serial"))
	int32 MaskedStairStepSerial = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Stair Step Uses Left Foot"))
	bool bMaskedStairStepLeftFoot = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(DisplayName="Detected Tread Depth", Units="cm"))
	float MaskedDetectedTreadDepth = 38.0f;

	/** Ramps above this angle release the strong height-matching solver. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Terrain|Foot IK", meta=(ClampMin="0.0", ClampMax="45.0", Units="deg"))
	float FootIKRampDisableAngle = 8.0f;

	/** Lower threshold prevents the solver flickering at ramp seams. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Terrain|Foot IK", meta=(ClampMin="0.0", ClampMax="45.0", Units="deg"))
	float FootIKRampReenableAngle = 5.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Terrain|Foot IK", meta=(DisplayName="Foot IK Tracing Enabled"))
	bool bMaskedFootIKTracingEnabled = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Air", meta=(DisplayName="Is Ascending"))
	bool bMaskedIsAscending = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Air", meta=(DisplayName="Is Descending"))
	bool bMaskedIsDescending = false;

	/** Distinguishes a controlled jump arc from an uncontrolled ledge or drop fall. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Air", meta=(DisplayName="Is Routine Jump"))
	bool bMaskedIsRoutineJump = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Air", meta=(DisplayName="Time To Jump Apex", Units="s"))
	float MaskedTimeToJumpApex = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Landing", meta=(DisplayName="Landing Severity"))
	EMaskedLandingSeverity MaskedLandingSeverity = EMaskedLandingSeverity::None;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Landing", meta=(DisplayName="Landing Impact Speed", Units="cm/s"))
	float MaskedLandingImpactSpeed = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Landing", meta=(DisplayName="Landing Recovery Alpha", ClampMin="0.0", ClampMax="1.0"))
	float MaskedLandingRecoveryAlpha = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Crouched"))
	bool bMaskedIsCrouched = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Is Aiming"))
	bool bMaskedIsAiming = false;

	/** Smoothed crouch transition value for animation blending and procedural posing. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Crouch Alpha", ClampMin="0.0", ClampMax="1.0"))
	float MaskedCrouchAlpha = 0.0f;

	/** Smoothed aim transition value for aim pose blending. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Aim Alpha", ClampMin="0.0", ClampMax="1.0"))
	float MaskedAimAlpha = 0.0f;

	/** Signed procedural body lean supplied by the character movement policy. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Turn Lean Angle", Units="deg"))
	float MaskedTurnLeanAngle = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Is Turning In Place"))
	bool bMaskedIsTurningInPlace = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Turn In Place Angle", Units="deg"))
	float MaskedTurnInPlaceAngle = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Direction Change Alpha", ClampMin="0.0", ClampMax="1.0"))
	float MaskedDirectionChangeAlpha = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Signed Direction Change Angle", Units="deg"))
	float MaskedSignedDirectionChangeAngle = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|State", meta=(DisplayName="Sprint Requested"))
	bool bMaskedSprintRequested = false;

	/** Suggested playback multiplier for locomotion clips authored below sprint speed. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Locomotion Play Rate"))
	float MaskedLocomotionPlayRate = 1.0f;

	/** Matches the ordinary walk cycle's two foot contacts to the physical stair cadence. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Terrain|Stairs", meta=(ClampMin="0.5", ClampMax="2.0"))
	float StairLocomotionPlayRate = 1.08f;

	/** Approximate remaining physical stop distance, excluding speed-dependent friction. */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion|Animation", meta=(DisplayName="Estimated Stop Distance", Units="cm"))
	float MaskedEstimatedStopDistance = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn045LeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn045RightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn090LeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn090RightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn135LeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn135RightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn180LeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning")
	TObjectPtr<UAnimSequence> Turn180RightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning", meta=(ClampMin="0.1"))
	float TurnInPlaceAnimationPlayRate = 1.50f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turning", meta=(ClampMin="0.0", Units="s"))
	float TurnInPlaceBlendTime = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkStartLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkStartRightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkStopLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkStopRightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunStartLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunStartRightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintStartLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintStartRightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunStopLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunStopRightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintStopLeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintStopRightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkPivot090LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkPivot090RightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkPivot180LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> WalkPivot180RightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunPivot090LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunPivot090RightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunPivot180LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> RunPivot180RightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintPivot090LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintPivot090RightAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintPivot180LeftAnimation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions")
	TObjectPtr<UAnimSequence> SprintPivot180RightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", Units="s"))
	float GroundTransitionBlendTime = 0.08f;
	/** Lets a planted direction change resolve into the running cycle without exposing a pose snap. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", Units="s"))
	float PivotToLocomotionBlendTime = 0.30f;
	/** Longer fade so a sprint reface can already be in the sprint cycle before it yields. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", Units="s"))
	float SprintPivotToLocomotionBlendTime = 0.45f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", Units="s"))
	float SprintStartBlendTime = 0.22f;
	/** Idle into the walk takeoff. Short enough to stay responsive, long enough to hide the pose pop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", Units="s"))
	float WalkStartBlendInTime = 0.14f;
	/** Plant plus the first walking stride so the outgoing pose already matches the walk cycle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", Units="s"))
	float WalkStartVisibleDuration = 0.95f;
	/** Crossfade while both the start clip and the walk loop are already in a walking pose. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", Units="s"))
	float WalkStartToLocomotionBlendTime = 0.38f;
	/** Direction changes at or below this speed use walk reface clips. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", Units="cm/s"))
	float WalkPivotMaximumSpeed = 200.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.1"))
	float GroundTransitionPlayRate = 1.15f;
	/** Side/back input beyond this angle uses a planted reface start instead of a forward start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float DirectionalStartPivotAngle = 50.0f;
	/** Angles beyond this point select the 180-degree reface start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
	float DirectionalStartReversalAngle = 135.0f;
	/** Prevents directional blend parameters snapping across sectors at low speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.0", Units="deg/s"))
	float MovementDirectionTurnSpeed = 720.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions|Directional", meta=(ClampMin="0.1"))
	float DirectionalStartPlayRate = 1.02f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PivotAnimationTriggerAlpha = 0.42f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="0.0", Units="cm/s"))
	float PivotAnimationMinimumSpeed = 140.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="1.0", Units="cm"))
	float WalkStrideDistance = 130.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="1.0", Units="cm"))
	float RunStrideDistance = 220.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Transitions", meta=(ClampMin="1.0", Units="cm"))
	float SprintStrideDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchIdleAnimation;

	/** Forward loop and fallback for any unassigned directional loop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkForwardRightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkRightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkBackwardRightAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkBackwardAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkBackwardLeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkLeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchWalkForwardLeftAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> StandToCrouchAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchToStandAnimation;

	/** Legacy transition fallback retained for older animation sets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> LegacyCrouchTransitionAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch")
	TObjectPtr<UAnimSequence> CrouchToSprintAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0", Units="cm/s"))
	float CrouchWalkEnterSpeed = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0", Units="cm/s"))
	float CrouchWalkExitSpeed = 16.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0"))
	float CrouchAnimationBlendTime = 0.18f;

	/** Fast handoff used when input demands locomotion before a transition clip finishes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0"))
	float CrouchMovementInterruptBlendTime = 0.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.1"))
	float CrouchTransitionPlayRate = 1.15f;

	/** Starts blending out before the transition's least useful final contact frames. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0", Units="s"))
	float CrouchTransitionEndTrimTime = 0.20f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="0.0", Units="cm/s"))
	float CrouchTransitionInterruptSpeed = 8.0f;

	/** Physical speed represented by a 1.0 playback rate in the GASP crouch loops. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Crouch", meta=(ClampMin="1.0", Units="cm/s"))
	float CrouchWalkReferenceSpeed = 240.0f;

	/** Full-body takeoff used when jumping from an idle or very slow stance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air")
	TObjectPtr<UAnimSequence> JumpStartStandingAnimation;

	/** Full-body takeoff used when horizontal momentum is already established. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air")
	TObjectPtr<UAnimSequence> JumpStartMovingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air")
	TObjectPtr<UAnimSequence> FallLoopAnimation;

	/** Complete restrained jump arc used immediately for both standing and moving jumps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air")
	TObjectPtr<UAnimSequence> RoutineJumpLoopAnimation;

	/** Matches the authored jump arc to the character's physical time in the air. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air", meta=(ClampMin="0.1"))
	float RoutineJumpPlayRate = 0.88f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> LightLandingStandingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> LightLandingMovingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> HeavyLandingStandingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> HeavyLandingMovingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> LightLandingWalkingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> HeavyLandingWalkingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> LightLandingSprintingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing")
	TObjectPtr<UAnimSequence> HeavyLandingSprintingAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air", meta=(ClampMin="0.0", Units="cm/s"))
	float MovingJumpAnimationSpeed = 140.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air", meta=(Units="cm/s"))
	float FallLoopEnterVerticalSpeed = 40.0f;

	/** A routine jump becomes a true fall only after building meaningful downward speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air", meta=(ClampMax="0.0", Units="cm/s"))
	float RoutineJumpToFallVerticalSpeed = -500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float MovingLandingAnimationSpeed = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float WalkLandingMaximumSpeed = 210.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="cm/s"))
	float SprintLandingMinimumSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Air", meta=(ClampMin="0.0", Units="s"))
	float AirAnimationBlendTime = 0.10f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LandingAnimationBlendTime = 0.08f;

	/** Gentle pose-matched handoff from an ordinary landing recovery into idle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LightLandingReturnBlendTime = 0.22f;

	/** Heavier impacts need longer to release their compressed recovery pose. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float HeavyLandingReturnBlendTime = 0.30f;

	/** Moving landing clips hand directly into the live walk/run cycle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float MovingLandingReturnBlendTime = 0.14f;

	/** Movement remains responsive even while the longer idle return is blending. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LandingMovementInterruptBlendTime = 0.14f;

	/** Softer return used by ordinary self-initiated jumps with no impact reaction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float RoutineJumpLandingBlendTime = 0.16f;

	/** Blends out before authored stationary tail frames can hold the character in place. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LandingAnimationEndTrimTime = 0.14f;

	/** Authored impact frames measured from the GASP Land notify tracks. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LightLandingContactTime = 0.52f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float HeavyLandingContactTime = 1.02f;

	/** Only the compact weight-absorption portion plays before locomotion resumes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float LightLandingVisibleDuration = 0.30f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float MediumLandingVisibleDuration = 0.46f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float HeavyLandingVisibleDuration = 0.62f;

	/** Contact plus push-off only; subsequent authored steps are supplied by live locomotion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float MovingLightLandingVisibleDuration = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float MovingMediumLandingVisibleDuration = 0.27f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Landing", meta=(ClampMin="0.0", Units="s"))
	float MovingHeavyLandingVisibleDuration = 0.40f;

private:
	void UpdateFootIKTracing();
	void ResetLocomotionData();
	void UpdateAirAnimation();
	void UpdateCrouchAnimation();
	void UpdateTurnInPlaceAnimation();
	void UpdateGroundTransitionAnimation(float DeltaSeconds);
	UAnimSequence* SelectTurnInPlaceAnimation(float RequestedAngle) const;
	void StopTurnInPlaceAnimation(float BlendOutTime);
	UAnimSequence* SelectStartAnimation(bool bUseLeftFoot, bool bSprintStart) const;
	UAnimSequence* SelectStopAnimation(bool bUseLeftFoot, bool bWalkStop) const;
	UAnimSequence* SelectPivotAnimation(
		bool bTurnLeft,
		bool bUse180,
		EMaskedMovementGait PivotGait) const;
	EMaskedMovementGait GetGroundTransitionGait() const;
	void EnsureGroundTransitionAnimationsLoaded();
	UAnimSequence* LoadGroundTransitionAnimation(const TCHAR* Path) const;
	void PlayLocomotionStart(bool bSprintStart, bool bReuseCurrentFoot);
	void PlayGroundTransition(
		UAnimSequence* Animation,
		EMaskedGroundTransitionMode NewMode,
		float VisibleDuration,
		float PlayRate = -1.0f,
		float BlendOutTime = -1.0f,
		float BlendInTime = -1.0f);
	void StopGroundTransition(float BlendOutTime);
	EMaskedLandingLocomotionMode GetLandingLocomotionMode() const;
	UAnimSequence* SelectLandingAnimation(EMaskedLandingLocomotionMode LandingMode) const;
	UAnimMontage* PlayAirAnimation(
		UAnimSequence* Animation,
		bool bLoop,
		float PlayRate,
		float EndTrimTime,
		float StartPosition = 0.0f,
		float BlendOutTimeOverride = -1.0f);
	void StopAirAnimation(float BlendOutTime);
	UAnimSequence* SelectCrouchWalkAnimation(EMaskedCrouchDirection& OutDirection) const;
	UAnimMontage* PlayCrouchAnimation(UAnimSequence* Animation, bool bLoop, bool bReverse = false);
	void StopCrouchAnimation(float BlendOutTime);

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveCrouchMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveAirMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveTurnInPlaceMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveGroundTransitionMontage;

	EMaskedCrouchAnimationMode CrouchAnimationMode = EMaskedCrouchAnimationMode::None;
	EMaskedCrouchDirection ActiveCrouchDirection = EMaskedCrouchDirection::None;
	EMaskedAirAnimationMode AirAnimationMode = EMaskedAirAnimationMode::None;
	EMaskedGroundTransitionMode GroundTransitionMode = EMaskedGroundTransitionMode::None;
	bool bWasMaskedCrouched = false;
	bool bWasMaskedFalling = false;
	bool bActiveLandingUsesMovingAnimation = false;
	EMaskedLandingLocomotionMode ActiveLandingLocomotionMode = EMaskedLandingLocomotionMode::Standing;
	bool bWasMaskedTurningInPlace = false;
	bool bWasMaskedShouldMove = false;
	bool bWasMaskedAccelerating = false;
	bool bGroundPivotLatched = false;
	bool bNextStartUsesLeftFoot = false;
	bool bActiveGroundStartIsSprint = false;
	bool bActiveGroundStartIsDirectional = false;
	bool bActiveGroundStartUsesLeftFoot = false;
	float GroundPivotLatchTimeRemaining = 0.0f;
	float GroundTravelDistance = 0.0f;
	float PreviousGroundSpeed = 0.0f;
};
