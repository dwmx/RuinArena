#pragma once

#include "RAJumpPadRenderingComponent.generated.h"

UCLASS()
class RUNEARENA_API URAJumpPadRenderingComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FVector GameThreadLaunchVelocity;

	UPROPERTY()
	float GameThreadGravityZ;

public:
	URAJumpPadRenderingComponent(const FObjectInitializer& ObjectInitializer);

protected:
	/** UPrimitiveComponent interface */
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override { return true; }

	/** USceneComponent interface */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void TickComponent
	(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;
};