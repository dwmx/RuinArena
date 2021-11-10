#include "Gameplay/RAJumpPad.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_EDITORONLY_DATA
#include "Gameplay/RAJumpPadRenderingComponent.h"
#endif

#if WITH_EDITOR
void ARAJumpPad::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ARAJumpPad::CheckForErrors()
{
	Super::CheckForErrors();
}

void ARAJumpPad::PreSave(const class ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);

	if (GIsEditor && !IsTemplate() && !IsRunningCommandlet())
	{
		if (GetWorld() != nullptr)
		{
			AuthoredGravityZ = GetWorld()->GetDefaultGravityZ();
		}
	}
}
#endif

ARAJumpPad::ARAJumpPad(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneRootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
	RootComponent = SceneRootComponent;
	RootComponent->SetShouldUpdatePhysicsVolume(true);

	// Mesh
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	// Trigger box
	TriggerCollisionComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("TriggerCollisionComponent"));
	TriggerCollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
	TriggerCollisionComponent->SetupAttachment(RootComponent);
	TriggerCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ARAJumpPad::HandleTriggerOnComponentBeginOverlap);

#if WITH_EDITORONLY_DATA
	// Rendering component
	JumpPadRenderingComponent = ObjectInitializer.CreateDefaultSubobject<URAJumpPadRenderingComponent>(this, TEXT("JumpPadRenderingComponent"));
	// PostPhysicsComponentTick?
	JumpPadRenderingComponent->SetupAttachment(RootComponent);
#endif

	AirTimeSeconds = 1.0f;
}

const FVector& ARAJumpPad::GetTargetLandingLocation() const
{
	return TargetLandingLocation;
}

const float ARAJumpPad::GetAirTimeSeconds() const
{
	return AirTimeSeconds;
}

FVector ARAJumpPad::CalcLaunchVelocity(const AActor* ActorToLaunch)
{
	if (ActorToLaunch == nullptr)
	{
		ActorToLaunch = this;
	}

	FVector Target = ActorToWorld().TransformPosition(TargetLandingLocation) - ActorToLaunch->GetActorLocation();
	const float GravityZ = GetWorld()->GetDefaultGravityZ();
	if (GravityZ > AuthoredGravityZ)
	{
		Target.Z += GetDefault<ARACharacter>()->GetDefaultHalfHeight();
	}

	float SizeZ = Target.Z / AirTimeSeconds + 0.5f * -GravityZ * AirTimeSeconds;
	float SizeXY = Target.Size2D() / AirTimeSeconds;

	FVector LaunchVelocity = Target.GetSafeNormal2D() * SizeXY + FVector(0.0f, 0.0f, SizeZ);

	const ACharacter* Character = Cast<ACharacter>(ActorToLaunch);
	if (Character != nullptr && Character->GetCharacterMovement() != nullptr && Character->GetCharacterMovement()->GravityScale != 1.0f)
	{
		LaunchVelocity *= FMath::Sqrt(Character->GetCharacterMovement()->GravityScale);
	}

	return LaunchVelocity;
}

void ARAJumpPad::HandleTriggerOnComponentBeginOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (Cast<ARACharacter>(OtherActor) == nullptr)
	{
		return;
	}

	Launch(OtherActor);
}

void ARAJumpPad::Launch(AActor* ActorToLaunch)
{
	if (Cast<ARACharacter>(ActorToLaunch) != nullptr)
	{
		ARACharacter* CharacterToLaunch = Cast<ARACharacter>(ActorToLaunch);
		CharacterToLaunch->LaunchCharacter(CalcLaunchVelocity(CharacterToLaunch), true, true);
	}
}