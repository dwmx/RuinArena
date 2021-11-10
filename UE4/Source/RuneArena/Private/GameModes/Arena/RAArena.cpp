#include "GameModes/Arena/RAArena.h"

DEFINE_LOG_CATEGORY(LogArena);

namespace EArenaMatchStateName
{
	const FName ArenaIdle = FName(TEXT("ArenaIdle"));
	const FName ArenaPreMatch = FName(TEXT("ArenaPreMatch"));
	const FName ArenaMatchInProgress = FName(TEXT("ArenaMatchInProgress"));
	const FName ArenaPostMatch = FName(TEXT("ArenaPostMatch"));
	const FName ArenaInactive = FName(TEXT("ArenaInactive"));
}

namespace EArenaTeamIndex
{
	const uint8 TeamIndexChallengers = 0;
	const uint8 TeamIndexChampions = 1;
	const uint8 TeamIndexSpectators = 2;
}