#pragma once

#include "Player/RAPlayerState.h"
#include "RAArenaPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchStateChanged, class ARAArenaMatchState*, OldArenaMatchState, class ARAArenaMatchState*, NewArenaMatchState);

UCLASS()
class RUNEARENA_API ARAArenaPlayerState : public ARAPlayerState
{
	GENERATED_BODY()

protected:
	/** Reference to the arena match state this player is in, valid on server and client */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	class ARAArenaMatchState* ArenaMatchState;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Broadcasts when this player state's relevant arena match state updates */
	UPROPERTY(BlueprintAssignable, Category = "ArenaGame")
	FOnArenaMatchStateChanged OnArenaMatchStateChanged;

	void SetArenaMatchState(class ARAArenaMatchState* NewArenaMatchState);
	ARAArenaMatchState* GetArenaMatchState() { return ArenaMatchState; }
};