#include "Inventory/RAInventoryStateIdle.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateIdle::URAInventoryStateIdle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateIdle");
	bCanBeInteractedWith = true;
}

void URAInventoryStateIdle::BeginState()
{
	Super::BeginState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->Velocity = FVector(0.0f);
		Inventory->MovementComponent->SetUpdatedComponent(nullptr);
		Inventory->RotatingMovementComponent->RotationRate = FRotator(0.0f);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(nullptr);

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			ForceInventoryNetUpdate();
		}
	}
}

void URAInventoryStateIdle::EndState()
{
	Super::EndState();
}