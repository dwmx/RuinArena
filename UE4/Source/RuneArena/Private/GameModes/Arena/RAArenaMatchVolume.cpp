#include "GameModes/Arena/RAArenaMatchVolume.h"
#include "Inventory/RANewInventory.h"

ARAArenaMatchVolume::ARAArenaMatchVolume()
{}

bool ARAArenaMatchVolume::CheckShouldActorBeCleanedUp(AActor* Actor)
{
	if (Actor->IsA<ARANewInventory>() && Actor->GetOwner() == nullptr)
	{
		return true;
	}

	return false;
}

void ARAArenaMatchVolume::CleaUpArenaMatchVolume()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	for (auto It : OverlappingActors)
	{
		if (CheckShouldActorBeCleanedUp(It))
		{
			It->Destroy();
		}
	}
}