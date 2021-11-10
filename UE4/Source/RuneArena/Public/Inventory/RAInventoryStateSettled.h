#pragma once

#include "Inventory/RAInventoryState.h"
#include "RAInventoryStateSettled.generated.h"

UCLASS()
class URAInventoryStateSettled : public URAInventoryState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float DespawnTimeSeconds;

	virtual void HandleDespawnTimer();

public:
	URAInventoryStateSettled(const FObjectInitializer& ObjectInitializer);

	virtual void BeginState() override;
	virtual void EndState() override;
};