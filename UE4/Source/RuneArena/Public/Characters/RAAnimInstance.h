// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "Containers/Queue.h"
#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "RAEnum.h"
#include "RAAnimInstance.generated.h"

class UBlendSpace;
class UAnimMontage;
class ARACharacter;

//	URAAnimInstance
UCLASS()
class RUNEARENA_API URAAnimInstance :	public UAnimInstance
{
    GENERATED_BODY()
    using Super = UAnimInstance;

protected:
    UPROPERTY(BlueprintReadOnly)
    ARACharacter* RACharacterOwner;

	UPROPERTY(BlueprintReadOnly)
	bool bFullBodyAnim;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* PainMontage;

	virtual void NativeBeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "RuneArena")
	class ARACharacter* TryGetRACharacterOwner();

	UFUNCTION(BlueprintCallable, Category = "RuneArena|Animation")
	class UBlendSpace* GetCurrentBlendSpace();

public:
	UFUNCTION(BlueprintCallable)
	void SetFullBodyAnim(bool Value);

public:
	URAAnimInstance();
};
