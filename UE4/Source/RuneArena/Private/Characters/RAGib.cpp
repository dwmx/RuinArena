#include "Characters/RAGib.h"
#include "Components/DecalComponent.h"

ARAGib::ARAGib(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, FName(TEXT("MeshComponent")));
	MeshComponent->bReceivesDecals = false;
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = MeshComponent;

	LifeTimeSeconds = 5.0f;
	bReplicates = false;
}

void ARAGib::BeginPlay()
{
	Super::BeginPlay();

	if (!IsPendingKillPending())
	{
		MeshComponent->OnComponentHit.AddDynamic(this, &ARAGib::HandleMeshComponentHit);
		MeshComponent->SetSimulatePhysics(true);

		FTimerHandle TempTimerHandle;
		GetWorldTimerManager().SetTimer(TempTimerHandle, this, &ARAGib::HandleLifeTimeTimer, LifeTimeSeconds, false);
	}
}

void ARAGib::HandleMeshComponentHit
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& HitResult
)
{
	// Spawn blood decal
	//	This is being performed in blueprints now, but this is how you'd do it in CPP
	// 
	//if(HitResult.Component != nullptr && HitResult.Component->bReceivesDecals && bSpawnBloodDecals && BloodDecals.Num() > 0)
	//{
	//	const FRABloodDecalData& BloodDecalData = BloodDecals[FMath::RandHelper(BloodDecals.Num())];
	//	if (BloodDecalData.Material != nullptr)
	//	{
	//		static FName NAME_BloodDecal(TEXT("BloodDecal"));
	//		FHitResult BloodDecalHit;
	//		FVector TraceStart = GetActorLocation();
	//		FVector TraceEnd = GetActorLocation() - HitResult.Normal * 200.0f;
	//		FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams(NAME_BloodDecal, false, this);
	//
	//		if (GetWorld()->LineTraceSingleByChannel(BloodDecalHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionQueryParams))
	//		{
	//			UDecalComponent* BloodDecal = NewObject<UDecalComponent>(GetWorld());
	//			if (BloodDecal != nullptr)
	//			{
	//				if (HitResult.Component.IsValid() && HitResult.Component->Mobility == EComponentMobility::Movable)
	//				{
	//					BloodDecal->SetAbsolute(false, false, false);
	//					BloodDecal->AttachToComponent(HitResult.Component.Get(), FAttachmentTransformRules::KeepRelativeTransform);
	//				}
	//				else
	//				{
	//					BloodDecal->SetAbsolute(true, true, true);
	//				}
	//
	//				FVector2D AppliedBloodDecalScale = FMath::Lerp<FVector2D>(BloodDecalData.DecalScaleMin, BloodDecalData.DecalScaleMax, FMath::FRand());
	//				BloodDecal->DecalSize = FVector(1.0f, AppliedBloodDecalScale.X, AppliedBloodDecalScale.Y);
	//				BloodDecal->SetWorldLocation(BloodDecalHit.Location);
	//				BloodDecal->SetWorldRotation((-BloodDecalHit.Normal).Rotation() + FRotator(0.0f, 0.0f, 360.0f * FMath::FRand()));
	//				BloodDecal->SetDecalMaterial(BloodDecalData.Material);
	//				BloodDecal->RegisterComponentWithWorld(GetWorld());
	//			}
	//		}
	//	}
	//}

	K2_OnMeshComponentHit(OverlappedComponent, OtherActor, OtherComponent, NormalImpulse, HitResult);
}

void ARAGib::HandleLifeTimeTimer()
{
	Destroy();
}