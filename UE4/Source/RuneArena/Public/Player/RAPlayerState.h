#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "RAPlayerState.generated.h"

UCLASS()
class RUNEARENA_API ARAPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	bool bIsRconAdmin;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "PlayerState")
	class ARATeamInfo* Team;

	UFUNCTION()
	void OnRep_Team();

	UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
	float RespawnTimeLimitSeconds;
	float RespawnTimeStampSeconds;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastSetRespawnTimeLimitSeconds(float NewRespawnTimeLimitSeconds);

	/** Total frag count for this player */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Frags, Category = "PlayerState|Scoring")
	float Frags;

	UFUNCTION()
	virtual void OnRep_Frags();

	/** Total death count for this player */
	UPROPERTY(ReplicatedUsing = OnRep_Deaths, BlueprintReadOnly, Category = "PlayerState|Scoring", Meta = (AllowPrivateAccess = "true"))
	float Deaths;

	UFUNCTION()
	virtual void OnRep_Deaths();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	ARAPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void Reset() override;

	virtual bool GetIsRconAdmin() { return bIsRconAdmin; }
	virtual void SetIsRconAdmin(bool bNewIsRconAdmin);

	void SetTeam(class ARATeamInfo* NewTeam);
	class ARATeamInfo* GetTeam();
	const uint8 GetTeamIndex();

	UFUNCTION(BlueprintImplementableEvent, Category = "Teams")
	void K2_NotifyTeamChanged();

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	float GetDeaths() const;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void SetDeaths(const float NewDeaths);

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	float GetFrags() const;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void SetFrags(const float NewFrags);

	/** Increment death count by 1, or by some other specified amount */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void IncrementDeaths(float OptionalAmount = 1.0f);

	/** Increment frag count by 1, or by some other specified amount */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void IncrementFrags(float OptionalAmount = 1.0f);

	/** Increment score count by 1, or by some other specified amount */
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void IncrementScore(float OptionalAmount = 1.0f);

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Server_RequestChangeTeam(uint8 NewTeamIndex);

	void SetRespawnTimeRemainingSeconds(float NewRespawnTimeRemainingSeconds);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	virtual float GetRespawnTimeRemainingSeconds();
};