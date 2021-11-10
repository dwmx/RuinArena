#include "Characters/Abilities/RAGA_LedgeGrabClimb.h"
#include "Characters/RACharacter.h"
#include "Characters/Components/RACharacterMovementComponent.h"

URAGA_LedgeGrabClimb::URAGA_LedgeGrabClimb()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.LedgeGrabClimb")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.LedgeGrabClimb")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.IgnoreMovementInput")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.IgnoreRotationInput")));
}

void URAGA_LedgeGrabClimb::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		ARACharacter* Character = Cast<ARACharacter>(GetAvatarActorFromActorInfo());
		URACharacterMovementComponent* MovementComponent = nullptr;
		if (Character != nullptr)
		{
			MovementComponent = Cast<URACharacterMovementComponent>(Character->GetMovementComponent());
		}
		if (MovementComponent == nullptr)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		// Get ledge grab vectors
		FVector NewActorLocation;
		FVector LedgeGroundNormal;
		FVector LedgeWallNormal;

		MovementComponent->GetLedgeGrabCandidateVectors(NewActorLocation, LedgeGroundNormal, LedgeWallNormal);

		// TODO: Use ground normal to see if the surface is even walkable or not

		if (!Character->SetActorLocation(NewActorLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("Ledge grab ability failed to place actor in the MovementComponent's candidate position"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		FRotator NewActorRotation = (LedgeWallNormal * -1.0f).Rotation();
		NewActorRotation.Pitch = 0.0f;
		NewActorRotation.Roll = 0.0f;
		Character->SetActorRotation(NewActorRotation);
		MovementComponent->Velocity = FVector(0.0f, 0.0f, 0.0f);
	}

	if (!TryPlayAnimMontage(MontageToPlay))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}