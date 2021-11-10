#pragma once

#include "GameFramework/Info.h"
#include "RAArenaMatch.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnArenaMatchStateChangedSignature, class ARAArenaMatch*, ArenaMatch, FName, OldArenaMatchStateName, FName, NewArenaMatchStateName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchTeamSizeChangedSignature, class ARAArenaMatch*, ArenaMatch, int32, NewArenaMatchTeamSize);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaMatchTimeLimitChangedSignature, class ARAArenaMatch*, ArenaMatch, float, NewTimeLimitSeconds);

/**
* Represents and manages the arena's state
* Only spawned server side
* This class is designed with multi-arena support in mind
*/
UCLASS(Blueprintable)
class RUNEARENA_API ARAArenaMatch : public AInfo
{
	GENERATED_BODY()

protected:
	/** The controllers that are currently participating in this match */
	TArray<class AController*> ArenaMatchControllers;

	/** The controllers that are entering this match
	*	This is used to eject players and place them back in queue if they haven't started
	*	fighting in this match yet
	*/
	TArray<class AController*> ArenaMatchPendingControllers;

	virtual void CommitPendingControllers();

	/** Refreshed after each match
	*	In the event of a tie, all players are placed in the losers array
	*/
	TArray<class AController*> ArenaMatchMostRecentWinnerControllers;
	TArray<class AController*> ArenaMatchMostRecentLoserControllers;

	FTimerHandle ArenaMatchStateTimerHandle;

	virtual void SetArenaMatchStateTimer(float TimeSeconds);

	UFUNCTION()
	void HandleArenaMatchStateTimer();

	/** Time in seconds to countdown to arena entry */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	float ArenaMatchPreMatchTimeSeconds;

	/** Time limit for each arena match */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	float ArenaMatchTimeLimitSeconds;

	/** Clamp maximum for SetArenaMatchTimeLimit */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	float ArenaMatchTimeLimitSecondsMaximum;

	/** Clamp minimum for SetArenaMatchTimeLimit */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	float ArenaMatchTimeLimitSecondsMinimum;

	/** Time in seconds after an arena match before the next match can begin */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	float ArenaMatchPostMatchTimeSeconds;

	UPROPERTY(Transient)
	FName ArenaMatchStateName;

	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	FName GetArenaMatchStateName() const { return ArenaMatchStateName; }

	/** Update arena match state and calls transition functions */
	virtual void SetArenaMatchStateName(FName NewArenaMatchStateName);

	/** Overridable function called when match state changes */
	virtual void OnArenaMatchStateNameSet(FName OldArenaMatchStateName, FName NewArenaMatchStateName);

	UFUNCTION(BlueprintImplementableEvent, Category = "ArenaGame", meta = (DisplayName = "OnArenaMatchStateNameSet", ScriptName = "OnArenaMatchStateSet"))
	void K2_OnArenaMatchStateNameSet(FName OldArenaMatchStateName, FName NewArenaMatchStateName);

	/** Replicated arena match data */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	TSubclassOf<class ARAArenaMatchState> ArenaMatchStateClass;
	class ARAArenaMatchState* ArenaMatchState;

	/** Team size for this arena match */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	int32 ArenaMatchTeamSize;
	int32 ArenaMatchTeamSizePendingChange; // Saves team size change if match in progress

	/** Hard-limit maximum for this match's team size */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	int32 ArenaMatchTeamSizeMaximum;

	/** Hard-limit minimum for this match's team size */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ArenaGame")
	int32 ArenaMatchTeamSizeMinimum;

	virtual void PreInitializeComponents() override;
	
	virtual void InitArenaMatchState();

	virtual void UpdateArenaMatchStateArenaPlayers();

	virtual void BeginDestroy() override;

	virtual void PostAddControllerToArenaMatch(class AController* Controller);

	virtual void PostRemoveControllerFromArenaMatch(class AController* Controller);

	virtual void SpawnPlayersInArena();

	virtual void GiveControllerInventory(class AController* Controller);

	virtual void RestartPlayersInArena();

	virtual void CleanUpArena();

public:
	/** Fired when state name changes - valid for server logic only */
	FOnArenaMatchStateChangedSignature OnArenaMatchStateChanged;

	/** Fired when team size changes - valid for server logic only */
	FOnArenaMatchTeamSizeChangedSignature OnArenaMatchTeamSizeChanged;

	/** Fired when time limit changes - valid for server logic only */
	FOnArenaMatchTimeLimitChangedSignature OnArenaMatchTimeLimitChanged;

public:
	ARAArenaMatch(const FObjectInitializer& ObjectInitializer);

	/** Returns all controllers committed to this match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void GetArenaMatchCommittedControllersCopy(TArray<class AController*>& OutControllers);

	/** Returns all controllers pending entry to this match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void GetArenaMatchPendingControllersCopy(TArray<class AController*>& OutControllers);

	/** Returns all controllers, both pending and committed, in this match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void GetArenaMatchAllControllersCopy(TArray<class AController*>& OutControllers);

	/** Returns the controllers of this match's most recent winners */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void GetArenaMatchWinnerControllersCopy(TArray<class AController*>& OutControllers);

	/** Return the controllers of this match's most recent losers */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void GetArenaMatchLoserControllersCopy(TArray<class AController*>& OutControllers);

	/** Returns whether or not the specified controller is in this arena match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	bool ContainsController(const class AController* Controller);

	/** Returns whether or not the specified controller is pending to join in this arena match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	bool ContainsPendingController(const class AController* Controller);

	/** Returns true if this arena match is waiting to be used */
	bool CheckIsAvailableToStartNewArenaMatch();

	/** Returns the number of players required to be added in order to start */
	int32 CalculateAdditionalPlayersRequiredToStart();

	/** Set the team size for this arena match */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void SetArenaMatchTeamSize(int32 NewArenaMatchTeamSize);

	/** Sets the time limit for the match in progress state, but won't perform a live update */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	void SetArenaMatchTimeLimit(float NewArenaMatchTimeLimitSeconds);

	/** Adds the specified controllers to this arena match and sets their teams */
	void AddControllersToArenaMatch(const TArray<class AController*>& Controllers);

	/** Removes the specified controllers from this arena match and unsets their teams */
	void RemoveControllersFromArenaMatch(const TArray<class AController*>& Controllers);

	/** Remove all controllers from this arena match
	*	This will kick all players off of the competing teams, but it will not respawn them
	*/
	virtual void RemoveAllControllersFromArenaMatch();

	void EnableArenaMatch();

	/** Start the arena match with whatever controllers are present */
	void StartArenaMatch();

	/** Removes all pending controllers and returns to idle state */
	void AbortArenaMatch();

	class ARAArenaMatchState* GetArenaMatchState() { return ArenaMatchState; }

	/** Called by owning game mode when a player dies */
	virtual void Killed(class AController* KillerController, class AController* VictimController, class APawn* VictimPawn);

	/** Called by ArenaGameMode, allows individual arena matches to modify damages dealt to players */
	virtual float CalculateDesiredDamageModification(class APawn* VictimPawn, class APawn* InstigatorPawn, float InputDamage);

protected:
	/** Should be called any time the match needs to check for end condition
	*	If end condition passes, this function carries out the chain of actions necessary
	*	Returns true if match end condition is true
	*/
	bool TryProcessMatchEndCondition(bool bMatchTimedOut = false);
	void ProcessMatchEnd(uint8 WinningTeamIndex);
	void AnnounceWinners(uint8 WinningTeamIndex);
};