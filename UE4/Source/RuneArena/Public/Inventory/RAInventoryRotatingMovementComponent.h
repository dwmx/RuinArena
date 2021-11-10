#pragma once

#include "GameFramework/RotatingMovementComponent.h"
#include "RAInventoryRotatingMovementComponent.generated.h"

UCLASS()
class RUNEARENA_API URAInventoryRotatingMovementComponent : public URotatingMovementComponent
{
	GENERATED_BODY()

protected:
	bool bRotateToDesired;
	FRotator DesiredRotation;
	float RotateToDesiredSpeed;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	URAInventoryRotatingMovementComponent(const FObjectInitializer& ObjectInitializer);

	void SetRotateToDesired(bool bNewRotateToDesired, FRotator OptionalDesiredRotation = FRotator::ZeroRotator, float OptionalRotateToDesiredSpeed = 1.0f);
};