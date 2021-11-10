// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "RAInteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URAInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *	Interface for all Actors which can receive special interactions from the "Use" action.
 *	Examples include picking up weapons or food, pulling a lever, relighting a torch, (pulling the head off a corpse?).
 *	GameplayAbilities can then be specifically made and taylored to the Actor performing the interaction.
 *	GetInteractionAbilityActivationTags() is responsible for returning the GameplayTags associated with this interactable.
 */
class RUNEARENA_API IRAInteractableInterface
{
	GENERATED_BODY()

public:
	/** To return true if this Interactable can receive an interaction, false otherwise */
	virtual bool CanBeInteractedWith() = 0;

	virtual float GetInteractionPriority() const = 0;

	/** To return the ability activation tag that correspond to the ability responsible for interacting with this actor */
	virtual FGameplayTag GetInteractionAbilityActivationTag() = 0;

	/**
	*	Perform the interaction logic which is relevant to this Actor, and optionally return an Actor for things like pickups
	*/
	virtual void PerformInteraction(AActor* InteractionInstigator) = 0;
};
