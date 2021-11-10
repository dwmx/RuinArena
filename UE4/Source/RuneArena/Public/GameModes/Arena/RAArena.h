#pragma once

RUNEARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogArena, Log, All);

/** Current state of the arena game mode */
namespace EArenaMatchStateName
{
	extern RUNEARENA_API const FName ArenaIdle;				// GameMode is waiting for enough players to queue before starting
	extern RUNEARENA_API const FName ArenaPreMatch;			// An arena match is about to begin
	extern RUNEARENA_API const FName ArenaMatchInProgress;	// A match is currently in progress
	extern RUNEARENA_API const FName ArenaPostMatch;		// An arena match has just ended
	extern RUNEARENA_API const FName ArenaInactive;			// Arena does nothing, meant for post game
}

/** Constants for teams used in the arena */
namespace EArenaTeamIndex
{
	extern RUNEARENA_API const uint8 TeamIndexChallengers;
	extern RUNEARENA_API const uint8 TeamIndexChampions;
	extern RUNEARENA_API const uint8 TeamIndexSpectators;
}