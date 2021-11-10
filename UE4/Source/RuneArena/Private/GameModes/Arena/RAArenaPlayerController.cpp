#include "GameModes/Arena/RAArenaPlayerController.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"

ARAArenaPlayerController::ARAArenaPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void ARAArenaPlayerController::SaveInventoryClasses()
{
	SavedInventoryClasses.Empty();
	SavedWeaponClass = nullptr;

	ARACharacter* TempCharacter = GetPawn<ARACharacter>();
	if (TempCharacter != nullptr)
	{
		for (TRAInventoryIterator<ARANewInventory> It(TempCharacter); It; ++It)
		{
			SavedInventoryClasses.Add(It->GetClass());
		}

		if (TempCharacter->GetWeapon() != nullptr)
		{
			SavedWeaponClass = TempCharacter->GetWeapon()->GetClass();
		}
	}
}

void ARAArenaPlayerController::ClearSavedInventoryClasses()
{
	SavedInventoryClasses.Empty();
}

void ARAArenaPlayerController::GetSavedInventoryClassesCopy(TArray<TSubclassOf<ARANewInventory>>& OutSavedInventoryClasses)
{
	OutSavedInventoryClasses.Empty();
	for (auto It : SavedInventoryClasses)
	{
		OutSavedInventoryClasses.Add(It);
	}
}