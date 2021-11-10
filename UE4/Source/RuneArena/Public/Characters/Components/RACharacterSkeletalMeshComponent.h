#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "RACharacterSkeletalMeshComponent.generated.h"

DECLARE_DELEGATE_TwoParams(FOnChildAttachedSignature, class AActor*, class USceneComponent*);
DECLARE_DELEGATE_TwoParams(FOnChildDetachedSignature, class AActor*, class USceneComponent*);

UCLASS()
class RUNEARENA_API URACharacterSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachWeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachShieldSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachStowSocketName;

public:
	URACharacterSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void AttachActorToWeaponSocket(class AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void AttachActorToShieldSocket(class AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void AttachActorToStowSocket(class AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void DetachActor(class AActor* Actor);

public:
	FOnChildAttachedSignature OnChildActorAttached;
	FOnChildDetachedSignature OnChildActorDetached;

protected:
	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;
};