#include "Inventory/RASpawnerStateAsleep.h"
#include "Inventory/RASpawner.h"

URASpawnerStateAsleep::URASpawnerStateAsleep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SleepTimeSeconds = 10.0f;
}

void URASpawnerStateAsleep::BeginState()
{
	Super::BeginState();

	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		FTimerHandle TempTimerHandle;
		Spawner->GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, this, &URASpawnerStateAsleep::HandleSleepTimer, SleepTimeSeconds, false);
	
		if (Spawner->bReplicateSpawnerEffects)
		{
			Spawner->NetMulticastOnSpawnerSleep();
		}
	}
}

void URASpawnerStateAsleep::EndState()
{
	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		Spawner->GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndState();
}

void URASpawnerStateAsleep::HandleSleepTimer()
{
	ARASpawner* Spawner = GetSpawner();
	if (Spawner != nullptr && Spawner->GetLocalRole() == ROLE_Authority)
	{
		Spawner->SwitchSpawnerState(Spawner->SpawnerStateAwake);
	}
}