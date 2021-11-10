#pragma once

#include "Inventory/RASpawnerState.h"
#include "RASpawnerStateInactive.generated.h"

UCLASS()
class URASpawnerStateInactive : public URASpawnerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float SleepTimeSeconds;

	virtual void BeginState() override;
	virtual void EndState() override;

public:
	URASpawnerStateInactive(const FObjectInitializer& ObjectInitializer);
};