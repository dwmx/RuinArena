#include "GameModes/RATeamInfo.h"
#include "Player/RAPlayerState.h"
#include "Net/UnrealNetwork.h"

ARATeamInfo::ARATeamInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 1.0f;
	TeamIndex = 255;
}

void ARATeamInfo::AddToTeam(AController* Controller)
{
	if (Controller == nullptr)
	{
		return;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr)
	{
		return;
	}

	if (PlayerState->GetTeam() != nullptr)
	{
		RemoveFromTeam(Controller);
	}

	PlayerState->SetTeam(this);
	TeamControllers.Add(Controller);
}

void ARATeamInfo::RemoveFromTeam(class AController* Controller)
{
	if (Controller == nullptr || !TeamControllers.Contains(Controller))
	{
		return;
	}

	TeamControllers.Remove(Controller);

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState != nullptr)
	{
		PlayerState->SetTeam(nullptr);
	}
}

void ARATeamInfo::OnRep_TeamIndex()
{}

void ARATeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ARATeamInfo, TeamIndex, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ARATeamInfo, TeamColor, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ARATeamInfo, TeamName, COND_InitialOnly);
}

const uint8 ARATeamInfo::GetTeamIndex()
{
	return TeamIndex;
}

void ARATeamInfo::SetTeamIndex(uint8 NewTeamIndex)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	TeamIndex = NewTeamIndex;
}

void ARATeamInfo::SetTeamColor(FLinearColor NewTeamColor)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	TeamColor = NewTeamColor;
}

void ARATeamInfo::SetTeamName(FText NewTeamName)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	TeamName = NewTeamName;
}