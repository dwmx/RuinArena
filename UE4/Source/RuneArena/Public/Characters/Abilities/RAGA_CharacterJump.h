#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/RAGameplayAbility.h"
#include "RAGA_CharacterJump.generated.h"

UCLASS()
class RUNEARENA_API URAGA_CharacterJump : public URAGameplayAbility
{
	GENERATED_BODY()

	UFUNCTION()
	virtual void HandleOnLanded(const FHitResult& Hit);

public:
	URAGA_CharacterJump();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
};