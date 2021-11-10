#include "Inventory/RAPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

ARAPickup::ARAPickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, FName(TEXT("SceneComponent")));
	RootComponent = SceneComponent;

	InteractionVolumeComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, FName(TEXT("InteractionVolumeComponent")));
	InteractionVolumeComponent->SetupAttachment(RootComponent);
	InteractionVolumeComponent->SetHiddenInGame(true);
	InteractionVolumeComponent->SetCollisionProfileName(FName("Interactable"));

	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(TEXT("MeshComponent")));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	InteractionPriority = 3.0f;
	InteractionAbilityActivationTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Interact.Pickup")));

	bReplicates = true;
}

/** Start IRAInteractableInterface implementation */
bool ARAPickup::CanBeInteractedWith()
{
	return true;
}

float ARAPickup::GetInteractionPriority() const
{
	return InteractionPriority;
}

FGameplayTag ARAPickup::GetInteractionAbilityActivationTag()
{
	return InteractionAbilityActivationTag;
}

void ARAPickup::PerformInteraction(AActor* InteractionInstigator)
{
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(InteractionInstigator);
	if (TargetASI == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (TargetASC == nullptr)
	{
		return;
	}

	if (IsValid(GameplayEffectClass))
	{
		FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
		FGameplayEffectSpec GameplayEffectSpec(GameplayEffectClass.GetDefaultObject(), EffectContextHandle, -1.0f);
		TargetASC->ApplyGameplayEffectSpecToSelf(GameplayEffectSpec);
	}

	Destroy();
}
/** End IRAInteractableInterface implementation */