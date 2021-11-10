#include "Inventory/RASpawnerStateAwake.h"
#include "Inventory/RASpawner.h"
#include "Inventory/RANewInventory.h"

URASpawnerStateAwake::URASpawnerStateAwake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpawnedInventory = nullptr;
}

void URASpawnerStateAwake::BeginState()
{
	Super::BeginState();

	SpawnInventory();

	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority && Spawner->bReplicateSpawnerEffects)
	{
		Spawner->NetMulticastOnSpawnerWakeUp();
	}
}

void URASpawnerStateAwake::EndState()
{
	DestroyInventory();

	Super::EndState();
}

void URASpawnerStateAwake::SpawnInventory()
{
	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		if (IsValid(Spawner->InventoryToSpawnClass))
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FTransform Transform = Spawner->SpawnerSpawnTransform;
			Transform.SetLocation(Transform.GetLocation() + Spawner->GetActorLocation());
			Transform.SetRotation(Transform.GetRotation() + Spawner->GetActorRotation().Quaternion());

			SpawnedInventory = Cast<ARANewInventory>(GetWorld()->SpawnActor(Spawner->InventoryToSpawnClass, &Transform, SpawnParameters));
			if (SpawnedInventory != nullptr)
			{
				SpawnedInventory->OnInventoryAcquired.AddDynamic(this, &URASpawnerStateAwake::HandleOnInventoryAcquired);
			}
		}
	}
}

void URASpawnerStateAwake::HandleOnInventoryAcquired(ARANewInventory* Inventory)
{
	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		if (SpawnedInventory != nullptr)
		{
			SpawnedInventory->OnInventoryAcquired.RemoveAll(this);
			SpawnedInventory = nullptr;
		}
		Spawner->SwitchSpawnerState(Spawner->SpawnerStateAsleep);
	}
}

void URASpawnerStateAwake::DestroyInventory()
{
	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		if (SpawnedInventory != nullptr)
		{
			SpawnedInventory->Destroy();
		}
	}
}