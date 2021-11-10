// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Abilities/AnimNotifies/RAAbilityAnimNotify.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"


void URAAbilityAnimNotify::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NotifyTag, FGameplayEventData());
}