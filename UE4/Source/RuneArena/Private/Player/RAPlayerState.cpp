#include "Player/RAPlayerState.h"
#include "Characters/RACharacter.h"
#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/RAAttributeSetBase.h"
#include "GameModes/RAGameModeBase.h"
#include "GameModes/RATeamInfo.h"
#include "Net/UnrealNetwork.h"

ARAPlayerState::ARAPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsRconAdmin = false;
	Team = nullptr;
	Frags = 0;
	Deaths = 0;
}

void ARAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARAPlayerState, bIsRconAdmin);
	DOREPLIFETIME(ARAPlayerState, Team);
	DOREPLIFETIME(ARAPlayerState, Frags);
	DOREPLIFETIME(ARAPlayerState, Deaths);
}

void ARAPlayerState::Reset()
{
	Super::Reset();
	Deaths = 0.0f;
}

void ARAPlayerState::SetIsRconAdmin(bool bNewIsRconAdmin)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	bIsRconAdmin = bNewIsRconAdmin;
}

void ARAPlayerState::OnRep_Team()
{
}

void ARAPlayerState::SetTeam(class ARATeamInfo* NewTeam)
{
	Team = NewTeam;

	ARACharacter* Character = GetPawn<ARACharacter>();
	if (Character != nullptr)
	{
		Character->SetOwnerTeamInfo(NewTeam);
	}

	K2_NotifyTeamChanged();
}

ARATeamInfo* ARAPlayerState::GetTeam()
{
	return Team;
}

const uint8 ARAPlayerState::GetTeamIndex()
{
	if (Team == nullptr)
	{
		return 255;
	}
	return Team->GetTeamIndex();
}

void ARAPlayerState::OnRep_Deaths()
{
}

float ARAPlayerState::GetDeaths() const
{
	return Deaths;
}

void ARAPlayerState::SetDeaths(const float NewDeaths)
{
	Deaths = NewDeaths;
}

void ARAPlayerState::OnRep_Frags()
{}

float ARAPlayerState::GetFrags() const
{
	return Frags;
}

void ARAPlayerState::SetFrags(const float NewFrags)
{
	Frags = NewFrags;
}

void ARAPlayerState::IncrementDeaths(float OptionalAmount)
{
	SetDeaths(GetDeaths() + OptionalAmount);
}

void ARAPlayerState::IncrementScore(float OptionalAmount)
{
	SetScore(GetScore() + OptionalAmount);
}

void ARAPlayerState::IncrementFrags(float OptionalAmount)
{
	SetFrags(GetFrags() + OptionalAmount);
}

void ARAPlayerState::Server_RequestChangeTeam_Implementation(uint8 NewTeamIndex)
{
	ARAGameModeBase* Game = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
	if (Game == nullptr || !Game->CheckIsTeamGame() || !Game->GetAllowPlayerTeamChangeRequests())
	{
		return;
	}

	AController* Controller = Cast<AController>(GetOwner());
	if (Controller != nullptr)
	{
		if (Game->ChangeTeam(Controller, NewTeamIndex, true))
		{
			// TODO: Any successful team change stuff
		}
		else
		{
			// TODO: Any unsuccessful team change stuff
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("Attemping team change to: %d"), NewTeamIndex);
}

bool ARAPlayerState::Server_RequestChangeTeam_Validate(uint8 NewTeamIndex)
{
	return true;
}

void ARAPlayerState::NetMulticastSetRespawnTimeLimitSeconds_Implementation(float NewRespawnTimeLimitSeconds)
{
	RespawnTimeLimitSeconds = NewRespawnTimeLimitSeconds;
	RespawnTimeStampSeconds = GetWorld()->GetTimeSeconds();
}

void ARAPlayerState::SetRespawnTimeRemainingSeconds(float NewRespawnTimeRemainingSeconds)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	NetMulticastSetRespawnTimeLimitSeconds(NewRespawnTimeRemainingSeconds);
}

float ARAPlayerState::GetRespawnTimeRemainingSeconds()
{
	float DeltaSeconds = GetWorld()->GetTimeSeconds() - RespawnTimeStampSeconds;
	float TimeRemainingSeconds = RespawnTimeLimitSeconds - DeltaSeconds;
	return TimeRemainingSeconds;
}