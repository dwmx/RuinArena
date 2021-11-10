#include "Characters/Abilities/AttributeSets/RAAttributeSetBase.h"
#include "Characters/RACharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

URAAttributeSetBase::URAAttributeSetBase()
{}

void URAAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URAAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAAttributeSetBase, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAAttributeSetBase, MaxStrength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URAAttributeSetBase, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void URAAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAAttributeSetBase, Health, OldHealth)
}

void URAAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAAttributeSetBase, MaxHealth, OldMaxHealth);
}

void URAAttributeSetBase::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAAttributeSetBase, Strength, OldStrength);
}

void URAAttributeSetBase::OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAAttributeSetBase, MaxStrength, OldMaxStrength);
}

void URAAttributeSetBase::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URAAttributeSetBase, MovementSpeed, OldMovementSpeed);
}

void URAAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0, GetMaxHealthAttribute().GetNumericValue(this));
		return;
	}

	// Strength
	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.0f, GetMaxStrengthAttribute().GetNumericValue(this));
		return;
	}
}

void URAAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();

	UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
	AActor* SourceActor = nullptr;
	if (SourceASC != nullptr)
	{
		SourceActor = SourceASC->GetAvatarActor();
	}

	// Apply incoming damage to the health attribute
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float IncomingDamage = GetDamage();
		SetDamage(0.0f);

		if (IncomingDamage > 0.0f)
		{
			const float NewHealth = GetHealth() - IncomingDamage;
			SetHealth(FMath::Clamp<float>(NewHealth, 0.0f, GetMaxHealth()));
		}

		// TODO: Use HitResult to play a pain animation?
		FHitResult HitResult;
		if (Context.GetHitResult() != nullptr)
		{
			HitResult = *Context.GetHitResult();
		}

		OnPostDamageReceived.Broadcast(SourceActor, HitResult, IncomingDamage);
	}
	// Health clamp
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	// Strength clamp
	else if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		SetStrength(FMath::Clamp(GetStrength(), 0.0f, GetMaxStrength()));
	}
}