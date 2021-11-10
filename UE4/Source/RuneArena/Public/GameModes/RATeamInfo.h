#pragma once

//#include "GameFramework/Info.h"
#include "RATeamInfo.generated.h"

UCLASS()
class RUNEARENA_API ARATeamInfo : public AInfo
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Teams")
	TArray<AController*> TeamControllers;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamIndex, Category = "Teams")
	uint8 TeamIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Teams")
	FLinearColor TeamColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Teams")
	FText TeamName;

	UFUNCTION()
	virtual void OnRep_TeamIndex();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	ARATeamInfo(const FObjectInitializer& ObjectInitializer);

	const TArray<AController*>& GetTeamControllers() const { return TeamControllers; }

	virtual void AddToTeam(class AController* Controller);
	virtual void RemoveFromTeam(class AController* Controller);

	const uint8 GetTeamIndex();
	void SetTeamIndex(uint8 NewTeamIndex);
	void SetTeamColor(FLinearColor NewTeamColor);
	void SetTeamName(FText NewTeamName);
};