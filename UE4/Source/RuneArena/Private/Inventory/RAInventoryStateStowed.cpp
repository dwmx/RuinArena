#include "Inventory/RAInventoryStateStowed.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateStowed::URAInventoryStateStowed(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateStowed");
	bCanBeInteractedWith = false;
}

void URAInventoryStateStowed::BeginState()
{
	Super::BeginState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(nullptr);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(nullptr);

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			//ForceInventoryNetUpdate();
		}
	}
}

void URAInventoryStateStowed::EndState()
{
	Super::EndState();
}