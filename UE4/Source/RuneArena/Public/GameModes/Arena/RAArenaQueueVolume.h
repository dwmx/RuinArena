#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "RAArenaQueueVolume.generated.h"

UCLASS(Blueprintable)
class RUNEARENA_API ARAArenaQueueVolume : public ATriggerVolume
{
	GENERATED_BODY()

public:
	ARAArenaQueueVolume();
};