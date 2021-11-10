#pragma once

#include "RAArenaMatchState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchStateStateChangedSignature, FName, OldArenaMatchStateName, FName, NewArenaMatchStateName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchStateTeamSizeChangedSignature, int32, OldArenaMatchTeamSize, int32, NewArenaMatchTeamSize);

/** Network replicated state information about arena match */
UCLASS(Blueprintable)
class RUNEARENA_API ARAArenaMatchState : public AInfo
{
	GENERATED_BODY()

protected:
	/** Current state name of arena match */
	UPROPERTY(ReplicatedUsing = OnRep_ArenaMatchStateName, BlueprintReadOnly, Category = "ArenaGame")
	FName ArenaMatchStateName;
	FName OldArenaMatchStateName;

	UFUNCTION()
	void OnRep_ArenaMatchStateName();

	/** The time limit for the current arena match state */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	float ArenaMatchStateTimeLimitSeconds;

	/** The time in seconds that the last time update was received */
	float ArenaMatchStateTimeStampSeconds;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetArenaMatchStateTimer(float TimeLimitSeconds, float TimeRemainingSeconds);

	/** Team size required for this arena match */
	UPROPERTY(ReplicatedUsing = OnRep_ArenaMatchTeamSize, BlueprintReadOnly, Category = "ArenaGame")
	int32 ArenaMatchTeamSize;
	int32 OldArenaMatchTeamSize;

	UFUNCTION()
	void OnRep_ArenaMatchTeamSize();

	/** Array of player states that are currently in this match, valid on client and server */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	TArray<class APlayerState*> ArenaMatchPlayers;

	UFUNCTION(NetMulticast, Reliable, Category = "ArenaGame")
	void NetMulticast_UpdateArenaMatchPlayers(const TArray<class APlayerState*>& NewArenaMatchPlayers);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Fired when the state name within the ArenaMatchState object is updated, valid for server and clients */
	UPROPERTY(BlueprintAssignable, Category = "ArenaGame")
	FOnArenaMatchStateStateChangedSignature OnArenaMatchStateStateChanged;

	/** Fired when this arena match's team size has changed, valid for server and clients */
	UPROPERTY(BlueprintAssignable, Category = "ArenaGame")
	FOnArenaMatchStateTeamSizeChangedSignature OnArenaMatchTeamSizeChanged;

public:
	ARAArenaMatchState(const FObjectInitializer& ObjectInitializer);

	void SetArenaMatchStateName(FName NewArenaMatchStateName);

	void SetArenaMatchStateTimer(float TimeLimitSeconds, float TimeRemainingSeconds);

	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	float GetArenaMatchStateTimeRemainingSeconds();

	FName GetArenaMatchStateName() { return ArenaMatchStateName; }

	void SetArenaMatchTeamSize(int32 NewArenaMatchTeamSize);

	void UpdateArenaMatchPlayers(const TArray<class APlayerState*>& NewArenaMatchPlayers);
};