#pragma once

#include "Inventory/RAInventoryState.h"
#include "RAInventoryStateThrowing.generated.h"

UCLASS()
class URAInventoryStateThrowing : public URAInventoryState
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void HandleOnCollisionVolumeBeginOverlap
	(
		class UPrimitiveComponent* OverlappedComponent,
		class AActor* OtherActor,
		class UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSwepp,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleOnMovementComponentBounce(const FHitResult& ImpactHitResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void HandleOnMovementComponentStop(const FHitResult& ImpactHitResult);

public:
	URAInventoryStateThrowing(const FObjectInitializer& ObjectInitializer);

	virtual void BeginState() override;
	virtual void EndState() override;
};