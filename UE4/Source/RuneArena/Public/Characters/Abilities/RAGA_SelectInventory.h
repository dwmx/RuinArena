#pragma once

#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_SelectInventory.generated.h"

UCLASS()
class RUNEARENA_API URAGA_SelectInventory : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* MontageToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FGameplayTag MontageEventTagToListenFor;

	virtual void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData) override;

	int32 SelectInventoryCode;
	bool bPerformedSelectInventory;

	void PerformSelectInventory();

	void PerformStow();
	void PerformSelectNextWeapon();

public:
	URAGA_SelectInventory();

	virtual bool CanActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		OUT FGameplayTagContainer* OptionalRelevantTags
	) const override;

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
};