#pragma once

#include "Player/RAPlayerController.h"
#include "RAArenaPlayerController.generated.h"

UCLASS()
class RUNEARENA_API ARAArenaPlayerController : public ARAPlayerController
{
	GENERATED_BODY()

protected:
	/** Saves player's inventory from one match to the next, only valid on server */
	UPROPERTY(BlueprintReadOnly, Category = "ArenaGame")
	TArray<TSubclassOf<class ARANewInventory>> SavedInventoryClasses;

	TSubclassOf<class ARANewInventory> SavedWeaponClass;

public:
	ARAArenaPlayerController(const FObjectInitializer& ObjectInitializer);

	/** Saves a snapshot of all inventory classes owned by this controller's character */
	void SaveInventoryClasses();

	/** Clear out any previously saved inventory classes */
	void ClearSavedInventoryClasses();

	/** Get a copy array of all saved inventory classes on this controller */
	void GetSavedInventoryClassesCopy(TArray<TSubclassOf<class ARANewInventory>>& OutSavedInventoryClasses);

	TSubclassOf<class ARANewInventory> GetSavedWeaponClass() { return SavedWeaponClass; }
};