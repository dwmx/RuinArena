// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/RACharacterMovementComponent.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

URACharacterMovementComponent::URACharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
:	Super(ObjectInitializer)
{
    // Enable special movement
    FNavAgentProperties& NavAgentProperties = GetNavAgentPropertiesRef();
    NavAgentProperties.bCanCrouch = true;

    MaxWalkSpeed = 940.0f;
    MaxCustomMovementSpeed = MaxWalkSpeed;
    MaxAcceleration = 4000.0f;

    DodgeImpulseHorizontal = 1500.0f;
    DodgeImpulseVertical = 500.0f;

    DodgeLandedVelocityScalingFactor = FVector(1.0f, 1.0f, 1.0f);

    // Ledge grabbing
    LedgeGrabTraceChannel = ETraceTypeQuery::TraceTypeQuery4;
    LedgeGrabDistance = 4.0f;
    LedgeGrabStepUpLength = 32.0f;

    bOnCanGrabLedgeFired = false;

    bDrawDebugLedgeGrabSweep = false;
    DebugLedgeGrabSweepColor = FColor::Green;
    DebugLedgeGrabHitSweepColor = FColor::Red;

    // Double jumps
    DoubleJumpMultiplier = 1.5f;
    DoubleJumpThresholdTimeSeconds = 0.5f;

    // Wall dodges
    WallDodgeTraceDistance = 128.0f;

    // Set in order to replicate bIsDodging
    // This might not be a good idea, original MovementComponent seems to replicate with a separate class
    SetIsReplicatedByDefault(true);
}

void URACharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    ARACharacter* RACharacter = Cast<ARACharacter>(CharacterOwner);
    if (RACharacter != NULL)
    {
        RACharacter->bPressedDoubleForward = ((Flags & FSavedMove_RACharacter::FLAG_DoubleForwardPressed) != 0);
        RACharacter->bPressedDoubleBackward = ((Flags & FSavedMove_RACharacter::FLAG_DoubleBackwardPressed) != 0);
        RACharacter->bPressedDoubleRight = ((Flags & FSavedMove_RACharacter::FLAG_DoubleRightPressed) != 0);
        RACharacter->bPressedDoubleLeft = ((Flags & FSavedMove_RACharacter::FLAG_DoubleLeftPressed) != 0);
    }
}

void URACharacterMovementComponent::TickComponent
(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction
)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetCharacterOwner()->HasAuthority() && IsFalling())
    {
        CheckForLedgeGrabAndFireEvent();
    }
}

bool URACharacterMovementComponent::DoJump(bool bReplayingMoves)
{
    if (CharacterOwner && CharacterOwner->CanJump())
    {
        // Don't jump if we can't move up/down.
        if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
        {
            float JumpVelocity = FMath::Max(Velocity.Z, JumpZVelocity);
            float TimeSeconds = GetWorld()->GetTimeSeconds();
            if (TimeSeconds - LastJumpTimeSeconds <= DoubleJumpThresholdTimeSeconds)
            {
                JumpVelocity = FMath::Max(Velocity.Z, JumpZVelocity * DoubleJumpMultiplier);
            }

            Velocity.Z = JumpVelocity;
            LastJumpTimeSeconds = TimeSeconds;
            SetMovementMode(MOVE_Falling);
            return true;
        }
    }

    return false;
}

void URACharacterMovementComponent::CheckForLedgeGrabAndFireEvent()
{
    // Don't check for ledge grab while player is holding jump
    ACharacter* TempCharacter = GetCharacterOwner();
    if (TempCharacter != nullptr && TempCharacter->bPressedJump)
    {
        return;
    }

    if (bOnCanGrabLedgeFired || LedgeGrabColliderComponent == nullptr)
    {
        return;
    }

    UBoxComponent* BoxComponent = Cast<UBoxComponent>(LedgeGrabColliderComponent);
    if (BoxComponent == nullptr)
    {
        return;
    }

    UWorld* WorldInstance = GetWorld();
    if (WorldInstance == nullptr || GetOwner() == nullptr)
    {
        return;
    }

    FVector LocalX, LocalY, LocalZ;
    FRotator OwnerRotation = GetOwner()->GetActorRotation();
    UKismetMathLibrary::GetAxes(GetOwner()->GetActorRotation(), LocalX, LocalY, LocalZ);

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector BoxLocation = BoxComponent->GetComponentLocation();

    FHitResult OutSweepHit;

    FVector BoxOffset = BoxLocation - OwnerLocation;
    FVector SweepStart = OwnerLocation + LocalZ * FVector::DotProduct(BoxOffset, LocalZ);
    FVector SweepEnd = BoxLocation;

    EDrawDebugTrace::Type DrawDebugSweepType = EDrawDebugTrace::None;
    if (bDrawDebugLedgeGrabSweep)
    {
        DrawDebugSweepType = EDrawDebugTrace::ForOneFrame;
    }

    // Ignore every Actor along the Ownership hierarchy
    TArray<AActor*> IgnoreActors;
    AActor* ActorOwner = GetOwner();
    while (ActorOwner != nullptr)
    {
        IgnoreActors.Add(ActorOwner);
        ActorOwner = ActorOwner->GetOwner();
    }

    // First check - Box sweep into the owner's box component
    bool bBlockingHit = UKismetSystemLibrary::BoxTraceSingle
    (
        this,
        SweepStart,
        SweepEnd,
        BoxComponent->GetScaledBoxExtent(),
        OwnerRotation,
        LedgeGrabTraceChannel,
        false,
        IgnoreActors,
        DrawDebugSweepType,
        OutSweepHit,
        true,
        DebugLedgeGrabSweepColor,
        DebugLedgeGrabHitSweepColor
    );

    if (!bBlockingHit)
    {
        return;
    }

    // Second check - Find a ledge to grab which faces the character
    FHitResult OutTraceHit;

    FVector TraceStart = GetCharacterOwner()->GetActorLocation();
    //TraceStart.Z = BoxComponent->GetComponentLocation().Z;
    TraceStart.Z = OutSweepHit.ImpactPoint.Z;
    FVector TraceEnd = TraceStart + OutSweepHit.ImpactNormal * -1.0f * (GetCharacterOwner()->GetSimpleCollisionRadius() + LedgeGrabDistance);

    bBlockingHit = UKismetSystemLibrary::LineTraceSingle
    (
        this,
        TraceStart,
        TraceEnd,
        LedgeGrabTraceChannel,
        false,
        IgnoreActors,
        DrawDebugSweepType,
        OutTraceHit,
        true,
        DebugLedgeGrabSweepColor,
        DebugLedgeGrabHitSweepColor
    );

    if (!bBlockingHit)
    {
        return;
    }

    FVector WallNormal = OutTraceHit.ImpactNormal;

    // Third check - Find a ground position for the Character to step onto after climbing
    TraceEnd = OutTraceHit.ImpactPoint + OutTraceHit.ImpactNormal * -1.0f * LedgeGrabStepUpLength;
    TraceStart = TraceEnd + FVector(0.0f, 0.0f, 1.0f) * GetCharacterOwner()->GetSimpleCollisionHalfHeight();
    
    bBlockingHit = UKismetSystemLibrary::LineTraceSingle
    (
        this,
        TraceStart,
        TraceEnd,
        LedgeGrabTraceChannel,
        false,
        IgnoreActors,
        DrawDebugSweepType,
        OutTraceHit,
        true,
        DebugLedgeGrabSweepColor,
        DebugLedgeGrabHitSweepColor
    );

    if (!bBlockingHit)
    {
        return;
    }

    // Fourth check - Verify that the actual step-up location is within the ledge grab collider
    FVector CandidateLocation = OutTraceHit.ImpactPoint;
    float BoxTopZ = SweepStart.Z + BoxComponent->GetScaledBoxExtent().Z;
    float BoxBottomZ = SweepStart.Z - BoxComponent->GetScaledBoxExtent().Z;
    if (CandidateLocation.Z < BoxBottomZ || CandidateLocation.Z > BoxTopZ)
    {
        return;
    }

    // Final check - See if the character fits
    CandidateLocation = CandidateLocation + FVector(0.0f, 0.0f, 1.0f) * GetCharacterOwner()->GetSimpleCollisionHalfHeight();
    FVector SavedLocation = GetCharacterOwner()->GetActorLocation();
    if (GetCharacterOwner()->SetActorLocation(CandidateLocation))
    {
        GetCharacterOwner()->SetActorLocation(SavedLocation);
        LedgeGrabCandidateLocation = CandidateLocation;
        LedgeGrabCandidateGroundNormal = OutTraceHit.ImpactNormal;
        LedgeGrabCandidateWallNormal = WallNormal;
        bOnCanGrabLedgeFired = true;
        OnCanGrabLedge.Broadcast();
    }
}

void URACharacterMovementComponent::GetLedgeGrabCandidateVectors(FVector& CandidateLocation, FVector& CandidateGroundNormal, FVector& CandidateWallNormal)
{
    //check(bOnCanGrabLedgeFired); // The candidate vectors are only valid after firing the event
    CandidateLocation = LedgeGrabCandidateLocation;
    CandidateGroundNormal = LedgeGrabCandidateGroundNormal;
    CandidateWallNormal = LedgeGrabCandidateWallNormal;
}

void FSavedMove_RACharacter::Clear()
{
    Super::Clear();

    bPressedDoubleForward = false;
    bPressedDoubleBackward = false;
    bPressedDoubleRight = false;
    bPressedDoubleLeft = false;
}

void FSavedMove_RACharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

    ARACharacter* RACharacter = Cast<ARACharacter>(Character);
    if (RACharacter != NULL)
    {
        bPressedDoubleForward = RACharacter->bPressedDoubleForward;
        bPressedDoubleBackward = RACharacter->bPressedDoubleBackward;
        bPressedDoubleRight = RACharacter->bPressedDoubleRight;
        bPressedDoubleLeft = RACharacter->bPressedDoubleLeft;
    }

    // Round acceleration so that sent and local always match
    Acceleration.X = FMath::RoundToFloat(Acceleration.X);
    Acceleration.Y = FMath::RoundToFloat(Acceleration.Y);
    Acceleration.Z = FMath::RoundToFloat(Acceleration.Z);
}

uint8 FSavedMove_RACharacter::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (bPressedJump)
    {
        Result |= FLAG_JumpPressed;
    }

    if (bPressedDoubleForward)
    {
        Result |= FLAG_DoubleForwardPressed;
    }

    if (bPressedDoubleBackward)
    {
        Result |= FLAG_DoubleBackwardPressed;
    }

    if (bPressedDoubleRight)
    {
        Result |= FLAG_DoubleRightPressed;
    }

    if (bPressedDoubleLeft)
    {
        Result |= FLAG_DoubleLeftPressed;
    }

    return Result;
}

FSavedMovePtr FNetworkPredictionData_Client_RACharacter::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_RACharacter);
}

FNetworkPredictionData_Client* URACharacterMovementComponent::GetPredictionData_Client() const
{
    // Should only be called on clients in network games
    check(PawnOwner != NULL);
    //check(PawnOwner->GetLocalRole() < ROLE_Authority);

    if (!ClientPredictionData)
    {
        URACharacterMovementComponent* MutableThis = const_cast<URACharacterMovementComponent*>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_RACharacter(*this);
        MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.0f; // 2X character capsule radius
        MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.0f;
    }

    return ClientPredictionData;
}





bool URACharacterMovementComponent::DoDoubleTapForward()
{
    return PerformDodge(true, false, false, false);
}

bool URACharacterMovementComponent::DoDoubleTapBackward()
{
    return PerformDodge(false, true, false, false);
}

bool URACharacterMovementComponent::DoDoubleTapRight()
{
    return PerformDodge(false, false, true, false);
}

bool URACharacterMovementComponent::DoDoubleTapLeft()
{
    return PerformDodge(false, false, false, true);
}

bool URACharacterMovementComponent::PerformDodge(bool bForward, bool bBackward, bool bRight, bool bLeft)
{
    if (!HasValidData())
    {
        return false;
    }

    //if (!IsMovingOnGround())
    //{
    //    return false;
    //}

    float DodgeDirX = bForward ? 1.f : (bBackward ? -1.0f : 0.0f);
    float DodgeDirY = bLeft ? -1.f : (bRight ? 1.0f : 0.0f);
    float DodgeCrossX = (bLeft || bRight) ? 1.0f : 0.0f;
    float DodgeCrossY = (bForward || bBackward) ? 1.0f : 0.0f;

    FRotator TurnRot(0.f, CharacterOwner->GetActorRotation().Yaw, 0.0f);
    FRotationMatrix TurnRotMatrix = FRotationMatrix(TurnRot);
    FVector X = TurnRotMatrix.GetScaledAxis(EAxis::X);
    FVector Y = TurnRotMatrix.GetScaledAxis(EAxis::Y);

    FVector DodgeDir;
    FVector DodgeCross;
    if (IsMovingOnGround())
    {
        // Ground dodges
        DodgeDir = (DodgeDirX * X + DodgeDirY * Y).GetSafeNormal();
        DodgeCross = (DodgeCrossX * X + DodgeCrossY * Y).GetSafeNormal();
    }
    else
    {
        // Wall dodges
        FHitResult HitResult;
        FVector StartTrace = GetOwner()->GetActorLocation();
        FVector EndTrace;
        if (bBackward)
        {
            EndTrace = StartTrace + X * WallDodgeTraceDistance * 2.0f;
        }
        else if (bForward)
        {
            EndTrace = StartTrace + (-1.0f * X * WallDodgeTraceDistance * 2.0f);
        }
        else if (bLeft)
        {
            EndTrace = StartTrace + Y * WallDodgeTraceDistance * 2.0f;
        }
        else if (bRight)
        {
            EndTrace = StartTrace + (-1.0f * Y * WallDodgeTraceDistance * 2.0f);
        }

        FCollisionQueryParams CollisionQueryParams;
        CollisionQueryParams.AddIgnoredActor(GetOwner());
        for (TRAInventoryIterator<ARANewInventory> It(Cast<ARACharacter>(GetOwner())); It; ++It)
        {
            CollisionQueryParams.AddIgnoredActor(*It);
        }

        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECollisionChannel::ECC_Pawn, CollisionQueryParams))
        {
            EndTrace = StartTrace + (-1.0 * HitResult.Normal * WallDodgeTraceDistance);
            if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECollisionChannel::ECC_Pawn, CollisionQueryParams))
            {
                DodgeDir = HitResult.Normal;
                DodgeCross = (DodgeCrossX * X + DodgeCrossY * Y).GetSafeNormal();
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    Velocity = DodgeDir * DodgeImpulseHorizontal + (Velocity | DodgeCross)*DodgeCross;
    Velocity.Z = DodgeImpulseVertical;
    SetMovementMode(MOVE_Falling);
    bIsDodging = true;

    return true;
}

void URACharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
    Super::ProcessLanded(Hit, remainingTime, Iterations);

    if (bIsDodging)
    {
        Velocity = Velocity * DodgeLandedVelocityScalingFactor;
    }
    bIsDodging = false;
    bOnCanGrabLedgeFired = false;
}

bool URACharacterMovementComponent::GetIsDodging()
{
    return bIsDodging;
}

bool URACharacterMovementComponent::CanCrouchInCurrentState() const
{
    if (IsFalling())
    {
        return false;
    }

    return Super::CanCrouchInCurrentState();
}

void URACharacterMovementComponent::SetLedgeGrabColliderComponent(UPrimitiveComponent* NewLedgeGrabColliderComponent)
{
    check(NewLedgeGrabColliderComponent != nullptr);
    LedgeGrabColliderComponent = NewLedgeGrabColliderComponent;
}

void URACharacterMovementComponent::GetLifetimeReplicatedProps(TArray < FLifetimeProperty >& OutLifetimeProps) const
{
    FDoRepLifetimeParams Params;
    DOREPLIFETIME_WITH_PARAMS_FAST(URACharacterMovementComponent, bIsDodging, Params);

    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

/*
void URACharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
    SCOPE_CYCLE_COUNTER(STAT_CharPhysFalling);
    CSV_SCOPED_TIMING_STAT_EXCLUSIVE(CharPhysFalling);

    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime);
    FallAcceleration.Z = 0.f;
    const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration);

    float remainingTime = deltaTime;
    while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
    {
        Iterations++;
        float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
        remainingTime -= timeTick;

        const FVector OldLocation = UpdatedComponent->GetComponentLocation();
        const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
        bJustTeleported = false;

        RestorePreAdditiveRootMotionVelocity();

        const FVector OldVelocity = Velocity;

        // Apply input
        const float MaxDecel = GetMaxBrakingDeceleration();
        if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
        {
            // Compute Velocity
            {
                // Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
                TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
                Velocity.Z = 0.f;
                CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
                Velocity.Z = OldVelocity.Z;
            }
        }

        // Compute current gravity
        const FVector Gravity(0.f, 0.f, GetGravityZ());
        float GravityTime = timeTick;

        // If jump is providing force, gravity may be affected.
        bool bEndingJumpForce = false;
        if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
        {
            // Consume some of the force time. Only the remaining time (if any) is affected by gravity when bApplyGravityWhileJumping=false.
            const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, timeTick);
            GravityTime = bApplyGravityWhileJumping ? timeTick : FMath::Max(0.0f, timeTick - JumpForceTime);

            // Update Character state
            CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;
            if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
            {
                CharacterOwner->ResetJumpState();
                bEndingJumpForce = true;
            }
        }

        // Apply gravity
        Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

        // See if we need to sub-step to exactly reach the apex. This is important for avoiding "cutting off the top" of the trajectory as framerate varies.
        if (CharacterMovementCVars::ForceJumpPeakSubstep && OldVelocity.Z > 0.f && Velocity.Z <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
        {
            const FVector DerivedAccel = (Velocity - OldVelocity) / timeTick;
            if (!FMath::IsNearlyZero(DerivedAccel.Z))
            {
                const float TimeToApex = -OldVelocity.Z / DerivedAccel.Z;

                // The time-to-apex calculation should be precise, and we want to avoid adding a substep when we are basically already at the apex from the previous iteration's work.
                const float ApexTimeMinimum = 0.0001f;
                if (TimeToApex >= ApexTimeMinimum && TimeToApex < timeTick)
                {
                    const FVector ApexVelocity = OldVelocity + DerivedAccel * TimeToApex;
                    Velocity = ApexVelocity;
                    Velocity.Z = 0.f; // Should be nearly zero anyway, but this makes apex notifications consistent.

                    // We only want to move the amount of time it takes to reach the apex, and refund the unused time for next iteration.
                    remainingTime += (timeTick - TimeToApex);
                    timeTick = TimeToApex;
                    Iterations--;
                    NumJumpApexAttempts++;
                }
            }
        }

        //UE_LOG(LogCharacterMovement, Log, TEXT("dt=(%.6f) OldLocation=(%s) OldVelocity=(%s) NewVelocity=(%s)"), timeTick, *(UpdatedComponent->GetComponentLocation()).ToString(), *OldVelocity.ToString(), *Velocity.ToString());
        ApplyRootMotionToVelocity(timeTick);

        if (bNotifyApex && (Velocity.Z < 0.f))
        {
            // Just passed jump apex since now going down
            bNotifyApex = false;
            NotifyJumpApex();
        }

        // Compute change in position (using midpoint integration method).
        FVector Adjusted = 0.5f * (OldVelocity + Velocity) * timeTick;

        // Special handling if ending the jump force where we didn't apply gravity during the jump.
        if (bEndingJumpForce && !bApplyGravityWhileJumping)
        {
            // We had a portion of the time at constant speed then a portion with acceleration due to gravity.
            // Account for that here with a more correct change in position.
            const float NonGravityTime = FMath::Max(0.f, timeTick - GravityTime);
            Adjusted = (OldVelocity * NonGravityTime) + (0.5f * (OldVelocity + Velocity) * GravityTime);
        }

        // Move
        FHitResult Hit(1.f);
        SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

        if (!HasValidData())
        {
            return;
        }

        float LastMoveTimeSlice = timeTick;
        float subTimeTickRemaining = timeTick * (1.f - Hit.Time);

        if (IsSwimming()) //just entered water
        {
            remainingTime += subTimeTickRemaining;
            StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
            return;
        }
        else if (Hit.bBlockingHit)
        {
            if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
            {
                remainingTime += subTimeTickRemaining;
                ProcessLanded(Hit, remainingTime, Iterations);
                return;
            }
            else
            {
                // Compute impact deflection based on final velocity, not integration step.
                // This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
                Adjusted = Velocity * timeTick;

                // See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
                if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
                {
                    const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
                    FFindFloorResult FloorResult;
                    FindFloor(PawnLocation, FloorResult, false);
                    if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
                    {
                        remainingTime += subTimeTickRemaining;
                        ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
                        return;
                    }
                }

                HandleImpact(Hit, LastMoveTimeSlice, Adjusted);

                // If we've changed physics mode, abort.
                if (!HasValidData() || !IsFalling())
                {
                    return;
                }

                // Limit air control based on what we hit.
                // We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
                FVector VelocityNoAirControl = OldVelocity;
                FVector AirControlAccel = Acceleration;
                if (bHasLimitedAirControl)
                {
                    // Compute VelocityNoAirControl
                    {
                        // Find velocity *without* acceleration.
                        TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
                        TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);
                        Velocity.Z = 0.f;
                        CalcVelocity(timeTick, FallingLateralFriction, false, MaxDecel);
                        VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
                        VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
                    }

                    const bool bCheckLandingSpot = false; // we already checked above.
                    AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;
                    const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
                    Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
                }

                const FVector OldHitNormal = Hit.Normal;
                const FVector OldHitImpactNormal = Hit.ImpactNormal;
                FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);

                // Compute velocity after deflection (only gravity component for RootMotion)
                if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
                {
                    const FVector NewVelocity = (Delta / subTimeTickRemaining);
                    Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
                }

                if (subTimeTickRemaining > KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
                {
                    // Move in deflected direction.
                    SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

                    if (Hit.bBlockingHit)
                    {
                        // hit second wall
                        LastMoveTimeSlice = subTimeTickRemaining;
                        subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

                        if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
                        {
                            remainingTime += subTimeTickRemaining;
                            ProcessLanded(Hit, remainingTime, Iterations);
                            return;
                        }

                        HandleImpact(Hit, LastMoveTimeSlice, Delta);

                        // If we've changed physics mode, abort.
                        if (!HasValidData() || !IsFalling())
                        {
                            return;
                        }

                        // Act as if there was no air control on the last move when computing new deflection.
                        if (bHasLimitedAirControl && Hit.Normal.Z > VERTICAL_SLOPE_NORMAL_Z)
                        {
                            const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
                            Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
                        }

                        FVector PreTwoWallDelta = Delta;
                        TwoWallAdjust(Delta, Hit, OldHitNormal);

                        // Limit air control, but allow a slide along the second wall.
                        if (bHasLimitedAirControl)
                        {
                            const bool bCheckLandingSpot = false; // we already checked above.
                            const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;

                            // Only allow if not back in to first wall
                            if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
                            {
                                Delta += (AirControlDeltaV * subTimeTickRemaining);
                            }
                        }

                        // Compute velocity after deflection (only gravity component for RootMotion)
                        if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
                        {
                            const FVector NewVelocity = (Delta / subTimeTickRemaining);
                            Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
                        }

                        // bDitch=true means that pawn is straddling two slopes, neither of which he can stand on
                        bool bDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <= KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
                        SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
                        if (Hit.Time == 0.f)
                        {
                            // if we are stuck then try to side step
                            FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
                            if (SideDelta.IsNearlyZero())
                            {
                                SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
                            }
                            SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
                        }

                        if (bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f)
                        {
                            remainingTime = 0.f;
                            ProcessLanded(Hit, remainingTime, Iterations);
                            return;
                        }
                        else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >= WalkableFloorZ)
                        {
                            // We might be in a virtual 'ditch' within our perch radius. This is rare.
                            const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
                            const float ZMovedDist = FMath::Abs(PawnLocation.Z - OldLocation.Z);
                            const float MovedDist2DSq = (PawnLocation - OldLocation).SizeSquared2D();
                            if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
                            {
                                Velocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
                                Velocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
                                Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
                                Delta = Velocity * timeTick;
                                SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
                            }
                        }
                    }
                }
            }
        }

        if (Velocity.SizeSquared2D() <= KINDA_SMALL_NUMBER * 10.f)
        {
            Velocity.X = 0.f;
            Velocity.Y = 0.f;
        }
    }
}
*/