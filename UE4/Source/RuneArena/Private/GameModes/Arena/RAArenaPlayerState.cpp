#include "GameModes/Arena/RAArenaPlayerState.h"
#include "Net/UnrealNetwork.h"

void ARAArenaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ARAArenaPlayerState::SetArenaMatchState(ARAArenaMatchState* NewArenaMatchState)
{
	ARAArenaMatchState* OldArenaMatchState = ArenaMatchState;
	ArenaMatchState = NewArenaMatchState;

	if (OnArenaMatchStateChanged.IsBound())
	{
		OnArenaMatchStateChanged.Broadcast(OldArenaMatchState, ArenaMatchState);
	}
}