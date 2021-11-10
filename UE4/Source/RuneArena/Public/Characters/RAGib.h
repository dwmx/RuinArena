#pragma once

#include "GameFramework/Actor.h"
#include "RAGib.generated.h"

// Decal info for blood splattering
// May want to move this elsewhere later
USTRUCT(BlueprintType)
struct FRABloodDecalData
{
	GENERATED_USTRUCT_BODY()

	/** Material for this blood decal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BloodDecalData")
	class UMaterialInterface* Material;

	/** XY minimum scaling of this decal on applied surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BloodDecalData")
	FVector2D DecalScaleMin;

	/** XY maximum scaling of this decal on applied surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BloodDecalData")
	FVector2D DecalScaleMax;

	FRABloodDecalData()
		: Material(nullptr), DecalScaleMin(FVector(0.9f, 0.9f, 0.9f)), DecalScaleMax(FVector(1.1f, 1.1f, 1.1f))
	{}
};

UCLASS(BlueprintType, Abstract, NotPlaceable)
class RUNEARENA_API ARAGib : public AActor
{
	GENERATED_BODY()

protected:
	/** How long until this gib self destructs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gib")
	float LifeTimeSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gib")
	class UMeshComponent* MeshComponent;

	///** Particle systems spawned and attached on actor creation */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gib")
	//TArray<TSubclassOf<class UParticleSystem>> GibEffectsOnSpawn;
	//
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gib")
	//TArray<TSubclassOf<class UParticleSystem>> GibEffectsOnCollision;

	/** Array of blood decals which will randomly be applied to surfaces on collision */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gib")
	TArray<FRABloodDecalData> BloodDecals;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void HandleMeshComponentHit
	(
		class UPrimitiveComponent* OverlappedComponent,
		class AActor* OtherActor,
		class UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& HitResult
	);

	/** Fired when gib mesh collides with anything */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gib", Meta = (DisplayName = "OnMeshComponentHit"))
	void K2_OnMeshComponentHit
	(
		class UPrimitiveComponent* OverlappedComponent,
		class AActor* OtherActor,
		class UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& HitResult
	);

	virtual void HandleLifeTimeTimer();

public:
	ARAGib(const FObjectInitializer& ObjectInitializer);
};