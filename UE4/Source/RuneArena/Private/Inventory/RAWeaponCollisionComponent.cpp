#include "Inventory/RAWeaponCollisionComponent.h"
#include "Inventory/RANewInventory.h"
#include "Characters/RACharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

URAWeaponCollisionComponent::URAWeaponCollisionComponent()
{
	TraceChannel = ETraceTypeQuery::TraceTypeQuery3; // Corresponds to Weapon channel, should find a way to remap these names
	bCollisionEnabled = false;
	TraceCount = 6;
	bDrawDebugTrace = false;
	DebugTraceLineColor = FColor::Blue;
	DebugTraceHitLineColor = FColor::Red;
	DebugTraceLineDurationSeconds = 0.2f;
	bStruckWorld = false;
}

void URAWeaponCollisionComponent::SetSkeletalMesh(USkeletalMeshComponent* OwnerSkeletalMesh)
{
	SkeletalMesh = OwnerSkeletalMesh;
}

void URAWeaponCollisionComponent::EnableCollision()
{
	if (bCollisionEnabled)
	{
		return;
	}

	bCollisionEnabled = true;
	TraceLocations.Empty();
	GetTraceLocations(TraceLocations);
	StruckActors.Empty();
	bStruckWorld = false;
}

void URAWeaponCollisionComponent::TickCollision(float DeltaSeconds)
{
	if (!bCollisionEnabled)
	{
		return;
	}

	// Get the updated trace locations this frame
	TArray<FVector> NewTraceLocations;
	GetTraceLocations(NewTraceLocations);

	// Ignore every Actor along the Ownership hierarchy
	TArray<AActor*> TraceIgnoreActors;
	AActor* ActorOwner = GetOwner();
	while (ActorOwner != nullptr)
	{
		TraceIgnoreActors.Add(ActorOwner);
		ActorOwner = ActorOwner->GetOwner();
	}

	// Ignore the weapon owner's hierarchy
	if (GetOwner() != nullptr && GetOwner()->GetOwner() != nullptr)
	{
		ARACharacter* CharacterOwner = Cast<ARACharacter>(GetOwner()->GetOwner());
		if (CharacterOwner != nullptr)
		{
			for (TRAInventoryIterator<ARANewInventory> It(CharacterOwner); It; ++It)
			{
				TraceIgnoreActors.Add(*It);
			}
		}
	}

	// Perform traces
	for (int i = 0; i < TraceCount; ++i)
	{
		FHitResult OutTraceHit;
		FVector TraceStart = TraceLocations[i];
		FVector TraceEnd = NewTraceLocations[i];

		// Update trace location for next frame
		TraceLocations[i] = NewTraceLocations[i];

		EDrawDebugTrace::Type DrawDebugTraceType = EDrawDebugTrace::None;
		if (bDrawDebugTrace && DebugTraceLineDurationSeconds > 0.0f)
		{
			DrawDebugTraceType = EDrawDebugTrace::ForDuration;
		}

		// Kismet line trace has nice debug drawing built-in
		bool bBlockingHit = UKismetSystemLibrary::LineTraceSingle
		(
			this,
			TraceStart,
			TraceEnd,
			TraceChannel,
			false,
			TraceIgnoreActors,
			DrawDebugTraceType,
			OutTraceHit,
			true,
			DebugTraceLineColor,
			DebugTraceHitLineColor,
			DebugTraceLineDurationSeconds
		);

		if (!bBlockingHit || !ActorValidToBeStruck(OutTraceHit.Actor.Get()))
		{
			continue;
		}

		if (OutTraceHit.Actor.IsValid() && Cast<APawn>(OutTraceHit.Actor.Get()))
		{
			StruckActors.Add(OutTraceHit.Actor.Get());
		}
		else
		{
			bStruckWorld = true;
		}

		OnCollision.Broadcast(OutTraceHit);
	}
}

void URAWeaponCollisionComponent::DisableCollision()
{
	if (!bCollisionEnabled)
	{
		return;
	}

	bCollisionEnabled = false;
	TraceLocations.Empty();
	StruckActors.Empty();
}

void URAWeaponCollisionComponent::GetTraceLocations(OUT TArray<FVector>& OutTraceLocationArray)
{
	FVector BaseSocketLocation = GetBaseSocketLocation();
	FVector OffsetSocketLocation = GetOffsetSocketLocation();
	FVector SweepVector = OffsetSocketLocation - BaseSocketLocation;

	OutTraceLocationArray.Empty();

	for (int i = 0; i < TraceCount; ++i)
	{
		float t = (float)i / (float)(TraceCount - 1);

		OutTraceLocationArray.Push(BaseSocketLocation + (SweepVector * t));
	}
}

FVector URAWeaponCollisionComponent::GetBaseSocketLocation()
{
	if (SkeletalMesh == nullptr || !SkeletalMesh->DoesSocketExist(BaseSocketName))
	{
		return FVector(0.0f);
	}

	return SkeletalMesh->GetSocketLocation(BaseSocketName);
}

FVector URAWeaponCollisionComponent::GetOffsetSocketLocation()
{
	if (SkeletalMesh == nullptr || !SkeletalMesh->DoesSocketExist(OffsetSocketName))
	{
		return FVector(0.0f);
	}

	return SkeletalMesh->GetSocketLocation(OffsetSocketName);
}

bool URAWeaponCollisionComponent::ActorValidToBeStruck(const AActor* Actor)
{
	// Line trace is responsible for adding owner to the ignored actors array
	// Anything other than Pawns counts as environment
	if (Actor == nullptr || Cast<APawn>(Actor) == nullptr)
	{
		return !bStruckWorld;
	}

	if (StruckActors.Contains(Actor))
	{
		return false;
	}

	return true;
}