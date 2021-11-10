#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RAEC_Damage.generated.h"

UCLASS()
class RUNEARENA_API URAEC_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	URAEC_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};