#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RAAbilitySystemComponent.generated.h"

UCLASS()
class RUNEARENA_API URAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	using Super = UAbilitySystemComponent;

	friend class ARACharacter;

protected:
	bool bCharacterAbilitiesGiven;
	bool bStartupEffectsApplied;

	/** Actor currently being interacted with. */
	AActor* InteractionActor;

	/** Set by ARACharacter, grabbed by RAGA_SelectInventory */
	int32 PendingSelectInventoryCode;

public:
	URAAbilitySystemComponent();

	static URAAbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);

	AActor* GetInteractionActor();
	void SetInteractionActor(AActor* Actor);

	int32 GetPendingSelectInventoryCode();
	void SetPendingSelectInventoryCode(int32 NewPendingSelectInventoryCode);
};