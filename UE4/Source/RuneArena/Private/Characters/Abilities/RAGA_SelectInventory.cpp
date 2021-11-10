#include "Characters/Abilities/RAGA_SelectInventory.h"
#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAWeapon.h"

URAGA_SelectInventory::URAGA_SelectInventory()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact.SelectInventory")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact.SelectInventory")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Interact")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.LedgeGrabClimb")));
}

bool URAGA_SelectInventory::CanActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags
) const
{
	ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		return false;
	}

	URAAbilitySystemComponent* SourceASC = nullptr;
	if (Cast<IAbilitySystemInterface>(Character) != nullptr)
	{
		SourceASC = Cast<URAAbilitySystemComponent>(Cast<IAbilitySystemInterface>(Character)->GetAbilitySystemComponent());
	}
	if (SourceASC == nullptr)
	{
		return false;
	}

	if (SourceASC->GetPendingSelectInventoryCode() == 0)
	{
		// Player wants to stow, so something must be equipped
		if (Character->GetWeapon() == nullptr)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		// Player wants to switch weapons, so make sure there's something available
		for (TRAInventoryIterator<ARANewInventory> It(Character); It; ++It)
		{
			if (*It != Character->GetWeapon())
			{
				return true;
			}
		}
		return false;
	}
}

void URAGA_SelectInventory::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	URAAbilitySystemComponent* SourceASC = nullptr;
	if (Cast<IAbilitySystemInterface>(Character) != nullptr)
	{
		SourceASC = Cast<URAAbilitySystemComponent>(Cast<IAbilitySystemInterface>(Character)->GetAbilitySystemComponent());
	}
	if (SourceASC == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	SelectInventoryCode = SourceASC->GetPendingSelectInventoryCode();
	

	bPerformedSelectInventory = false;

	if (!TryPlayAnimMontage(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void URAGA_SelectInventory::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	// Ensure the function is performed even if something went wrong with animation
	if (!bPerformedSelectInventory)
	{
		PerformSelectInventory();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URAGA_SelectInventory::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	if (EventTag == MontageEventTagToListenFor)
	{
		PerformSelectInventory();
	}
}

void URAGA_SelectInventory::PerformSelectInventory()
{
	bPerformedSelectInventory = true;

	// Inventory selection only happens server side
	if (HasAuthority(&CurrentActivationInfo))
	{
		ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
		if (Character == nullptr)
		{
			return;
		}

		switch (SelectInventoryCode)
		{
		case 0:
			PerformStow();
			break;
		case 1:
			PerformSelectNextWeapon();
			break;
		default:
			PerformStow();
			break;
		}
	}
}

void URAGA_SelectInventory::PerformStow()
{
	// Inventory selection only happens server side
	if (HasAuthority(&CurrentActivationInfo))
	{
		ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
		if (Character == nullptr)
		{
			return;
		}

		ARANewInventory* Weapon = Character->GetWeapon();
		if (Weapon != nullptr)
		{
			Character->StowInventory(Weapon);
		}
	}
}

void URAGA_SelectInventory::PerformSelectNextWeapon()
{
	// Inventory selection only happens server side
	if (HasAuthority(&CurrentActivationInfo))
	{
		ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
		if (Character == nullptr)
		{
			return;
		}

		ARANewInventory* Weapon = Character->GetWeapon();

		// Select first inventory if nothing is equipped
		if (Weapon == nullptr)
		{
			TRAInventoryIterator<ARAWeapon> It(Character);
			if (It.IsValid())
			{
				Character->EquipInventory(*It);
			}
			return;
		}

		// Get the first inventory after weapon
		TRAInventoryIterator<ARAWeapon> It(Character);
		while (It && *It != Weapon)
		{
			++It;
		}

		if (It.IsValid() && *It == Weapon)
		{
			++It;
		}

		if (!It.IsValid())
		{
			It = TRAInventoryIterator<ARAWeapon>(Character);
		}

		Character->StowInventory(Weapon);
		Character->EquipInventory(*It);
	}
}