#pragma once

#include "Inventory/RAInventoryState.h"
#include "RAInventoryStateEquipped.generated.h"

UCLASS()
class URAInventoryStateEquipped : public URAInventoryState
{
	GENERATED_BODY()

protected:
	virtual void BeginState() override;
	virtual void EndState() override;

public:
	URAInventoryStateEquipped(const FObjectInitializer& ObjectInitializer);

	
};