#include "Player/Messaging/RALocalMessage.h"
#include "Player/RAPlayerController.h"
#include "Player/RAPlayerState.h"
#include "GameModes/RAHUDBase.h"
#include "Engine/Console.h"

URALocalMessage::URALocalMessage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsConsoleMessage = true;
}

FText URALocalMessage::GetText(int32 Switch, APlayerState* PlayerState1, APlayerState* PlayerState2, UObject* OptionalObject) const
{
	return FText::FromString(FString::Printf(TEXT("null message text")));
}

FText URALocalMessage::GetConsoleText(const FClientReceiveData& ClientData) const
{
	return GetText(ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
}

FText URALocalMessage::GetPlayerNameText(APlayerState* PlayerState) const
{
	FString PlayerName;
	if (PlayerState == nullptr)
	{
		PlayerName = FString::Printf(TEXT("null player text"));
	}
	else
	{
		PlayerName = PlayerState->GetPlayerName();
	}

	return FText::FromString(PlayerName);
}

void URALocalMessage::ClientReceive(const FClientReceiveData& ClientData) const
{
	// Print to console
	if (bIsConsoleMessage)
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(ClientData.LocalPC->Player);
		if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr)
		{
			FText ConsoleText = GetConsoleText(ClientData);
			if (!ConsoleText.IsEmpty())
			{
				LocalPlayer->ViewportClient->ViewportConsole->OutputText(ConsoleText.ToString());
			}
		}
	}

	// Send to HUD
	ARAPlayerController* PlayerController = Cast<ARAPlayerController>(ClientData.LocalPC);
	if (PlayerController != nullptr)
	{
		ARAHUDBase* PlayerHUD = Cast<ARAHUDBase>(PlayerController->GetHUD());
		if (PlayerHUD != nullptr)
		{
			FText LocalMessageText = FText::FromString(FString::Printf(TEXT("%s"), *ClientData.MessageString));

			if (LocalMessageText.IsEmpty())
			{
				LocalMessageText = GetText(ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
			}

			if (!LocalMessageText.IsEmpty())
			{
				PlayerHUD->ReceiveLocalMessage(
					GetClass(),
					Cast<ARAPlayerState>(ClientData.RelatedPlayerState_1),
					Cast<ARAPlayerState>(ClientData.RelatedPlayerState_2),
					LocalMessageText,
					ClientData.OptionalObject);
			}
		}
	}
}