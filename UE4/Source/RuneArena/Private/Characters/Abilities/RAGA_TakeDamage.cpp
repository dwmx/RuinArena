#include "Characters/Abilities/RAGA_TakeDamage.h"
#include "Characters/Abilities/AbilityTasks/RAAT_PlayMontageAndWaitForEvent.h"
#include "AbilitySystemComponent.h"

URAGA_TakeDamage::URAGA_TakeDamage()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Only the server can trigger and cancel this ability
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.TakeDamage"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Combat.TakeDamage"));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Ability"));
}

void URAGA_TakeDamage::ActivateAbility
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
	}

	URAAT_PlayMontageAndWaitForEvent* Task = URAAT_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(this, NAME_None, MontageToPlay, FGameplayTagContainer(), 1.0, NAME_None, false, 1.0f);
	Task->OnBlendOut.AddDynamic(this, &URAGA_TakeDamage::OnMontageCompleted);
	Task->OnCompleted.AddDynamic(this, &URAGA_TakeDamage::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &URAGA_TakeDamage::OnMontageCancelled);
	Task->OnCancelled.AddDynamic(this, &URAGA_TakeDamage::OnMontageCancelled);
	Task->EventReceived.AddDynamic(this, &URAGA_TakeDamage::EventReceived);
	Task->ReadyForActivation();

	// GameplayEffect
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (SourceASC != nullptr && IsValid(TakeDamageEffectClass))
	{
		FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), CurrentActorInfo);
		FGameplayEffectSpec EffectSpec(TakeDamageEffectClass.GetDefaultObject(), EffectContextHandle);
		SourceASC->ApplyGameplayEffectSpecToSelf(EffectSpec);
	}

	// Blueprint Event
	OnTakeDamage();
}

void URAGA_TakeDamage::OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_TakeDamage::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_TakeDamage::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{}