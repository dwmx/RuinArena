#pragma once

#include "Inventory/RASpawnerState.h"
#include "RASpawnerStateAwake.generated.h"

UCLASS()
class URASpawnerStateAwake : public URASpawnerState
{
	GENERATED_BODY()

protected:
	virtual void BeginState() override;
	virtual void EndState() override;

	class ARANewInventory* SpawnedInventory;
	void DestroyInventory();

	virtual void SpawnInventory();

	UFUNCTION()
	virtual void HandleOnInventoryAcquired(class ARANewInventory* Inventory);

public:
	URASpawnerStateAwake(const FObjectInitializer& ObjectInitializer);
};