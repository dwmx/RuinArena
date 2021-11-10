#include "Characters/Abilities/AnimNotifies/RAANS_Attack.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/RACharacter.h"

URAANS_Attack::URAANS_Attack()
{
	// Default to the right hand socket
	InventorySocketTag = FGameplayTag::RequestGameplayTag("Skeleton.Inventory.Socket.RightHand");

	// Specific events that the Attack GameplayAbility listens for
	NotifyBeginTag = FGameplayTag::RequestGameplayTag("Event.Montage.Combat.BeginAttack");
	NotifyTickTag = FGameplayTag::RequestGameplayTag("Event.Montage.Combat.TickAttack");
	NotifyEndTag = FGameplayTag::RequestGameplayTag("Event.Montage.Combat.EndAttack");
}

FGameplayEventData URAANS_Attack::GetGameplayEventData()
{
	FGameplayEventData EventData;
	EventData.InstigatorTags.AddTag(InventorySocketTag);
	return EventData;
}