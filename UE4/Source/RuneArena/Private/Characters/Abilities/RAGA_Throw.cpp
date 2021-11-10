#include "Characters/Abilities/RAGA_Throw.h"
#include "Characters/RACharacter.h"
#include "Characters/RACharacterEquipSetAsset.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAWeapon.h"
#include "Kismet/KismetMathLibrary.h"

URAGA_Throw::URAGA_Throw()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Throw")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Throw")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat")));

	bPerformedThrow = false;
}

void URAGA_Throw::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ARACharacter* Character = nullptr;
	if (ActorInfo->AvatarActor.IsValid())
	{
		Character = Cast<ARACharacter>(ActorInfo->AvatarActor.Get());
	}

	if (Character == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bPerformedThrow = false;

	URACharacterEquipSetAsset* EquipSet = Character->GetCurrentEquipSetAsset();
	UAnimMontage* MontageToPlay = nullptr;
	if (EquipSet != nullptr)
	{
		MontageToPlay = EquipSet->Throw;
	}
	
	if (!TryPlayAnimMontage(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void URAGA_Throw::EndAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	// Call here just in case the montage never sent the throw event
	PerformThrow();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URAGA_Throw::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	if (EventTag == FGameplayTag::RequestGameplayTag("Event.Montage.Combat.Throw"))
	{
		PerformThrow();
	}
}

FVector URAGA_Throw::CalcThrowReleaseLocation_Implementation(AActor* ActorToThrow, const FVector& ViewRotationForward)
{
	return ActorToThrow->GetActorLocation();
}

FVector URAGA_Throw::CalcThrowReleaseVelocity_Implementation(AActor* ActorToThrow, const FVector& ViewRotationForward)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		return FVector(0.0f);
	}

	float ActorMass = 20.0f;
	float ThrowMagnitudeX = 1024.0f * (20.0f / ActorMass);

	return ViewRotationForward * ThrowMagnitudeX;
}

FRotator URAGA_Throw::CalcThrowReleaseRotation_Implementation(AActor* ActorToThrow, const FVector& ViewRotationForward)
{
	FVector ViewX, ViewY, ViewZ;
	FRotator ViewRotationForwardRotator = ViewRotationForward.Rotation();
	UKismetMathLibrary::GetAxes(ViewRotationForwardRotator, ViewX, ViewY, ViewZ);
	return (-1.0f * ViewY).Rotation();
}

FRotator URAGA_Throw::CalcThrowReleaseRotationRate_Implementation(AActor* ActorToThrow, const FVector& ViewRotationForward)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		return FRotator(0.0f);
	}

	float ActorMass = 20.0f;
	float RotationRatePitch = 360.0f * (20.0f / ActorMass);

	return FRotator(RotationRatePitch, 0.0f, 0.0f);
}

void URAGA_Throw::PerformThrow()
{
	if (bPerformedThrow)
	{
		return;
	}
	bPerformedThrow = true;

	ARACharacter* RACharacter = nullptr;
	if (GetCurrentActorInfo() != nullptr)
	{
		if (GetCurrentActorInfo()->AvatarActor.IsValid())
		{
			RACharacter = Cast<ARACharacter>(GetCurrentActorInfo()->AvatarActor.Get());
		}
	}

	if (RACharacter == nullptr)
	{
		return;
	}

	ARANewInventory* Weapon = RACharacter->GetWeapon();
	if (Weapon == nullptr)
	{
		return;
	}

	RACharacter->ReleaseInventory(Weapon, ERACharacterInventoryReleasePolicy::ReleaseAndForget);

	// Throw weapon
	FVector ViewForward = RACharacter->GetViewRotation().Vector();
	FRotator ViewForwardRotator = ViewForward.Rotation();

	FVector ReleaseLocation = CalcThrowReleaseLocation(Weapon, ViewForward);
	FVector ReleaseVelocity = CalcThrowReleaseVelocity(Weapon, ViewForward);
	FRotator ReleaseRotation = CalcThrowReleaseRotation(Weapon, ViewForward);
	FRotator ReleaseRotationRate = CalcThrowReleaseRotationRate(Weapon, ViewForward);

	Weapon->SetInstigator(Cast<APawn>(GetAvatarActorFromActorInfo()));
	Weapon->ThrowFrom(ReleaseLocation, ReleaseRotation, ReleaseVelocity, ReleaseRotationRate);

	// Apply gameplay effect
	ARAWeapon* RAWeapon = Cast<ARAWeapon>(Weapon);
	if (RAWeapon != nullptr)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(ThrowDamageGameplayEffectClass, GetAbilityLevel());
		EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), RAWeapon->GetDamage());
		RAWeapon->SetThrowGameplayEffect(EffectSpecHandle);
	}
}