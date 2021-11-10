#pragma once

#include "CoreMinimal.h"
#include "RAJumpPad.generated.h"

UCLASS()
class RUNEARENA_API ARAJumpPad : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JumpPad")
	class USceneComponent* SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JumpPad")
	class UStaticMeshComponent* MeshComponent;

	/** Triggering collision volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JumpPad")
	class UPrimitiveComponent* TriggerCollisionComponent;

	/** The location this jump pad launches to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (MakeEditWidget = ""))
	FVector TargetLandingLocation;

	/** Number of seconds it takes to go from jump pad to landing location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPad", meta = (ClampMin = "0.1"))
	float AirTimeSeconds;

	/** For modified gravity gametypes */
	UPROPERTY()
	float AuthoredGravityZ;

#if WITH_EDITORONLY_DATA
	/** Editor rendering component for the jump pad */
	UPROPERTY()
	class URAJumpPadRenderingComponent* JumpPadRenderingComponent;
#endif

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void CheckForErrors() override;
	virtual void PreSave(const class ITargetPlatform* TargetPlatform) override;

protected:
#endif

public:
	ARAJumpPad(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	const FVector& GetTargetLandingLocation() const;

	UFUNCTION(BlueprintCallable)
	const float GetAirTimeSeconds() const;

	/** Use ActorToLaunch's location to determine launch velocity, or generic launch velocity if none provided */
	UFUNCTION(BlueprintCallable, Category = "JumpPad")
	FVector CalcLaunchVelocity(const AActor* ActorToLaunch = nullptr);

protected:
	/** Handler for trigger collider's overlap event */
	UFUNCTION()
	void HandleTriggerOnComponentBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** Launch the ActorToLaunch */
	void Launch(AActor* ActorToLaunch);
};