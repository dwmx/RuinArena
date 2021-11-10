#include "Inventory/RASpawnerState.h"
#include "Inventory/RASpawner.h"

URASpawnerState::URASpawnerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

ARASpawner* URASpawnerState::GetSpawner()
{
	return Cast<ARASpawner>(GetOuter());
}

void URASpawnerState::BeginState()
{}

void URASpawnerState::EndState()
{}