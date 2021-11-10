#pragma once

#include "CoreMinimal.h"
#include "RAInventoryState.generated.h"

UCLASS(DefaultToInstanced, EditInlineNew, CustomConstructor, Within=RANewInventory)
class URAInventoryState : public UObject
{
	GENERATED_BODY()

	friend class ARANewInventory;

protected:
	/** Unique name associated with this state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName InventoryStateName;

	/** Can this inventory actor be picked up in this state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanBeInteractedWith;

	class ARANewInventory* GetInventory();

	virtual void BeginState();
	virtual void EndState();

	/** Force an update to all clients, usually called in begin state */
	virtual void ForceInventoryNetUpdate();

public:
	URAInventoryState(const FObjectInitializer& ObjectInitializer);
};