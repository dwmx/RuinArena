#include "Inventory/RASpawner.h"
#include "Inventory/RASpawnerState.h"
#include "Inventory/RASpawnerStateInactive.h"
#include "Inventory/RASpawnerStateAsleep.h"
#include "Inventory/RASpawnerStateAwake.h"
#include "Inventory/RANewInventory.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ARASpawner::ARASpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpawnerStateInactive = ObjectInitializer.CreateDefaultSubobject<URASpawnerStateInactive>(this, TEXT("SpawnerStateInactive"));
	SpawnerStateAsleep = ObjectInitializer.CreateDefaultSubobject<URASpawnerStateAsleep>(this, TEXT("SpawnerStateAsleep"));
	SpawnerStateAwake = ObjectInitializer.CreateDefaultSubobject<URASpawnerStateAwake>(this, TEXT("SpawnerStateAwake"));
	SpawnerState = nullptr;

	SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	SceneComponent->SetMobility(EComponentMobility::Static);
	RootComponent = SceneComponent;

	InteractionVolumeComponent = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("InteractionVolumeComponent"));
	InteractionVolumeComponent->SetMobility(EComponentMobility::Static);
	InteractionVolumeComponent->SetHiddenInGame(true);
	InteractionVolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionVolumeComponent->SetCollisionProfileName(FName("Interactable"));
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(InteractionVolumeComponent);
	if (Capsule != nullptr)
	{
		Capsule->InitCapsuleSize(64.0f, 75.0f);
	}
	InteractionVolumeComponent->SetupAttachment(RootComponent);

	MeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	FVector SpawnLocation = FVector(0.0f, 0.0f, 80.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FVector SpawnScale = FVector(1.0f);
	SpawnerSpawnTransform = FTransform(SpawnRotation, SpawnLocation, SpawnScale);

	bReplicateSpawnerEffects = true;
	bReplicates = bReplicateSpawnerEffects;
}

#if WITH_EDITOR
void ARASpawner::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ARASpawner, bReplicateSpawnerEffects))
	{
		if (bReplicateSpawnerEffects)
		{
			bReplicates = true;
		}
	}
}
#endif

void ARASpawner::BeginPlay()
{
	Super::BeginPlay();

	SwitchSpawnerState(SpawnerStateAwake);
}

void ARASpawner::SwitchSpawnerState(URASpawnerState* NextSpawnerState)
{
	if (SpawnerState == NextSpawnerState)
	{
		return;
	}

	if (SpawnerState != nullptr)
	{
		SpawnerState->EndState();
	}

	SpawnerState = NextSpawnerState;

	if (SpawnerState != nullptr)
	{
		SpawnerState->BeginState();
	}
}

void ARASpawner::NetMulticastOnSpawnerWakeUp_Implementation()
{
	K2_OnSpawnerWakeUp();
}

void ARASpawner::NetMulticastOnSpawnerSleep_Implementation()
{
	K2_OnSpawnerSleep();
}

void ARASpawner::Reset()
{
	SwitchSpawnerState(SpawnerStateInactive);
	SwitchSpawnerState(SpawnerStateAwake);
}

void ARASpawner::SpawnerWakeUp()
{
	SwitchSpawnerState(SpawnerStateAwake);
}

void ARASpawner::SpawnerDeactivate()
{
	SwitchSpawnerState(SpawnerStateInactive);
}