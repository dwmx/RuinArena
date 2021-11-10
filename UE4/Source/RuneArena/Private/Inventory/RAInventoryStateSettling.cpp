#include "Inventory/RAInventoryStateSettling.h"
#include "Inventory/RANewInventory.h"
#include "Inventory/RAInventoryMovementComponent.h"
#include "Inventory/RAInventoryRotatingMovementComponent.h"

URAInventoryStateSettling::URAInventoryStateSettling(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryStateName = FName("InventoryStateSettling");
	bCanBeInteractedWith = true;
}

void URAInventoryStateSettling::BeginState()
{
	Super::BeginState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(Inventory->CollisionVolumeComponent);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(Inventory->CollisionVolumeComponent);
		Inventory->RotatingMovementComponent->SetRotateToDesired(true, FRotator(0.0f, 0.0f, 0.0f), 1.0f);
		Inventory->CollisionVolumeComponent->SetCollisionProfileName(FName("WeaponSettling"));
		Inventory->CollisionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->CollisionVolumeComponent->OnComponentBeginOverlap.AddDynamic(this, &URAInventoryStateSettling::HandleOnCollisionVolumeBeginOverlap);
			Inventory->MovementComponent->OnProjectileBounce.AddDynamic(this, &URAInventoryStateSettling::HandleOnMovementComponentBounce);
			Inventory->MovementComponent->OnProjectileStop.AddDynamic(this, &URAInventoryStateSettling::HandleOnMovementComponentStop);

			ForceInventoryNetUpdate();
		}
	}
}

void URAInventoryStateSettling::EndState()
{
	Super::EndState();

	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr)
	{
		Inventory->MovementComponent->SetUpdatedComponent(nullptr);
		Inventory->RotatingMovementComponent->SetUpdatedComponent(nullptr);
		Inventory->RotatingMovementComponent->SetRotateToDesired(false);
		Inventory->CollisionVolumeComponent->SetCollisionProfileName(FName("WeaponThrown"));
		Inventory->CollisionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (Inventory->GetLocalRole() == ROLE_Authority)
		{
			Inventory->CollisionVolumeComponent->OnComponentBeginOverlap.RemoveAll(this);
			Inventory->MovementComponent->OnProjectileBounce.RemoveAll(this);
			Inventory->MovementComponent->OnProjectileStop.RemoveAll(this);
		}
	}
}

void URAInventoryStateSettling::HandleOnCollisionVolumeBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSwepp,
	const FHitResult& SweepResult
)
{}

void URAInventoryStateSettling::HandleOnMovementComponentBounce(const FHitResult& ImpactHitResult, const FVector& ImpactVelocity)
{}

void URAInventoryStateSettling::HandleOnMovementComponentStop(const FHitResult& ImpactHitResult)
{
	ARANewInventory* Inventory = GetInventory();
	if (Inventory != nullptr && Inventory->GetLocalRole() == ROLE_Authority)
	{
		Inventory->SwitchInventoryState(Inventory->InventoryStateSettled);
	}
}