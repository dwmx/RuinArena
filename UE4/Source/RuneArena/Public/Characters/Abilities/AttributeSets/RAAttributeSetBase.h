#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RAAttributeSetBase.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FPostDamageReceivedSignature, AActor*, const FHitResult&, float);

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class RUNEARENA_API URAAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	using Super = UAttributeSet;

public:
	URAAttributeSetBase();
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Current health, the owner is expected to die when 0. Capped by MaxHealth. */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, Health)

	/** The max value assignable to Health */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, MaxHealth)

	/** Strength attribute, which modifies various player mechanics and triggers bloodlust */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|Strength", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, Strength)

	/** Max value for the strength attribute */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|Strength", ReplicatedUsing = OnRep_MaxStrength)
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, MaxStrength)

	/** Movement speed */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|MovementSpeed", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, MovementSpeed)

	/** Damage, temporary variable for storing the result of RAEC_Damage */
	UPROPERTY(BlueprintReadOnly, Category = "RuneArena|Damage")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URAAttributeSetBase, Damage);

	/** Fired when damage is registered on this attribute set, after it has been deducted from health */
	FPostDamageReceivedSignature OnPostDamageReceived;

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	UFUNCTION()
	virtual void OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength);

	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);
};

#undef ATTRIBUTE_ACCESSORS