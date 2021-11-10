#include "Characters/Abilities/RAGA_Die.h"
#include "Characters/RACharacter.h"
#include "Components/CapsuleComponent.h"
#include "Characters/Components/RACharacterMovementComponent.h"
#include "GameModes/RAGameModeBase.h"
#include "AbilitySystemComponent.h"

URAGA_Die::URAGA_Die()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Die")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Die")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
}

void URAGA_Die::ActivateAbility
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

	if (HasAuthority(&ActivationInfo))
	{
		ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
		if (Character != nullptr)
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Character->GetCharacterMovement()->GravityScale = 0.0f;
			Character->GetCharacterMovement()->Velocity = FVector(0.0f);
		}

		if (DieGameplayEffectClass != nullptr)
		{
			FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), CurrentActorInfo);
			FGameplayEffectSpec GameplayEffectSpec(DieGameplayEffectClass.GetDefaultObject(), EffectContextHandle);
			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(GameplayEffectSpec);
		}

		bPerformedDie = false;
	}

	if (!TryPlayAnimMontage(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void URAGA_Die::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// Make sure the Die logic executes before this ability ends
	if (!bPerformedDie)
	{
		PerformDie();
	}
}

void URAGA_Die::OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	PerformDie();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_Die::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	PerformDie();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_Die::PerformDie()
{
	//if (HasAuthority(&CurrentActivationInfo))
	//{
	//	bPerformedDie = true;
	//	ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
	//	if (Character != nullptr)
	//	{
	//		Character->NotifyDeathAbilityEnded();
	//	}
	//}
}