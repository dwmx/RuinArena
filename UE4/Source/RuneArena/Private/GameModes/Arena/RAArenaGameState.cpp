#include "GameModes/Arena/RAArenaGameState.h"
#include "GameModes/Arena/RAArenaGameMode.h"
#include "Net/UnrealNetwork.h"

ARAArenaGameState::ARAArenaGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void ARAArenaGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARAArenaGameState, ArenaMatchState);
}

void ARAArenaGameState::NetMulticastUpdateArenaPlayerQueue_Implementation(const TArray<ARAPlayerState*>& NewArenaPlayerQueue)
{
	ArenaPlayerQueue.Empty();
	for (auto It : NewArenaPlayerQueue)
	{
		ArenaPlayerQueue.Add(It);
	}
}

void ARAArenaGameState::UpdateArenaPlayerQueue(const TArray<ARAPlayerState*>& NewArenaPlayerQueue)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	NetMulticastUpdateArenaPlayerQueue(NewArenaPlayerQueue);
}