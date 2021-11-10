// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RACharacterMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCanGrabLedgeSignature);

UCLASS()
class RUNEARENA_API URACharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly)
    float LastJumpTimeSeconds;

    /** The amount of seconds in between jumps required to trigger double jump bonus velocity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement")
    float DoubleJumpThresholdTimeSeconds;

    /** Factor to multiply normal jump by when applying a double jump */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement")
    float DoubleJumpMultiplier;

    /** How far a wall can be from the character's location in order to perform a wall dodge */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterMovement|Dodging")
    float WallDodgeTraceDistance;

    UPROPERTY(Replicated)
    bool bIsDodging;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement")
    FVector DodgeLandedVelocityScalingFactor;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab")
    TEnumAsByte<ETraceTypeQuery> LedgeGrabTraceChannel;

    /** How far the Character must be from the ledge in order to grab it, measured from the boundary of the capsule radius */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab")
    float LedgeGrabDistance;

    /** How far forward onto the ledge the Character will move when ledge is grabbed, measured from capsule origin */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab")
    float LedgeGrabStepUpLength;

    FVector LedgeGrabCandidateLocation;
    FVector LedgeGrabCandidateGroundNormal;
    FVector LedgeGrabCandidateWallNormal;

    UPrimitiveComponent* LedgeGrabColliderComponent;
    bool bOnCanGrabLedgeFired;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab|Debug")
    bool bDrawDebugLedgeGrabSweep;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab|Debug")
    FColor DebugLedgeGrabSweepColor;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|LedgeGrab|Debug")
    FColor DebugLedgeGrabHitSweepColor;

    /** Overridden to allow stepup during ascending portion of jumps */
    //virtual void PhysFalling(float deltaTime, int32 Iterations) override;

public:
    FOnCanGrabLedgeSignature OnCanGrabLedge;

    /** Overridden to implement double jumps */
    virtual bool DoJump(bool bReplayingMoves) override;

    UPROPERTY(Category = Dodging, EditAnywhere, BlueprintReadWrite)
    float DodgeImpulseHorizontal;

    UPROPERTY(Category = Dodging, EditAnywhere, BlueprintReadWrite)
    float DodgeImpulseVertical;

public:
	URACharacterMovementComponent(const class FObjectInitializer& ObjectInitializer);

    virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

    virtual void UpdateFromCompressedFlags(uint8 Flags) override;

    /** Tick */
    virtual void TickComponent
    (
        float DeltaTime,
        enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    void CheckForLedgeGrabAndFireEvent();

    /** Double tap entry points, used for dodging */
    virtual bool DoDoubleTapForward();
    virtual bool DoDoubleTapBackward();
    virtual bool DoDoubleTapRight();
    virtual bool DoDoubleTapLeft();

    UFUNCTION(BlueprintCallable, Category = "RuneArena")
    bool GetIsDodging();

    virtual bool CanCrouchInCurrentState() const override;

    /** Ledge grabbing */
    void SetLedgeGrabColliderComponent(UPrimitiveComponent* LedgeGrabColliderComponent);
    void GetLedgeGrabCandidateVectors(FVector& CandidateLocation, FVector& CandidateGroundNormal, FVector& CandidateWallNormal);

    /** Networking */
    virtual void GetLifetimeReplicatedProps(TArray < FLifetimeProperty >& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    bool PerformDodge(bool bForward, bool bBackward, bool bRight, bool bLeft);

    /** Overridden for clearing dodge flag */
    virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
};

/** Custom SavedMove struct for dodge functionality */
class RUNEARENA_API FSavedMove_RACharacter : public FSavedMove_Character
{
public:
    using Super = FSavedMove_Character;

    FSavedMove_RACharacter()
    {}

    bool bPressedDoubleForward;
    bool bPressedDoubleBackward;
    bool bPressedDoubleRight;
    bool bPressedDoubleLeft;

    virtual void Clear() override;
    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
    virtual uint8 GetCompressedFlags() const override;

    enum RACompressedFlags
    {
        // FLAG_JumpPressed = 0x01,
        // FLAG_WantsToCrouch = 0x02,
        FLAG_DoubleForwardPressed = 0x04,
        FLAG_DoubleBackwardPressed = 0x08,
        FLAG_DoubleRightPressed = 0x10,
        FLAG_DoubleLeftPressed = 0x20
    };
};

/** Custom client network prediction struct for dodge functionality */
class RUNEARENA_API FNetworkPredictionData_Client_RACharacter : public FNetworkPredictionData_Client_Character
{
public:
    using Super = FNetworkPredictionData_Client_Character;

    FNetworkPredictionData_Client_RACharacter(const UCharacterMovementComponent& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement) {}

    virtual FSavedMovePtr AllocateNewMove() override;
};