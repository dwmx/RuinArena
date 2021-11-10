#pragma once

#include "CoreMinimal.h"
#include "GameModes/RAGameModeBase.h"
#include "RAArenaGameMode.generated.h"

UCLASS(Abstract)
class RUNEARENA_API ARAArenaGameMode : public ARAGameModeBase
{
	GENERATED_BODY()

protected:
	/** Class to be used for maintaining the arena matches */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Classes")
	TSubclassOf<class ARAArenaMatch> ArenaMatchClass;

	/** The current arena match */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	class ARAArenaMatch* ArenaMatch;

	/** Event handler for state changes on ArenaMatch actors */
	UFUNCTION()
	void HandleOnArenaMatchStateChanged(class ARAArenaMatch* InArenaMatch, FName OldArenaMatchStateName, FName NewArenaMatchStateName);

	/** Event handler for team size changes on ArenaMatch actors */
	UFUNCTION()
	void HandleOnArenaMatchTeamSizeChanged(class ARAArenaMatch* InArenaMatch, int32 NewArenaMatchTeamSize);

	/** Event handler for time limit changes on ArenaMatch actors */
	UFUNCTION()
	void HandleOnArenaMatchTimeLimitChanged(class ARAArenaMatch* InArenaMatch, float NewArenaMatchTimeLimitSeconds);

	/** Overridden to initialize ArenaMatchState immediately after primary state machine */
	virtual void StartMatch() override;

	/** Overridden to deactivate arena once the match has ended */
	virtual void EndMatch() override;

	/** Queue of controllers waiting join an arena match */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	TArray<class AController*> ControllerQueue;

	/** Determined by the lesser of champions starts and challengers starts */
	/** TODO: These will need to be off loaded to arena match states for multi arena support */
	UPROPERTY(BlueprintReadOnly)
	int32 MaximumLevelSupportedArenaTeamSize;

	int32 CalculateMaximumLevelSupportedArenaTeamSize();

	virtual void BeginPlay() override;

	bool CheckControllerCanEnterQueue(class AController* Controller);

	UFUNCTION()
	void OnQueueZoneOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "ArenaGame", meta = (DisplayName = "OnQueueZoneOverlapBegin", ScriptName = "OnQueueZoneOverlapBegin"))
	void K2_OnQueueZoneOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	void AddControllerToQueue(class AController* Controller);

	void UpdateArenaGameStatePlayerQueue();

	UFUNCTION(BlueprintImplementableEvent, Category = "ArenaGame", meta = (DisplayName = "PostAddControllerToQueue", ScriptName = "PostAddControllerToQueue"))
	void K2_PostAddControllerToQueue(class AController* Controller);

	UFUNCTION()
	void OnQueueZoneOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "ArenaGame", meta = (DisplayName = "OnQueueZoneOverlapEnd", ScriptName = "OnQueueZoneOverlapEnd"))
	void K2_OnQueueZoneOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	void RemoveControllerFromQueue(class AController* Controller);

	UFUNCTION(BlueprintImplementableEvent, Category = "ArenaGame", meta = (DisplayName = "PostRemoveControllerFromQueue", ScriptName = "PostRemoveControllerFromQueue"))
	void K2_PostRemoveControllerFromQueue(class AController* Controller);

	/** Checks for arena match start condition and starts the match if condition is met.
	*	Should be called any time something happens to trigger a match start, like pawn
	*	entering queue or the arena being freed up.
	*/
	void CheckAndTryToStartArenaMatch();

public:
	ARAArenaGameMode(const FObjectInitializer& ObjectInitializer);

	/** Overridden to disqualify ArenaPlayerStarts from player respawning */
	virtual AActor* ChoosePlayerStart_Implementation(class AController* Player) override;

	/** Overridden to prevent damage between players not currently in the arena */
	virtual float CalculateDesiredDamageModification(class APawn* VictimPawn, class APawn* InstigatorPawn, float InputDamage) override;

	/** Overridden to pass death events to arena match */
	virtual void Killed(class AController* KillerController, class AController* VictimController, class APawn* VictimPawn) override;

	/** Overridden to prevent dead arena players from respawning until match end */
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;

	/** RconAdmin interface */
protected:
	virtual void RconAuthorized_TeamSize_Implementation(class ARAPlayerController* Controller, int32 NewTeamSize) override;
	virtual void RconAuthorized_TimeLimit_Implementation(class ARAPlayerController* Controller, float NewtimeLimitMinutes) override;
};