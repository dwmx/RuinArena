#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGameplayAbility.h"
#include "RAGA_Dodge.generated.h"

UCLASS()
class RUNEARENA_API URAGA_Dodge : public URAGameplayAbility
{
	GENERATED_BODY()

public:
	URAGA_Dodge();

	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};

UCLASS()
class RUNEARENA_API URAGA_WallDodge : public URAGameplayAbility
{
	GENERATED_BODY()

public:
	URAGA_WallDodge();

	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};