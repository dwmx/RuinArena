#include "Player/UserInterface/RAUserWidget.h"
#include "GameModes/RAHUDBase.h"

URAUserWidget::URAUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

ARAHUDBase* URAUserWidget::GetOwnerRAHUDBase()
{
	if (GetOwningPlayer() != nullptr)
	{
		return Cast<ARAHUDBase>(GetOwningPlayer()->GetHUD<ARAHUDBase>());
	}
	return nullptr;
}

FString URAUserWidget::GetTimeSecondsString_FormatMMSS(float TimeSeconds)
{
	int32 Truncated = FMath::TruncToInt(TimeSeconds + 1.0f);
	int32 Minutes = Truncated / 60;
	Minutes = FMath::Min<int32>(Minutes, 99);
	int32 Seconds = Truncated % 60;

	int32 Char1 = 0;
	if (Minutes >= 10)
	{
		Char1 = Minutes / 10;
	}
	int32 Char2 = Minutes % 10;

	int32 Char3 = 0;
	if (Seconds >= 10)
	{
		Char3 = Seconds / 10;
	}
	int32 Char4 = Seconds % 10;

	return FString::Printf(TEXT("%d%d:%d%d"), Char1, Char2, Char3, Char4);
}