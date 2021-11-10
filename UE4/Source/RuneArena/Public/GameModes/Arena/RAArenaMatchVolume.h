#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "RAArenaMatchVolume.generated.h"

UCLASS(Blueprintable)
class RUNEARENA_API ARAArenaMatchVolume : public ATriggerVolume
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	virtual bool CheckShouldActorBeCleanedUp(class AActor* Actor);

public:
	ARAArenaMatchVolume();

	/** Passes each actor through CheckShouldActorBeCleanedUp and destroys if returned true */
	UFUNCTION(BlueprintCallable, Category = "ArenaGame")
	virtual void CleaUpArenaMatchVolume();
};