#pragma once

#include "GameFramework/LocalMessage.h"
#include "RALocalMessage.generated.h"

UCLASS(Blueprintable, Abstract, NotPlaceable, Meta = (ShowWorldContextPin))
class RUNEARENA_API URALocalMessage : public ULocalMessage
{
	GENERATED_BODY()

protected:
	/** Should this message print to console */
	UPROPERTY(EditAnywhere, Category = "Messaging")
	bool bIsConsoleMessage;

	virtual FText GetText(int32 Switch = 0, class APlayerState* PlayerState1 = nullptr, class APlayerState* PlayerState2 = nullptr, class UObject* OptionalObject = nullptr) const;

	virtual FText GetConsoleText(const FClientReceiveData& ClientData) const;

	virtual FText GetPlayerNameText(class APlayerState* PlayerState) const;

public:
	URALocalMessage(const FObjectInitializer& ObjectInitializer);

	virtual void ClientReceive(const FClientReceiveData& ClientData) const override;
};