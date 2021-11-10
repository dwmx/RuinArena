#pragma once

#include "GameFramework/Actor.h"
#include "RAInteractableInterface.h"
#include "RAPickup.generated.h"

/** 
*	Base class for any consumable pickup item (food, runes, etc)
*/
UCLASS(BlueprintType)
class ARAPickup
	: public AActor, public IRAInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
	class UPrimitiveComponent* InteractionVolumeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	class UMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionPriority;

	/** Activation tag corresponding to the ability that performs the pickup's interaction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FGameplayTag InteractionAbilityActivationTag;

	/** The gameplay effect applied to whoever consumes this pickup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSubclassOf<class UGameplayEffect> GameplayEffectClass;

public:
	ARAPickup(const FObjectInitializer& ObjectInitializer);

	/** Start IRAInteractableInterface implementation */
	virtual bool CanBeInteractedWith() override;
	virtual float GetInteractionPriority() const override;
	virtual FGameplayTag GetInteractionAbilityActivationTag() override;
	virtual void PerformInteraction(AActor* InteractionInstigator) override;
	/** End IRAInteractableInterface implementation */
};