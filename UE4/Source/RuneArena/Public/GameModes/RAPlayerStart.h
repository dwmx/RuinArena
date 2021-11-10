#pragma once

#include "GameFramework/PlayerStart.h"
#include "RAPlayerStart.generated.h"

UCLASS()
class RUNEARENA_API ARAPlayerStart : public APlayerStart
{
	GENERATED_BODY()

protected:
	/** In team games, indicates which team index this spawn is intended for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teams")
	uint8 TeamIndex;

public:
	ARAPlayerStart(const FObjectInitializer& ObjectInitializer);

	const uint8 GetTeamIndex() { return TeamIndex; }
};