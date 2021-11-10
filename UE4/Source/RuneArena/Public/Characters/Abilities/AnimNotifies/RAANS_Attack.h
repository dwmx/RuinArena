#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AnimNotifies/RAAbilityAnimNotifyState.h"
#include "RAANS_Attack.generated.h"

/** AnimStateNotify to be used specifically with the Attack gameplay ability */
UCLASS()
class RUNEARENA_API URAANS_Attack : public URAAbilityAnimNotifyState
{
	GENERATED_BODY()

protected:
	/** Refers to the specific socket which is performing the attack */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	FGameplayTag InventorySocketTag;

public:
	URAANS_Attack();

protected:
	virtual FGameplayEventData GetGameplayEventData() override;
};