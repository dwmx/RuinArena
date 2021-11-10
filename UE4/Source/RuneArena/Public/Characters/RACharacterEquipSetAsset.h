#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Engine/DataAsset.h"
#include "RACharacterEquipSetAsset.generated.h"

class UBlendSpace;
class UAnimMontage;

USTRUCT()
struct RUNEARENA_API FCombatAnim
{
	GENERATED_USTRUCT_BODY()

	FCombatAnim();

	UPROPERTY(EditAnywhere)
	UAnimMontage* AttackAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* RecoverAnimMontage;
};

USTRUCT()
struct RUNEARENA_API FCombatAnimSet
{
	GENERATED_USTRUCT_BODY()

	FCombatAnimSet();

	UPROPERTY(EditAnywhere)
	TArray<FCombatAnim> Neutral;

	UPROPERTY(EditAnywhere)
	TArray<FCombatAnim> Front;

	UPROPERTY(EditAnywhere)
	TArray<FCombatAnim> Back;

	UPROPERTY(EditAnywhere)
	TArray<FCombatAnim> Left;

	UPROPERTY(EditAnywhere)
	TArray<FCombatAnim> Right;
};

USTRUCT()
struct RUNEARENA_API FCombatAnimStateSet
{
	GENERATED_USTRUCT_BODY()

	FCombatAnimStateSet();

	UPROPERTY(EditAnywhere)
	FCombatAnimSet Neutral;

	UPROPERTY(EditAnywhere)
	FCombatAnimSet Falling;

	UPROPERTY(EditAnywhere)
	FCombatAnimSet Dodging;

	UPROPERTY(EditAnywhere)
	FCombatAnimSet Crouching;
};

UCLASS()
class RUNEARENA_API URACharacterEquipSetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

	friend class ARACharacter;

protected:
	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Locomotion")
	UBlendSpace* Locomotion_Standing;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Locomotion")
	UBlendSpace* Locomotion_Crouching;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Locomotion")
	UAnimMontage* Locomotion_Jump;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_RightHandHigh;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_RightHandMiddle;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_RightHandLow;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_LeftHandHigh;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_LeftHandMiddle;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PickUp_LeftHandLow;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Use_Lever;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Use_Pump;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* PowerUp;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Taunt;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Stow_RightHandRightHip;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Stow_RightHandLeftHip;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Interaction")
	UAnimMontage* Stow_RightHandBack;

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Combat")
	FCombatAnimStateSet CombatSet;

public:
	URACharacterEquipSetAsset();

	UPROPERTY(EditAnywhere, Category = "RuneArena|Animation|Combat")
	UAnimMontage* Throw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuneArena|Sound")
	const class USoundBase* WeaponSwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuneArena|Sound")
	const class USoundBase* WeaponImpactFleshSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuneArena|Sound")
	const class USoundBase* WeaponImpactDirtSound;
};