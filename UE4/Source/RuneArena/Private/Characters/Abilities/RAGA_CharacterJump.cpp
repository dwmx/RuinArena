#include "Characters/Abilities/RAGA_CharacterJump.h"
#include "Characters/RACharacter.h"

URAGA_CharacterJump::URAGA_CharacterJump()
{
	AbilityInputID = ERAAbilityInputID::Jump;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Jump")));
	//UE_LOG(LogTemp, Warn, "Character jump ability was just fired");
}

void URAGA_CharacterJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		Character->LandedDelegate.AddDynamic(this, &URAGA_CharacterJump::HandleOnLanded);
		Character->Jump();
	}
}

void URAGA_CharacterJump::HandleOnLanded(const FHitResult& Hit)
{
	//ACharacter* Character = CastChecked<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character != nullptr)
	{
		Character->Jump();
	}
}

bool URAGA_CharacterJump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	//if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	//{
	//	return false;
	//}
	//
	//const ARACharacter* Character = CastChecked<ARACharacter>(ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	//return Character != nullptr && Character->CanJump();
}

void URAGA_CharacterJump::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &URAGA_CharacterJump::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->LandedDelegate.RemoveAll(this);
	Character->StopJumping();
}