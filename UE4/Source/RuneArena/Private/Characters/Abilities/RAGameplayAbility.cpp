// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Abilities/RAGameplayAbility.h"
#include "Characters/RACharacter.h"

URAGameplayAbility::URAGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
}

ARACharacter* URAGameplayAbility::GetOwningCharacterFromActorInfo()
{
	AActor* OwningActor = GetOwningActorFromActorInfo();
	if (OwningActor == nullptr)
	{
		return nullptr;
	}

	ARACharacter* OwningCharacter = Cast<ARACharacter>(OwningActor);
	if (OwningCharacter == nullptr)
	{
		return nullptr;
	}

	return OwningCharacter;
}

ERADirection URAGameplayAbility::GetOwnerAccelerationDirectionEnum()
{
	ARACharacter* OwningCharacter = GetOwningCharacterFromActorInfo();
	if (OwningCharacter == nullptr)
	{
		return ERADirection::Neutral;
	}

	return OwningCharacter->GetAccelerationDirectionEnum();
}