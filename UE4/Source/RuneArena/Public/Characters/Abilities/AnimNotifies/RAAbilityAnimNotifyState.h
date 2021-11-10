#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "RAAbilityAnimNotifyState.generated.h"

/** An AnimNotifyState class which sends event tags directly to an ability system */
UCLASS()
class RUNEARENA_API URAAbilityAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "RuneArena")
	FGameplayTag NotifyBeginTag;

	UPROPERTY(EditAnywhere, Category = "RuneArena")
	FGameplayTag NotifyTickTag;

	UPROPERTY(EditAnywhere, Category = "RuneArena")
	FGameplayTag NotifyEndTag;

	virtual FGameplayEventData GetGameplayEventData();

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};