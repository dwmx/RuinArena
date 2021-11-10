#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "RAInventoryMovementComponent.generated.h"

UCLASS()
class RUNEARENA_API URAInventoryMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	URAInventoryMovementComponent(const FObjectInitializer& ObjectInitializer);
};