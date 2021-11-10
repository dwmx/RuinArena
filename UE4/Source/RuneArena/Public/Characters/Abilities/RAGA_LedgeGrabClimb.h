#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGA_AsyncAnimMontageAbility.h"
#include "RAGA_LedgeGrabClimb.generated.h"

UCLASS()
class RUNEARENA_API URAGA_LedgeGrabClimb : public URAGA_AsyncAnimMontageAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "RuneArena")
	class UAnimMontage* MontageToPlay;

public:
	URAGA_LedgeGrabClimb();

	virtual void ActivateAbility
	(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};