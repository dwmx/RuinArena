#include "Player/Messaging/RAChatMessage.h"
#include "Player/RAPlayerController.h"
#include "Player/RAPlayerState.h"

FText URAChatMessage::GetConsoleText(const FClientReceiveData& ClientData) const
{
	FText PlayerName = GetPlayerNameText(ClientData.RelatedPlayerState_1);
	FFormatNamedArguments Args;
	Args.Add(TEXT("PlayerName"), PlayerName);
	Args.Add(TEXT("PlayerMessage"), FText::FromString(ClientData.MessageString));
	return FText::Format(NSLOCTEXT("RALocalMessage", "ChatMessage", "{PlayerName}: {PlayerMessage}"), Args);
}

void URAChatMessage::ClientReceiveChat(const FClientReceiveData& ClientReceiveData) const
{
	Super::ClientReceive(ClientReceiveData);
}