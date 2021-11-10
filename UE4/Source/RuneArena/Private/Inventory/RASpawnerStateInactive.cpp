#include "Inventory/RASpawnerStateInactive.h"
#include "Inventory/RASpawner.h"

URASpawnerStateInactive::URASpawnerStateInactive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void URASpawnerStateInactive::BeginState()
{
	Super::BeginState();
}

void URASpawnerStateInactive::EndState()
{
	Super::EndState();
}