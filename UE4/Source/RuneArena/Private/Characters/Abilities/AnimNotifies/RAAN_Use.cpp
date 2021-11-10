#include "Characters/Abilities/AnimNotifies/RAAN_Use.h"

URAAN_Use::URAAN_Use()
{
	NotifyTag = FGameplayTag::RequestGameplayTag("Event.Montage.Interaction.Use");
}