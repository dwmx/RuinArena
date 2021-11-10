#include "GameModes/Arena/RAArenaMatchState.h"
#include "GameModes/Arena/RAArena.h"
#include "GameModes/Arena/RAArenaMatch.h"
#include "GameModes/Arena/RAArenaPlayerState.h"
#include "Net/UnrealNetwork.h"

ARAArenaMatchState::ARAArenaMatchState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 1.0f;

	ArenaMatchStateName = EArenaMatchStateName::ArenaInactive;
	OldArenaMatchStateName = ArenaMatchStateName;
	ArenaMatchStateTimeLimitSeconds = 0.0f;
}

void ARAArenaMatchState::OnRep_ArenaMatchStateName()
{
	if (OnArenaMatchStateStateChanged.IsBound())
	{
		OnArenaMatchStateStateChanged.Broadcast(OldArenaMatchStateName, ArenaMatchStateName);
	}
	OldArenaMatchStateName = ArenaMatchStateName;
}

void ARAArenaMatchState::SetArenaMatchStateName(FName NewArenaMatchStateName)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	OldArenaMatchStateName = ArenaMatchStateName;
	ArenaMatchStateName = NewArenaMatchStateName;
	if (OnArenaMatchStateStateChanged.IsBound())
	{
		OnArenaMatchStateStateChanged.Broadcast(OldArenaMatchStateName, ArenaMatchStateName);
	}
}

void ARAArenaMatchState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARAArenaMatchState, ArenaMatchStateName);
	DOREPLIFETIME(ARAArenaMatchState, ArenaMatchTeamSize);
}

void ARAArenaMatchState::NetMulticast_SetArenaMatchStateTimer_Implementation(float TimeLimitSeconds, float TimeRemainingSeconds)
{
	ArenaMatchStateTimeLimitSeconds = TimeLimitSeconds;

	ArenaMatchStateTimeStampSeconds = GetWorld()->GetTimeSeconds();
}

void ARAArenaMatchState::SetArenaMatchStateTimer(float TimeLimitSeconds, float TimeRemainingSeconds)
{
	NetMulticast_SetArenaMatchStateTimer(TimeLimitSeconds, TimeRemainingSeconds);
}

float ARAArenaMatchState::GetArenaMatchStateTimeRemainingSeconds()
{
	float DeltaSeconds = GetWorld()->GetTimeSeconds() - ArenaMatchStateTimeStampSeconds;
	float TimeRemainingSeconds = ArenaMatchStateTimeLimitSeconds - DeltaSeconds;
	return TimeRemainingSeconds;
}

void ARAArenaMatchState::SetArenaMatchTeamSize(int32 NewArenaMatchTeamSize)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	OldArenaMatchTeamSize = ArenaMatchTeamSize;
	ArenaMatchTeamSize = NewArenaMatchTeamSize;
	if (OnArenaMatchTeamSizeChanged.IsBound())
	{
		OnArenaMatchTeamSizeChanged.Broadcast(OldArenaMatchTeamSize, ArenaMatchTeamSize);
	}
}

void ARAArenaMatchState::OnRep_ArenaMatchTeamSize()
{
	if (OnArenaMatchTeamSizeChanged.IsBound())
	{
		OnArenaMatchTeamSizeChanged.Broadcast(OldArenaMatchTeamSize, ArenaMatchTeamSize);
	}
	OldArenaMatchTeamSize = ArenaMatchTeamSize;
}

void ARAArenaMatchState::UpdateArenaMatchPlayers(const TArray<APlayerState*>& NewArenaMatchPlayers)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	NetMulticast_UpdateArenaMatchPlayers(NewArenaMatchPlayers);
}

void ARAArenaMatchState::NetMulticast_UpdateArenaMatchPlayers_Implementation(const TArray<APlayerState*>& NewArenaMatchPlayers)
{
	ArenaMatchPlayers.Empty();
	for (auto It : NewArenaMatchPlayers)
	{
		ArenaMatchPlayers.Add(It);
		ARAArenaPlayerState* ArenaPlayerState = Cast<ARAArenaPlayerState>(It);
		if (ArenaPlayerState != nullptr)
		{
			ArenaPlayerState->SetArenaMatchState(this);
		}
	}
}