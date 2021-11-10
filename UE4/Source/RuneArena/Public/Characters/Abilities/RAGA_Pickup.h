#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_Pickup.generated.h"

UCLASS()
class RUNEARENA_API URAGA_Pickup : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	AActor* InteractionActor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	class UAnimMontage* MontageToPlay;

	bool bPerformedAcquisition;

public:
	URAGA_Pickup();

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

protected:
	virtual void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData) override;

	void PerformAcquisition();
};