#pragma once

#include "Player/Messaging/RALocalMessage.h"
#include "RAKilledMessage.generated.h"

UCLASS()
class RUNEARENA_API URAKilledMessage : public URALocalMessage
{
	GENERATED_BODY()

protected:
	virtual FText GetConsoleText(const FClientReceiveData& ClientData) const override;
};