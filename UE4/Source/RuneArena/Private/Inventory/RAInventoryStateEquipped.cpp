#include "Inventory/RAInventoryStateEquipped.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateEquipped::URAInventoryStateEquipped(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateEquipped");
	bCanBeInteractedWith = false;
}

void URAInventoryStateEquipped::BeginState()
{
	Super::BeginState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(nullptr);
		//Inventory->MovementComponent->SetInterpolatedComponent(nullptr);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(nullptr);

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			//ForceInventoryNetUpdate();
		}
	}
}

void URAInventoryStateEquipped::EndState()
{
	Super::EndState();
}