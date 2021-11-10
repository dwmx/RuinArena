#pragma once

#include "CoreMinimal.h"
#include "GameModes/RAGameModeBase.h"
#include "RALobbyGameMode.generated.h"

UCLASS(Abstract)
class RUNEARENA_API ARALobbyGameMode : public ARAGameModeBase
{
	GENERATED_BODY()
	using Super = ARAGameModeBase;
};