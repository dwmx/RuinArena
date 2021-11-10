#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"
#include "Inventory/RAInventoryStateIdle.h"
#include "Inventory/RAInventoryStateSettling.h"
#include "Inventory/RAInventoryStateSettled.h"
#include "Inventory/RAInventoryStateThrowing.h"
#include "Inventory/RAInventoryStateStowed.h"
#include "Inventory/RAInventoryStateEquipped.h"
#include "Characters/RACharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogInventory);

ARANewInventory::ARANewInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NextInventory = nullptr;

	InventoryStateIdle = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateIdle>(this, TEXT("InventoryStateIdle"));
	InventoryStateSettling = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateSettling>(this, TEXT("InventoryStateSettling"));
	InventoryStateSettled = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateSettled>(this, TEXT("InventoryStateSettled"));
	InventoryStateThrowing = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateThrowing>(this, TEXT("InventoryStateThrowing"));
	InventoryStateStowed = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateStowed>(this, TEXT("InventoryStateStowed"));
	InventoryStateEquipped = ObjectInitializer.CreateDefaultSubobject<URAInventoryStateEquipped>(this, TEXT("InventoryStateEquipped"));
	InventoryState = InventoryStateIdle;

	CollisionVolumeComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("CollisionVolumeComponent"));
	CollisionVolumeComponent->SetHiddenInGame(true);
	CollisionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionVolumeComponent->SetCollisionProfileName(FName("WeaponThrown"));
	RootComponent = CollisionVolumeComponent;

	InteractionVolumeComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("InteractionVolumeComponent"));
	InteractionVolumeComponent->SetupAttachment(RootComponent);
	InteractionVolumeComponent->SetHiddenInGame(true);
	InteractionVolumeComponent->SetCollisionProfileName(FName("Interactable"));

	SkeletalMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	MovementComponent = ObjectInitializer.CreateDefaultSubobject<URAInventoryMovementComponent>(this, TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(nullptr);

	RotatingMovementComponent = ObjectInitializer.CreateDefaultSubobject<URAInventoryRotatingMovementComponent>(this, TEXT("RotatingMovementComponent"));
	RotatingMovementComponent->PivotTranslation = SkeletalMeshComponent->GetSkeletalCenterOfMass();
	RotatingMovementComponent->RotationRate = FRotator(0.0f);
	RotatingMovementComponent->SetUpdatedComponent(nullptr);

	PickupAbilityActivationTag = FGameplayTag::RequestGameplayTag(FName("Ability.Interact.Pickup"));

	bReplicates = true;
	SetReplicateMovement(false);

	InteractionPriority = 5.0f;
}

void ARANewInventory::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MovementComponent != nullptr)
	{
		MovementComponent->SetUpdatedComponent(nullptr);
	}

	if (RotatingMovementComponent != nullptr)
	{
		RotatingMovementComponent->SetUpdatedComponent(nullptr);
	}

	if (SkeletalMeshComponent != nullptr)
	{
		if (SkeletalMeshComponent->DoesSocketExist(EquipSocketName))
		{
			FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(EquipSocketName, ERelativeTransformSpace::RTS_Actor);
			EquipSocketOffset = -1.0f * SocketTransform.GetLocation();
		}
		else
		{
			UE_LOG(LogInventory, Warning, TEXT("No equip socket specified for %s. Defaulting to ZeroVector."), *GetClass()->GetName());
		}

		if (RotatingMovementComponent != nullptr)
		{
			RotatingMovementComponent->PivotTranslation = SkeletalMeshComponent->GetSkeletalCenterOfMass();
		}
	}
}

void ARANewInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARANewInventory, NextInventory);
	DOREPLIFETIME(ARANewInventory, InventoryNetUpdateParameters);
	DOREPLIFETIME(ARANewInventory, InventoryStateName);
}

void ARANewInventory::OnRep_InventoryNetUpdateParameters()
{
	SetActorLocation(InventoryNetUpdateParameters.Location);
	SetActorRotation(InventoryNetUpdateParameters.Rotation);
	MovementComponent->Velocity = InventoryNetUpdateParameters.LinearVelocity;
	RotatingMovementComponent->RotationRate.Pitch = InventoryNetUpdateParameters.PitchRotationRate;
	RotatingMovementComponent->RotationRate.Yaw = InventoryNetUpdateParameters.YawRotationRate;
	RotatingMovementComponent->RotationRate.Roll = InventoryNetUpdateParameters.RollRotationRate;
}

void ARANewInventory::OnRep_InventoryStateName()
{
	if (InventoryStateName == InventoryStateIdle->InventoryStateName)			SwitchInventoryState(InventoryStateIdle);
	else if (InventoryStateName == InventoryStateSettling->InventoryStateName)	SwitchInventoryState(InventoryStateSettling);
	else if (InventoryStateName == InventoryStateSettled->InventoryStateName)	SwitchInventoryState(InventoryStateSettled);
	else if (InventoryStateName == InventoryStateThrowing->InventoryStateName)	SwitchInventoryState(InventoryStateThrowing);
	else if (InventoryStateName == InventoryStateEquipped->InventoryStateName)	SwitchInventoryState(InventoryStateEquipped);
	else if (InventoryStateName == InventoryStateStowed->InventoryStateName)	SwitchInventoryState(InventoryStateStowed);
}

void ARANewInventory::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Replace this with goto initial state
	MovementComponent->SetUpdatedComponent(nullptr);
	MovementComponent->SetInterpolatedComponent(nullptr);
	RotatingMovementComponent->SetUpdatedComponent(nullptr);
}

void ARANewInventory::Destroyed()
{
	ARACharacter* CharacterOwner = Cast<ARACharacter>(GetOwner());
	if (CharacterOwner != nullptr)
	{
		CharacterOwner->ReleaseInventory(this, ERACharacterInventoryReleasePolicy::ReleaseAndForget);
	}
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::Destroyed();
}

void ARANewInventory::SwitchInventoryState(URAInventoryState* NextInventoryState)
{
	if (InventoryState == NextInventoryState)
	{
		return;
	}
	
	if (InventoryState != nullptr)
	{
		InventoryState->EndState();
	}
	
	InventoryState = NextInventoryState;
	
	if (InventoryState != nullptr)
	{
		InventoryState->BeginState();
	}
}

void ARANewInventory::NotifyThrowCollision(const FHitResult& ImpactHitResult, const FVector& ImpactVelocity)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AActor* StruckActor = ImpactHitResult.Actor.Get();
		if (StruckActor != nullptr)
		{
			IAbilitySystemInterface* TargetASC = Cast<IAbilitySystemInterface>(StruckActor);
			if (TargetASC != nullptr && ThrowGameplayEffectSpecHandle.IsValid())
			{
				TargetASC->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*ThrowGameplayEffectSpecHandle.Data.Get());
			}
		}
	}

	K2_OnThrowCollision(ImpactHitResult, ImpactVelocity);
}

/** Start IRAInteractableInterface implementation */
bool ARANewInventory::CanBeInteractedWith()
{
	if (GetTearOff())
	{
		return false;
	}

	if (InventoryState != nullptr)
	{
		return InventoryState->bCanBeInteractedWith;
	}

	return true;
}

float ARANewInventory::GetInteractionPriority() const
{
	return InteractionPriority;
}

FGameplayTag ARANewInventory::GetInteractionAbilityActivationTag()
{
	return PickupAbilityActivationTag;
}

void ARANewInventory::PerformInteraction(AActor* InteractionInstigator)
{
	ARACharacter* InteractionCharacter = Cast<ARACharacter>(InteractionInstigator);
	if (InteractionCharacter != nullptr)
	{
		InteractionCharacter->AcquireInventory(this, ERACharacterInventoryAcquirePolicy::AcquireAndEquip);
	}
}
/** End IRAInteractableInterface implementation */

void ARANewInventory::ThrowFrom(const FVector& ReleaseLocation, const FRotator& ReleaseRotation, const FVector& ThrowVelocity, const FRotator& ThrowRotationRate)
{
	SetActorLocation(ReleaseLocation);
	SetActorRotation(ReleaseRotation);
	MovementComponent->Velocity = ThrowVelocity;
	RotatingMovementComponent->RotationRate = ThrowRotationRate;
	SwitchInventoryState(InventoryStateThrowing);
}

void ARANewInventory::DropFrom(const FVector& ReleaseLocation, const FVector& DropVelocity)
{
	SetActorLocation(ReleaseLocation);
	MovementComponent->Velocity = DropVelocity;
	SwitchInventoryState(InventoryStateSettling);
}

void ARANewInventory::SetThrowGameplayEffect(FGameplayEffectSpecHandle GameplayEffectSpecHandle)
{
	ThrowGameplayEffectSpecHandle = GameplayEffectSpecHandle;
}

void ARANewInventory::NotifyAcquired()
{
	if (OnInventoryAcquired.IsBound())
	{
		OnInventoryAcquired.Broadcast(this);
	}
	K2_OnInventoryAcquired();
}

void ARANewInventory::NotifyReleased()
{
	K2_OnInventoryReleased();
}

void ARANewInventory::NotifyEquipped()
{
	SwitchInventoryState(InventoryStateEquipped);
	K2_OnInventoryEquipped();
}

void ARANewInventory::NotifyStowed()
{
	SwitchInventoryState(InventoryStateStowed);
	K2_OnInventoryStowed();
}

FVector ARANewInventory::GetLocalOffsetForEquip()
{
	return EquipSocketOffset;
}

void ARANewInventory::DespawnInventory()
{
	NetMulticastDespawnInventory();
}

void ARANewInventory::NetMulticastDespawnInventory_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		TearOff();
	}

	K2_OnInventoryDespawn();
	Destroy();
}