#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_Die.generated.h"

UCLASS()
class RUNEARENA_API URAGA_Die : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	/**
	*	GameplayEffect class applied to source at the moment death occurs.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> DieGameplayEffectClass;

	UPROPERTY(EditAnywhere, Category = "RuneArena")
	class UAnimMontage* MontageToPlay;

	bool bPerformedDie;

public:
	URAGA_Die();

	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	virtual void OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData) override;
	virtual void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData) override;

	void PerformDie();
};