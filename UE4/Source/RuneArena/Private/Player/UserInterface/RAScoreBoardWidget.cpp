#include "Player/UserInterface/RAScoreBoardWidget.h"
#include "GameModes/RAGameStateBase.h"
#include "Player/RAPlayerState.h"
#include "GameFramework/GameStateBase.h"

FRAPlayerScoreData::FRAPlayerScoreData()
{
	PlayerName = TEXT("Unnamed Player");
	PlayerScore = 0;
	PlayerDeaths = 0;
	PlayerPing = 0;
	PlayerTeam = 255;
}

URAScoreBoardWidget::URAScoreBoardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

TArray<ARAPlayerState*> URAScoreBoardWidget::GetPlayerStates()
{
	TArray<ARAPlayerState*> Result;
	ARAGameStateBase* GameState = Cast<ARAGameStateBase>(GetWorld()->GetGameState());
	if (GameState != nullptr)
	{
		for (auto It : GameState->PlayerArray)
		{
			Result.Add(Cast<ARAPlayerState>(It));
		}
	}
	return Result;
}

TArray<ARAPlayerState*> URAScoreBoardWidget::GetPlayerStatesOnTeam(uint8 TeamIndex)
{
	TArray<ARAPlayerState*> Result;
	ARAGameStateBase* GameState = Cast<ARAGameStateBase>(GetWorld()->GetGameState());
	if (GameState != nullptr)
	{
		for (auto It : GameState->PlayerArray)
		{
			ARAPlayerState* PlayerState = Cast<ARAPlayerState>(It);
			if (PlayerState != nullptr && PlayerState->GetTeamIndex() == TeamIndex)
			{
				Result.Add(Cast<ARAPlayerState>(It));
			}
		}
	}
	return Result;
}

void URAScoreBoardWidget::SortPlayerStatesByScore(TArray<ARAPlayerState*>& InPlayerStates, TArray<ARAPlayerState*>& OutPlayerStates)
{
	InPlayerStates.Sort([](const ARAPlayerState& A, const ARAPlayerState& B) { return A.GetScore() < B.GetScore(); });
	OutPlayerStates = InPlayerStates;
}

void URAScoreBoardWidget::SortPlayersByFrags(TArray<ARAPlayerState*>& InPlayerStates, TArray<ARAPlayerState*>& OutPlayerStates)
{
	InPlayerStates.Sort([](const ARAPlayerState& A, const ARAPlayerState& B) { return A.GetFrags() < B.GetFrags(); });
	OutPlayerStates = InPlayerStates;
}

void URAScoreBoardWidget::UpdatePlayerScores()
{
	UWorld* WorldInstance = GetWorld();
	if (WorldInstance == nullptr)
	{
		return;
	}

	AGameStateBase* GameState = WorldInstance->GetGameState<AGameStateBase>();
	if (GameState == nullptr)
	{
		return;
	}

	// Update array
	PlayerScoreDataArray.Empty();
	PlayerScoreDataArray.Reserve(GameState->PlayerArray.Num());

	for (auto It : GameState->PlayerArray)
	{
		ARAPlayerState* PlayerState = Cast<ARAPlayerState>(It);

		PlayerScoreDataArray.Add(FRAPlayerScoreData());
		FRAPlayerScoreData& PlayerScoreData = PlayerScoreDataArray.Last();

		PlayerScoreData.PlayerName = PlayerState->GetPlayerName();
		PlayerScoreData.PlayerScore = PlayerState->GetScore();
		PlayerScoreData.PlayerFrags = PlayerState->GetFrags();
		PlayerScoreData.PlayerDeaths = PlayerState->GetDeaths();
		PlayerScoreData.PlayerPing = PlayerState->GetPing();
		PlayerScoreData.PlayerTeam = PlayerState->GetTeamIndex();
	}
}