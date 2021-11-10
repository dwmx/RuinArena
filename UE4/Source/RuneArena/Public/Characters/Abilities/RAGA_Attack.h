#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_Attack.generated.h"

UCLASS()
class RUNEARENA_API URAGA_Attack : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	/**
	*	Gameplay effect class applied to source when the attack begins.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> BeginAttackEffectClass;

	/**
	*	Gameplay effect class applied to target when struck.
	*	This GE must use the RAEC_Damage execution calculation in order for attributes to be affected.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	TSubclassOf<class UGameplayEffect> DamageGameplayEffectClass;

	/** The source's strength attribute is multiplied by this amount and then added on top of the base damage to be dealt to the target */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	float DamageIncreaseByStrengthCoefficient;

	/** The amount of base damage dealt to a target is multiplied by this amount and added back to the instigator's strength */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	float StrengthGainCoefficient;

	class ARACharacter* RACharacter;
	class ARAWeapon* RAWeapon;

	TArray<UAnimMontage*> AttackChain;
	TArray<UAnimMontage*> RecoveryMontages;
	int32 AttackChainIndex;
	bool bPendingAttack;
	bool bRecovering;

	bool bAttackActive;

public:
	URAGA_Attack();

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
	void RepeatActivateAbility();
	bool TryPerformAttack();
	bool TryPerformRecovery();

	virtual void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData) override;

	/** Listens for combat specific events and sends them to handler functions */
	virtual void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData) override;
	
	void HandleOnBeginAttack();
	void HandleOnTickAttack(float DeltaSeconds);
	void HandleOnEndAttack();

	virtual void DoBeginAttack();
	virtual void DoTickAttack(float DeltaSeconds);
	virtual void DoEndAttack();

	/** Handler for OnWeaponStruckActor event from the Owner's weapon */
	UFUNCTION()
	void HandleOnWeaponStruckActor(AActor* StruckActor, const FHitResult& HitResult);

	/** Fired immediately after the weapon's hit box becomes activated */
	UFUNCTION(BlueprintImplementableEvent, Category = "RuneArena")
	void OnBeginAttack();

	/** Fired each tick that the weapon's hit box is active */
	UFUNCTION(BlueprintImplementableEvent, Category = "RuneArena")
	void OnTickAttack(float DeltaSeconds);

	/** Fired immediately after the weapon's hit box becomes deactivated */
	UFUNCTION(BlueprintImplementableEvent, Category = "RuneArena")
	void OnEndAttack();

	/**
	*	Fired when the weapon strikes another actor.
	*	This is fired after the damage gameplay effect has been applied,
	*	so no further damage should be calculated using this event.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "RuneArena")
	void OnWeaponStructActor(AActor* StructActor, const FHitResult& HitResult);
};