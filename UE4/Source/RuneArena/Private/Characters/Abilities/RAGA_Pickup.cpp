#include "Characters/Abilities/RAGA_Pickup.h"
#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAPickup.h"
#include "RAInteractableInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

URAGA_Pickup::URAGA_Pickup()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact.Pickup")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact.Pickup")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.IgnoreMovementInput")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.IgnoreRotationInput")));

	//ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat")));
	//BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
}

bool URAGA_Pickup::CanActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags
) const
{
	// ASC must have an InteractionActor prepared
	URAAbilitySystemComponent* SourceASC = Cast<URAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (SourceASC == nullptr || SourceASC->GetInteractionActor() == nullptr)
	{
		return false;
	}

	// Character cannot be in-air
	ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
	UCharacterMovementComponent* MovementComponent = nullptr;
	if (Character != nullptr)
	{
		MovementComponent = Character->GetCharacterMovement();
	}

	if (MovementComponent == nullptr || MovementComponent->IsFalling())
	{
		return false;
	}

	return true;
}

void URAGA_Pickup::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Initialize activation
	InteractionActor = nullptr;
	bPerformedAcquisition = false;

	// Grab InteractionActor
	URAAbilitySystemComponent* SourceASC = Cast<URAAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (SourceASC != nullptr)
	{
		InteractionActor = SourceASC->GetInteractionActor();
	}

	if (InteractionActor == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Try to play the animation
	if (!TryPlayAnimMontage(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void URAGA_Pickup::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	// Fail safe - perform acquisition here in case the montage event was never received
	if (!bPerformedAcquisition)
	{
		PerformAcquisition();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URAGA_Pickup::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// Event received from RAAN_Use (should be renamed to RAAN_Pickup)
	if (EventTag == FGameplayTag::RequestGameplayTag("Event.Montage.Interaction.Use"))
	{
		PerformAcquisition();
	}
}

void URAGA_Pickup::PerformAcquisition()
{
	bPerformedAcquisition = true;

	if (!HasAuthority(&CurrentActivationInfo) || InteractionActor == nullptr)
	{
		return;
	}

	ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		return;
	}

	IRAInteractableInterface* Interactable = Cast<IRAInteractableInterface>(InteractionActor);
	if (Interactable == nullptr)
	{
		return;
	}

	Interactable->PerformInteraction(GetAvatarActorFromActorInfo());
}