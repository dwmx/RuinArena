#pragma once

#include "Inventory/RAInventoryState.h"
#include "RAInventoryStateIdle.generated.h"

UCLASS()
class URAInventoryStateIdle : public URAInventoryState
{
	GENERATED_BODY()

public:
	URAInventoryStateIdle(const FObjectInitializer& ObjectInitializer);

	virtual void BeginState() override;
	virtual void EndState() override;
};