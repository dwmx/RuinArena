#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryRotatingMovementComponent::URAInventoryRotatingMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRotateToDesired = false;
	DesiredRotation = FRotator::ZeroRotator;
	RotateToDesiredSpeed = 1.0f;
}

void URAInventoryRotatingMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (!bRotateToDesired)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	FRotator NewRotation = FMath::QInterpTo(CurrentRotation.Quaternion(), DesiredRotation.Quaternion(), DeltaTime, RotateToDesiredSpeed).Rotator();
	GetOwner()->SetActorRotation(NewRotation);
}

void URAInventoryRotatingMovementComponent::SetRotateToDesired(bool bNewRotateToDesired, FRotator OptionalDesiredRotation, float OptionalRotateToDesiredSpeed)
{
	bRotateToDesired = bNewRotateToDesired;
	DesiredRotation = OptionalDesiredRotation;
	RotateToDesiredSpeed = OptionalRotateToDesiredSpeed;
}