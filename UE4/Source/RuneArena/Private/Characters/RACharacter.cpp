// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RACharacter.h"
#include "Characters/Components/RACharacterMovementComponent.h"
#include "Characters/Components/RACharacterSkeletalMeshComponent.h"
#include "Characters/Components/RACameraBoomComponent.h"
#include "Characters/Abilities/AttributeSets/RAAttributeSetBase.h"
#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/Abilities/RAGameplayAbility.h"
#include "Characters/Abilities/RAGA_Attack.h"
#include "Characters/RACharacterEquipSetAsset.h"
#include "GameModes/RAGameModeBase.h"
#include "RAInteractableInterface.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAWeapon.h"
#include "Inventory/RAShield.h"

#include "Player/RAPlayerState.h"
#include "Player/RAPlayerController.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"	// BreakRotIntoAxes
#include "Kismet/GameplayStatics.h"		// GetAllActorsWithInterface
#include "Engine/World.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"				// Camera

using URACharacter_SkeletalMeshComponentType = URACharacterSkeletalMeshComponent;
using URACharacter_AbilitySystemComponentType = URAAbilitySystemComponent;
using URACharacter_AttributeSetBaseType = URAAttributeSetBase;
using URACharacter_CameraBoomComponentType = URACameraBoomComponent;

// Sets default values
ARACharacter::ARACharacter(const FObjectInitializer& ObjectInitializer)
    : Super
(
    ObjectInitializer
    .SetDefaultSubobjectClass<URACharacterMovementComponent>(ACharacter::CharacterMovementComponentName)
    .SetDefaultSubobjectClass<URACharacterSkeletalMeshComponent>(ACharacter::MeshComponentName)
)
{
    // CameraBoomComponent
    CameraBoomComponent = Cast<USpringArmComponent>(CreateDefaultSubobject<URACharacter_CameraBoomComponentType>(TEXT("CameraBoom")));
    check(CameraBoomComponent != nullptr);
    CameraBoomComponent->bUsePawnControlRotation = true;
    CameraBoomComponent->SetupAttachment(RootComponent);

    // CameraComponent
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(CameraBoomComponent);

    // Interaction volume
    InteractionVolumeComponent = Cast<UPrimitiveComponent>(CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolumeComponent")));
    InteractionVolumeComponent->SetupAttachment(RootComponent);
    InteractionVolumeComponent->SetHiddenInGame(true);
    InteractionVolumeComponent->SetCollisionProfileName(FName("Interactable"));

    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    bPressedDoubleForward = false;
    bPressedDoubleBackward = false;
    bPressedDoubleRight = false;
    bPressedDoubleLeft = false;

    // Ledge grab collider
    LedgeGrabColliderComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("LedgeGrabColliderComponent"));
    LedgeGrabColliderComponent->SetupAttachment(RootComponent);
    LedgeGrabColliderComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    LedgeGrabColliderComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    LedgeGrabColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
    LedgeGrabColliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
    LedgeGrabColliderComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

    // Movement component
    RACharacterMovementComponent = Cast<URACharacterMovementComponent>(GetCharacterMovement());
    if (RACharacterMovementComponent != nullptr)
    {
        RACharacterMovementComponent->SetLedgeGrabColliderComponent(LedgeGrabColliderComponent);
        RACharacterMovementComponent->OnCanGrabLedge.AddDynamic(this, &ARACharacter::HandleCanGrabLedge);
    }

    // Ability system component
    AbilitySystemComponent = CreateDefaultSubobject<URACharacter_AbilitySystemComponentType>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("State.IgnoreRotationInput")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARACharacter::HandleOnGameplayTagCountChanged);

    AttributeSetBase = CreateDefaultSubobject<URACharacter_AttributeSetBaseType>(TEXT("AttributeSetBase"));

    // SkeletalMesh
    // This options MUST be enabled when using animation-driven collision detection.
    // Without this enabled, the dedicated server does NOT update the pose, and will not perform accurate traces
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

    DeathCleanupTimerSeconds = 15.0f;
}

void ARACharacter::Reset()
{
    // Super::Reset destroys this pawn, so don't call it
    InitializeDefaultAttributes();
}

void ARACharacter::OnRep_OwnerTeamInfo()
{
    K2_OnOwnerTeamInfoChanged(OwnerTeamInfo);
    ApplyTeamFeaturesToCharacter(OwnerTeamInfo);
}

void ARACharacter::SetOwnerTeamInfo(ARATeamInfo* NewOwnerTeamInfo)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }
    OwnerTeamInfo = NewOwnerTeamInfo;
    K2_OnOwnerTeamInfoChanged(OwnerTeamInfo);
    ApplyTeamFeaturesToCharacter(OwnerTeamInfo);
}

void ARACharacter::ApplyTeamFeaturesToCharacter(ARATeamInfo* TeamInfo)
{
    K2_ApplyTeamFeaturesToCharacter(TeamInfo);
}

void ARACharacter::NotifyDeathAbilityEnded()
{
    //if (!HasAuthority())
    //{
    //    return;
    //}
    //
    //UWorld* WorldInstance = GetWorld();
    //if (WorldInstance != nullptr)
    //{
    //    ARAGameModeBase* GameMode = Cast<ARAGameModeBase>(WorldInstance->GetAuthGameMode());
    //    if (GameMode != nullptr)
    //    {
    //        GameMode->OnCharacterDied(GetController(), this);
    //        if (AbilitySystemComponent != nullptr)
    //        {
    //            AbilitySystemComponent->DestroyActiveState();
    //        }
    //        Destroy();
    //    }
    //}
}

void ARACharacter::Destroyed()
{
    ClearInventory(ERACharacterInventoryReleasePolicy::ReleaseAndDestroy);

    if (HasAuthority())
    {
        RemoveGrantedStartupCharacterAbilityClasses();
    }

    Super::Destroyed();
}

void ARACharacter::HandleOnGameplayTagCountChanged(const FGameplayTag GameplayTag, int32 GameplayTagCount)
{
    if (GameplayTag == FGameplayTag::RequestGameplayTag(FName("State.IgnoreRotationInput")))
    {
        if (GameplayTagCount > 0)
        {
            bUseControllerRotationYaw = false;
        }
        else
        {
            bUseControllerRotationYaw = true;
        }
    }
}

void ARACharacter::HandleCanGrabLedge()
{
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    FGameplayTagContainer ActivationTags;
    ActivationTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.LedgeGrabClimb")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
}



void ARACharacter::CheckJumpInput(float DeltaTime)
{
    if (RACharacterMovementComponent != NULL)
    {
        if (bPressedJump)
        {
            //JumpKeyHoldTime += DeltaTime;

            //if (RACharacterMovementComponent->IsFalling() && JumpCurrentCount == 0)
            //{
            //    JumpCurrentCount++;
            //}

            const bool bDidJump = CanJump() && RACharacterMovementComponent->DoJump(bClientUpdating);
            if (!bWasJumping && bDidJump)
            {
                //JumpCurrentCount++;
                OnJumped();
            }
            
            bWasJumping = bDidJump;
        }
        else if (bPressedDoubleForward || bPressedDoubleBackward || bPressedDoubleRight || bPressedDoubleLeft)
        {
            Dodge();
        }

        //// If the jump key is no longer pressed and the character is no longer falling,
        //// but it still "looks" like the character was jumping, reset the counters.
        //else if (bWasJumping && !RACharacterMovementComponent->IsFalling())
        //{
        //    ResetJumpState();
        //}
    }
}

// Overridden to ignore hold time and max jumps
bool ARACharacter::CanJumpInternal_Implementation() const
{
    // Ensure the character isn't currently crouched.
    bool bCanJump = !bIsCrouched;

    // Ensure that the CharacterMovement state is valid
    //bCanJump &= CharacterMovement->CanAttemptJump();

    URACharacterMovementComponent* CMC = Cast<URACharacterMovementComponent>(GetCharacterMovement());
    if (CMC == nullptr)
    {
        return false;
    }

    bCanJump &= CMC->CanAttemptJump();

    if (bCanJump)
    {
        if (CMC->IsFalling())
        {
            return false;
        }
        //// Ensure JumpHoldTime and JumpCount are valid.
        //if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
        //{
        //    if (JumpCurrentCount == 0 && CharacterMovement->IsFalling())
        //    {
        //        bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
        //    }
        //    else
        //    {
        //        bCanJump = JumpCurrentCount < JumpMaxCount;
        //    }
        //}
        //else
        //{
        //    // Only consider JumpKeyHoldTime as long as:
        //    // A) The jump limit hasn't been met OR
        //    // B) The jump limit has been met AND we were already jumping
        //    const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
        //    bCanJump = bJumpKeyHeld &&
        //        ((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
        //}
    }

    return bCanJump;
}

void ARACharacter::ClearJumpInput(float DeltaTime)
{
    //if (bPressedJump)
    //{
    //    JumpKeyHoldTime += DeltaTime;
    //
    //    // Don't disable bPressedJump right away if it's still held.
    //    // Don't modify JumpForceTimeRemaining because a frame of update may be remaining.
    //    if (JumpKeyHoldTime >= GetJumpMaxHoldTime())
    //    {
    //        bPressedJump = false;
    //    }
    //}
    //else
    //{
    //    JumpForceTimeRemaining = 0.0f;
    //    bWasJumping = false;
    //}

    ClearDodgeInput();
}

void ARACharacter::ResetJumpState()
{
    //bPressedJump = false;
    //bWasJumping = false;
    //JumpKeyHoldTime = 0.0f;
    //JumpForceTimeRemaining = 0.0f;

    //if (CharacterMovement && !CharacterMovement->IsFalling())
    //{
    //    JumpCurrentCount = 0;
    //    JumpCurrentCountPreJump = 0;
    //}
}

bool ARACharacter::CanDodge() const
{
    // Experimenting with wall dodges
    return (RACharacterMovementComponent != NULL);//&& RACharacterMovementComponent->IsMovingOnGround());
}

bool ARACharacter::Dodge()
{
    if (CanDodge())
    {
        if (RACharacterMovementComponent != NULL)
        {
            bool bMovingOnGround = RACharacterMovementComponent->IsMovingOnGround();

            if (bPressedDoubleForward)
            {
                if (AbilitySystemComponent != nullptr)
                {
                    FGameplayTag DodgeAbilityTag;
                    if (bMovingOnGround)
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Forward"));
                    }
                    else
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Forward"));
                    }
                    FGameplayEventData EventData;
                    EventData.EventTag = DodgeAbilityTag;
                    AbilitySystemComponent->HandleGameplayEvent(DodgeAbilityTag, &EventData);
                }
                //RACharacterMovementComponent->DoDoubleTapForward();
                return true;
            }
            if (bPressedDoubleBackward)
            {
                if (AbilitySystemComponent != nullptr)
                {
                    FGameplayTag DodgeAbilityTag;
                    if (bMovingOnGround)
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Backward"));
                    }
                    else
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Backward"));
                    }
                    FGameplayEventData EventData;
                    EventData.EventTag = DodgeAbilityTag;
                    AbilitySystemComponent->HandleGameplayEvent(DodgeAbilityTag, &EventData);
                }
                //RACharacterMovementComponent->DoDoubleTapBackward();
                return true;
            }
            if (bPressedDoubleRight)
            {
                if (AbilitySystemComponent != nullptr)
                {
                    FGameplayTag DodgeAbilityTag;
                    if (bMovingOnGround)
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Right"));
                    }
                    else
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Right"));
                    }
                    FGameplayEventData EventData;
                    EventData.EventTag = DodgeAbilityTag;
                    AbilitySystemComponent->HandleGameplayEvent(DodgeAbilityTag, &EventData);
                }
                //RACharacterMovementComponent->DoDoubleTapRight();
                return true;
            }
            if (bPressedDoubleLeft)
            {
                if (AbilitySystemComponent != nullptr)
                {
                    FGameplayTag DodgeAbilityTag;
                    if (bMovingOnGround)
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Left"));
                    }
                    else
                    {
                        DodgeAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Left"));
                    }
                    FGameplayEventData EventData;
                    EventData.EventTag = DodgeAbilityTag;
                    AbilitySystemComponent->HandleGameplayEvent(DodgeAbilityTag, &EventData);
                }
                //RACharacterMovementComponent->DoDoubleTapLeft();
                return true;
            }
        }
    }

    ClearDodgeInput();
    // NeedsClientAdjustment()?
    return false;
}

void ARACharacter::ClearDodgeInput()
{
    bPressedDoubleForward = false;
    bPressedDoubleBackward = false;
    bPressedDoubleRight = false;
    bPressedDoubleLeft = false;
}









bool ARACharacter::IsIgnoringMovementInput()
{
    if (AbilitySystemComponent != nullptr)
    {
        if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.IgnoreMovementInput"))))
        {
            return true;
        }
    }
    return false;
}

void ARACharacter::MoveForward(float Value)
{
    if (IsIgnoringMovementInput())
    {
        return;
    }

    if (Value != 0.0f)
    {
        const FRotator Rotation = GetControlRotation();
        FRotator YawRotation = FRotator(0, Rotation.Yaw, 0);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void ARACharacter::MoveRight(float Value)
{
    if (IsIgnoringMovementInput())
    {
        return;
    }

    if (Value != 0.0f)
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void ARACharacter::MoveUp(float Value)
{
    if (IsIgnoringMovementInput())
    {
        return;
    }

    if (Value != 0.0f)
    {
        AddMovementInput(FVector(0.0f, 0.0f, 1.0f), Value);
    }
}

void ARACharacter::DoubleTap(bool bForward, bool bBackward, bool bRight, bool bLeft)
{
    if (IsIgnoringMovementInput())
    {
        return;
    }

    bPressedDoubleForward = bForward;
    bPressedDoubleBackward = bBackward;
    bPressedDoubleRight = bRight;
    bPressedDoubleLeft = bLeft;
}

static ERADirection GetVectorDirectionEnum(FVector Vector, FRotator Rotator)
{
	Vector.Normalize();
	FRotator RotatorYaw(0.0f, Rotator.Yaw, 0.0f);
	FVector RotatorX, RotatorY, RotatorZ;
	UKismetMathLibrary::BreakRotIntoAxes(RotatorYaw, RotatorX, RotatorY, RotatorZ);

	float AccelDPForward = FVector::DotProduct(Vector, RotatorX);
	float AccelDPRight = FVector::DotProduct(Vector, RotatorY);
	const float Epsilon = 0.1f;

	if (AccelDPRight > Epsilon)
	{
		return ERADirection::Right;
	}
	if (AccelDPRight < -Epsilon)
	{
		return ERADirection::Left;
	}
	if (AccelDPForward > Epsilon)
	{
		return ERADirection::Front;
	}
	if (AccelDPForward < -Epsilon)
	{
		return ERADirection::Back;
	}
	return ERADirection::Neutral;
}

ERADirection ARACharacter::GetInputDirectionEnum()
{
    if (GetController() == nullptr)
    {
        return ERADirection::Neutral;
    }

    float AxisMoveForward = GetController()->GetInputAxisValue(TEXT("MoveForward"));
    float AxisMoveBackward = GetController()->GetInputAxisValue(TEXT("MoveBackward"));
    float AxisMoveRight = GetController()->GetInputAxisValue(TEXT("MoveRight"));
    float AxisMoveLeft = GetController()->GetInputAxisValue(TEXT("MoveLeft"));

    FVector Direction = FVector(0.0f);

    if (AxisMoveForward != 0.0f)
    {
        Direction += FVector(1.0f, 0.0f, 0.0f);
    }
    if (AxisMoveBackward != 0.0f)
    {
        Direction += FVector(-1.0f, 0.0f, 0.0);
    }
    if (AxisMoveRight != 0.0f)
    {
        Direction += FVector(0.0f, 1.0f, 0.0f);
    }
    if (AxisMoveLeft != 0.0f)
    {
        Direction += FVector(0.0, -1.0f, 0.0f);
    }

    Direction.Normalize();
    
    float DirectionDotForward = FVector::DotProduct(Direction, FVector(1.0f, 0.0f, 0.0f));
    if (DirectionDotForward > 0.8f)
    {
        return ERADirection::Front;
    }
    if (DirectionDotForward < -0.8f)
    {
        return ERADirection::Back;
    }

    float DirectionDotRight = FVector::DotProduct(Direction, FVector(0.0f, 1.0f, 0.0f));
    if (DirectionDotRight > 0.2f)
    {
        return ERADirection::Right;
    }
    if (DirectionDotRight < -0.2f)
    {
        return ERADirection::Left;
    }

    return ERADirection::Neutral;
}

ERADirection ARACharacter::GetAccelerationDirectionEnum()
{
	FVector Acceleration = GetCharacterMovement()->GetCurrentAcceleration();
	FRotator Rotator = GetControlRotation();
	return GetVectorDirectionEnum(Acceleration, Rotator);
}

ERADirection ARACharacter::GetVelocityDirectionEnum()
{
	FVector Velocity = GetCharacterMovement()->Velocity;
	FRotator Rotator = GetActorRotation();
	return GetVectorDirectionEnum(Velocity, Rotator);
}

UAbilitySystemComponent* ARACharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARACharacter::BeginPlay()
{
	Super::BeginPlay();
	
    check(AbilitySystemComponent != nullptr); // Character is useless without the ASC

    URACharacter_AttributeSetBaseType* RAASB = Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase);
    check(RAASB != nullptr); // Character is also useless without the ASB

    // Listen for attribute change events from the ASC
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetMaxHealthAttribute()).AddUObject(this, &ARACharacter::HandleOnMaxHealthChange);
    OnHealthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetHealthAttribute()).AddUObject(this, &ARACharacter::HandleOnHealthChange);
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetMaxStrengthAttribute()).AddUObject(this, &ARACharacter::HandleOnMaxStrengthChange);
    OnStrengthChangeDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetStrengthAttribute()).AddUObject(this, &ARACharacter::HandleOnStrengthChange);

    // Listen for damage events directly from the ASB
    RAASB->OnPostDamageReceived.AddUObject(this, &ARACharacter::HandleOnPostDamageReceived);

    K2_OnSpawned();
}

void ARACharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (CheckIsDead())
    {
        return;
    }

    UpdateInteractableActorCurrent();
}

void ARACharacter::UpdateInteractableActorCurrent(bool bInvalidate)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    AActor* BestInteractableActor = nullptr;
    if (!bInvalidate)
    {
        BestInteractableActor = FindBestInteractableActor();
    }

    if (InteractableActorCurrent != BestInteractableActor)
    {
        InteractableActorPrevious = InteractableActorCurrent;
        InteractableActorCurrent = BestInteractableActor;
        if (OnInteractableActorCurrentChanged.IsBound())
        {
            OnInteractableActorCurrentChanged.Broadcast(InteractableActorPrevious, InteractableActorCurrent);
        }
    }
}

void ARACharacter::OnRep_InteractableActorCurrent()
{
    if (OnInteractableActorCurrentChanged.IsBound())
    {
        OnInteractableActorCurrentChanged.Broadcast(InteractableActorPrevious, InteractableActorCurrent);
    }
    InteractableActorPrevious = InteractableActorCurrent;
}

AActor* ARACharacter::FindBestInteractableActor()
{
    TArray<AActor*> OverlappingActors;
    InteractionVolumeComponent->GetOverlappingActors(OverlappingActors);

    TArray<AActor*> OverlappingInteractables;
    for (auto It : OverlappingActors)
    {
        IRAInteractableInterface* Interactable = Cast<IRAInteractableInterface>(It);

        if (Interactable == nullptr || !Interactable->CanBeInteractedWith())
        {
            continue;
        }

        ARANewInventory* InteractableInventory = Cast<ARANewInventory>(Interactable);
        if (InteractableInventory != nullptr && !CheckCanAcquireInventory(InteractableInventory))
        {
            continue;
        }

        OverlappingInteractables.Add(It);
    }

    OverlappingInteractables.Sort
    (
        [](const AActor& A, const AActor& B)
        {
            const IRAInteractableInterface* InteractableA = Cast<IRAInteractableInterface>(&A);
            const IRAInteractableInterface* InteractableB = Cast<IRAInteractableInterface>(&B);
            return InteractableA->GetInteractionPriority() < InteractableB->GetInteractionPriority();
        }
    );

    if (OverlappingInteractables.Num() == 0)
    {
        return nullptr;
    }
    return OverlappingInteractables[0];
}

void ARACharacter::RemoveGrantedStartupCharacterAbilityClasses()
{
    if (GetLocalRole() != ROLE_Authority || AbilitySystemComponent == nullptr)
    {
        return;
    }

    URACharacter_AbilitySystemComponentType* ASC = Cast<URACharacter_AbilitySystemComponentType>(AbilitySystemComponent);
    if (ASC == nullptr || !ASC->bCharacterAbilitiesGiven)
    {
        return;
    }

    // Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
    TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
    {
        if ((Spec.SourceObject == this) && StartupCharacterAbilityClasses.Contains(Spec.Ability->GetClass()))
        {
            AbilitiesToRemove.Add(Spec.Handle);
        }
    }

    // Do in two passes so the removal happens after we have the full list
    for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
    {
        ASC->ClearAbility(AbilitiesToRemove[i]);
    }

    ASC->bCharacterAbilitiesGiven = false;
}

void ARACharacter::HandleOnHealthChange(const FOnAttributeChangeData& Data)
{
    if (OnHealthChange.IsBound())
    {
        OnHealthChange.Broadcast(Data.NewValue);
    }

    // Check for death
    if (Data.NewValue <= 0.0f)
    {
        APawn* InstigatorPawn = GetInstigator<APawn>();
        AController* InstigatorController = nullptr;
        if (InstigatorPawn != nullptr)
        {
            InstigatorController = InstigatorPawn->GetController<AController>();
        }

        Died(InstigatorController);
    }
}

void ARACharacter::HandleOnMaxHealthChange(const FOnAttributeChangeData & Data)
{
    if (OnMaxHealthChange.IsBound())
    {
        OnMaxHealthChange.Broadcast(Data.NewValue);
    }
}

void ARACharacter::HandleOnStrengthChange(const FOnAttributeChangeData& Data)
{
    if (OnStrengthChange.IsBound())
    {
        OnStrengthChange.Broadcast(Data.NewValue);
    }

    // Do nothing for now
    // Probably want to check for bloodlust here in the future though?
}

void ARACharacter::HandleOnMaxStrengthChange(const FOnAttributeChangeData& Data)
{
    if (OnMaxStrengthChange.IsBound())
    {
        OnMaxStrengthChange.Broadcast(Data.NewValue);
    }
}

void ARACharacter::HandleOnPostDamageReceived(AActor* Source, const FHitResult& HitResult, float DamageDealt)
{
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    // For right now, it is assumed that the death ability will always take over if Health <= 0.0
    // To avoid activating the TakeDamage ability and then immediately canceling it right after,
    // only perform TakeDamage if Health is > 0.0
    if (GetHealth() > 0.0f)
    {
        // Take damage ability will play the hit reaction animation
        // Cancel any active TakeDamage ability so that it can reactivate
        FGameplayTagContainer ActivationTags;
        ActivationTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.TakeDamage"));
        AbilitySystemComponent->CancelAbilities(&ActivationTags);
        AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
    }

    // Broadcast important events
    if (GetLocalRole() == ROLE_Authority)
    {
        // For client-side effects
        NetMulticastCharacterTakeDamage(HitResult.Location, HitResult.Normal, DamageDealt);

        // For hurt confirmers
        if (OnDamageTaken.IsBound())
        {
            OnDamageTaken.Broadcast(Source, DamageDealt);
        }

        // For hit confirmers
        ARACharacter* CharacterSource = Cast<ARACharacter>(Source);
        if (CharacterSource != nullptr && CharacterSource->OnDamageGiven.IsBound())
        {
            CharacterSource->OnDamageGiven.Broadcast(this, DamageDealt);
        }
    }
}

void ARACharacter::NetMulticastCharacterTakeDamage_Implementation(const FVector& DamageLocation, const FVector& DamageNormal, float DamageDealt)
{
    K2_OnCharacterTakeDamage(DamageLocation, DamageNormal, DamageDealt);
}

// Serverside initialization
void ARACharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystemComponent != nullptr)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        InitializeDefaultAttributes();
        ApplyAllStartupEffects();
        GrantStartupCharacterAbilityClasses();

        //ARAPlayerController* PC = Cast<ARAPlayerController>(GetController());
        //if (PC != nullptr)
        //{
        //    PC->CreateHUD();
        //}
    }

    SetOwner(NewController);
}

void ARACharacter::InitializeDefaultAttributes()
{
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    if (DefaultAttributes == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
        return;
    }

    // Can run on server and client
    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, 0, EffectContext);
    if (NewHandle.IsValid())
    {
        FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent);
    }
}

void ARACharacter::ApplyAllStartupEffects()
{
    if (GetLocalRole() != ROLE_Authority || AbilitySystemComponent == nullptr)
    {
        return;
    }

    URACharacter_AbilitySystemComponentType* ASC = Cast<URACharacter_AbilitySystemComponentType>(AbilitySystemComponent);
    if (ASC == nullptr || ASC->bStartupEffectsApplied)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    for (auto& It : StartupEffects)
    {
        FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(It, 0, EffectContext);
        if (NewHandle.IsValid())
        {
            FActiveGameplayEffectHandle ActiveGEHandle = ASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), ASC);
        }
    }

    ASC->bStartupEffectsApplied = true;
}

void ARACharacter::GrantStartupCharacterAbilityClasses()
{
    // Grant abilities on the server
    if (GetLocalRole() != ROLE_Authority || AbilitySystemComponent == nullptr)
    {
        return;
    }

    URACharacter_AbilitySystemComponentType* ASC = Cast<URACharacter_AbilitySystemComponentType>(AbilitySystemComponent);
    if (ASC == nullptr || ASC->bCharacterAbilitiesGiven)
    {
        return;
    }

    // Add startup abilities
    for (auto& It : StartupCharacterAbilityClasses)
    {
        FGameplayAbilitySpec GameplayAbilitySpec(It, 1, -1, this);
        FGameplayAbilitySpecHandle GameplayAbilitySpecHandle = ASC->GiveAbility(GameplayAbilitySpec);
    }

    ASC->bCharacterAbilitiesGiven = true;
}

void ARACharacter::CameraIn()
{
    URACharacter_CameraBoomComponentType* CBC = Cast<URACharacter_CameraBoomComponentType>(CameraBoomComponent);
    if (CBC != nullptr)
    {
        CBC->DecrementArmLength();
    }
}

void ARACharacter::CameraOut()
{
    URACharacter_CameraBoomComponentType* CBC = Cast<URACharacter_CameraBoomComponentType>(CameraBoomComponent);
    if (CBC != nullptr)
    {
        CBC->IncrementArmLength();
    }
}

float ARACharacter::GetHealth() const
{
    return Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase)->GetHealth();
}

FOnGameplayAttributeValueChange& ARACharacter::GetOnHealthChangeDelegate()
{
    URACharacter_AttributeSetBaseType* RAASB = Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase);
    return AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetHealthAttribute());
}

float ARACharacter::GetMaxHealth() const
{
    return Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase)->GetMaxHealth();
}

FOnGameplayAttributeValueChange& ARACharacter::GetOnMaxHealthChangeDelegate()
{
    URACharacter_AttributeSetBaseType* RAASB = Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase);
    return AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetMaxHealthAttribute());
}

float ARACharacter::GetStrength() const
{
    return Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase)->GetStrength();
}

FOnGameplayAttributeValueChange& ARACharacter::GetOnStrengthChangeDelegate()
{
    URACharacter_AttributeSetBaseType* RAASB = Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase);
    return AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetStrengthAttribute());
}

float ARACharacter::GetMaxStrength() const
{
    return Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase)->GetMaxStrength();
}

FOnGameplayAttributeValueChange& ARACharacter::GetOnMaxStrengthChangeDelegate()
{
    URACharacter_AttributeSetBaseType* RAASB = Cast<URACharacter_AttributeSetBaseType>(AttributeSetBase);
    return AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RAASB->GetMaxStrengthAttribute());
}

void ARACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARACharacter, OwnerTeamInfo);
    DOREPLIFETIME(ARACharacter, Weapon);
    DOREPLIFETIME(ARACharacter, Shield);
    DOREPLIFETIME_CONDITION(ARACharacter, InteractableActorCurrent, COND_OwnerOnly);
}

URACharacterEquipSetAsset* ARACharacter::GetCurrentEquipSetAsset()
{
    URACharacterEquipSetAsset* Result = nullptr;
    if (Weapon != nullptr && EquipSetAssetMap.Find(Weapon->GetClass()) != nullptr)
    {
        Result = EquipSetAssetMap[Weapon->GetClass()];
    }

    if (Result == nullptr)
    {
        Result = EquipSetAssetDefault;
    }

    return Result;
}

UBlendSpace* ARACharacter::GetCurrentLocomotionBlendSpace()
{
    URACharacterEquipSetAsset* EquipSetAsset = GetCurrentEquipSetAsset();
    if (EquipSetAsset == nullptr)
    {
        return nullptr;
    }
    
    UBlendSpace* Result = nullptr;
    if (bIsCrouched)
    {
        Result = EquipSetAsset->Locomotion_Crouching;
    }

    if (Result == nullptr)
    {
        Result = EquipSetAsset->Locomotion_Standing;
    }

    return Result;
}

void ARACharacter::GetCurrentAttackChain(TArray<class UAnimMontage*>* OutAttackChain, TArray<class UAnimMontage*>* OutRecoveryMontages)
{
    OutAttackChain->Empty();
    OutRecoveryMontages->Empty();

    URACharacterEquipSetAsset* EquipSetAsset = GetCurrentEquipSetAsset();
    if (EquipSetAsset == nullptr)
    {
        return;
    }

    FCombatAnimStateSet* CombatAnimStateSet = &EquipSetAsset->CombatSet;
    if (CombatAnimStateSet == nullptr)
    {
        return;
    }

    bool bIsDodging = false;
    if (RACharacterMovementComponent != nullptr)
    {
        bIsDodging = RACharacterMovementComponent->GetIsDodging();
    }

    bool bIsFalling = false;
    //ERADirection AccelDirection = GetAccelerationDirectionEnum();
    //ERADirection AccelDirection = GetInputDirectionEnum();
    ERADirection AttackDirection = DoAttackDirection;
    if (RACharacterMovementComponent != nullptr)
    {
        if (RACharacterMovementComponent->IsFalling() && AttackDirection == ERADirection::Front && !bIsDodging)
        {
            bIsFalling = true;
        }
    }

    FCombatAnimSet* CombatAnimSet = &CombatAnimStateSet->Neutral;
    if (bIsFalling)
    {
        CombatAnimSet = &CombatAnimStateSet->Falling;
    }

    if (CombatAnimSet == nullptr)
    {
        return;
    }

    TArray<FCombatAnim>* CombatAnimArray = nullptr;

    switch (AttackDirection)
    {
    case ERADirection::Neutral:
        CombatAnimArray = &CombatAnimSet->Neutral;
        break;
    case ERADirection::Front:
        CombatAnimArray = &CombatAnimSet->Front;
        break;
    case ERADirection::Back:
        CombatAnimArray = &CombatAnimSet->Back;
        break;
    case ERADirection::Left:
        CombatAnimArray = &CombatAnimSet->Left;
        break;
    case ERADirection::Right:
        CombatAnimArray = &CombatAnimSet->Right;
        break;
    }

    if (CombatAnimArray == nullptr)
    {
        return;
    }

    for (auto& It : *CombatAnimArray)
    {
        OutAttackChain->Push(It.AttackAnimMontage);
        OutRecoveryMontages->Push(It.RecoverAnimMontage);
    }
}

// Called from player controller
void ARACharacter::ControllerJumpActionPressed()
{
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }
    
    FGameplayTagContainer ActivationTags;
    ActivationTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Jump"));
    AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
}

void ARACharacter::ControllerJumpActionReleased()
{
    FGameplayTagContainer CancelTags;
    CancelTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Jump"));
    AbilitySystemComponent->CancelAbilities(&CancelTags);
}

void ARACharacter::ControllerCrouchActionPressed()
{
    Crouch();
}

void ARACharacter::ControllerCrouchActionReleased()
{
    UnCrouch();
}

void ARACharacter::Attack()
{
    ControllerAttackActionPressed();
}

void ARACharacter::ControllerAttackActionPressed()
{
    ERADirection Direction = GetInputDirectionEnum();
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerAttack(Direction);
        DoAttack(Direction);
    }
    else
    {
        DoAttack(Direction);
    }

    //// Activate the ability locally
    //if (AbilitySystemComponent != nullptr)
    //{
    //    // Try to activate attack ability
    //    FGameplayTagContainer GameplayTagContainer;
    //    GameplayTagContainer.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.Attack"));
    //
    //    if(!AbilitySystemComponent->TryActivateAbilitiesByTag(GameplayTagContainer))
    //    {
    //        if (GetLocalRole() != ROLE_Authority)
    //        {
    //            ServerAttack();
    //            DoAttack();
    //        }
    //        else
    //        {
    //            DoAttack();
    //        }
    //    }
    //}
}

void ARACharacter::ServerAttack_Implementation(ERADirection Direction)
{
    DoAttack(Direction);
}

void ARACharacter::DoAttack(ERADirection Direction)
{
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    DoAttackDirection = Direction;

    FGameplayTagContainer ActivationTags;
    ActivationTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.Attack"));
    if (!AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags))
    {
        const FGameplayEventData GameplayEventData;
        AbilitySystemComponent->HandleGameplayEvent(FGameplayTag::RequestGameplayTag("Ability.Combat.Attack.Combo"), &GameplayEventData);
    }
}

ERADirection ARACharacter::GetDoAttackDirection() const
{
    return DoAttackDirection;
}

void ARACharacter::ControllerAttackActionReleased()
{}

void ARACharacter::ControllerThrowActionPressed()
{
    if (AbilitySystemComponent != nullptr)
    {
        FGameplayTagContainer ActivationTags;
        ActivationTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Throw")));
        AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
    }
}

void ARACharacter::ControllerThrowActionReleased()
{}

void ARACharacter::ControllerUseActionPressed()
{
    Interact();
}

void ARACharacter::Interact()
{
    if (GetLocalRole() != ROLE_Authority)
    {
        InteractSetup(); // Required for ability animation to play out client-side
    }
    ServerInteract();
}

void ARACharacter::ServerInteract_Implementation()
{
    InteractSetup();

    if (InteractableActorCurrent != nullptr)
    {
        IRAInteractableInterface* Interactable = Cast<IRAInteractableInterface>(InteractableActorCurrent);
        FGameplayTag ActivationTag = Interactable->GetInteractionAbilityActivationTag();
        if (ActivationTag.IsValid())
        {
            FGameplayTagContainer ActivationTags;
            ActivationTags.AddTag(ActivationTag);
            AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
        }
    }
}

void ARACharacter::InteractSetup()
{
    if (InteractableActorCurrent != nullptr)
    {
        URAAbilitySystemComponent* ASC = Cast<URAAbilitySystemComponent>(AbilitySystemComponent);
        if (ASC != nullptr)
        {
            ASC->SetInteractionActor(InteractableActorCurrent);
        }
    }
}

void ARACharacter::ControllerUseActionReleased()
{}

const UPrimitiveComponent* ARACharacter::GetInteractionVolumeComponent() const
{
    return InteractionVolumeComponent;
}

void ARACharacter::SelectInventory(int32 InventoryCode)
{
    ServerSelectInventory(InventoryCode);
}

void ARACharacter::ServerSelectInventory_Implementation(int32 InventoryCode)
{
    URAAbilitySystemComponent* ASC = Cast<URAAbilitySystemComponent>(AbilitySystemComponent);
    if (ASC != nullptr)
    {
        ASC->SetPendingSelectInventoryCode(InventoryCode);
        FGameplayTagContainer ActivationTags;
        ActivationTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact.SelectInventory")));
        AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
    }
}

void ARACharacter::ControllerCameraInActionPressed()
{
    CameraIn();
}

void ARACharacter::ControllerCameraInActionReleased()
{}

void ARACharacter::ControllerCameraOutActionPressed()
{
    CameraOut();
}

void ARACharacter::ControllerCameraOutActionReleased()
{}

bool ARACharacter::Died(AController* InstigatorController)
{
    if (GetLocalRole() < ROLE_Authority || CheckIsDead())
    {
        return false;
    }

    // Server - Invalidate the interactable actor so any un-doing effects can play
    UpdateInteractableActorCurrent(true);

    if (Weapon != nullptr)
    {
        ARANewInventory* SavedWeapon = Weapon;
        ReleaseInventory(Weapon, ERACharacterInventoryReleasePolicy::ReleaseAndForget);
        FVector DropLocation = SavedWeapon->GetActorLocation();
        FVector DropVelocity = FVector(0.0f);
        SavedWeapon->DropFrom(DropLocation, DropVelocity);
    }
    ClearInventory(ERACharacterInventoryReleasePolicy::ReleaseAndDestroy);
    TearOff(); // Marks character as dead
    AController* VictimController = Controller;

    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->Killed(InstigatorController, VictimController, this);
    }

    if (VictimController != nullptr)
    {
        VictimController->PawnPendingDestroy(this);
    }

    // TODO: broadcast OnDied delegate

    PlayDying();

    return true;
}

bool ARACharacter::CheckIsDead()
{
    return GetTearOff();
}

void ARACharacter::PlayDying()
{
    FTimerHandle TempTimerHandle;
    GetWorldTimerManager().SetTimer(TempTimerHandle, this, &ARACharacter::DeathCleanupTimer, DeathCleanupTimerSeconds, false);

    EnableRagdoll();

    K2_OnPlayDying();
}

void ARACharacter::EnableRagdoll()
{
    if (IsActorBeingDestroyed() || !GetMesh()->IsRegistered())
    {
        return;
    }

    UCharacterMovementComponent* TempMovementComponent = GetCharacterMovement();
    if (TempMovementComponent != nullptr)
    {
        TempMovementComponent->UnCrouch(true);
        if (RootComponent == GetMesh() && GetMesh()->IsSimulatingPhysics())
        {
            return;
        }
    }

    SetActorEnableCollision(true);
    if (!GetMesh()->ShouldTickPose())
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance == nullptr || !AnimInstance->IsPostUpdatingAnimation())
        {
            GetMesh()->TickAnimation(0.0f, false);
            GetMesh()->RefreshBoneTransforms();
            GetMesh()->UpdateComponentToWorld();
        }
    }
    TempMovementComponent->ApplyAccumulatedForces(0.0f);
    TempMovementComponent->StopActiveMovement();
    TempMovementComponent->Velocity = FVector::ZeroVector;

    if (GetMesh()->GetBodyInstance() != nullptr)
    {
        GetMesh()->GetBodyInstance()->bUseCCD = true;
    }

    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetAllBodiesNotifyRigidBodyCollision(true);
    GetMesh()->UpdateKinematicBonesToAnim(GetMesh()->GetComponentSpaceTransforms(), ETeleportType::TeleportPhysics, true);
    GetMesh()->SetAllPhysicsLinearVelocity(GetMovementComponent()->Velocity, false);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->RefreshBoneTransforms();
    GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
    GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    RootComponent = GetMesh();
    GetMesh()->SetGenerateOverlapEvents(true);
    GetMesh()->SetShouldUpdatePhysicsVolume(true);
    GetMesh()->RegisterClothTick(true);
    GetCapsuleComponent()->SetSimulatePhysics(false);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    GetCapsuleComponent()->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform);
}

void ARACharacter::DeathCleanupTimer()
{
    Destroy();
}

void ARACharacter::DieStart()
{
//    if (AbilitySystemComponent == nullptr)
//    {
//        return;
//    }
//    
//    AbilitySystemComponent->CancelAllAbilities();
//    
//    FGameplayTagContainer ActivationTags;
//    ActivationTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Die")));
//    AbilitySystemComponent->TryActivateAbilitiesByTag(ActivationTags);
}

void ARACharacter::TornOff()
{
    Super::TornOff();

    // Client - Invalidate the interactable actor so any un-doing effects can play
    UpdateInteractableActorCurrent(true);

    PlayDying();
}

bool ARACharacter::CheckCanAcquireInventory(ARANewInventory* InventoryToAcquire)
{
    if (InventoryToAcquire == nullptr)
    {
        return false;
    }

    UClass* InventoryClass = InventoryToAcquire->GetClass();
    for (TRAInventoryIterator<ARANewInventory> It(this); It; ++It)
    {
        // Allow only one of each class of inventory
        if (It->GetClass() == InventoryClass)
        {
            return false;
        }
    }

    return true;
}

bool ARACharacter::AcquireInventory(ARANewInventory* InventoryToAcquire, ERACharacterInventoryAcquirePolicy AcquirePolicy)
{
    if (InventoryToAcquire == nullptr)
    {
        return false;
    }

    if (!CheckCanAcquireInventory(InventoryToAcquire))
    {
        return false;
    }

    if (Inventory == nullptr)
    {
        Inventory = InventoryToAcquire;
    }
    else
    {
        ARANewInventory* InventoryTail = Inventory;
        while (InventoryTail->NextInventory != nullptr)
        {
            if (InventoryTail == InventoryToAcquire)
            {
                return false;
            }
            InventoryTail = InventoryTail->NextInventory;
        }

        InventoryTail->NextInventory = InventoryToAcquire;
    }

    InventoryToAcquire->SetOwner(this);
    K2_OnAcquiredInventory(InventoryToAcquire);
    InventoryToAcquire->NotifyAcquired();

    switch (AcquirePolicy)
    {
    case ERACharacterInventoryAcquirePolicy::AcquireAndEquip:
        EquipInventory(InventoryToAcquire);
        break;
    case ERACharacterInventoryAcquirePolicy::AcquireAndStow:
        StowInventory(InventoryToAcquire);
        break;
    }

    return true;
}

void ARACharacter::ReleaseInventory(ARANewInventory* InventoryToRelease, ERACharacterInventoryReleasePolicy ReleasePolicy)
{
    if (Inventory == nullptr || InventoryToRelease == nullptr)
    {
        return;
    }

    if (Inventory == InventoryToRelease)
    {
        Inventory = Inventory->NextInventory;
    }
    else
    {
        ARANewInventory* InventoryIt = Inventory;
        while (InventoryIt->NextInventory != nullptr && InventoryIt->NextInventory != InventoryToRelease)
        {
            InventoryIt = InventoryIt->NextInventory;
        }

        if (InventoryIt->NextInventory == InventoryToRelease)
        {
            InventoryIt->NextInventory = InventoryIt->NextInventory->NextInventory;
        }
        else
        {
            return;
        }
    }

    if (InventoryToRelease == Weapon)
    {
        Weapon = nullptr;
    }

    if (!InventoryToRelease->IsPendingKillPending())
    {
        InventoryToRelease->NextInventory = nullptr;
        InventoryToRelease->SetOwner(nullptr);
        URACharacterSkeletalMeshComponent* CharacterMesh = Cast<URACharacterSkeletalMeshComponent>(GetMesh());
        if (CharacterMesh != nullptr)
        {
            CharacterMesh->DetachActor(InventoryToRelease);
        }
        K2_OnReleasedInventory(InventoryToRelease);

        switch (ReleasePolicy)
        {
        case ERACharacterInventoryReleasePolicy::ReleaseAndDestroy:
            InventoryToRelease->Destroy();
            break;
        case ERACharacterInventoryReleasePolicy::ReleaseAndForget:
            InventoryToRelease->NotifyReleased();
            break;
        }
    }
}

void ARACharacter::ClearInventory(ERACharacterInventoryReleasePolicy ReleasePolicy)
{
    TArray<ARANewInventory*> InventoriesToRelease;
    for (TRAInventoryIterator<ARANewInventory> It(this); It; ++It)
    {
        InventoriesToRelease.Add(*It);
    }

    for (auto It : InventoriesToRelease)
    {
        ReleaseInventory(It, ReleasePolicy);
    }
}

void ARACharacter::EquipInventory(ARANewInventory* InventoryToEquip)
{
    if (InventoryToEquip == nullptr)
    {
        return;
    }

    TRAInventoryIterator<ARANewInventory> It(this);
    while (It && *It != InventoryToEquip)
    {
        ++It;
    }

    if (!It.IsValid())
    {
        return;
    }

    if (Cast<ARAWeapon>(*It) != nullptr)
    {
        if (Weapon != nullptr)
        {
            StowInventory(Weapon);
        }

        URACharacterSkeletalMeshComponent* CharacterMesh = Cast<URACharacterSkeletalMeshComponent>(GetMesh());
        if (CharacterMesh != nullptr)
        {
            Weapon = *It;
            CharacterMesh->AttachActorToWeaponSocket(Weapon);
            Weapon->AddActorLocalOffset(Weapon->GetLocalOffsetForEquip());
            Weapon->NotifyEquipped();
        }
    }
    else if (Cast<ARAShield>(*It) != nullptr)
    {
        if (Shield != nullptr)
        {
            // TODO: Replace this with a drop ability?
            ARANewInventory* InventoryToRelease = Shield;
            ReleaseInventory(InventoryToRelease, ERACharacterInventoryReleasePolicy::ReleaseAndForget);
            Shield = nullptr;
        }

        URACharacterSkeletalMeshComponent* CharacterMesh = Cast<URACharacterSkeletalMeshComponent>(GetMesh());
        if (CharacterMesh != nullptr)
        {
            Shield = *It;
            CharacterMesh->AttachActorToShieldSocket(Shield);
            Shield->AddActorLocalOffset(Shield->GetLocalOffsetForEquip());
            Shield->NotifyEquipped();
        }
    }
}

void ARACharacter::StowInventory(ARANewInventory* InventoryToStow)
{
    if (InventoryToStow == nullptr)
    {
        return;
    }

    TRAInventoryIterator<ARANewInventory> It(this);
    while (It && *It != InventoryToStow)
    {
        ++It;
    }

    if (!It.IsValid())
    {
        return;
    }

    URACharacterSkeletalMeshComponent* CharacterMesh = Cast<URACharacterSkeletalMeshComponent>(GetMesh());
    if (CharacterMesh != nullptr)
    {
        CharacterMesh->AttachActorToStowSocket(*It);
        if (*It == Weapon)
        {
            Weapon = nullptr;
        }
        It->NotifyStowed();
    }
}

ARANewInventory* ARACharacter::GetWeapon()
{
    return Weapon;
}