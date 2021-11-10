#include "Characters/Abilities/RAGA_Attack.h"
#include "Characters/Abilities/AbilityTasks/RAAT_PlayMontageAndWaitForEvent.h"
#include "Characters/RACharacter.h"
#include "Inventory/RAWeapon.h"
#include "AbilitySystemComponent.h"

URAGA_Attack::URAGA_Attack()
{
	AbilityInputID = ERAAbilityInputID::Attack;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat")));

	DamageIncreaseByStrengthCoefficient = 0.1f;
	StrengthGainCoefficient = 0.2f;
}

void URAGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RACharacter = nullptr;
	RAWeapon = nullptr;

	// Get character
	ARACharacter* RACharacterTemp = nullptr;
	if (ActorInfo->AvatarActor.IsValid())
	{
		RACharacterTemp = Cast<ARACharacter>(ActorInfo->AvatarActor.Get());
	}
	
	if (RACharacterTemp == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get weapon
	ARAWeapon* RAWeaponTemp = Cast<ARAWeapon>(RACharacterTemp->GetWeapon());
	if (RAWeaponTemp == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cache character and weapon
	RACharacter = RACharacterTemp;
	RAWeapon = RAWeaponTemp;

	// Grab the attack chain and start the attack
	RACharacter->GetCurrentAttackChain(&AttackChain, &RecoveryMontages);

	// Only the server listens for collisions
	if (HasAuthority(&CurrentActivationInfo))
	{
		RAWeapon->OnWeaponStruckActor.AddDynamic(this, &URAGA_Attack::HandleOnWeaponStruckActor);
	}

	AttackChainIndex = 0;
	bPendingAttack = false;
	bRecovering = false;

	bAttackActive = false;

	if (!TryPerformAttack())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void URAGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// If this point was reached and the attack is still active, then the montage failed to send an EndAttack event
	if (bAttackActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack ability was ended before the EndAttack event was received. This indicates an AnimNotifyState issue."));

		if (RAWeapon != nullptr)
		{
			RAWeapon->OnEndAttack();
		}
		OnEndAttack();
		bAttackActive = false;
	}

	// Remove Weapon events
	if (RAWeapon != nullptr)
	{
		RAWeapon->OnWeaponStruckActor.RemoveAll(this);
	}
}

void URAGA_Attack::RepeatActivateAbility()
{
	bPendingAttack = true;
}

bool URAGA_Attack::TryPerformAttack()
{
	UAnimMontage* MontageToPlay = nullptr;
	if (AttackChainIndex >= 0 && AttackChainIndex < AttackChain.Num())
	{
		MontageToPlay = AttackChain[AttackChainIndex];
	}

	if (MontageToPlay == nullptr)
	{
		return false;
	}

	bPendingAttack = false;
	AttackChainIndex++;

	return TryPlayAnimMontage(MontageToPlay);
}

bool URAGA_Attack::TryPerformRecovery()
{
	UAnimMontage* MontageToPlay = nullptr;
	int32 RecoveryIndex = AttackChainIndex - 1;

	if (RecoveryIndex >= 0 && RecoveryIndex < RecoveryMontages.Num())
	{
		MontageToPlay = RecoveryMontages[RecoveryIndex];
	}

	if (MontageToPlay == nullptr)
	{
		return false;
	}

	bPendingAttack = false;
	bRecovering = true;

	return TryPlayAnimMontage(MontageToPlay);
}

void URAGA_Attack::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// Try to combo attack
	if (!bRecovering && bPendingAttack && TryPerformAttack())
	{
		return;
	}

	// No combo, try to recover
	if (!bRecovering && TryPerformRecovery())
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URAGA_Attack::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// Listen for specific events sent from AbilitySystemAnimNotifies (RAANS_Attack)
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.Combat.TickAttack")))
	{
		HandleOnTickAttack(0.0f);
		return;
	}
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.Combat.BeginAttack")))
	{
		HandleOnBeginAttack();
		return;
	}
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.Combat.EndAttack")))
	{
		HandleOnEndAttack();
		return;
	}

	// Listen for combo attack events (repeat inputs)
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Attack.Combo")))
	{
		RepeatActivateAbility();
		return;
	}
}

void URAGA_Attack::HandleOnBeginAttack()
{
	DoBeginAttack();
}

void URAGA_Attack::HandleOnTickAttack(float DeltaSeconds)
{
	DoTickAttack(DeltaSeconds);
}

void URAGA_Attack::HandleOnEndAttack()
{
	DoEndAttack();
}

void URAGA_Attack::DoBeginAttack()
{
	if (!HasAuthority(&CurrentActivationInfo) || RACharacter == nullptr || RAWeapon == nullptr || bAttackActive)
	{
		return;
	}

	RAWeapon->OnBeginAttack();

	// Apply BeginAttack gameplay effect to owning ASC
	if (IsValid(BeginAttackEffectClass) && RAWeapon != nullptr)
	{
		FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), CurrentActorInfo);
		EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), RAWeapon);

		FGameplayEffectSpec GameplayEffectSpec(BeginAttackEffectClass.GetDefaultObject(), EffectContextHandle);

		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToSelf(GameplayEffectSpec);
	}

	OnBeginAttack();
	bAttackActive = true;
}

void URAGA_Attack::DoTickAttack(float DeltaSeconds)
{
	if (!HasAuthority(&CurrentActivationInfo) || RACharacter == nullptr || RAWeapon == nullptr || !bAttackActive)
	{
		return;
	}

	RAWeapon->OnTickAttack(DeltaSeconds);
	OnTickAttack(DeltaSeconds);
}

void URAGA_Attack::DoEndAttack()
{
	if (!HasAuthority(&CurrentActivationInfo) || RACharacter == nullptr || RAWeapon == nullptr || !bAttackActive)
	{
		return;
	}

	RAWeapon->OnEndAttack();
	OnEndAttack();
	bAttackActive = false;
}

void URAGA_Attack::HandleOnWeaponStruckActor(AActor* StruckActor, const FHitResult& HitResult)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// Set Instigator on the struck Actor for things like score tracking
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn != nullptr && StruckActor != nullptr)
	{
		StruckActor->SetInstigator(AvatarPawn);
	}

	// Gameplay effect to apply to the target
	FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), CurrentActorInfo);
	EffectContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), RAWeapon);
	EffectContextHandle.AddHitResult(HitResult);

	// Apply damage gameplay effect to any actors with an ASC
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(StruckActor);
	if (AbilitySystemInterface != nullptr)
	{
		UAbilitySystemComponent* TargetASC = AbilitySystemInterface->GetAbilitySystemComponent();
		if (TargetASC != nullptr && IsValid(DamageGameplayEffectClass) && RAWeapon != nullptr)
		{
			// Only strike the player if they can be struck
			// If you try to strike the player after they're already dead, this seems to cause problems with death ability
			FGameplayTagContainer HitBlockGameplayTags;
			HitBlockGameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Combat.Die")));
			HitBlockGameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
			if (!TargetASC->HasAnyMatchingGameplayTags(HitBlockGameplayTags))
			{
				// Damage ExecCalc uses these to determine target damage and source strength boost
				FGameplayEffectSpec GameplayEffectSpec(DamageGameplayEffectClass.GetDefaultObject(), EffectContextHandle);
				GameplayEffectSpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), RAWeapon->GetDamage());
				GameplayEffectSpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.DamageIncreaseByStrengthCoefficient"), DamageIncreaseByStrengthCoefficient);
				GameplayEffectSpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.StrengthGainCoefficient"), StrengthGainCoefficient);

				TargetASC->ApplyGameplayEffectSpecToSelf(GameplayEffectSpec);
			}
		}
	}
	/// If the struct Actor has no ASC, then we can't apply the GE. But we still want to apply any Cues, so spawn them manually
	//lse
	//
	//	if (IsValid(DamageGameplayEffectClass))
	//	{
	//		FGameplayCueParameters GameplayCueParameters;
	//		GameplayCueParameters.EffectContext = EffectContextHandle;
	//
	//		UGameplayEffect* GameplayEffect = Cast<UGameplayEffect>(DamageGameplayEffectClass.GetDefaultObject());
	//		for (auto& It : GameplayEffect->GameplayCues)
	//		{
	//			for (auto& ItTag : It.GameplayCueTags)
	//			{
	//				GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(ItTag, GameplayCueParameters);
	//			}
	//		}
	//	}
	//

	// If the weapon has an effect, apply it to owner
	if (RAWeapon != nullptr)
	{
		TSubclassOf<UGameplayEffect> WeaponGEClass = RAWeapon->GetOnWeaponStruckActorGameplayEffectClass();
		if (IsValid(WeaponGEClass))
		{
			IAbilitySystemInterface* SourceASI = Cast<IAbilitySystemInterface>(GetOwningActorFromActorInfo());
			if (SourceASI != nullptr)
			{
				UAbilitySystemComponent* SourceASC = SourceASI->GetAbilitySystemComponent();
				if (SourceASC != nullptr)
				{
					FGameplayEffectSpec GameplayEffectSpec(WeaponGEClass.GetDefaultObject(), EffectContextHandle);
					SourceASC->ApplyGameplayEffectSpecToSelf(GameplayEffectSpec);
				}
			}
		}
	}

	// Allow the blueprint to add further logic
	OnWeaponStructActor(StruckActor, HitResult);
}