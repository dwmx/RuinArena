#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AnimNotifies/RAAbilityAnimNotify.h"
#include "RAAN_Throw.generated.h"

/** AnimNotify to be used specifically with the Throw gameplay ability */
UCLASS()
class RUNEARENA_API URAAN_Throw : public URAAbilityAnimNotify
{
	GENERATED_BODY()

public:
	URAAN_Throw();
};