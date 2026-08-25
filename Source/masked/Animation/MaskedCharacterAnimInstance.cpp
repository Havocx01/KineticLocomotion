// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaskedCharacterAnimInstance.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

UMaskedCharacterAnimInstance::UMaskedCharacterAnimInstance()
{
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn045LeftFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_045_L.M_Neutral_Stand_Turn_045_L"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn045RightFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_045_R.M_Neutral_Stand_Turn_045_R"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn090LeftFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_090_L.M_Neutral_Stand_Turn_090_L"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn090RightFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_090_R.M_Neutral_Stand_Turn_090_R"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn135LeftFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_135_L.M_Neutral_Stand_Turn_135_L"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn135RightFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_135_R.M_Neutral_Stand_Turn_135_R"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn180LeftFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_180_L.M_Neutral_Stand_Turn_180_L"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Turn180RightFinder(
		TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Stand_Turn_180_R.M_Neutral_Stand_Turn_180_R"));

	Turn045LeftAnimation = Turn045LeftFinder.Object;
	Turn045RightAnimation = Turn045RightFinder.Object;
	Turn090LeftAnimation = Turn090LeftFinder.Object;
	Turn090RightAnimation = Turn090RightFinder.Object;
	Turn135LeftAnimation = Turn135LeftFinder.Object;
	Turn135RightAnimation = Turn135RightFinder.Object;
	Turn180LeftAnimation = Turn180LeftFinder.Object;
	Turn180RightAnimation = Turn180RightFinder.Object;

#define MASKED_LOAD_GROUND_TRANSITION(Member, Path) \
	{ static ConstructorHelpers::FObjectFinder<UAnimSequence> Finder(TEXT(Path)); Member = Finder.Object; }
	MASKED_LOAD_GROUND_TRANSITION(WalkStartLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Start_F_Lfoot.M_Neutral_Walk_Start_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(WalkStartRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Start_F_Rfoot.M_Neutral_Walk_Start_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(WalkStopLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Stop_F_Lfoot.M_Neutral_Walk_Stop_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(WalkStopRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Stop_F_Rfoot.M_Neutral_Walk_Stop_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(RunStartLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Start_F_Lfoot.M_Neutral_Run_Start_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(RunStartRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Start_F_Rfoot.M_Neutral_Run_Start_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(SprintStartLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Start_F_Lfoot.M_Neutral_Sprint_Start_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(SprintStartRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Start_F_Rfoot.M_Neutral_Sprint_Start_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(RunStopLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Stop_F_Lfoot.M_Neutral_Run_Stop_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(RunStopRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Stop_F_Rfoot.M_Neutral_Run_Stop_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(SprintStopLeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Stop_F_Lfoot.M_Neutral_Sprint_Stop_F_Lfoot");
	MASKED_LOAD_GROUND_TRANSITION(SprintStopRightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Stop_F_Rfoot.M_Neutral_Sprint_Stop_F_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(WalkPivot090LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_L_090.M_Neutral_Walk_Reface_Start_F_L_090");
	MASKED_LOAD_GROUND_TRANSITION(WalkPivot090RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_R_090.M_Neutral_Walk_Reface_Start_F_R_090");
	MASKED_LOAD_GROUND_TRANSITION(WalkPivot180LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_L_180.M_Neutral_Walk_Reface_Start_F_L_180");
	MASKED_LOAD_GROUND_TRANSITION(WalkPivot180RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_R_180.M_Neutral_Walk_Reface_Start_F_R_180");
	MASKED_LOAD_GROUND_TRANSITION(RunPivot090LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Reface_Start_F_L_090.M_Neutral_Run_Reface_Start_F_L_090");
	MASKED_LOAD_GROUND_TRANSITION(RunPivot090RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Reface_Start_F_R_090.M_Neutral_Run_Reface_Start_F_R_090");
	MASKED_LOAD_GROUND_TRANSITION(RunPivot180LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Reface_Start_F_L_180.M_Neutral_Run_Reface_Start_F_L_180");
	MASKED_LOAD_GROUND_TRANSITION(RunPivot180RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Run_Reface_Start_F_R_180.M_Neutral_Run_Reface_Start_F_R_180");
	MASKED_LOAD_GROUND_TRANSITION(SprintPivot090LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Reface_Start_F_L_090.M_Neutral_Sprint_Reface_Start_F_L_090");
	MASKED_LOAD_GROUND_TRANSITION(SprintPivot090RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Reface_Start_F_R_090.M_Neutral_Sprint_Reface_Start_F_R_090");
	MASKED_LOAD_GROUND_TRANSITION(SprintPivot180LeftAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Reface_Start_F_L_180.M_Neutral_Sprint_Reface_Start_F_L_180");
	MASKED_LOAD_GROUND_TRANSITION(SprintPivot180RightAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Reface_Start_F_R_180.M_Neutral_Sprint_Reface_Start_F_R_180");
	MASKED_LOAD_GROUND_TRANSITION(LightLandingWalkingAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Walk_Light_Rfoot.M_Neutral_Jump_F_Land_Walk_Light_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(HeavyLandingWalkingAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Walk_Heavy_Rfoot.M_Neutral_Jump_F_Land_Walk_Heavy_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(LightLandingSprintingAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Sprint_Light_Rfoot.M_Neutral_Jump_F_Land_Sprint_Light_Rfoot");
	MASKED_LOAD_GROUND_TRANSITION(HeavyLandingSprintingAnimation, "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Sprint_Heavy_Rfoot.M_Neutral_Jump_F_Land_Sprint_Heavy_Rfoot");
#undef MASKED_LOAD_GROUND_TRANSITION
}

void UMaskedCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MaskedCharacterOwner = Cast<AmaskedCharacter>(TryGetPawnOwner());
	bWasMaskedCrouched = IsValid(MaskedCharacterOwner) && MaskedCharacterOwner->IsCharacterCrouched();
	bWasMaskedShouldMove = IsValid(MaskedCharacterOwner)
		&& MaskedCharacterOwner->GetVelocity().Size2D() > 5.0f;
	EnsureGroundTransitionAnimationsLoaded();
}

void UMaskedCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(MaskedCharacterOwner))
	{
		MaskedCharacterOwner = Cast<AmaskedCharacter>(TryGetPawnOwner());
	}

	if (!IsValid(MaskedCharacterOwner))
	{
		UpdateFootIKTracing();
		ResetLocomotionData();
		return;
	}

	EnsureGroundTransitionAnimationsLoaded();
	UpdateFootIKTracing();

	const UCharacterMovementComponent* MovementComponent = MaskedCharacterOwner->GetCharacterMovement();
	MaskedWorldVelocity = MaskedCharacterOwner->GetVelocity();
	MaskedWorldAcceleration = MovementComponent->GetCurrentAcceleration();
	MaskedGroundSpeed = MaskedWorldVelocity.Size2D();
	MaskedVerticalSpeed = MaskedWorldVelocity.Z;

	const FTransform& ActorTransform = MaskedCharacterOwner->GetActorTransform();
	const FVector LocalVelocity = ActorTransform.InverseTransformVectorNoScale(MaskedWorldVelocity);
	const FVector LocalAcceleration = ActorTransform.InverseTransformVectorNoScale(MaskedWorldAcceleration);
	const float TargetMovementDirection = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
	MaskedAccelerationDirection = LocalAcceleration.SizeSquared2D() > 1.0f
		? FMath::RadiansToDegrees(FMath::Atan2(LocalAcceleration.Y, LocalAcceleration.X))
		: 0.0f;
	if (MaskedGroundSpeed > 5.0f)
	{
		MaskedMovementDirection = FMath::FixedTurn(
			MaskedMovementDirection,
			TargetMovementDirection,
			MovementDirectionTurnSpeed * DeltaSeconds);
	}

	MaskedMovementGait = MaskedCharacterOwner->GetCurrentMovementGait();
	bMaskedIsFalling = MovementComponent->IsFalling();
	bMaskedIsTraversingStairs = MaskedCharacterOwner->IsTraversingStairs();
	bMaskedIsAscendingStairs = MaskedCharacterOwner->IsAscendingStairs();
	MaskedStairTraversalAlpha = MaskedCharacterOwner->GetStairTraversalAlpha();
	MaskedDetectedStepHeight = MaskedCharacterOwner->GetLastDetectedStepHeight();
	MaskedStairStepSerial = MaskedCharacterOwner->GetStairStepSerial();
	bMaskedStairStepLeftFoot = MaskedCharacterOwner->WasLastStairStepLeftFoot();
	MaskedDetectedTreadDepth = MaskedCharacterOwner->GetDetectedStairTreadDepth();
	bMaskedIsAscending = bMaskedIsFalling && MaskedVerticalSpeed > 0.0f;
	bMaskedIsDescending = bMaskedIsFalling && MaskedVerticalSpeed <= 0.0f;
	bMaskedIsRoutineJump = bMaskedIsFalling && MaskedCharacterOwner->IsAirborneFromJumpInput();
	const float GravityMagnitude = FMath::Abs(MovementComponent->GetGravityZ());
	MaskedTimeToJumpApex = bMaskedIsAscending && GravityMagnitude > KINDA_SMALL_NUMBER
		? MaskedVerticalSpeed / GravityMagnitude
		: 0.0f;
	MaskedLandingSeverity = MaskedCharacterOwner->GetLastLandingSeverity();
	MaskedLandingImpactSpeed = MaskedCharacterOwner->GetLastLandingImpactSpeed();
	MaskedLandingHorizontalSpeed = MaskedCharacterOwner->GetLastLandingHorizontalSpeed();
	MaskedLandingRecoveryAlpha = MaskedCharacterOwner->GetLandingRecoveryAlpha();
	bMaskedIsCrouched = MaskedCharacterOwner->IsCharacterCrouched();
	MaskedCrouchAlpha = MaskedCharacterOwner->GetCrouchAlpha();
	bMaskedIsAiming = MaskedCharacterOwner->IsAiming();
	MaskedAimAlpha = MaskedCharacterOwner->GetAimAlpha();
	bMaskedIsAccelerating = MaskedWorldAcceleration.SizeSquared2D() > 1.0f;
	bMaskedIsBraking = !bMaskedIsAccelerating && MaskedGroundSpeed > 5.0f && MovementComponent->IsMovingOnGround();
	bMaskedIsBrakingFromSprint = MaskedCharacterOwner->IsBrakingFromSprint();
	bMaskedShouldMove = MaskedGroundSpeed > 5.0f;
	MaskedTurnLeanAngle = MaskedCharacterOwner->GetTurnLeanAngle();
	bMaskedIsTurningInPlace = MaskedCharacterOwner->IsTurningInPlace();
	MaskedTurnInPlaceAngle = MaskedCharacterOwner->GetTurnInPlaceRequestedAngle();
	MaskedDirectionChangeAlpha = MaskedCharacterOwner->GetDirectionChangeAlpha();
	MaskedSignedDirectionChangeAngle = MaskedCharacterOwner->GetSignedDirectionChangeAngle();
	bMaskedSprintRequested = MaskedCharacterOwner->IsSprintRequested();

	if (MaskedMovementGait == EMaskedMovementGait::Sprinting)
	{
		MaskedLocomotionPlayRate = FMath::Clamp(MaskedGroundSpeed / 500.0f, 1.0f, 1.25f);
	}
	else
	{
		MaskedLocomotionPlayRate = FMath::Lerp(
			1.0f,
			StairLocomotionPlayRate,
			MaskedStairTraversalAlpha);
	}

	const float BrakingDeceleration = MovementComponent->BrakingDecelerationWalking;
	MaskedEstimatedStopDistance = BrakingDeceleration > KINDA_SMALL_NUMBER
		? FMath::Square(MaskedGroundSpeed) / (2.0f * BrakingDeceleration)
		: 0.0f;

	UpdateAirAnimation();
	UpdateCrouchAnimation();
	UpdateTurnInPlaceAnimation();
	UpdateGroundTransitionAnimation(DeltaSeconds);

	// A sharp sprint steer deliberately drops the movement policy to a planted
	// turn. Keep the sprint cycle ready underneath that full-body transition so
	// its blend-out reveals forward locomotion instead of briefly revealing jog.
	if (bMaskedSprintRequested
		&& (GroundTransitionMode == EMaskedGroundTransitionMode::Pivoting
			|| GroundTransitionMode == EMaskedGroundTransitionMode::Starting))
	{
		MaskedMovementGait = EMaskedMovementGait::Sprinting;
		MaskedLocomotionPlayRate = FMath::Clamp(MaskedGroundSpeed / 500.0f, 1.0f, 1.25f);
	}
}

FString UMaskedCharacterAnimInstance::GetGroundTransitionDebugName() const
{
	switch (GroundTransitionMode)
	{
	case EMaskedGroundTransitionMode::Starting:
		return TEXT("START");
	case EMaskedGroundTransitionMode::Stopping:
		return TEXT("STOP");
	case EMaskedGroundTransitionMode::Pivoting:
		return TEXT("PIVOT");
	default:
		return TEXT("OFF");
	}
}

FString UMaskedCharacterAnimInstance::GetLandingLocomotionDebugName() const
{
	switch (ActiveLandingLocomotionMode)
	{
	case EMaskedLandingLocomotionMode::Walking:
		return TEXT("WALK");
	case EMaskedLandingLocomotionMode::Running:
		return TEXT("RUN");
	case EMaskedLandingLocomotionMode::Sprinting:
		return TEXT("SPRINT");
	default:
		return TEXT("STAND");
	}
}

void UMaskedCharacterAnimInstance::UpdateTurnInPlaceAnimation()
{
	const bool bCanPlayTurn = bMaskedIsTurningInPlace
		&& !bMaskedIsFalling
		&& !bMaskedIsCrouched
		&& MaskedGroundSpeed <= 25.0f;

	if (bCanPlayTurn && !bWasMaskedTurningInPlace)
	{
		StopAirAnimation(TurnInPlaceBlendTime);
		StopCrouchAnimation(TurnInPlaceBlendTime);
		UAnimSequence* TurnAnimation = SelectTurnInPlaceAnimation(MaskedTurnInPlaceAngle);
		if (IsValid(TurnAnimation))
		{
			ActiveTurnInPlaceMontage = PlaySlotAnimationAsDynamicMontage(
				TurnAnimation,
				TEXT("DefaultSlot"),
				TurnInPlaceBlendTime,
				TurnInPlaceBlendTime,
				TurnInPlaceAnimationPlayRate,
				1,
				0.04f);
		}
	}
	else if (!bCanPlayTurn && bWasMaskedTurningInPlace)
	{
		StopTurnInPlaceAnimation(TurnInPlaceBlendTime);
	}

	bWasMaskedTurningInPlace = bCanPlayTurn;
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectTurnInPlaceAnimation(float RequestedAngle) const
{
	const bool bTurnRight = RequestedAngle >= 0.0f;
	const float AbsoluteAngle = FMath::Abs(RequestedAngle);
	if (AbsoluteAngle <= 67.5f)
	{
		return bTurnRight ? Turn045RightAnimation.Get() : Turn045LeftAnimation.Get();
	}
	if (AbsoluteAngle <= 112.5f)
	{
		return bTurnRight ? Turn090RightAnimation.Get() : Turn090LeftAnimation.Get();
	}
	if (AbsoluteAngle <= 157.5f)
	{
		return bTurnRight ? Turn135RightAnimation.Get() : Turn135LeftAnimation.Get();
	}
	return bTurnRight ? Turn180RightAnimation.Get() : Turn180LeftAnimation.Get();
}

void UMaskedCharacterAnimInstance::StopTurnInPlaceAnimation(float BlendOutTime)
{
	if (IsValid(ActiveTurnInPlaceMontage) && Montage_IsPlaying(ActiveTurnInPlaceMontage))
	{
		Montage_Stop(BlendOutTime, ActiveTurnInPlaceMontage);
	}
	ActiveTurnInPlaceMontage = nullptr;
}

void UMaskedCharacterAnimInstance::UpdateGroundTransitionAnimation(float DeltaSeconds)
{
	GroundPivotLatchTimeRemaining = FMath::Max(
		0.0f,
		GroundPivotLatchTimeRemaining - DeltaSeconds);

	if (IsValid(ActiveGroundTransitionMontage)
		&& !Montage_IsPlaying(ActiveGroundTransitionMontage))
	{
		ActiveGroundTransitionMontage = nullptr;
		GroundTransitionMode = EMaskedGroundTransitionMode::None;
		bActiveGroundStartIsSprint = false;
		bActiveGroundStartIsDirectional = false;
	}

	const bool bCanUseGroundTransitions = !bMaskedIsFalling
		&& !bMaskedIsCrouched
		&& !bMaskedIsTraversingStairs
		&& !bMaskedIsTurningInPlace
		&& MaskedLandingRecoveryAlpha <= 0.05f;
	if (!bCanUseGroundTransitions)
	{
		StopGroundTransition(GroundTransitionBlendTime);
		bGroundPivotLatched = false;
		GroundPivotLatchTimeRemaining = 0.0f;
		bWasMaskedShouldMove = bMaskedShouldMove;
		bWasMaskedAccelerating = bMaskedIsAccelerating;
		PreviousGroundSpeed = MaskedGroundSpeed;
		return;
	}

	if (MaskedGroundSpeed > 5.0f)
	{
		GroundTravelDistance += MaskedGroundSpeed * DeltaSeconds;
	}
	const float StrideDistance = bMaskedSprintRequested && MaskedGroundSpeed > 350.0f
		? SprintStrideDistance
		: (MaskedGroundSpeed > 190.0f ? RunStrideDistance : WalkStrideDistance);
	const float NormalizedStridePhase = FMath::Fmod(
		GroundTravelDistance,
		FMath::Max(StrideDistance, 1.0f)) / FMath::Max(StrideDistance, 1.0f);
	const bool bUseLeftFoot = NormalizedStridePhase < 0.5f;

	const bool bWantsPivot = MaskedDirectionChangeAlpha >= PivotAnimationTriggerAlpha
		&& MaskedGroundSpeed >= PivotAnimationMinimumSpeed;
	const EMaskedMovementGait TransitionGait = GetGroundTransitionGait();
	const bool bWalkPivot = TransitionGait == EMaskedMovementGait::Walking;
	const bool bSprintPivot = TransitionGait == EMaskedMovementGait::Sprinting;
	if (bWantsPivot && !bGroundPivotLatched)
	{
		const bool bTurnLeft = MaskedSignedDirectionChangeAngle < 0.0f;
		const bool bUse180 = FMath::Abs(MaskedSignedDirectionChangeAngle) >= 130.0f;
		UAnimSequence* PivotAnimation = SelectPivotAnimation(
			bTurnLeft,
			bUse180,
			TransitionGait);
		// GASP reface clips include an authored acceleration stride after the
		// plant. Cutting at the turn itself leaves the outgoing foot pose far
		// from the locomotion cycle and causes a visible snap. Carry that first
		// stride through, then blend while both poses are already running.
		const float PivotDuration = bUse180
			? (bSprintPivot ? 1.16f : (bWalkPivot ? 0.86f : 1.00f))
			: (bSprintPivot ? 0.92f : (bWalkPivot ? 0.70f : 0.80f));
		const float PivotPlayRate = bWalkPivot
			? 1.00f
			: FMath::GetMappedRangeValueClamped(
				FVector2D(PivotAnimationMinimumSpeed, 620.0f),
				FVector2D(1.00f, 1.24f),
				MaskedGroundSpeed);
		const float PivotBlendOut = bSprintPivot
			? SprintPivotToLocomotionBlendTime
			: PivotToLocomotionBlendTime;
		PlayGroundTransition(
			PivotAnimation,
			EMaskedGroundTransitionMode::Pivoting,
			PivotDuration,
			PivotPlayRate,
			PivotBlendOut);
		bGroundPivotLatched = true;
		GroundPivotLatchTimeRemaining = bUse180 ? 0.58f : 0.42f;
	}
	else if (MaskedDirectionChangeAlpha <= 0.15f
		&& GroundPivotLatchTimeRemaining <= 0.0f)
	{
		bGroundPivotLatched = false;
	}

	const bool bJustReleasedMovement = bWasMaskedAccelerating
		&& !bMaskedIsAccelerating
		&& MaskedGroundSpeed > 45.0f;
	const bool bSprintStop = bMaskedIsBrakingFromSprint || PreviousGroundSpeed >= 430.0f;
	if ((bJustReleasedMovement || bMaskedIsBrakingFromSprint)
		&& !bWantsPivot
		&& GroundTransitionMode != EMaskedGroundTransitionMode::Stopping
		&& !bSprintStop)
	{
		const bool bWalkStop = PreviousGroundSpeed <= 190.0f;
		const float StopDurationFromDistance = FMath::GetMappedRangeValueClamped(
			FVector2D(15.0f, 180.0f),
			FVector2D(0.38f, 0.74f),
			MaskedEstimatedStopDistance);
		const float StopPlayRate = FMath::GetMappedRangeValueClamped(
			FVector2D(15.0f, 180.0f),
			FVector2D(1.25f, 0.94f),
			MaskedEstimatedStopDistance);
		PlayGroundTransition(
			SelectStopAnimation(bUseLeftFoot, bWalkStop),
			EMaskedGroundTransitionMode::Stopping,
			StopDurationFromDistance,
			StopPlayRate);
	}

	const bool bJustStartedMoving = bMaskedShouldMove
		&& !bWasMaskedShouldMove
		&& bMaskedIsAccelerating;
	if (bJustStartedMoving && GroundTransitionMode == EMaskedGroundTransitionMode::None)
	{
		PlayLocomotionStart(bMaskedSprintRequested, false);
	}

	// W then Shift still committed a walk takeoff. Holding that plant while the
	// capsule sprints is the "floating" slide. Promote to a sprint start while
	// the pose is still near idle; later, just yield to the sprint cycle.
	if (GroundTransitionMode == EMaskedGroundTransitionMode::Starting
		&& bMaskedSprintRequested
		&& bMaskedIsAccelerating
		&& !bActiveGroundStartIsSprint)
	{
		const float StartPosition = IsValid(ActiveGroundTransitionMontage)
			? Montage_GetPosition(ActiveGroundTransitionMontage)
			: 0.0f;
		if (StartPosition <= 0.42f)
		{
			PlayLocomotionStart(true, true);
		}
		else
		{
			StopGroundTransition(0.12f);
		}
	}

	// Every one-shot remains interruptible. Player intent always wins over the
	// authored tail so a start never holds during a stop and vice versa.
	if ((GroundTransitionMode == EMaskedGroundTransitionMode::Starting && !bMaskedIsAccelerating)
		|| (GroundTransitionMode == EMaskedGroundTransitionMode::Stopping && bMaskedIsAccelerating))
	{
		StopGroundTransition(0.06f);
	}

	bWasMaskedShouldMove = bMaskedShouldMove;
	bWasMaskedAccelerating = bMaskedIsAccelerating;
	PreviousGroundSpeed = MaskedGroundSpeed;
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectStartAnimation(
	bool bUseLeftFoot,
	bool bSprintStart) const
{
	if (bSprintStart)
	{
		UAnimSequence* SprintStart = bUseLeftFoot
			? SprintStartLeftAnimation.Get()
			: SprintStartRightAnimation.Get();
		if (IsValid(SprintStart))
		{
			return SprintStart;
		}
		UAnimSequence* RunStart = bUseLeftFoot
			? RunStartLeftAnimation.Get()
			: RunStartRightAnimation.Get();
		if (IsValid(RunStart))
		{
			return RunStart;
		}
	}
	return bUseLeftFoot ? WalkStartLeftAnimation.Get() : WalkStartRightAnimation.Get();
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectStopAnimation(
	bool bUseLeftFoot,
	bool bWalkStop) const
{
	if (bWalkStop)
	{
		return bUseLeftFoot ? WalkStopLeftAnimation.Get() : WalkStopRightAnimation.Get();
	}
	return bUseLeftFoot ? RunStopLeftAnimation.Get() : RunStopRightAnimation.Get();
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectPivotAnimation(
	bool bTurnLeft,
	bool bUse180,
	EMaskedMovementGait PivotGait) const
{
	if (PivotGait == EMaskedMovementGait::Sprinting)
	{
		if (bUse180)
		{
			return bTurnLeft ? SprintPivot180LeftAnimation.Get() : SprintPivot180RightAnimation.Get();
		}
		return bTurnLeft ? SprintPivot090LeftAnimation.Get() : SprintPivot090RightAnimation.Get();
	}
	if (PivotGait == EMaskedMovementGait::Walking)
	{
		if (bUse180)
		{
			UAnimSequence* Walk180 = bTurnLeft
				? WalkPivot180LeftAnimation.Get()
				: WalkPivot180RightAnimation.Get();
			return IsValid(Walk180)
				? Walk180
				: (bTurnLeft ? RunPivot180LeftAnimation.Get() : RunPivot180RightAnimation.Get());
		}
		UAnimSequence* Walk090 = bTurnLeft
			? WalkPivot090LeftAnimation.Get()
			: WalkPivot090RightAnimation.Get();
		return IsValid(Walk090)
			? Walk090
			: (bTurnLeft ? RunPivot090LeftAnimation.Get() : RunPivot090RightAnimation.Get());
	}
	if (bUse180)
	{
		return bTurnLeft ? RunPivot180LeftAnimation.Get() : RunPivot180RightAnimation.Get();
	}
	return bTurnLeft ? RunPivot090LeftAnimation.Get() : RunPivot090RightAnimation.Get();
}

EMaskedMovementGait UMaskedCharacterAnimInstance::GetGroundTransitionGait() const
{
	// Ordinary locomotion is a walk in the blend space until sprint is held.
	// Using jog/run reface for that band is what made L/R switches look like a
	// sprint plant that then snapped back to walking.
	if (bMaskedSprintRequested)
	{
		return EMaskedMovementGait::Sprinting;
	}
	return EMaskedMovementGait::Walking;
}

UAnimSequence* UMaskedCharacterAnimInstance::LoadGroundTransitionAnimation(const TCHAR* Path) const
{
	return LoadObject<UAnimSequence>(nullptr, Path);
}

void UMaskedCharacterAnimInstance::EnsureGroundTransitionAnimationsLoaded()
{
	auto Resolve = [this](TObjectPtr<UAnimSequence>& Animation, const TCHAR* Path)
	{
		if (!IsValid(Animation))
		{
			Animation = LoadGroundTransitionAnimation(Path);
		}
		if (UAnimSequence* Sequence = Animation.Get())
		{
			Sequence->bEnableRootMotion = false;
			Sequence->bForceRootLock = true;
		}
	};

	Resolve(WalkPivot090LeftAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_L_090.M_Neutral_Walk_Reface_Start_F_L_090"));
	Resolve(WalkPivot090RightAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_R_090.M_Neutral_Walk_Reface_Start_F_R_090"));
	Resolve(WalkPivot180LeftAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_L_180.M_Neutral_Walk_Reface_Start_F_L_180"));
	Resolve(WalkPivot180RightAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Reface_Start_F_R_180.M_Neutral_Walk_Reface_Start_F_R_180"));
	Resolve(SprintStartLeftAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Start_F_Lfoot.M_Neutral_Sprint_Start_F_Lfoot"));
	Resolve(SprintStartRightAnimation, TEXT("/Game/_AnimationLibrary/GASP/M_Neutral_Sprint_Start_F_Rfoot.M_Neutral_Sprint_Start_F_Rfoot"));
}

void UMaskedCharacterAnimInstance::PlayLocomotionStart(bool bSprintStart, bool bReuseCurrentFoot)
{
	const EMaskedMovementGait StartGait = bSprintStart
		? EMaskedMovementGait::Sprinting
		: EMaskedMovementGait::Walking;
	const float AbsoluteStartAngle = FMath::Abs(MaskedAccelerationDirection);
	const bool bDirectionalStart = AbsoluteStartAngle >= DirectionalStartPivotAngle;
	const float SprintBlendIn = bReuseCurrentFoot ? 0.12f : GroundTransitionBlendTime;
	if (bDirectionalStart)
	{
		const bool bTurnLeft = MaskedAccelerationDirection < 0.0f;
		const bool bUse180 = AbsoluteStartAngle >= DirectionalStartReversalAngle;
		const float DirectionalDuration = bSprintStart
			? (bUse180 ? 0.84f : 0.68f)
			: (bUse180 ? 0.88f : 0.78f);
		PlayGroundTransition(
			SelectPivotAnimation(bTurnLeft, bUse180, StartGait),
			EMaskedGroundTransitionMode::Starting,
			DirectionalDuration,
			bSprintStart ? 1.00f : 0.96f,
			bSprintStart ? SprintStartBlendTime : WalkStartToLocomotionBlendTime,
			bSprintStart ? SprintBlendIn : WalkStartBlendInTime);
		bGroundPivotLatched = true;
		GroundPivotLatchTimeRemaining = bUse180 ? 0.62f : 0.46f;
		bActiveGroundStartIsDirectional = true;
	}
	else
	{
		const bool bStartWithLeftFoot = bReuseCurrentFoot
			? bActiveGroundStartUsesLeftFoot
			: bNextStartUsesLeftFoot;
		if (!bReuseCurrentFoot)
		{
			bActiveGroundStartUsesLeftFoot = bStartWithLeftFoot;
			bNextStartUsesLeftFoot = !bNextStartUsesLeftFoot;
		}
		PlayGroundTransition(
			SelectStartAnimation(bStartWithLeftFoot, bSprintStart),
			EMaskedGroundTransitionMode::Starting,
			bSprintStart ? 0.58f : WalkStartVisibleDuration,
			bSprintStart ? 1.00f : 0.96f,
			bSprintStart ? SprintStartBlendTime : WalkStartToLocomotionBlendTime,
			bSprintStart ? SprintBlendIn : WalkStartBlendInTime);
		bActiveGroundStartIsDirectional = false;
	}
	bActiveGroundStartIsSprint = bSprintStart;
}

void UMaskedCharacterAnimInstance::PlayGroundTransition(
	UAnimSequence* Animation,
	EMaskedGroundTransitionMode NewMode,
	float VisibleDuration,
	float PlayRate,
	float BlendOutTime,
	float BlendInTime)
{
	if (!IsValid(Animation))
	{
		return;
	}

	StopAirAnimation(GroundTransitionBlendTime);
	StopTurnInPlaceAnimation(GroundTransitionBlendTime);
	StopGroundTransition(GroundTransitionBlendTime);
	const float EndTrimTime = FMath::Max(
		Animation->GetPlayLength() - VisibleDuration,
		0.0f);
	const float ResolvedBlendOutTime = BlendOutTime >= 0.0f
		? BlendOutTime
		: GroundTransitionBlendTime;
	const float ResolvedBlendInTime = BlendInTime >= 0.0f
		? BlendInTime
		: GroundTransitionBlendTime;
	ActiveGroundTransitionMontage = PlaySlotAnimationAsDynamicMontage(
		Animation,
		TEXT("DefaultSlot"),
		ResolvedBlendInTime,
		ResolvedBlendOutTime,
		PlayRate > 0.0f ? PlayRate : GroundTransitionPlayRate,
		1,
		EndTrimTime);
	GroundTransitionMode = IsValid(ActiveGroundTransitionMontage)
		? NewMode
		: EMaskedGroundTransitionMode::None;
}

void UMaskedCharacterAnimInstance::StopGroundTransition(float BlendOutTime)
{
	if (IsValid(ActiveGroundTransitionMontage)
		&& Montage_IsPlaying(ActiveGroundTransitionMontage))
	{
		Montage_Stop(BlendOutTime, ActiveGroundTransitionMontage);
	}
	ActiveGroundTransitionMontage = nullptr;
	GroundTransitionMode = EMaskedGroundTransitionMode::None;
	bActiveGroundStartIsSprint = false;
	bActiveGroundStartIsDirectional = false;
}

void UMaskedCharacterAnimInstance::UpdateFootIKTracing()
{
	bool bShouldTrace = false;
	if (IsValid(MaskedCharacterOwner))
	{
		const UCharacterMovementComponent* MovementComponent = MaskedCharacterOwner->GetCharacterMovement();
		const FFindFloorResult& Floor = MovementComponent->CurrentFloor;
		const bool bGrounded = MovementComponent->IsMovingOnGround();
		const bool bHasWalkableFloor = Floor.bBlockingHit && Floor.IsWalkableFloor();
		const float GroundAngle = bHasWalkableFloor
			? FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Floor.HitResult.ImpactNormal.Z, -1.0f, 1.0f)))
			: 0.0f;
		const float RampThreshold = bMaskedFootIKTracingEnabled
			? FootIKRampDisableAngle
			: FootIKRampReenableAngle;
		const bool bContinuousRamp = bHasWalkableFloor
			&& GroundAngle >= RampThreshold
			&& !MaskedCharacterOwner->IsTraversingStairs();
		const bool bStairTraversal = MaskedCharacterOwner->IsTraversingStairs();

		// Independent traces on stair treads pull the pelvis with every capsule snap.
		// The character already eases mesh and camera height across the flight, so
		// the authored gait on a smoothed body reads as a climb instead of a bounce.
		bShouldTrace = bGrounded && !bContinuousRamp && !bStairTraversal;
	}
	bMaskedFootIKTracingEnabled = bShouldTrace;

	// The existing Control Rig exposes this as a generated custom AnimGraph pin.
	// Its generated suffix changes whenever the graph is reconstructed, so locate
	// it by its stable prefix and drive the appropriate surface-specific policy.
	static const FString FootIKPropertyPrefix(TEXT("__CustomProperty_ShouldDoIKTrace_"));
	for (TFieldIterator<FBoolProperty> PropertyIterator(GetClass()); PropertyIterator; ++PropertyIterator)
	{
		FBoolProperty* Property = *PropertyIterator;
		if (Property->GetName().StartsWith(FootIKPropertyPrefix))
		{
			Property->SetPropertyValue_InContainer(this, bShouldTrace);
			break;
		}
	}
}

void UMaskedCharacterAnimInstance::UpdateAirAnimation()
{
	if (bMaskedIsFalling)
	{
		StopCrouchAnimation(CrouchMovementInterruptBlendTime);

		if (!bWasMaskedFalling)
		{
			// A loop starts on the first airborne frame, so the grounded run graph can
			// never leak into a moving jump. Routine jumps use a compact, relaxed pose;
			// uncontrolled drops use the broader fall pose.
			const bool bUseRoutineJumpAnimation = bMaskedIsRoutineJump
				&& IsValid(RoutineJumpLoopAnimation);
			UAnimSequence* AirborneAnimation = bUseRoutineJumpAnimation
				? RoutineJumpLoopAnimation.Get()
				: FallLoopAnimation.Get();
			if (!IsValid(AirborneAnimation))
			{
				AirborneAnimation = FallLoopAnimation.Get();
			}
			const float AirbornePlayRate = bUseRoutineJumpAnimation
				? RoutineJumpPlayRate
				: 1.0f;
			ActiveAirMontage = PlayAirAnimation(
				AirborneAnimation,
				true,
				AirbornePlayRate,
				0.0f);
			AirAnimationMode = ActiveAirMontage
				? (bUseRoutineJumpAnimation
					? EMaskedAirAnimationMode::JumpStart
					: EMaskedAirAnimationMode::Falling)
				: EMaskedAirAnimationMode::None;
		}
		else if (AirAnimationMode == EMaskedAirAnimationMode::JumpStart
			&& MaskedVerticalSpeed <= RoutineJumpToFallVerticalSpeed)
		{
			// A normal hop keeps its compact pose through descent. Only an extended
			// drop earns the more dramatic fall silhouette.
			ActiveAirMontage = PlayAirAnimation(FallLoopAnimation.Get(), true, 1.0f, 0.0f);
			AirAnimationMode = ActiveAirMontage
				? EMaskedAirAnimationMode::Falling
				: EMaskedAirAnimationMode::None;
		}
		else if (!bMaskedIsRoutineJump
			&& MaskedVerticalSpeed <= FallLoopEnterVerticalSpeed
			&& AirAnimationMode != EMaskedAirAnimationMode::Falling)
		{
			ActiveAirMontage = PlayAirAnimation(FallLoopAnimation.Get(), true, 1.0f, 0.0f);
			AirAnimationMode = ActiveAirMontage
				? EMaskedAirAnimationMode::Falling
				: EMaskedAirAnimationMode::None;
		}

		bWasMaskedFalling = true;
		return;
	}

	if (bWasMaskedFalling)
	{
		bWasMaskedFalling = false;
		const float GroundedBlendTime = MaskedLandingSeverity == EMaskedLandingSeverity::None
			? RoutineJumpLandingBlendTime
			: LandingAnimationBlendTime;
		StopAirAnimation(GroundedBlendTime);

		const EMaskedLandingLocomotionMode LandingMode = GetLandingLocomotionMode();
		UAnimSequence* LandingAnimation = SelectLandingAnimation(LandingMode);
		if (IsValid(LandingAnimation))
		{
			const bool bUseMovingLanding = LandingMode != EMaskedLandingLocomotionMode::Standing;
			const float LandingPlayRate = MaskedLandingSeverity == EMaskedLandingSeverity::Medium
				? 1.10f
				: 1.0f;
			const bool bLightLanding = MaskedLandingSeverity == EMaskedLandingSeverity::Light;
			const float LandingContactTime = bLightLanding
				? LightLandingContactTime
				: HeavyLandingContactTime;
			const float LandingVisibleDuration = bUseMovingLanding
				? (bLightLanding
					? MovingLightLandingVisibleDuration
					: (MaskedLandingSeverity == EMaskedLandingSeverity::Medium
						? MovingMediumLandingVisibleDuration
						: MovingHeavyLandingVisibleDuration))
				: (bLightLanding
					? LightLandingVisibleDuration
					: (MaskedLandingSeverity == EMaskedLandingSeverity::Medium
						? MediumLandingVisibleDuration
						: HeavyLandingVisibleDuration));
			const float LandingBlendOutTriggerTime = FMath::Max(
				LandingAnimation->GetPlayLength() - LandingContactTime - LandingVisibleDuration,
				LandingAnimationEndTrimTime);
			const float LandingReturnBlendTime = bUseMovingLanding
				? MovingLandingReturnBlendTime
				: (bLightLanding
					? LightLandingReturnBlendTime
					: (MaskedLandingSeverity == EMaskedLandingSeverity::Medium
						? FMath::Lerp(LightLandingReturnBlendTime, HeavyLandingReturnBlendTime, 0.5f)
						: HeavyLandingReturnBlendTime));
			ActiveAirMontage = PlayAirAnimation(
				LandingAnimation,
				false,
				LandingPlayRate,
				LandingBlendOutTriggerTime,
				LandingContactTime,
				LandingReturnBlendTime);
			AirAnimationMode = ActiveAirMontage
				? EMaskedAirAnimationMode::Landing
				: EMaskedAirAnimationMode::None;
			bActiveLandingUsesMovingAnimation = IsValid(ActiveAirMontage)
				&& bUseMovingLanding;
			ActiveLandingLocomotionMode = IsValid(ActiveAirMontage)
				? LandingMode
				: EMaskedLandingLocomotionMode::Standing;
		}
		return;
	}

	if (AirAnimationMode == EMaskedAirAnimationMode::Landing
		&& !bActiveLandingUsesMovingAnimation
		&& bMaskedIsAccelerating
		&& MaskedGroundSpeed > 35.0f)
	{
		StopAirAnimation(LandingMovementInterruptBlendTime);
		return;
	}

	if (AirAnimationMode == EMaskedAirAnimationMode::Landing
		&& (!IsValid(ActiveAirMontage) || !Montage_IsPlaying(ActiveAirMontage)))
	{
		ActiveAirMontage = nullptr;
		AirAnimationMode = EMaskedAirAnimationMode::None;
		bActiveLandingUsesMovingAnimation = false;
		ActiveLandingLocomotionMode = EMaskedLandingLocomotionMode::Standing;
	}
}

EMaskedLandingLocomotionMode UMaskedCharacterAnimInstance::GetLandingLocomotionMode() const
{
	if (MaskedLandingHorizontalSpeed < MovingLandingAnimationSpeed)
	{
		return EMaskedLandingLocomotionMode::Standing;
	}
	if (MaskedLandingHorizontalSpeed <= WalkLandingMaximumSpeed)
	{
		return EMaskedLandingLocomotionMode::Walking;
	}
	if (MaskedLandingHorizontalSpeed >= SprintLandingMinimumSpeed)
	{
		return EMaskedLandingLocomotionMode::Sprinting;
	}
	return EMaskedLandingLocomotionMode::Running;
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectLandingAnimation(
	EMaskedLandingLocomotionMode LandingMode) const
{
	const bool bHeavyLanding = MaskedLandingSeverity == EMaskedLandingSeverity::Medium
		|| MaskedLandingSeverity == EMaskedLandingSeverity::Heavy;
	switch (LandingMode)
	{
	case EMaskedLandingLocomotionMode::Walking:
		return bHeavyLanding
			? HeavyLandingWalkingAnimation.Get()
			: LightLandingWalkingAnimation.Get();
	case EMaskedLandingLocomotionMode::Running:
		return bHeavyLanding
			? HeavyLandingMovingAnimation.Get()
			: LightLandingMovingAnimation.Get();
	case EMaskedLandingLocomotionMode::Sprinting:
		return bHeavyLanding
			? HeavyLandingSprintingAnimation.Get()
			: LightLandingSprintingAnimation.Get();
	default:
		break;
	}

	switch (MaskedLandingSeverity)
	{
	case EMaskedLandingSeverity::Light:
		return LightLandingStandingAnimation.Get();
	case EMaskedLandingSeverity::Medium:
	case EMaskedLandingSeverity::Heavy:
		return HeavyLandingStandingAnimation.Get();
	default:
		return nullptr;
	}
}

UAnimMontage* UMaskedCharacterAnimInstance::PlayAirAnimation(
	UAnimSequence* Animation,
	bool bLoop,
	float PlayRate,
	float EndTrimTime,
	float StartPosition,
	float BlendOutTimeOverride)
{
	if (!IsValid(Animation))
	{
		return nullptr;
	}

	StopAirAnimation(AirAnimationBlendTime);
	const float MontageBlendOutTime = BlendOutTimeOverride >= 0.0f
		? BlendOutTimeOverride
		: (bLoop ? AirAnimationBlendTime : LandingAnimationBlendTime);
	return PlaySlotAnimationAsDynamicMontage(
		Animation,
		TEXT("DefaultSlot"),
		AirAnimationBlendTime,
		MontageBlendOutTime,
		PlayRate,
		bLoop ? 9999 : 1,
		bLoop ? -1.0f : EndTrimTime,
		StartPosition);
}

void UMaskedCharacterAnimInstance::StopAirAnimation(float BlendOutTime)
{
	if (IsValid(ActiveAirMontage) && Montage_IsPlaying(ActiveAirMontage))
	{
		Montage_Stop(BlendOutTime, ActiveAirMontage);
	}

	ActiveAirMontage = nullptr;
	AirAnimationMode = EMaskedAirAnimationMode::None;
	bActiveLandingUsesMovingAnimation = false;
	ActiveLandingLocomotionMode = EMaskedLandingLocomotionMode::Standing;
}

void UMaskedCharacterAnimInstance::UpdateCrouchAnimation()
{
	// Never layer a grounded crouch pose over the airborne state. The character
	// also clears its crouched capsule when leaving a ledge, so both systems hand
	// control back to the normal falling graph on the same frame.
	if (bMaskedIsFalling)
	{
		StopCrouchAnimation(CrouchMovementInterruptBlendTime);
		bWasMaskedCrouched = bMaskedIsCrouched;
		return;
	}

	const bool bCrouchStateChanged = bMaskedIsCrouched != bWasMaskedCrouched;
	if (bCrouchStateChanged)
	{
		// The supplied transition clips contain unreliable foot-contact frames.
		// Crossfading directly between two grounded poses is smoother, never drives
		// the toes below the floor, and remains instantly responsive to movement.
		StopCrouchAnimation(CrouchAnimationBlendTime);
		bWasMaskedCrouched = bMaskedIsCrouched;
		if (!bMaskedIsCrouched)
		{
			return;
		}
	}

	if (!bMaskedIsCrouched)
	{
		if (CrouchAnimationMode == EMaskedCrouchAnimationMode::Idle
			|| CrouchAnimationMode == EMaskedCrouchAnimationMode::Walking)
		{
			StopCrouchAnimation(CrouchAnimationBlendTime);
		}
		return;
	}

	const bool bCurrentlyWalking = CrouchAnimationMode == EMaskedCrouchAnimationMode::Walking;
	const bool bShouldWalk = bCurrentlyWalking
		? MaskedGroundSpeed > CrouchWalkExitSpeed
		: MaskedGroundSpeed > CrouchWalkEnterSpeed;
	const EMaskedCrouchAnimationMode DesiredMode = bShouldWalk
		? EMaskedCrouchAnimationMode::Walking
		: EMaskedCrouchAnimationMode::Idle;

	EMaskedCrouchDirection DesiredDirection = EMaskedCrouchDirection::None;
	UAnimSequence* DesiredAnimation = bShouldWalk
		? SelectCrouchWalkAnimation(DesiredDirection)
		: CrouchIdleAnimation.Get();
	const bool bDirectionChanged = bShouldWalk && ActiveCrouchDirection != DesiredDirection;
	if (CrouchAnimationMode != DesiredMode || bDirectionChanged)
	{
		ActiveCrouchMontage = PlayCrouchAnimation(DesiredAnimation, true);
		CrouchAnimationMode = ActiveCrouchMontage ? DesiredMode : EMaskedCrouchAnimationMode::None;
		ActiveCrouchDirection = ActiveCrouchMontage ? DesiredDirection : EMaskedCrouchDirection::None;
	}

	if (bShouldWalk && IsValid(ActiveCrouchMontage))
	{
		// Match the authored stride to actual capsule speed, including gentle starts.
		const float WalkPlayRate = FMath::Clamp(
			MaskedGroundSpeed / FMath::Max(CrouchWalkReferenceSpeed, 1.0f),
			0.35f,
			0.85f);
		Montage_SetPlayRate(ActiveCrouchMontage, WalkPlayRate);
	}
}

UAnimSequence* UMaskedCharacterAnimInstance::SelectCrouchWalkAnimation(
	EMaskedCrouchDirection& OutDirection) const
{
	const float Direction = FMath::UnwindDegrees(MaskedMovementDirection);
	UAnimSequence* SelectedAnimation = CrouchWalkAnimation.Get();

	if (Direction >= -22.5f && Direction < 22.5f)
	{
		OutDirection = EMaskedCrouchDirection::Forward;
	}
	else if (Direction >= 22.5f && Direction < 67.5f)
	{
		OutDirection = EMaskedCrouchDirection::ForwardRight;
		SelectedAnimation = CrouchWalkForwardRightAnimation.Get();
	}
	else if (Direction >= 67.5f && Direction < 112.5f)
	{
		OutDirection = EMaskedCrouchDirection::Right;
		SelectedAnimation = CrouchWalkRightAnimation.Get();
	}
	else if (Direction >= 112.5f && Direction < 157.5f)
	{
		OutDirection = EMaskedCrouchDirection::BackwardRight;
		SelectedAnimation = CrouchWalkBackwardRightAnimation.Get();
	}
	else if (Direction >= 157.5f || Direction < -157.5f)
	{
		OutDirection = EMaskedCrouchDirection::Backward;
		SelectedAnimation = CrouchWalkBackwardAnimation.Get();
	}
	else if (Direction >= -157.5f && Direction < -112.5f)
	{
		OutDirection = EMaskedCrouchDirection::BackwardLeft;
		SelectedAnimation = CrouchWalkBackwardLeftAnimation.Get();
	}
	else if (Direction >= -112.5f && Direction < -67.5f)
	{
		OutDirection = EMaskedCrouchDirection::Left;
		SelectedAnimation = CrouchWalkLeftAnimation.Get();
	}
	else
	{
		OutDirection = EMaskedCrouchDirection::ForwardLeft;
		SelectedAnimation = CrouchWalkForwardLeftAnimation.Get();
	}

	return IsValid(SelectedAnimation) ? SelectedAnimation : CrouchWalkAnimation.Get();
}

UAnimMontage* UMaskedCharacterAnimInstance::PlayCrouchAnimation(
	UAnimSequence* Animation,
	bool bLoop,
	bool bReverse)
{
	if (!IsValid(Animation))
	{
		return nullptr;
	}

	StopCrouchAnimation(CrouchAnimationBlendTime);
	UAnimMontage* NewMontage = PlaySlotAnimationAsDynamicMontage(
		Animation,
		TEXT("DefaultSlot"),
		CrouchAnimationBlendTime,
		CrouchAnimationBlendTime,
		bLoop ? 1.0f : CrouchTransitionPlayRate,
		bLoop ? 9999 : 1,
		bLoop ? -1.0f : CrouchTransitionEndTrimTime);

	if (IsValid(NewMontage) && bReverse)
	{
		Montage_SetPosition(NewMontage, FMath::Max(Animation->GetPlayLength() - KINDA_SMALL_NUMBER, 0.0f));
		Montage_SetPlayRate(NewMontage, -CrouchTransitionPlayRate);
	}

	return NewMontage;
}

void UMaskedCharacterAnimInstance::StopCrouchAnimation(float BlendOutTime)
{
	if (IsValid(ActiveCrouchMontage) && Montage_IsPlaying(ActiveCrouchMontage))
	{
		Montage_Stop(BlendOutTime, ActiveCrouchMontage);
	}

	ActiveCrouchMontage = nullptr;
	CrouchAnimationMode = EMaskedCrouchAnimationMode::None;
	ActiveCrouchDirection = EMaskedCrouchDirection::None;
}

void UMaskedCharacterAnimInstance::ResetLocomotionData()
{
	MaskedWorldVelocity = FVector::ZeroVector;
	MaskedWorldAcceleration = FVector::ZeroVector;
	MaskedGroundSpeed = 0.0f;
	MaskedLandingHorizontalSpeed = 0.0f;
	MaskedVerticalSpeed = 0.0f;
	MaskedMovementDirection = 0.0f;
	MaskedAccelerationDirection = 0.0f;
	MaskedMovementGait = EMaskedMovementGait::Walking;
	bMaskedShouldMove = false;
	bMaskedIsAccelerating = false;
	bMaskedIsBraking = false;
	bMaskedIsBrakingFromSprint = false;
	bMaskedIsFalling = false;
	bMaskedFootIKTracingEnabled = false;
	bMaskedIsTraversingStairs = false;
	bMaskedIsAscendingStairs = false;
	MaskedStairTraversalAlpha = 0.0f;
	MaskedDetectedStepHeight = 0.0f;
	MaskedStairStepSerial = 0;
	bMaskedStairStepLeftFoot = false;
	MaskedDetectedTreadDepth = 38.0f;
	bMaskedIsAscending = false;
	bMaskedIsDescending = false;
	bMaskedIsRoutineJump = false;
	MaskedTimeToJumpApex = 0.0f;
	MaskedLandingSeverity = EMaskedLandingSeverity::None;
	MaskedLandingImpactSpeed = 0.0f;
	MaskedLandingRecoveryAlpha = 0.0f;
	bMaskedIsCrouched = false;
	MaskedCrouchAlpha = 0.0f;
	bMaskedIsAiming = false;
	MaskedAimAlpha = 0.0f;
	MaskedTurnLeanAngle = 0.0f;
	bMaskedIsTurningInPlace = false;
	MaskedTurnInPlaceAngle = 0.0f;
	MaskedDirectionChangeAlpha = 0.0f;
	MaskedSignedDirectionChangeAngle = 0.0f;
	bMaskedSprintRequested = false;
	MaskedLocomotionPlayRate = 1.0f;
	MaskedEstimatedStopDistance = 0.0f;
	bWasMaskedCrouched = false;
	bWasMaskedFalling = false;
	bWasMaskedTurningInPlace = false;
	bWasMaskedShouldMove = false;
	bWasMaskedAccelerating = false;
	bGroundPivotLatched = false;
	GroundPivotLatchTimeRemaining = 0.0f;
	GroundTravelDistance = 0.0f;
	PreviousGroundSpeed = 0.0f;
	StopAirAnimation(0.1f);
	StopCrouchAnimation(0.1f);
	StopTurnInPlaceAnimation(0.1f);
	StopGroundTransition(0.1f);
}
