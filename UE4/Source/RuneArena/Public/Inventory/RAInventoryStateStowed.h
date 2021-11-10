#pragma once

#include "Inventory/RAInventoryState.h"
#include "RAInventoryStateStowed.generated.h"

UCLASS()
class URAInventoryStateStowed : public URAInventoryState
{
	GENERATED_BODY()

public:
	URAInventoryStateStowed(const FObjectInitializer& ObjectInitializer);

	virtual void BeginState() override;
	virtual void EndState() override;
};