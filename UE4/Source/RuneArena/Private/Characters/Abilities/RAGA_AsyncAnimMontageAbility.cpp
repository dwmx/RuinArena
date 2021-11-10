#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "Characters/Abilities/AbilityTasks/RAAT_PlayMontageAndWaitForEvent.h"

bool URAGA_AsyncAnimMontageAbility::TryPlayAnimMontage(UAnimMontage* AnimMontageToPlay)
{
	if (AnimMontageToPlay == nullptr)
	{
		return false;
	}

	URAAT_PlayMontageAndWaitForEvent* Task = URAAT_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(this, NAME_None, AnimMontageToPlay, FGameplayTagContainer(), 1.0, NAME_None, false, 1.0f);
	Task->OnBlendOut.AddDynamic(this, &URAGA_AsyncAnimMontageAbility::OnMontageCompleted);
	Task->OnCompleted.AddDynamic(this, &URAGA_AsyncAnimMontageAbility::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &URAGA_AsyncAnimMontageAbility::OnMontageCancelled);
	Task->OnCancelled.AddDynamic(this, &URAGA_AsyncAnimMontageAbility::OnMontageCancelled);
	Task->EventReceived.AddDynamic(this, &URAGA_AsyncAnimMontageAbility::EventReceived);
	Task->ReadyForActivation();

	return true;
}

void URAGA_AsyncAnimMontageAbility::OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_AsyncAnimMontageAbility::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_AsyncAnimMontageAbility::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// To be implemented in child classes
}