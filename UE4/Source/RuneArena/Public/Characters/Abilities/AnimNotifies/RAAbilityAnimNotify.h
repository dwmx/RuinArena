#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "RAAbilityAnimNotify.generated.h"

/** An AnimNotify class which sends event tags directly to an ability system */
UCLASS()
class RUNEARENA_API URAAbilityAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag NotifyTag;

protected:
	virtual void Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation) override;
};