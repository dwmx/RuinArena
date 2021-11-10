#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_Throw.generated.h"

UCLASS()
class RUNEARENA_API URAGA_Throw : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	/**
	*	The gameplay effect applied to source at the instant the weapon is thrown.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> ThrowGameplayEffectClass;

	/**
	*	The gameplay effect applied to target when the thrown weapon strikes another actor.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> ThrowDamageGameplayEffectClass;

	bool bPerformedThrow;

public:
	URAGA_Throw();

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

	void PerformThrow();

	/** Calculate the initial release location of the actor upon throwing */
	UFUNCTION(BlueprintNativeEvent)
	FVector CalcThrowReleaseLocation(AActor* ActorToThrow, const FVector& ViewRotationForward);

	/** Calculate the velocity applied to the actor immediately upon being thrown */
	UFUNCTION(BlueprintNativeEvent)
	FVector CalcThrowReleaseVelocity(AActor* ActorToThrow, const FVector& ViewRotationForward);

	/** Calculate the initial release rotation of the actor upon throwing */
	UFUNCTION(BlueprintNativeEvent)
	FRotator CalcThrowReleaseRotation(AActor* ActorToThrow, const FVector& ViewRotationForward);

	/** Calculate the rotation rate applied to the actor immediately upon being thrown */
	UFUNCTION(BlueprintNativeEvent)
	FRotator CalcThrowReleaseRotationRate(AActor* ActorToThrow, const FVector& ViewRotationForward);
};