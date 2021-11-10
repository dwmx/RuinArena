#pragma once

#include "Blueprint/UserWidget.h"
#include "RAUserWidget.generated.h"

UCLASS()
class RUNEARENA_API URAUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** RAHUD accessor for blueprints */
	UFUNCTION(BlueprintCallable, Category = "Widget|RuneArena")
	class ARAHUDBase* GetOwnerRAHUDBase();

	/** Utility function for converting seconds to a MM:SS formatted string */
	UFUNCTION(BlueprintCallable, Category = "Widget")
	FString GetTimeSecondsString_FormatMMSS(float TimeSeconds);

public:
	URAUserWidget(const FObjectInitializer& ObjectInitializer);
};