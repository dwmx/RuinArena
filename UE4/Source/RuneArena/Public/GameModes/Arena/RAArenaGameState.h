#pragma once

#include "CoreMinimal.h"
#include "GameModes/RAGameStateBase.h"
#include "RAArenaGameState.generated.h"

UCLASS()
class RUNEARENA_API ARAArenaGameState : public ARAGameStateBase
{
	GENERATED_BODY()
	friend class ARAArenaGameMode;

protected:
	/** The main arena match state object for arena game
	*	TODO: If multiarena game mode is implemented in the future, this will have to be replaced with
	*	an array of match states
	*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ArenaGame")
	class ARAArenaMatchState* ArenaMatchState;

	/** The ordered queue of players in the arena game */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "ArenaGame")
	TArray<class ARAPlayerState*> ArenaPlayerQueue;

	/** Sends updated queue */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastUpdateArenaPlayerQueue(const TArray<class ARAPlayerState*>& NewArenaPlayerQueue);

public:
	ARAArenaGameState(const FObjectInitializer& ObjectInitializer);

	void UpdateArenaPlayerQueue(const TArray<class ARAPlayerState*>& NewArenaPlayerQueue);
};