#pragma once

#include "GameModes/RATeamInfo.h"
#include "RAArenaTeamInfo.generated.h"

UCLASS()
class RUNEARENA_API ARAArenaTeamInfo : public ARATeamInfo
{
	GENERATED_BODY()

public:
	ARAArenaTeamInfo(const FObjectInitializer& ObjectInitializer);
};