// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RAEnum.h"
#include "Abilities/GameplayAbility.h"
#include "RuneArena.h"
#include "RAGameplayAbility.generated.h"

UCLASS()
class RUNEARENA_API URAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	URAGameplayAbility();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ERAAbilityInputID AbilityInputID = ERAAbilityInputID::None;

protected:
	class ARACharacter* GetOwningCharacterFromActorInfo();

	UFUNCTION(BlueprintCallable)
	ERADirection GetOwnerAccelerationDirectionEnum();
};
