#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGameplayAbility.h"
#include "RAGA_AsyncAnimMontageAbility.generated.h"

/**
*	Base gameplay ability class which provides the structure for playing replicated
*	animation montages and responding to events received from those montages.
*	This is a base class used for abilities like Attack, Use and Taunt.
*/
UCLASS()
class RUNEARENA_API URAGA_AsyncAnimMontageAbility : public URAGameplayAbility
{
	GENERATED_BODY()

protected:
	bool TryPlayAnimMontage(class UAnimMontage* AnimMontageToPlay);

	UFUNCTION()
	virtual void OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	virtual void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	virtual void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
};