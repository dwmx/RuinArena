#include "GameModes/Arena/RAArenaGameMessage.h"
#include "GameModes/Arena/RAArenaMatchState.h"

int32 URAArenaGameMessage::MakeSwitch(int32 Index, int32 CallerData)
{
	Index = Index & 0x0000FFFF;
	CallerData = CallerData & 0x0000FFFF;
	return (CallerData << 16) | Index;
}

int32 URAArenaGameMessage::UnpackSwitchIndex(int32 Switch) const
{
	return Switch & 0x0000FFFF;
}

int32 URAArenaGameMessage::UnpackSwitchCallerData(int32 Switch) const
{
	return (Switch >> 16) & 0x0000FFFF;
}

URAArenaGameMessage::URAArenaGameMessage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ArenaMatchTimeLimitText = NSLOCTEXT("RAArenaGameMessage", "ArenaMatchTimeLimit", "Arena match time limit changed to {Minutes} minutes");
	ArenaMatchTeamSizeText = NSLOCTEXT("RAArenaGameMessage", "ArenaMatchTeamSize", "Arena match team size changed to {TeamSize}");
	YourTurnToFightText = NSLOCTEXT("RAArenaGameMessage", "YourTurnToFight", "Get ready, it's your turn to fight...");
}

FText URAArenaGameMessage::GetText(int32 Switch, APlayerState* PlayerState1, APlayerState* PlayerState2, UObject* OptionalObject) const
{
	int32 Index = UnpackSwitchIndex(Switch);
	int32 CallerData = UnpackSwitchCallerData(Switch);

	switch (Index)
	{
	case Index::ArenaMatchTimeLimit:	return GetArenaMatchTimeLimitText(CallerData, Cast<ARAArenaMatchState>(OptionalObject));
	case Index::ArenaMatchTeamSize:		return GetArenaMatchTeamSizeText(CallerData, Cast<ARAArenaMatchState>(OptionalObject));
	case Index::YourTurnToFight:		return GetYourTurnToFightText();
	}

	return Super::GetText(Switch, PlayerState1, PlayerState2, OptionalObject);
}

FText URAArenaGameMessage::GetArenaMatchTimeLimitText(int32 CallerData, ARAArenaMatchState* ArenaMatchState) const
{
	FFormatNamedArguments Args;
	Args.Add("Minutes", CallerData);
	return FText::Format(ArenaMatchTimeLimitText, Args);
}

FText URAArenaGameMessage::GetArenaMatchTeamSizeText(int32 CallerData, ARAArenaMatchState* ArenaMatchState) const
{
	FFormatNamedArguments Args;
	Args.Add("TeamSize", CallerData);
	return FText::Format(ArenaMatchTeamSizeText, Args);
}

FText URAArenaGameMessage::GetYourTurnToFightText() const
{
	FFormatNamedArguments Args;
	return FText::Format(YourTurnToFightText, Args);
}