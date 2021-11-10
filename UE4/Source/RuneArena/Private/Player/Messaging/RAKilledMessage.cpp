#include "Player/Messaging/RAKilledMessage.h"
#include "Player/RAPlayerState.h"

FText URAKilledMessage::GetConsoleText(const FClientReceiveData& ClientData) const
{
	FString Player1Name;
	FString Player2Name;

	if (ClientData.RelatedPlayerState_1 != nullptr)
	{
		Player1Name = ClientData.RelatedPlayerState_1->GetPlayerName();
	}

	if (ClientData.RelatedPlayerState_2 != nullptr)
	{
		Player2Name = ClientData.RelatedPlayerState_2->GetPlayerName();
	}

	return FText::FromString(FString::Printf(TEXT("%s killed %s"), *Player1Name, *Player2Name));
}