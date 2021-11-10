#pragma once

#include "GameFramework/Actor.h"
#include "RAInteractableInterface.h"
#include "Characters/RACharacter.h"
#include "RANewInventory.generated.h"

RUNEARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryAcquiredSignature, class ARANewInventory*, Inventory);

USTRUCT()
struct FInventoryNetUpdateParameters
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector LinearVelocity;

	// Rotators are replicated as mod 360
	// Probably a better solution than this, but it works for now
	UPROPERTY()
	float PitchRotationRate;
	UPROPERTY()
	float YawRotationRate;
	UPROPERTY()
	float RollRotationRate;
};

UCLASS(BlueprintType)
class RUNEARENA_API ARANewInventory
	: public AActor, public IRAInteractableInterface
{
	GENERATED_BODY()

	template<typename> friend class TRAInventoryIterator;

	friend class URAInventoryState;
	friend class URAInventoryStateIdle;
	friend class URAInventoryStateSettling;
	friend class URAInventoryStateSettled;
	friend class URAInventoryStateThrowing;
	friend class URAInventoryStateStowed;
	friend class URAInventoryStateEquipped;

	friend bool ARACharacter::AcquireInventory(class ARANewInventory* InventoryToAcquire, ERACharacterInventoryAcquirePolicy AcquirePolicy);
	friend void ARACharacter::ReleaseInventory(class ARANewInventory* InventoryToRelease, ERACharacterInventoryReleasePolicy ReleasePolicy);

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory")
	class ARANewInventory* NextInventory;

	/** Used for forcing client side corrections */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryNetUpdateParameters)
	FInventoryNetUpdateParameters InventoryNetUpdateParameters;

	UFUNCTION()
	void OnRep_InventoryNetUpdateParameters();

	/** Used to synchronize client and server state */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryStateName, BlueprintReadOnly, Category = "Inventory")
	FName InventoryStateName;

	UFUNCTION()
	void OnRep_InventoryStateName();

	class URAInventoryState* InventoryState;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateIdle;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateSettling;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateSettled;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateThrowing;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateStowed;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	class URAInventoryState* InventoryStateEquipped;

	void SwitchInventoryState(class URAInventoryState* NextInventoryState);

	/** Name of the socket on this inventory's mesh that indicates where it should be held from */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName EquipSocketName;
	FVector EquipSocketOffset;

	/** Activation tag corresponding to picking up this inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag PickupAbilityActivationTag;

	/** Collision volume for this inventory actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	class UPrimitiveComponent* CollisionVolumeComponent;

	/** Interaction volume for this inventory actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	class UPrimitiveComponent* InteractionVolumeComponent;

	/** Handles replicated movement of this inventory actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class URAInventoryMovementComponent* MovementComponent;

	/** Handles replicated rotation movement of this inventory actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class URAInventoryRotatingMovementComponent* RotatingMovementComponent;

	/** Skeletal mesh for display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	class USkeletalMeshComponent* SkeletalMeshComponent;

	/** Higher values means interacting characters will prefer this over other nearby interactables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float InteractionPriority;

	/**
	*	Gameplay effect applied to struck target when this Inventory is thrown.
	*	This is instantiated by the throw ability and then set via SetThrowGameplayEffect
	*/
	FGameplayEffectSpecHandle ThrowGameplayEffectSpecHandle;

	/** Destroys this actor and informs network to play related effects */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastDespawnInventory();
	virtual void DespawnInventory();

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Destroyed() override;

	virtual void BeginPlay() override;

	virtual void NotifyThrowCollision(const FHitResult& ImpactHitResult, const FVector& ImpactVelocity);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnThrowCollision"))
	void K2_OnThrowCollision(const FHitResult & ImpactHitResult, const FVector & ImpactVelocity);

public:
	ARANewInventory(const FObjectInitializer& ObjectInitializer);

	/** Start IRAInteractableInterface implementation */
	virtual bool CanBeInteractedWith() override;
	virtual float GetInteractionPriority() const override;
	virtual FGameplayTag GetInteractionAbilityActivationTag() override;
	virtual void PerformInteraction(AActor* InteractionInstigator) override;
	/** End IRAInteractableInterface implementation */

	virtual void ThrowFrom(const FVector& ReleaseLocation, const FRotator& ReleaseRotation, const FVector& ThrowVelocity, const FRotator& ThrowRotationRate);
	virtual void DropFrom(const FVector& ReleaseLocation, const FVector& DropVelocity);

	/** Fired when this inventory actor has been acquired */
	FOnInventoryAcquiredSignature OnInventoryAcquired;

	virtual void NotifyAcquired();
	virtual void NotifyReleased();
	virtual void NotifyEquipped();
	virtual void NotifyStowed();

	virtual FVector GetLocalOffsetForEquip();

	void SetThrowGameplayEffect(FGameplayEffectSpecHandle GameplayEffectSpecHandle);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryAcquired"))
	void K2_OnInventoryAcquired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryReleased"))
	void K2_OnInventoryReleased();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryEquipped"))
	void K2_OnInventoryEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryStowed"))
	void K2_OnInventoryStowed();

	/** Valid on server and client. Fired at the instant this inventory enters the throwing state. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryThrowStart"))
	void K2_OnInventoryThrowStart();

	/** Valid on server and client. Fired at the instant this inventory exits the throwing state. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryThrowEnd"))
	void K2_OnInventoryThrowEnd();

	/**
	*	Valid on server and client. Fired immediately after despawn timer expires. TearOff is called on the
	*	server immediately after replicating this call, so clients can go wild with effects.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory", Meta = (DisplayName = "OnInventoryDespawn"))
	void K2_OnInventoryDespawn();
};

template<typename T = ARANewInventory> class RUNEARENA_API TRAInventoryIterator
{
protected:
	static const int32 MaximumIterations = 50;
	const class ARACharacter* CharacterOwner;
	class ARANewInventory* CurrentInventory;
	class ARANewInventory* NextInventory;
	int32 Index;

	inline bool CheckIsValidForIteration(class ARANewInventory* Inventory)
	{
		if (Inventory == nullptr
		|| Inventory->GetOwner() != CharacterOwner
		|| !Inventory->IsA(T::StaticClass()))
		{
			return false;
		}
		return true;
	}

public:
	TRAInventoryIterator(const ARACharacter* InCharacterOwner)
		: CharacterOwner(InCharacterOwner), Index(0)
	{
		if (CharacterOwner == nullptr)
		{
			CurrentInventory = nullptr;
			NextInventory = nullptr;
		}
		else
		{
			CurrentInventory = CharacterOwner->Inventory;
			if (CurrentInventory != nullptr)
			{
				NextInventory = CurrentInventory->NextInventory;
				if (!CheckIsValidForIteration(CurrentInventory))
				{
					++(*this);
				}
			}
			else
			{
				NextInventory = nullptr;
			}
		}
	}

	void operator++()
	{
		do
		{
			Index++;
			if (Index > MaximumIterations)
			{
				UE_LOG(LogInventory, Warning, TEXT("Inventory iterator reached maximum iterations: %d"), MaximumIterations);
				CurrentInventory = nullptr;
			}
			else
			{
				CurrentInventory = NextInventory;
				if (CurrentInventory != nullptr)
				{
					NextInventory = CurrentInventory->NextInventory;
				}
			}
		} while (CurrentInventory != nullptr && !CheckIsValidForIteration(CurrentInventory));
	}

	FORCEINLINE bool IsValid() const
	{
		return CurrentInventory != nullptr;
	}

	FORCEINLINE operator bool() const
	{
		return IsValid();
	}

	FORCEINLINE T* operator*() const
	{
		checkSlow(CurrentInventory != nullptr && CurrentInventory->IsA(T::StaticClass()));
		return (T*)CurrentInventory;
	}

	FORCEINLINE T* operator->() const
	{
		checkSlow(CurrentInventory != nullptr && CurrentInventory->IsA(T::StaticClass()));
		return (T*)CurrentInventory;
	}
};