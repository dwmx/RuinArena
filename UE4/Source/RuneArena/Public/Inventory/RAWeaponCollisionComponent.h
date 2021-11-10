#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "RAWeaponCollisionComponent.generated.h"

/** Delegate for OnCollision events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionSignature, const FHitResult&, HitResult);

/**
*	Actor component which is responsible for performing all collision detection for Weapon actors.
*	Weapon is responsible for calling EnableCollision, DisableCollision, and TickCollision when appropriate.
*	Weapon must bind to the OnCollision delegate in order to receive collision events.
*/
UCLASS()
class RUNEARENA_API URAWeaponCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	FName BaseSocketName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	FName OffsetSocketName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|Collision")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena")
	int TraceCount;

	/** Saved trace locations from last frame */
	TArray<FVector> TraceLocations;

	/** Actors struck since EnableCollision was called */
	TArray<AActor*> StruckActors;
	bool bStruckWorld;

	/** To be set by Owner using SetSkeletalMesh */
	class USkeletalMeshComponent* SkeletalMesh;

	bool bCollisionEnabled;

	/** Draw visual debug information when true */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|Debug")
	bool bDrawDebugTrace;

	/** Debug color for each trace line */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|Debug")
	FColor DebugTraceLineColor;

	/** Debug color for each trace line after hitting */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|Debug")
	FColor DebugTraceHitLineColor;

	/** Debug life time for each trace line */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "RuneArena|Debug")
	float DebugTraceLineDurationSeconds;

public:
	URAWeaponCollisionComponent();

	/** Must be called by Owner after creation */
	void SetSkeletalMesh(class USkeletalMeshComponent* OwnerSkeletalMesh);

	/** To be called by Owner at the start of an attack */
	void EnableCollision();

	/**
	*	To be called by Owner between attack frames.
	*	This function is used instead of TickComponent so that AnimNotifyState Ticks
	*	can be the driver for performing animation-based collision.
	*/
	void TickCollision(float DeltaSeconds);

	/** To be called by Owner at the end of an attack */
	void DisableCollision();

	/** Owner is expected to bind to this delegate to receive collision events */
	FOnCollisionSignature OnCollision;

protected:
	void GetTraceLocations(OUT TArray<FVector>& OutTraceLocationArray);

	FVector GetBaseSocketLocation();
	FVector GetOffsetSocketLocation();
	bool ActorValidToBeStruck(const AActor* Actor);
};