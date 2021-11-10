#include "Characters/Components/RACameraBoomComponent.h"
#include "DrawDebugHelpers.h"

URACameraBoomComponent::URACameraBoomComponent(const FObjectInitializer& ObjectInitializer)
{
	MinimumArmLength = 180.0f;
	MaximumArmLength = 600.0f;
	DefaultArmLengthIncrementAmount = 30.0f;
	CameraArmLengthLagSpeed = 5.0f;
	CurrentArmLength = TargetArmLength;
}

void URACameraBoomComponent::IncrementArmLength(float Amount)
{
	if (Amount == 0.0f)
	{
		Amount = DefaultArmLengthIncrementAmount;
	}

	if (MaximumArmLength < MinimumArmLength)
	{
		float Swap = MinimumArmLength;
		MinimumArmLength = MaximumArmLength;
		MaximumArmLength = Swap;
	}

	float ResultArmLength = TargetArmLength + Amount;
	ResultArmLength = FMath::Clamp(ResultArmLength, MinimumArmLength, MaximumArmLength);
	TargetArmLength = ResultArmLength;
}

void URACameraBoomComponent::DecrementArmLength(float Amount)
{
	if (Amount == 0.0f)
	{
		Amount = DefaultArmLengthIncrementAmount;
	}
	IncrementArmLength(-Amount);
}

void URACameraBoomComponent::OnRegister()
{
	USceneComponent::OnRegister();

	// enforce reasonable limits to avoid potential div-by-zero
	CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
	CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);

	// Set initial location (without lag).
	UpdateDesiredArmLocationExtended(false, false, false, false, 0.f);
}

void URACameraBoomComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	USceneComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDesiredArmLocationExtended(bDoCollisionTest, bEnableCameraLag, bEnableCameraRotationLag, bEnableCameraArmLengthLag, DeltaTime);
}

void URACameraBoomComponent::UpdateDesiredArmLocationExtended(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, bool bDoArmLengthLag, float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	// Apply 'lag' to rotation if desired
	if (bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Arm length lag
	if (bDoArmLengthLag)
	{
		CurrentArmLength = FMath::FInterpTo(CurrentArmLength, TargetArmLength, DeltaTime, CameraArmLengthLagSpeed);
	}
	else
	{
		CurrentArmLength = TargetArmLength;
	}

	// Build full-length vector for tracing purposes
	FVector TracingOrigin = DesiredLoc;
	FVector TracingLoc = DesiredLoc;
	TracingLoc -= DesiredRot.Vector() * TargetArmLength;
	TracingLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * CurrentArmLength;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	if (bDoTrace && (TargetArmLength != 0.0f))
	{
		bIsCameraFixed = true;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());

		FHitResult Result;
		GetWorld()->SweepSingleByChannel(Result, TracingOrigin, TracingLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

		UnfixedCameraPosition = TracingLoc;

		ResultLoc = DesiredLoc;
		if (Result.bBlockingHit)
		{
			float BlockingDistance = (Result.Location - TracingOrigin).Size();
			if (BlockingDistance < CurrentArmLength)
			{
				CurrentArmLength = BlockingDistance;
				ResultLoc = Result.Location;
			}
		}

		//ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc)
		{
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// Remove the roll component that comes along with slerp
	DesiredRot.Roll = 0.0f;

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}