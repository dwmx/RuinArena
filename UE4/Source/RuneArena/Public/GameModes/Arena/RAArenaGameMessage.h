#pragma once

#include "Player/Messaging/RALocalMessage.h"
#include "RAArenaGameMessage.generated.h"

UCLASS()
class RUNEARENA_API URAArenaGameMessage : public URALocalMessage
{
	GENERATED_BODY()

public:
	struct Index
	{
		static const int32 ArenaMatchTimeLimit = 0;
		static const int32 ArenaMatchTeamSize = 1;
		static const int32 YourTurnToFight = 2;
	};

protected:
	/** Displayed when the time limit of an arena match has updated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText ArenaMatchTimeLimitText;

	/** Displayed when the team size of an arena match has updated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText ArenaMatchTeamSizeText;

	/** Displayed when a match is about to begin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	FText YourTurnToFightText;

protected:
	virtual FText GetText(int32 Switch = 0, class APlayerState* PlayerState1 = nullptr, class APlayerState* PlayerState2 = nullptr, class UObject* OptionalObject = nullptr) const override;

	virtual FText GetArenaMatchTimeLimitText(int32 CallerData, class ARAArenaMatchState* ArenaMatchState) const;
	virtual FText GetArenaMatchTeamSizeText(int32 CallerData, class ARAArenaMatchState* ArenaMatchState) const;
	virtual FText GetYourTurnToFightText() const;

public:
	URAArenaGameMessage(const FObjectInitializer& ObjectInitializer);

	static int32 MakeSwitch(int32 Index, int32 CallerData);

protected:
	int32 UnpackSwitchIndex(int32 Switch) const;
	int32 UnpackSwitchCallerData(int32 Switch) const;
};