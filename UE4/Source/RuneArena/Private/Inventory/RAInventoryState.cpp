#include "Inventory/RAInventoryState.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryState::URAInventoryState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryState");
	bCanBeInteractedWith = true;
}

ARANewInventory* URAInventoryState::GetInventory()
{
	return Cast<ARANewInventory>(GetOuter());
}

void URAInventoryState::BeginState()
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->InventoryStateName = InventoryStateName;
		}
	}
}

void URAInventoryState::EndState()
{}

void URAInventoryState::ForceInventoryNetUpdate()
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr && Inventory->GetLocalRole() == ROLE_Authority)
	{
		Inventory->InventoryNetUpdateParameters.Location = Inventory->GetActorLocation();
		Inventory->InventoryNetUpdateParameters.Rotation = Inventory->GetActorRotation();
		Inventory->InventoryNetUpdateParameters.LinearVelocity = Inventory->MovementComponent->Velocity;
		Inventory->InventoryNetUpdateParameters.PitchRotationRate = Inventory->RotatingMovementComponent->RotationRate.Pitch;
		Inventory->InventoryNetUpdateParameters.YawRotationRate = Inventory->RotatingMovementComponent->RotationRate.Yaw;
		Inventory->InventoryNetUpdateParameters.RollRotationRate = Inventory->RotatingMovementComponent->RotationRate.Roll;
	}
}