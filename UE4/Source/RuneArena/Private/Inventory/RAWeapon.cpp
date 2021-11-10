#include "Inventory/RAWeapon.h"
#include "Inventory/RAWeaponCollisionComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ARAWeapon::ARAWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WeaponCollisionComponent = ObjectInitializer.CreateDefaultSubobject<URAWeaponCollisionComponent>(this, TEXT("WeaponCollisionComponent"));
	WeaponCollisionComponent->SetSkeletalMesh(Cast<USkeletalMeshComponent>(SkeletalMeshComponent));
	WeaponCollisionComponent->OnCollision.AddDynamic(this, &ARAWeapon::HandleWeaponCollisionComponentOnCollision);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//InventoryType = ERAInventoryType::Primary;
	bReplicates = true;
	Damage = 10.0f;
}

void ARAWeapon::HandleWeaponCollisionComponentOnCollision(const FHitResult& HitResult)
{
	if (OnWeaponStruckActor.IsBound())
	{
		OnWeaponStruckActor.Broadcast(HitResult.Actor.Get(), HitResult);
	}
}

void ARAWeapon::OnBeginAttack()
{
	WeaponCollisionComponent->EnableCollision();
}

void ARAWeapon::OnTickAttack(float DeltaSeconds)
{
	WeaponCollisionComponent->TickCollision(DeltaSeconds);
}

void ARAWeapon::OnEndAttack()
{
	WeaponCollisionComponent->DisableCollision();
}