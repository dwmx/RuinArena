#pragma once

#include "CoreMinimal.h"
#include "GameModes/RAGameModeBase.h"
#include "RADeathMatchGameMode.generated.h"

UCLASS(Abstract)
class RUNEARENA_API ARADeathMatchGameMode : public ARAGameModeBase
{
	GENERATED_BODY()
	using Super = ARAGameModeBase;
};