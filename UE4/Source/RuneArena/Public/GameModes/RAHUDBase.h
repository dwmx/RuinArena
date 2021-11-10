#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RAHUDBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnReceiveLocalMessageSignature,
	TSubclassOf<class URALocalMessage>, LocalMessageClass,
	class ARAPlayerState*, RelatedPlayerState1,
	class ARAPlayerState*, RelatedPlayerState2,
	FText, LocalMessageText,
	class UObject*, OptionalObject);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReceiveActionSaySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveActionShowScoresSignature, bool, bShowScores);

UCLASS()
class RUNEARENA_API ARAHUDBase : public AHUD
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "HUD")
	TArray<TSubclassOf<class URAUserWidget>> UserWidgetClasses;

protected:
	/** Overridden for widget initialization */
	virtual void BeginPlay() override;

	/** Valid on client and server. Fired when the owning player controller receives a damage taken event from its character */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", Meta = (DisplayName = "OnCharacterTakenDamage"))
	void K2_OnCharacterTakenDamage(class AActor* InstigatorActor, float DamageAmount);

	/** Valid on client and server. Fired when the owning player controller receives a damage given event from its character */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", Meta = (DisplayName = "OnCharacterGivenDamage"))
	void K2_OnCharacterGivenDamage(class AActor* VictimActor, float DamageAmount);

public:
	/** Broadcast whenever this HUD actor receives a local message */
	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnReceiveLocalMessageSignature OnReceiveLocalMessage;

	/** Broadcast when the Say action has been input by the owner */
	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnReceiveActionSaySignature OnReceiveActionSay;

	/** Broadcast when the ShowScores action has been input or released by the owner */
	UPROPERTY(BlueprintAssignable, Category = "HUD")
	FOnReceiveActionShowScoresSignature OnReceiveActionShowScores;

	/** Received from URALocalMessage on ClientReceive actions */
	void ReceiveLocalMessage(
		TSubclassOf<class URALocalMessage> LocalMessageClass,
		class ARAPlayerState* RelatedPlayerState1,
		class ARAPlayerState* RelatedPlayerState2,
		FText LocalMessageText,
		class UObject* OptionalObject = nullptr);

	/** Received from ARAPlayerController on Say action inputs */
	void ReceiveActionSay();

	/** Received from ARAPlayerController on ShowScores action inputs */
	void ReceiveShowScores(bool bShowScores);

	/** Received from ARAPlayerController when the relevant character has taken damage */
	void ReceiveCharacterTakenDamage(class AActor* InstigatorActor, float DamageAmount);

	/** Received from ARAPlayerController when the relevant character has given damage */
	void ReceiveCharacterGivenDamage(class AActor* VictimActor, float DamageAmount);
};