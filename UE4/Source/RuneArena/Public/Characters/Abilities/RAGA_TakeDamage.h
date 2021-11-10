#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGameplayAbility.h"
#include "RAGA_TakeDamage.generated.h"

UCLASS()
class RUNEARENA_API URAGA_TakeDamage : public URAGameplayAbility
{
	GENERATED_BODY()

protected:
	/** GameplayEffect to apply to source on damaged */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> TakeDamageEffectClass;

	/** Montage to play on damaged */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	class UAnimMontage* MontageToPlay;

public:
	URAGA_TakeDamage();

	void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UFUNCTION()
	void OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);

	/** Fired at the moment the actor takes damage */
	UFUNCTION(BlueprintImplementableEvent, Category = "RuneArena")
	void OnTakeDamage();
};