#include "GameModes/RAHUDBase.h"
#include "Player/RAPlayerController.h"
#include "Player/UserInterface/RAUserWidget.h"

void ARAHUDBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize user widgets
	ARAPlayerController* PlayerController = Cast<ARAPlayerController>(GetOwningPlayerController());
	if (PlayerController != nullptr)
	{
		for (auto It : UserWidgetClasses)
		{
			if (IsValid(It))
			{
				UUserWidget* CreatedWidget = CreateWidget(PlayerController, It);
				if (CreatedWidget != nullptr)
				{
					CreatedWidget->AddToViewport();
				}
				else
				{
					UE_LOG(LogPlayerController, Warning, TEXT("RAHUDBase failed to create user widget"));
				}
			}
			else
			{
				UE_LOG(LogPlayerController, Warning, TEXT("RAHUDBase contains an invalid widget class"));
			}
		}
	}
}

void ARAHUDBase::ReceiveLocalMessage(
	TSubclassOf<URALocalMessage> LocalMessageClass,
	ARAPlayerState* RelatedPlayerState1,
	ARAPlayerState* RelatedPlayerState2,
	FText LocalMessageText,
	UObject* OptionalObject)
{
	if (OnReceiveLocalMessage.IsBound())
	{
		OnReceiveLocalMessage.Broadcast(
			LocalMessageClass,
			RelatedPlayerState1,
			RelatedPlayerState2,
			LocalMessageText,
			OptionalObject);
	}
}

void ARAHUDBase::ReceiveActionSay()
{
	if (OnReceiveActionSay.IsBound())
	{
		OnReceiveActionSay.Broadcast();
	}
}

void ARAHUDBase::ReceiveShowScores(bool bShowScores)
{
	if (OnReceiveActionShowScores.IsBound())
	{
		OnReceiveActionShowScores.Broadcast(bShowScores);
	}
}

void ARAHUDBase::ReceiveCharacterTakenDamage(AActor* InstigatorActor, float DamageAmount)
{
	K2_OnCharacterTakenDamage(InstigatorActor, DamageAmount);
}

void ARAHUDBase::ReceiveCharacterGivenDamage(AActor* VictimActor, float DamageAmount)
{
	K2_OnCharacterGivenDamage(VictimActor, DamageAmount);
}