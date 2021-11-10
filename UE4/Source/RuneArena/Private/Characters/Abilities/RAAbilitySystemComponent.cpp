#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/RACharacter.h"

#include "AbilitySystemGlobals.h"

URAAbilitySystemComponent::URAAbilitySystemComponent()
	:	Super(),
		bCharacterAbilitiesGiven(false),
		bStartupEffectsApplied(false)
{}

URAAbilitySystemComponent* URAAbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent)
{
	return Cast<URAAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

AActor* URAAbilitySystemComponent::GetInteractionActor()
{
	return InteractionActor;
}

void URAAbilitySystemComponent::SetInteractionActor(AActor* Actor)
{
	InteractionActor = Actor;
}

int32 URAAbilitySystemComponent::GetPendingSelectInventoryCode()
{
	return PendingSelectInventoryCode;
}

void URAAbilitySystemComponent::SetPendingSelectInventoryCode(int32 NewPendingSelectInventoryCode)
{
	PendingSelectInventoryCode = NewPendingSelectInventoryCode;
}