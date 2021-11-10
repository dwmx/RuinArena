#pragma once

#include "Player/Messaging/RALocalMessage.h"
#include "RAGameMessage.generated.h"

UCLASS()
class RUNEARENA_API URAGameMessage : public URALocalMessage
{
	GENERATED_BODY()

public:
	struct Index
	{
		static const int32 RconAuthorized = 0;
		static const int32 RconNormal = 1;
		static const int32 PlayerJoined = 2;
		static const int32 PlayerLeft = 3;
		static const int32 TimeLimit = 4;
		static const int32 TeamSize = 5;
	};

protected:
	/** Displayed when a player logs into admin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText RconAuthorizedText;

	/** Displayed when a player logs out of admin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText RconNormalText;

	/** Displayed when a player joins the game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText PlayerJoinedText;

	/** Displayed when a player leaves the game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText PlayerLeftText;

	/** Displayed when a time limit change has been broadcast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText TimeLimitText;

	/** Displayed when a team size change has been broadcast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText TeamSizeText;

protected:
	virtual FText GetText(int32 Switch = 0, class APlayerState* PlayerState1 = nullptr, class APlayerState* PlayerState2 = nullptr, class UObject* OptionalObject = nullptr) const override;

	virtual FText GetRconAuthorizedText(class APlayerState* PlayerState) const;
	virtual FText GetRconNormalText(class APlayerState* PlayerState) const;
	virtual FText GetPlayerJoinedText(class APlayerState* PlayerState) const;
	virtual FText GetPlayerLeftText(class APlayerState* PlayerState) const;
	virtual FText GetTimeLimitText(int32 CallerData) const;
	virtual FText GetTeamSizeText(int32 CallerData) const;

public:
	URAGameMessage(const FObjectInitializer& ObjectInitializer);

	/** Creates the switch for this message */
	static int32 MakeSwitch(int32 Index, int32 CallerData);

protected:
	int32 UnpackSwitchIndex(int32 Switch) const;
	int32 UnpackSwitchCallerData(int32 Switch) const;
};