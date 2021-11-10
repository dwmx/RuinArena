#pragma once

#include "GameFramework/Actor.h"
#include "RASpawner.generated.h"

UCLASS()
class RUNEARENA_API ARASpawner : public AActor
{
	GENERATED_BODY()

	friend class URASpawnerState;
	friend class URASpawnerStateInactive;
	friend class URASpawnerStateAsleep;
	friend class URASpawnerStateAwake;

protected:
	class URASpawnerState* SpawnerState;

	/** Spawner is laying dormant and will not wake up until an explicit call to SpawnerWakeUp */
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	class URASpawnerState* SpawnerStateInactive;

	/** Spawner is dormant with a respawn timer running */
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	class URASpawnerState* SpawnerStateAsleep;

	/** Spawner has no timer running, and its actor is available */
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	class URASpawnerState* SpawnerStateAwake;

	void SwitchSpawnerState(class URASpawnerState* NextSpawnerState);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	class USceneComponent* SceneComponent;

	/** Overlaps with character interaction volumes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	class UPrimitiveComponent* InteractionVolumeComponent;

	/** Skeletal mesh display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	class UMeshComponent* MeshComponent;

	/** The inventory class that this spawner spawns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<class ARANewInventory> InventoryToSpawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", Meta = (MakeEditWidget = "true"))
	FTransform SpawnerSpawnTransform;

	virtual void BeginPlay() override;

	/** If true, this spawner will replicate its special effects to clients */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bReplicateSpawnerEffects;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif

	/** Unreliable net multicast used for playing effects on spawner wake up */
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastOnSpawnerWakeUp();

	/** Fired when this spawners wakes up, valid on server and unreliable RPC to clients when bReplicateSpawnerEffects is true */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner", Meta = (DisplayName = "OnSpawnerWakeUp"))
	void K2_OnSpawnerWakeUp();

	/** Unreliable net multicast used for playing effects on spawner sleep */
	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastOnSpawnerSleep();

	/** Fired when this spawners sleeps, valid on server and unreliable RPC to clients when bReplicateSpawnerEffects is true */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner", Meta = (DisplayName = "OnSpawnerSleep"))
	void K2_OnSpawnerSleep();

public:
	ARASpawner(const FObjectInitializer& ObjectInitializer);

	virtual void Reset() override;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	virtual void SpawnerWakeUp();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	virtual void SpawnerDeactivate();
};