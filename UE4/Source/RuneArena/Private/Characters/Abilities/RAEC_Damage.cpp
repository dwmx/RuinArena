#include "Characters/Abilities/RAEC_Damage.h"
#include "Characters/Abilities/AttributeSets/RAAttributeSetBase.h"
#include "GameModes/RAGameModeBase.h"
#include "Player/RAPlayerController.h"
#include "AbilitySystemComponent.h"

struct RADamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	RADamageStatics()
	{
		// Capture the source's strength and damage with snapshot
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAAttributeSetBase, Strength, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URAAttributeSetBase, Damage, Source, true);
	}
};

static const RADamageStatics& DamageStatics()
{
	static RADamageStatics DamageStatics;
	return DamageStatics;
}

URAEC_Damage::URAEC_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
}

void URAEC_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;

	// Grab base damage from the Data.Damage gameplay tag
	float BaseDamage = 0.0f;
	BaseDamage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), false, 0.0);
	BaseDamage = FMath::Max<float>(BaseDamage, 0.0);

	// Use source's strength to add a damage boost
	float StrengthDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().StrengthDef, EvaluationParams, StrengthDamage);
	StrengthDamage = StrengthDamage * Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.DamageIncreaseByStrengthCoefficient"), false, 0.0f);

	// Aggregate damages
	float AggregatedDamage = 0.0f;
	if (BaseDamage > 0.0f)
	{
		AggregatedDamage += BaseDamage;
	}

	// Apply strength separately so that it doesn't feed back into strength gain
	float AggregatedDamageWithStrength = AggregatedDamage;
	if (StrengthDamage > 0.0f)
	{
		AggregatedDamageWithStrength += StrengthDamage;
	}

	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	APawn* SourcePawn = nullptr;
	if (SourceASC != nullptr)
	{
		SourcePawn = Cast<APawn>(SourceASC->GetAvatarActor());
	}
	APawn* TargetPawn = nullptr;
	if (TargetASC != nullptr)
	{
		TargetPawn = Cast<APawn>(TargetASC->GetAvatarActor());
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(SourceASC);
	ARAGameModeBase* GameMode = World->GetAuthGameMode<ARAGameModeBase>();

	// Friendly fire damage check
	bool bFriendlyFire = false;
	if (GameMode != nullptr && GameMode->CheckIsTeamGame())
	{
		if (SourceASC != nullptr && TargetASC != nullptr)
		{
			if (SourcePawn != nullptr && TargetPawn != nullptr)
			{
				ARAPlayerController* SourceController = SourcePawn->GetController<ARAPlayerController>();
				ARAPlayerController* TargetController = TargetPawn->GetController<ARAPlayerController>();

				if (SourceController != nullptr && TargetController != nullptr)
				{
					uint8 SourceTeamIndex = SourceController->GetTeamIndex();
					uint8 TargetTeamIndex = TargetController->GetTeamIndex();

					if (SourceTeamIndex == TargetTeamIndex)
					{
						bFriendlyFire = true;
						float FriendlyFireMultiplier = GameMode->GetFriendlyFireMultiplier();
						AggregatedDamage = AggregatedDamage * FriendlyFireMultiplier;
						AggregatedDamageWithStrength = AggregatedDamageWithStrength * FriendlyFireMultiplier;
					}
				}
			}
		}
	}

	// Allow game mode to modify the resulting damage
	if (GameMode != nullptr)
	{
		AggregatedDamage = GameMode->CalculateDesiredDamageModification(TargetPawn, SourcePawn, AggregatedDamage);
		AggregatedDamageWithStrength = GameMode->CalculateDesiredDamageModification(TargetPawn, SourcePawn, AggregatedDamageWithStrength);
	}

	// Damage is applied to the Damage attribute on the Target's attribute set
	// AttributeSetBase will then subtract the Damage attribute value from the Health attribute value in PostGameplayEffectExecute
	if (AggregatedDamageWithStrength > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, AggregatedDamageWithStrength));
	}

	// Apply a GE to the Source based on how much base damage it did to the Target
	if (SourceASC != nullptr)
	{
		UGameplayEffect* InstigatorGE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("DamageInstigatorGE")));
		if (InstigatorGE != nullptr)
		{
			int32 ModifierIndex = InstigatorGE->Modifiers.Num();
			InstigatorGE->Modifiers.SetNum(ModifierIndex + 1);

			// Give the source a strength buff based on damage dealt
			float DamageToStrengthCoefficient = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.StrengthGainCoefficient"), false, 0.0f);

			FGameplayModifierInfo& StrengthModifier = InstigatorGE->Modifiers[ModifierIndex];

			float StrengthModifierMagnitude = AggregatedDamage * DamageToStrengthCoefficient;
			if (bFriendlyFire)
			{
				StrengthModifierMagnitude = 0.0f; // No shenanigans
			}

			StrengthModifier.ModifierMagnitude = FScalableFloat(StrengthModifierMagnitude);
			StrengthModifier.ModifierOp = EGameplayModOp::Additive;
			StrengthModifier.Attribute = URAAttributeSetBase::GetStrengthAttribute();

			SourceASC->ApplyGameplayEffectToSelf(InstigatorGE, 1.0f, SourceASC->MakeEffectContext());
		}
	}
}