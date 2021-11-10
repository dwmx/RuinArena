#include "Inventory/RAInventoryStateThrowing.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateThrowing::URAInventoryStateThrowing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateThrowing");
	bCanBeInteractedWith = false;
}

void URAInventoryStateThrowing::BeginState()
{
	Super::BeginState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(Inventory->CollisionVolumeComponent);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(Inventory->CollisionVolumeComponent);
		Inventory->CollisionVolumeComponent->SetCollisionProfileName(FName("WeaponThrown"));
		Inventory->CollisionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		if (Inventory->GetInstigator() != nullptr)
		{
			// Don't collide with the throw
			Inventory->CollisionVolumeComponent->MoveIgnoreActors.Add(Inventory->GetInstigator());
		}

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->CollisionVolumeComponent->OnComponentBeginOverlap.AddDynamic(this, &URAInventoryStateThrowing::HandleOnCollisionVolumeBeginOverlap);
			Inventory->MovementComponent->OnProjectileBounce.AddDynamic(this, &URAInventoryStateThrowing::HandleOnMovementComponentBounce);
			Inventory->MovementComponent->OnProjectileStop.AddDynamic(this, &URAInventoryStateThrowing::HandleOnMovementComponentStop);

			ForceInventoryNetUpdate();
		}

		Inventory->K2_OnInventoryThrowStart();
	}
}

void URAInventoryStateThrowing::EndState()
{
	Super::EndState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(nullptr);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(nullptr);
		Inventory->CollisionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Inventory->GetInstigator() != nullptr)
		{
			Inventory->CollisionVolumeComponent->MoveIgnoreActors.Remove(Inventory->GetInstigator());
		}

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->CollisionVolumeComponent->OnComponentBeginOverlap.RemoveAll(this);
			Inventory->MovementComponent->OnProjectileBounce.RemoveAll(this);
			Inventory->MovementComponent->OnProjectileStop.RemoveAll(this);
		}

		Inventory->K2_OnInventoryThrowEnd();
	}
}

void URAInventoryStateThrowing::HandleOnCollisionVolumeBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSwepp,
	const FHitResult& SweepResult
)
{}

void URAInventoryStateThrowing::HandleOnMovementComponentBounce(const FHitResult& ImpactHitResult, const FVector& ImpactVelocity)
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr && Inventory->GetLocalRole() == ROLE_Authority)
	{
		Inventory->NotifyThrowCollision(ImpactHitResult, ImpactVelocity);
		Inventory->SwitchInventoryState(Inventory->InventoryStateSettling);
	}
}

void URAInventoryStateThrowing::HandleOnMovementComponentStop(const FHitResult& ImpactHitResult)
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr && Inventory->GetLocalRole() == ROLE_Authority)
	{
		FVector ZeroVec = FVector(0.0f, 0.0f, 0.0f);
		Inventory->NotifyThrowCollision(ImpactHitResult, ZeroVec);
		Inventory->SwitchInventoryState(Inventory->InventoryStateSettling);
	}
}