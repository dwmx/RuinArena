#include "Characters/Components/RACharacterSkeletalMeshComponent.h"

static const FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules
(
	EAttachmentRule::SnapToTarget,
	EAttachmentRule::SnapToTarget,
	EAttachmentRule::KeepWorld,
	false
);

static const FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules
(
	EDetachmentRule::KeepWorld,
	EDetachmentRule::KeepWorld,
	EDetachmentRule::KeepWorld,
	false
);


URACharacterSkeletalMeshComponent::URACharacterSkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URACharacterSkeletalMeshComponent::AttachActorToWeaponSocket(AActor* Actor)
{
	if (Actor == nullptr || GetSocketByName(AttachWeaponSocketName) == nullptr)
	{
		return;
	}

	Actor->AttachToComponent(this, AttachmentRules, AttachWeaponSocketName);
}

void URACharacterSkeletalMeshComponent::AttachActorToShieldSocket(AActor* Actor)
{
	if (Actor == nullptr || GetSocketByName(AttachShieldSocketName) == nullptr)
	{
		return;
	}

	Actor->AttachToComponent(this, AttachmentRules, AttachShieldSocketName);
}

void URACharacterSkeletalMeshComponent::AttachActorToStowSocket(AActor* Actor)
{
	if (Actor == nullptr || GetSocketByName(AttachStowSocketName) == nullptr)
	{
		return;
	}

	Actor->AttachToComponent(this, AttachmentRules, AttachStowSocketName);
}

void URACharacterSkeletalMeshComponent::DetachActor(AActor* Actor)
{
	Actor->DetachFromActor(DetachmentRules);
}

void URACharacterSkeletalMeshComponent::OnChildAttached(USceneComponent* ChildComponent)
{
	if (OnChildActorAttached.IsBound())
	{
		OnChildActorAttached.Execute(ChildComponent->GetAttachmentRootActor(), ChildComponent);
	}
}

void URACharacterSkeletalMeshComponent::OnChildDetached(USceneComponent* ChildComponent)
{
	if (OnChildActorDetached.IsBound())
	{
		OnChildActorDetached.Execute(ChildComponent->GetAttachmentRootActor(), ChildComponent);
	}
}