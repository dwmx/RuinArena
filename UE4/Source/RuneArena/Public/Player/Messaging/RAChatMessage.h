#pragma once

#include "Player/Messaging/RALocalMessage.h"
#include "RAChatMessage.generated.h"

UCLASS()
class RUNEARENA_API URAChatMessage : public URALocalMessage
{
	GENERATED_BODY()

protected:
	virtual FText GetConsoleText(const FClientReceiveData& ClientData) const override;

public:
	virtual void ClientReceiveChat(const FClientReceiveData& ClientReceiveData) const;
};