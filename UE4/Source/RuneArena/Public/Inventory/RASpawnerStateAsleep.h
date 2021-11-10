#pragma once

#include "Inventory/RASpawnerState.h"
#include "RASpawnerStateAsleep.generated.h"

UCLASS()
class URASpawnerStateAsleep : public URASpawnerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float SleepTimeSeconds;

	virtual void BeginState() override;
	virtual void EndState() override;

	virtual void HandleSleepTimer();

public:
	URASpawnerStateAsleep(const FObjectInitializer& ObjectInitializer);
};