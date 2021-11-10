#pragma once

#include "CoreMinimal.h"
#include "Inventory/RANewInventory.h"
#include "RAWeapon.generated.h"

/** Delegates for notifying when this weapon strikes an actor */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponStruckActorSignature, AActor*, StruckActor, const FHitResult&, HitResult);

UCLASS()
class RUNEARENA_API ARAWeapon : public ARANewInventory
{
	GENERATED_BODY()

public:
	FWeaponStruckActorSignature OnWeaponStruckActor;

protected:
	/** The component responsible for performing collision checks during attacks */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Weapon")
	class URAWeaponCollisionComponent* WeaponCollisionComponent;

	/** Event handler for weapon collision component. This will be called server side only. */
	UFUNCTION()
	void HandleWeaponCollisionComponentOnCollision(const FHitResult& HitResult);

	/** The gameplay effect which will be applied to this weapon's owner on attack */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> OnWeaponStruckActorGameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage;

public:	
	/** Default properties */
	ARAWeapon(const FObjectInitializer& ObjectInitializer);

	/** Received from attack ability */
	void OnBeginAttack();
	void OnTickAttack(float DeltaSeconds);
	void OnEndAttack();

	float GetDamage() { return Damage; }
	TSubclassOf<class UGameplayEffect> GetOnWeaponStruckActorGameplayEffectClass() { return OnWeaponStruckActorGameplayEffectClass; }
};
