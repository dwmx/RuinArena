#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "RACameraBoomComponent.generated.h"

UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent), hideCategories = (Mobility))
class RUNEARENA_API URACameraBoomComponent : public USpringArmComponent
{
	GENERATED_BODY()
	using Super = USpringArmComponent;

protected:
	/** The minimum arm length for this CameraBoom */
	UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
	float MinimumArmLength;

	/** The maximum arm length for this CameraBoom */
	UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
	float MaximumArmLength;

	/** The default increment / decrement amount when changing the camera's arm length */
	UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
	float DefaultArmLengthIncrementAmount;

	/** Used for interpolating camera offset */
	float CurrentArmLength;

	/** If true, camera will lag as it readjusts to full arm length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag)
	uint32 bEnableCameraArmLengthLag : 1;

	/** If bEnableCameraArmLengthLag is true, controls how quickly the camera reaches its target arm length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Lag, meta = (editcondition = "bEnableCameraArmLengthLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraArmLengthLagSpeed;

public:
	URACameraBoomComponent(const FObjectInitializer& ObjectInitializer);

	/** Increase length of camera arm */
	UFUNCTION(Category = "Camera", BlueprintCallable)
	void IncrementArmLength(float Amount = 0.0f);

	/** Decreases length of camera arm */
	UFUNCTION(Category = "Camera", BlueprintCallable)
	void DecrementArmLength(float Amount = 0.0f);

protected:
	/** Overridden to send DoArmLengthLag to UpdateDesiredArmLocation */
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Extended version of UpdateDesiredArmLocation, to take in bDoArmLengthLag */
	virtual void UpdateDesiredArmLocationExtended(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, bool bDoArmLengthLag, float DeltaTime);
};