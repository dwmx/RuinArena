#pragma once

#include "Player/UserInterface/RAUserWidget.h"
#include "RAScoreBoardWidget.generated.h"

USTRUCT(BlueprintType)
struct RUNEARENA_API FRAPlayerScoreData
{
	GENERATED_USTRUCT_BODY()

	FRAPlayerScoreData();

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	int32 PlayerScore;

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	int32 PlayerFrags;

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	int32 PlayerDeaths;

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	int32 PlayerPing;

	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	uint8 PlayerTeam;
};

UCLASS()
class RUNEARENA_API URAScoreBoardWidget : public URAUserWidget
{
	GENERATED_BODY()

protected:
	/** Returns all player states known by this server or client */
	UFUNCTION(BlueprintCallable, Category = "Widget|RuneArena")
	TArray<class ARAPlayerState*> GetPlayerStates();

	/** Returns all player states on specified team index known by this server or client */
	UFUNCTION(BlueprintCallable, Category = "Widget|RuneArena")
	TArray<class ARAPlayerState*> GetPlayerStatesOnTeam(uint8 TeamIndex);

	/** Sort the array of player states based on score */
	UFUNCTION(BlueprintCallable, Category = "Widget|RuneArena")
	void SortPlayerStatesByScore(UPARAM(ref) TArray<class ARAPlayerState*>& InPlayerStates, TArray<class ARAPlayerState*>& OutPlayerStates);

	/** Sort the array of player states based on score */
	UFUNCTION(BlueprintCallable, Category = "Widget|RuneArena")
	void SortPlayersByFrags(UPARAM(ref) TArray<class ARAPlayerState*>& InPlayerStates, TArray<class ARAPlayerState*>& OutPlayerStates);

public:
	URAScoreBoardWidget(const FObjectInitializer& ObjectInitializer);


	// TODO: Remove these functions
protected:
	UPROPERTY(BlueprintReadOnly, Category = "ScoreBoard")
	TArray<FRAPlayerScoreData> PlayerScoreDataArray;

public:
	UFUNCTION(BlueprintCallable, Category = "ScoreBoard")
	void UpdatePlayerScores();
};