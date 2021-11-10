#include "Inventory/RAInventoryStateSettled.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateSettled::URAInventoryStateSettled(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateSettled");
	DespawnTimeSeconds = 15.0f;
}

void URAInventoryStateSettled::BeginState()
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
			FTimerHandle TempTimerHandle;
			Inventory->GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, this, &URAInventoryStateSettled::HandleDespawnTimer, DespawnTimeSeconds, false);
			ForceInventoryNetUpdate();
		}
	}
}

void URAInventoryStateSettled::EndState()
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		}
	}

	Super::EndState();
}

void URAInventoryStateSettled::HandleDespawnTimer()
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->DespawnInventory();
		}
	}
}