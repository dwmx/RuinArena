// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Misc/DateTime.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "RAEnum.h"
#include "RuneArena.h"
#include "RACharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableActorCurrentChangedSignature, AActor*, OldInteractableActor, AActor*, NewInteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeValueChange, float, NewAttributeValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTakenSignature, class AActor*, InstigatorActor, float, DamageTaken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageGivenSignature, class AActor*, VictimActor, float, DamageGiven);

/** Defines the behavior of calls made to AcquireInventory */
UENUM(BlueprintType)
enum class ERACharacterInventoryAcquirePolicy : uint8
{
    AcquireAndEquip     UMETA(DisplayName = "Acquire and Equip"),
    AcquireAndStow      UMETA(DisplayName = "Acquire and Stow")
};

/** Defines the behavior of calls made to ReleaseInventory */
UENUM(BlueprintType)
enum class ERACharacterInventoryReleasePolicy : uint8
{
    ReleaseAndForget    UMETA(DisplayName = "Release and Forget"),
    ReleaseAndDestroy   UMETA(DisplayName = "Release and Destroy")
};


// Main Character class
UCLASS()
class RUNEARENA_API ARACharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
    using Super = ACharacter;

public:
    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnAttributeValueChange OnMaxHealthChange;

    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnAttributeValueChange OnHealthChange;

    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnAttributeValueChange OnMaxStrengthChange;

    UPROPERTY(BlueprintAssignable, Category = "Attributes")
    FOnAttributeValueChange OnStrengthChange;

protected:
    /** Third person camera attachment component */
    UPROPERTY(Category = "Character", VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoomComponent;

    /** Primary camera, attaches to CameraBoomComponent */
    UPROPERTY(Category = "Character", VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* CameraComponent;

    /** Collision component which overlaps with interactable objects */
    UPROPERTY(Category = "Character", VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
    class UPrimitiveComponent* InteractionVolumeComponent;

    /** The current actor that would be interacted with if the user were to press the use key */
    UPROPERTY(ReplicatedUsing = OnRep_InteractableActorCurrent, BlueprintReadOnly)
    AActor* InteractableActorCurrent;
    AActor* InteractableActorPrevious;

    UFUNCTION()
    virtual void OnRep_InteractableActorCurrent();

    /** Get new value for InteractableActorCurrent */
    virtual void UpdateInteractableActorCurrent(bool bInvalidate = false);
    virtual AActor* FindBestInteractableActor();

    virtual void Tick(float DeltaSeconds) override;

    /** Collider component which, when falling, checks for grabbable ledges */
    UPROPERTY(Category = "Character", EditAnywhere)
    class UPrimitiveComponent* LedgeGrabColliderComponent;

    /** AbilitySystemComponent which implements all major character functionality */
    UPROPERTY(Category = "Abilities", VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
    class UAbilitySystemComponent* AbilitySystemComponent;

    /** Default set of abilities granted to this Character at startup */
    UPROPERTY(Category = "Abilities", EditAnywhere, BlueprintReadOnly)
    TArray<TSubclassOf<class URAGameplayAbility>> StartupCharacterAbilityClasses;

    /** Grant / remove all of the GameplayAbilities in the StartupCharacterAbilityClasses array */
    virtual void GrantStartupCharacterAbilityClasses();
    virtual void RemoveGrantedStartupCharacterAbilityClasses();

    /** Event listener for AbilitySystemComponent tag changes */
    virtual void HandleOnGameplayTagCountChanged(const FGameplayTag GameplayTag, int32 GameplayTagCount);

    /** AttributeSet which is used in conjunction with the AbilitySystemComponent */
    UPROPERTY()
    class UAttributeSet* AttributeSetBase;

    /** GameplayEffect which applies all default attributes at startup */
    UPROPERTY(Category = "Abilities|Attributes", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class UGameplayEffect> DefaultAttributes;

    /** Applies the DefaultAttributes GameplayEffect to this Character */
    virtual void InitializeDefaultAttributes();

    /** GameplayEffects which are applied to this charater at startup */
    UPROPERTY(Category = "Abilities", EditAnywhere, BlueprintReadOnly)
    TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

    /** Applies all of the GameplayEffects in the StartupEffects array to this Character */
    virtual void ApplyAllStartupEffects();

    /** Listens for Health attribute changes in AttributeSetBase */
    FDelegateHandle OnHealthChangeDelegateHandle;
    virtual void HandleOnHealthChange(const FOnAttributeChangeData& Data);
    void HandleOnMaxHealthChange(const FOnAttributeChangeData& Data);

    /** Listens for Strength attribute changes in AttributeSetBase */
    FDelegateHandle OnStrengthChangeDelegateHandle;
    virtual void HandleOnStrengthChange(const FOnAttributeChangeData& Data);
    void HandleOnMaxStrengthChange(const FOnAttributeChangeData& Data);

public:
    FOnDamageTakenSignature OnDamageTaken;
    FOnDamageGivenSignature OnDamageGiven;

protected:
    /** Listens for OnPostDamageReceived event from AttributeSetBase */
    UFUNCTION()
    virtual void HandleOnPostDamageReceived(AActor* Source, const FHitResult& HitResult, float DamageDealt);

    /** Fired immediately after taking damage */
    UFUNCTION(Category = "Abilities|Attributes", BlueprintImplementableEvent, Meta = (DisplayName = "On Post Damage Received"))
    void BP_OnPostDamageReceived(AActor* Source, const FHitResult& HitResult, float DamageDealt);

    /** The default EquipSetAsset to fall back on */
    UPROPERTY(Category = "Character", EditAnywhere, BlueprintReadOnly)
    class URACharacterEquipSetAsset* EquipSetAssetDefault;

    /** Maps UClasses to EquipSetAssets, to provide unique sets of animations for different held items */
    UPROPERTY(Category = "Character", EditAnywhere, BlueprintReadOnly)
    TMap<UClass*, class URACharacterEquipSetAsset*> EquipSetAssetMap;

protected:
    /** The team info belonging to this character's owner */
    UPROPERTY(ReplicatedUsing = OnRep_OwnerTeamInfo, BlueprintReadOnly, Category = "Teams")
    class ARATeamInfo* OwnerTeamInfo;

    UFUNCTION()
    void OnRep_OwnerTeamInfo();

    /** Client and server, Called when the owner's team has changed */
    UFUNCTION(BlueprintImplementableEvent, Category = "Teams", Meta = (DisplayName = "OnOwnerTeamInfoChanged"))
    void K2_OnOwnerTeamInfoChanged(class ARATeamInfo* NewOwnerTeamInfo);

    /** Called any time the team features of this character need to be applied (respawn, team change, etc)
    *   This is the correct place to do things like update team color, but this is not the right place to
    *   perform team change effects. Use OnOwnerTeamChanged for that.
    *   Called on both client and server.
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "Teams", Meta = (DisplayName = "ApplyTeamFeaturesToCharacter"))
    void K2_ApplyTeamFeaturesToCharacter(class ARATeamInfo* TeamInfo);
    virtual void ApplyTeamFeaturesToCharacter(class ARATeamInfo* TeamInfo);

public:
    void SetOwnerTeamInfo(class ARATeamInfo* NewOwnerTeam);

protected:
    /** Valid on server and client. Fired immediately after CPP BeginPlay */
    UFUNCTION(BlueprintImplementableEvent, Category = "Character", Meta = (DisplayName = "OnSpawned"))
    void K2_OnSpawned();

    /** Overridden to add gameplay abilities at startup */
    virtual void BeginPlay() override;
    virtual void Destroyed() override;

public:
    ARACharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Overridden to reapply default attribute set */
    virtual void Reset() override;

    /** IAbilitySystemInterface implementation */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void PossessedBy(AController* NewController) override;

    /** Currently used for Interact ability, may want to switch out for a GetOverlappingInteractables function */
    const class UPrimitiveComponent* GetInteractionVolumeComponent() const;

    /** Accessor for Health attribute */
    UFUNCTION(Category = "Abilities|Attributes", BlueprintCallable)
    virtual float GetHealth() const;
    FOnGameplayAttributeValueChange& GetOnHealthChangeDelegate();

    /** Accessor for MaxHealth attribute */
    UFUNCTION(Category = "Abilities|Attributes", BlueprintCallable)
    virtual float GetMaxHealth() const;
    FOnGameplayAttributeValueChange& GetOnMaxHealthChangeDelegate();

    /** Accessor for Strength attribute */
    UFUNCTION(Category = "Abilities|Attributes", BlueprintCallable)
    virtual float GetStrength() const;
    FOnGameplayAttributeValueChange& GetOnStrengthChangeDelegate();

    /** Accessor for MaxStrength attribute */
    UFUNCTION(Category = "Abilities|Attributes", BlueprintCallable)
    virtual float GetMaxStrength() const;
    FOnGameplayAttributeValueChange& GetOnMaxStrengthChangeDelegate();

    UFUNCTION(BlueprintCallable)
    bool CheckIsDead();

    /** Decrease the distance between camera and character */
    UFUNCTION(BlueprintCallable, Category = Camera)
    void CameraIn();

    /** Increase the distance between camera and character */
    UFUNCTION(BlueprintCallable, Category = Camera)
    void CameraOut();

    /** Fired when this Character's interactable actor has updated */
    UPROPERTY(EditAnywhere, BlueprintAssignable)
    FOnInteractableActorCurrentChangedSignature OnInteractableActorCurrentChanged;

    /** Inventory interaction */
    UFUNCTION(BlueprintCallable)
    virtual void SelectInventory(int InventoryCode);

    UFUNCTION(Server, Reliable)
    void ServerSelectInventory(int32 InventoryCode);

    /**
    * End-points for input actions from player controller
    */
    virtual void ControllerJumpActionPressed();
    virtual void ControllerJumpActionReleased();
    virtual void ControllerCrouchActionPressed();
    virtual void ControllerCrouchActionReleased();
    virtual void ControllerAttackActionPressed();
    virtual void ControllerAttackActionReleased();
    virtual void ControllerThrowActionPressed();
    virtual void ControllerThrowActionReleased();
    virtual void ControllerUseActionPressed();
    virtual void ControllerUseActionReleased();
    virtual void ControllerCameraInActionPressed();
    virtual void ControllerCameraInActionReleased();
    virtual void ControllerCameraOutActionPressed();
    virtual void ControllerCameraOutActionReleased();





public:
    virtual void NotifyDeathAbilityEnded();

protected:
    virtual void DieStart();

public:
    UFUNCTION(BlueprintCallable, Category = "RuneArena")
    class URACharacterEquipSetAsset* GetCurrentEquipSetAsset();

    /** Retrieve locomotion blend space from active equip set asset */
    class UBlendSpace* GetCurrentLocomotionBlendSpace();

    /** Get the chain of attack animations in the instant of calling */
    void GetCurrentAttackChain(TArray<class UAnimMontage*>* OutAttackChain, TArray<class UAnimMontage*>* OutRecoveryMontages);

public:
    /** Double tap direction inputs used for dodging */
    bool bPressedDoubleForward;
    bool bPressedDoubleBackward;
    bool bPressedDoubleRight;
    bool bPressedDoubleLeft;

    virtual void CheckJumpInput(float DeltaTime) override;

    virtual void ClearJumpInput(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "RuneArena|Movement")
    virtual bool CanDodge() const;

    virtual bool Dodge();

    virtual void ClearDodgeInput();

protected:
    virtual bool CanJumpInternal_Implementation() const override;
    virtual void ResetJumpState() override;

    bool IsIgnoringMovementInput();

    /** Cached RACharacterMovementComponent, casted from CharacterMovement */
    class URACharacterMovementComponent* RACharacterMovementComponent;

    /** Delegate handler for MovementComponent's OnCanGrabLedge */
    UFUNCTION()
    void HandleCanGrabLedge();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

   

    /** Character movement interface */
public:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void DoubleTap(bool bForward, bool bBackward, bool bRight, bool bLeft);

public:
    ERADirection GetInputDirectionEnum();
	ERADirection GetAccelerationDirectionEnum();
	ERADirection GetVelocityDirectionEnum();

    ERADirection GetDoAttackDirection() const;

protected:
    ERADirection DoAttackDirection;

public:
    UFUNCTION(Server, Reliable)
    void ServerAttack(ERADirection Direction);

    virtual void DoAttack(ERADirection Direction);

    // Temp solution to get non-player characters to attack
    UFUNCTION(BlueprintCallable, Category = "RuneArena")
    void Attack();

    // NEW DIED FUNCTIONS
protected:
    /** How long the torn off dead character will stick around before being destroyed */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Character")
    float DeathCleanupTimerSeconds;

    /**
    *   Valid on server and client. Fired immediately after CPP PlayDying has been called.
    *   At this point, TearOff has been called on Character.
    *   This is where on-death effects should be played, but not where on-destruction effects should be played.
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "Character", Meta = (DisplayName = "OnPlayDying"))
    void K2_OnPlayDying();
    virtual void PlayDying();

    /** Current only used in death state, after TearOff */
    virtual void EnableRagdoll();

    UFUNCTION()
    virtual void DeathCleanupTimer();

    virtual void TornOff() override;

    /** Gib class to spawn on hits for this character */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    TSubclassOf<class ARAGib> CharacterGibClass;

    UFUNCTION(NetMulticast, Unreliable)
    void NetMulticastCharacterTakeDamage(const FVector& DamageLocation, const FVector& DamageNormal, float DamageDealt);

    /** Valid on server and client. Fired immediately character receives damage. Play effects like gib spawning here. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Character", Meta = (DisplayName = "OnCharacterTakeDamage"))
    void K2_OnCharacterTakeDamage(const FVector& DamageLocation, const FVector& DamageNormal, float DamageDealt);

public:
    UFUNCTION(BlueprintCallable, Category = "Character", meta = (DisplayName = "Died"))
    virtual bool Died(class AController* InstigatorController);

    // Inventory
protected:
    /** If this character has a valid InteractableActor, it will attempt to perform the interaction */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void Interact();
    void InteractSetup();

    UFUNCTION(Server, Reliable, Category = "Interaction")
    void ServerInteract();

    /** Allows internal rejection of inventory actors */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    virtual bool CheckCanAcquireInventory(class ARANewInventory* InventoryToAcquire);

public:
    /** Attempt to acquire an Actor */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AcquireInventory(class ARANewInventory* InventoryToAcquire, ERACharacterInventoryAcquirePolicy AcquirePolicy);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ReleaseInventory(class ARANewInventory* InventoryToRelease, ERACharacterInventoryReleasePolicy ReleasePolicy);

    /** Wipes all inventory actors from this character */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory(ERACharacterInventoryReleasePolicy ReleasePolicy);

 protected:
    /** Fired immediately after this character acquired an inventory actor */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnAcquiredInventory"))
    void K2_OnAcquiredInventory(class ARANewInventory* AcquiredInventory);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnReleasedInventory"))
    void K2_OnReleasedInventory(class ARANewInventory* ReleasedInventory);

    UPROPERTY(Replicated)
    class ARANewInventory* Weapon;
    UPROPERTY(Replicated)
    class ARANewInventory* Shield;
    class ARANewInventory* Inventory;
    template<typename> friend class TRAInventoryIterator;

public:
    /** Equip the provided inventory. The provided inventory must be acquired by this character */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void EquipInventory(class ARANewInventory* InventoryToEquip);

    /** Stow all equipped inventories */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void StowInventory(class ARANewInventory* InventoryToStow);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    class ARANewInventory* GetWeapon();
};
