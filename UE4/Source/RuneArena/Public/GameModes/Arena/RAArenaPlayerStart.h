#pragma once

#include "GameModes/RAPlayerStart.h"
#include "RAArenaPlayerStart.generated.h"

UCLASS()
class RUNEARENA_API ARAArenaPlayerStart : public ARAPlayerStart
{
	GENERATED_BODY()

public:
	ARAArenaPlayerStart(const FObjectInitializer& ObjectInitializer);
};