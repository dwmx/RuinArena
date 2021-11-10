// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/RAAnimInstance.h"
#include "Characters/RACharacter.h"
//#include "Pickups/RAWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h" // BreakRotIntoAxes

URAAnimInstance::URAAnimInstance()
:	bFullBodyAnim(false),
	DeathMontage(nullptr),
	PainMontage(nullptr)
{}

void URAAnimInstance::SetFullBodyAnim(bool Value)
{
	bFullBodyAnim = Value;
}

void URAAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

ARACharacter* URAAnimInstance::TryGetRACharacterOwner()
{
	return Cast<ARACharacter>(TryGetPawnOwner());
}

UBlendSpace* URAAnimInstance::GetCurrentBlendSpace()
{
    ARACharacter* CharacterOwner = TryGetRACharacterOwner();
    if (CharacterOwner == nullptr)
    {
        return nullptr;
    }

    return CharacterOwner->GetCurrentLocomotionBlendSpace();
}