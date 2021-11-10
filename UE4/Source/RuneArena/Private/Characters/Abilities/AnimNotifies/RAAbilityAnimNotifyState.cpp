#include "Characters/Abilities/AnimNotifies/RAAbilityAnimNotifyState.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

FGameplayEventData URAAbilityAnimNotifyState::GetGameplayEventData()
{
	return FGameplayEventData();
}

void URAAbilityAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NotifyBeginTag, GetGameplayEventData());
}

void URAAbilityAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NotifyTickTag, GetGameplayEventData());
}

void URAAbilityAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NotifyEndTag, GetGameplayEventData());
}