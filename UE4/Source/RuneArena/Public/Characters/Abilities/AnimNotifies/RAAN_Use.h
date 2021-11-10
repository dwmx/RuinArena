#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/AnimNotifies/RAAbilityAnimNotify.h"
#include "RAAN_Use.generated.h"

/** AnimNotify to be used specifically with the Use gameplay ability */
UCLASS()
class RUNEARENA_API URAAN_Use : public URAAbilityAnimNotify
{
	GENERATED_BODY()

public:
	URAAN_Use();
};