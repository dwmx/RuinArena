#include "Player/Messaging/RAGameMessage.h"

int32 URAGameMessage::MakeSwitch(int32 Index, int32 CallerData)
{
	Index = Index & 0x0000FFFF;
	CallerData = CallerData & 0x0000FFFF;
	return (CallerData << 16) | Index;
}

int32 URAGameMessage::UnpackSwitchIndex(int32 Switch) const
{
	return Switch & 0x0000FFFF;
}

int32 URAGameMessage::UnpackSwitchCallerData(int32 Switch) const
{
	return (Switch >> 16) & 0x0000FFFF;
}

URAGameMessage::URAGameMessage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RconAuthorizedText = NSLOCTEXT("RAGameMessage", "RconAuthorizedMessage", "{PlayerName} became a server administrator");
	RconNormalText = NSLOCTEXT("RAGameMessage", "RconNormalMessage", "{PlayerName} gave up administrator rights");
	PlayerJoinedText = NSLOCTEXT("RAGameMessage", "PlayerJoinedText", "{PlayerName} joined the game");
	PlayerLeftText = NSLOCTEXT("RAGameMessage", "PlayerLeftText", "{PlayerName} left the game");
	TimeLimitText = NSLOCTEXT("RAGameMessage", "TimeLimitText", "Time limit changed to {Minutes} minutes");
	TeamSizeText = NSLOCTEXT("RAGameMessage", "TeamSizeText", "Team size changed to {TeamSize}");
}

FText URAGameMessage::GetText(int32 Switch, APlayerState* PlayerState1, APlayerState* PlayerState2, UObject* OptionalObject) const
{
	int32 Index = UnpackSwitchIndex(Switch);
	int32 CallerData = UnpackSwitchCallerData(Switch);

	switch (Index)
	{
	case Index::RconAuthorized:	return GetRconAuthorizedText(PlayerState1);
	case Index::RconNormal:		return GetRconNormalText(PlayerState1);
	case Index::PlayerJoined:	return GetPlayerJoinedText(PlayerState1);
	case Index::PlayerLeft:		return GetPlayerLeftText(PlayerState1);
	case Index::TimeLimit:		return GetTimeLimitText(CallerData);
	case Index::TeamSize:		return GetTeamSizeText(CallerData);
	}

	return Super::GetText(Switch, PlayerState1, PlayerState2, OptionalObject);
}

FText URAGameMessage::GetRconAuthorizedText(APlayerState* PlayerState) const
{
	FFormatNamedArguments Args;
	Args.Add("PlayerName", GetPlayerNameText(PlayerState));
	return FText::Format(RconAuthorizedText, Args);
}

FText URAGameMessage::GetRconNormalText(APlayerState* PlayerState) const
{
	FFormatNamedArguments Args;
	Args.Add("PlayerName", GetPlayerNameText(PlayerState));
	return FText::Format(RconNormalText, Args);
}

FText URAGameMessage::GetPlayerJoinedText(APlayerState* PlayerState) const
{
	FFormatNamedArguments Args;
	Args.Add("PlayerName", GetPlayerNameText(PlayerState));
	return FText::Format(PlayerJoinedText, Args);
}

FText URAGameMessage::GetPlayerLeftText(APlayerState* PlayerState) const
{
	FFormatNamedArguments Args;
	Args.Add("PlayerName", GetPlayerNameText(PlayerState));
	return FText::Format(PlayerLeftText, Args);
}

FText URAGameMessage::GetTimeLimitText(int32 CallerData) const
{
	FFormatNamedArguments Args;
	Args.Add("Minutes", CallerData);
	return FText::Format(TimeLimitText, Args);
}

FText URAGameMessage::GetTeamSizeText(int32 CallerData) const
{
	FFormatNamedArguments Args;
	Args.Add("TeamSize", CallerData);
	return FText::Format(TeamSizeText, Args);
}