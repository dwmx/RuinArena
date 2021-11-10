#include "Characters/Abilities/RAGA_Dodge.h"
#include "Characters/RACharacter.h"
#include "Characters/Components/RACharacterMovementComponent.h"

URAGA_Dodge::URAGA_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dodge")));
	//ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dodge")));

	FAbilityTriggerData AbilityTriggerData;
	AbilityTriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Forward"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Backward"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Right"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Left"));
	AbilityTriggers.Add(AbilityTriggerData);
}

void URAGA_Dodge::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	ARACharacter* Character = CastChecked<ARACharacter>(ActorInfo->AvatarActor.Get());
	URACharacterMovementComponent* MovementComponent = nullptr;
	if (Character != nullptr)
	{
		MovementComponent = Cast<URACharacterMovementComponent>(Character->GetMovementComponent());
	}

	if (MovementComponent == nullptr || TriggerEventData == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Dodge based on incoming direction
	if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Forward")))
	{
		MovementComponent->DoDoubleTapForward();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Backward")))
	{
		MovementComponent->DoDoubleTapBackward();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Right")))
	{
		MovementComponent->DoDoubleTapRight();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.Dodge.Left")))
	{
		MovementComponent->DoDoubleTapLeft();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}



URAGA_WallDodge::URAGA_WallDodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge")));
	//ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Dodge")));

	FAbilityTriggerData AbilityTriggerData;
	AbilityTriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Forward"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Backward"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Right"));
	AbilityTriggers.Add(AbilityTriggerData);

	AbilityTriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Left"));
	AbilityTriggers.Add(AbilityTriggerData);
}

void URAGA_WallDodge::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	ARACharacter* Character = CastChecked<ARACharacter>(ActorInfo->AvatarActor.Get());
	URACharacterMovementComponent* MovementComponent = nullptr;
	if (Character != nullptr)
	{
		MovementComponent = Cast<URACharacterMovementComponent>(Character->GetMovementComponent());
	}

	if (MovementComponent == nullptr || TriggerEventData == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Dodge based on incoming direction
	if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Forward")))
	{
		MovementComponent->DoDoubleTapForward();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Backward")))
	{
		MovementComponent->DoDoubleTapBackward();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Right")))
	{
		MovementComponent->DoDoubleTapRight();
	}
	else if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.WallDodge.Left")))
	{
		MovementComponent->DoDoubleTapLeft();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}