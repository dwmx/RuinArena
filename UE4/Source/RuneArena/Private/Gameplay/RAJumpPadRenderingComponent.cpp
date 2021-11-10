#include "Gameplay/RAJumpPadRenderingComponent.h"
#include "Gameplay/RAJumpPad.h"
#include "Engine/CollisionProfile.h"

#if !UE_SERVER

class RUNEARENA_API URAJumpPadRenderingProxy : public FPrimitiveSceneProxy
{
private:
	FVector JumpPadLocation;
	FVector JumpPadTargetLandingLocation;
	FVector JumpPadLaunchVelocity;
	float JumpPadAirTimeSeconds;
	float GravityZ;

public:
	/** Cache jump pad properties */
	URAJumpPadRenderingProxy(const UPrimitiveComponent* InComponent) : FPrimitiveSceneProxy(InComponent)
	{
		ARAJumpPad* JumpPad = Cast<ARAJumpPad>(InComponent->GetOwner());

		if (JumpPad != nullptr)
		{
			JumpPadLocation = JumpPad->GetActorLocation();
			JumpPadTargetLandingLocation = JumpPad->GetTargetLandingLocation();
			JumpPadLaunchVelocity = JumpPad->CalcLaunchVelocity();
			JumpPadAirTimeSeconds = JumpPad->GetAirTimeSeconds();
			if (JumpPad->GetWorld() != nullptr)
			{
				GravityZ = JumpPad->GetWorld()->GetGravityZ();
			}
			else
			{
				GravityZ = 0.0f;
			}
		}
	}

	virtual void GetDynamicMeshElements
	(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector
	) const override
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				static const float LINE_THICKNESS = 20;
				static const int32 NUM_DRAW_LINES = 16;
				//static const float FALL_DAMAGE_VELOCITY

				FVector Start = JumpPadLocation;
				float TimeTick = JumpPadAirTimeSeconds / NUM_DRAW_LINES;

				for (int32 i = 1; i < NUM_DRAW_LINES; ++i)
				{
					float TimeElapsed = TimeTick * i;
					FVector End = JumpPadLocation + (JumpPadLaunchVelocity * TimeElapsed);
					End.Z -= (-GravityZ * FMath::Pow(TimeElapsed, 2)) / 2;

					float Speed = FMath::Clamp(FMath::Abs(Start.Z - End.Z) / TimeTick, 0.0f, 1.0f);
					FColor LineColor = FColor::MakeRedToGreenColorFromScalar(1.0f - Speed);

					PDI->DrawLine(Start, End, LineColor, 0, LINE_THICKNESS);
					Start = End;
				}
			}
		}
	}

	virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this));
	}

	virtual SIZE_T GetTypeHash() const
	{
		return *((uint32*)this);
	}

	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && (IsSelected());// || View->Family->EngineShowFlags.Navigation);
		Result.bDynamicRelevance = true;
		Result.bNormalTranslucency = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}
};

#endif

URAJumpPadRenderingComponent::URAJumpPadRenderingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mobility = EComponentMobility::Stationary;

	BodyInstance.SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	bIsEditorOnly = true;
	bHiddenInGame = true;
	SetGenerateOverlapEvents(false);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	bTickInEditor = true;
}

FPrimitiveSceneProxy* URAJumpPadRenderingComponent::CreateSceneProxy()
{
#if UE_SERVER
	return nullptr;
#else
	return new URAJumpPadRenderingProxy(this);
#endif
}

FBoxSphereBounds URAJumpPadRenderingComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox BoxBounds;

	if (GExitPurge || HasAnyFlags(RF_BeginDestroyed) || GetWorld() == nullptr)
	{
		return FBoxSphereBounds(BoxBounds);
	}

	ARAJumpPad* JumpPad = Cast<ARAJumpPad>(GetOwner());
	if (JumpPad != nullptr)
	{
		FVector JumpPadLocation = JumpPad->GetActorLocation();
		FVector JumpPadTargetLandingLocation = JumpPad->GetTargetLandingLocation();
		FVector JumpPadLaunchVelocity = GameThreadLaunchVelocity;
		float JumpPadAirTimeSeconds = JumpPad->GetAirTimeSeconds();
		float GravityZ = -GameThreadGravityZ;

		BoxBounds += JumpPadLocation;
		BoxBounds += JumpPad->ActorToWorld().TransformPosition(JumpPadTargetLandingLocation);

		if (GameThreadGravityZ != 0.0f)
		{
			float ApexTime = JumpPadLaunchVelocity.Z / GravityZ;
			if (ApexTime > 0.0f && ApexTime < JumpPadAirTimeSeconds)
			{
				FVector Apex = JumpPadLocation + (JumpPadLaunchVelocity * ApexTime);
				Apex.Z -= (GravityZ * FMath::Pow(ApexTime, 2)) / 2;
				BoxBounds += Apex;
			}
		}
	}

	return FBoxSphereBounds(BoxBounds);
}

void URAJumpPadRenderingComponent::TickComponent
(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ARAJumpPad* JumpPad = Cast<ARAJumpPad>(GetOwner());
	if (JumpPad != nullptr)
	{
		GameThreadLaunchVelocity = JumpPad->CalcLaunchVelocity();
		GameThreadGravityZ = JumpPad->GetWorld()->GetGravityZ();
	}
}