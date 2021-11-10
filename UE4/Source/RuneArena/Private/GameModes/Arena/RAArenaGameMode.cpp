#include "GameModes/Arena/RAArenaGameMode.h"
#include "GameModes/Arena/RAArena.h"
#include "GameModes/Arena/RAArenaQueueVolume.h"
#include "GameModes/Arena/RAArenaPlayerController.h"
#include "GameModes/Arena/RAArenaPlayerState.h"
#include "GameModes/Arena/RAArenaPlayerStart.h"
#include "GameModes/Arena/RAArenaGameState.h"
#include "GameModes/Arena/RAArenaTeamInfo.h"
#include "GameModes/Arena/RAArenaMatch.h"
#include "GameModes/Arena/RAArenaMatchState.h"
//#include "Player/Messaging/RAGameMessage.h"
#include "GameModes/Arena/RAArenaGameMessage.h"
#include "Player/RAPlayerController.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"

ARAArenaGameMode::ARAArenaGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ARAArenaGameState::StaticClass();
	ArenaMatchClass = ARAArenaMatch::StaticClass();
	TeamClass = ARAArenaTeamInfo::StaticClass();
	PlayerStateClass = ARAArenaPlayerState::StaticClass();
	PlayerControllerClass = ARAArenaPlayerController::StaticClass();
	NumTeams = 3; // Champions, challengers, spectators
	bTeamGame = true;
	bAllowPlayerTeamChangeRequests = false;
}

void ARAArenaGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Figure out initial team size
	for (TActorIterator<ARAArenaQueueVolume> QueueVolumeIt(GetWorld()); QueueVolumeIt; ++QueueVolumeIt)
	{
		QueueVolumeIt->OnActorBeginOverlap.AddDynamic(this, &ARAArenaGameMode::OnQueueZoneOverlapBegin);
		QueueVolumeIt->OnActorEndOverlap.AddDynamic(this, &ARAArenaGameMode::OnQueueZoneOverlapEnd);
	}

	MaximumLevelSupportedArenaTeamSize = CalculateMaximumLevelSupportedArenaTeamSize();
	if (MaximumLevelSupportedArenaTeamSize <= 0)
	{
		UE_LOG(LogArena, Warning, TEXT("Maximum level supported team size is 0. Arena zone must contain Champions and Challengers player starts"));
	}

	// Spawn arena match actor
	if (ArenaMatchClass == nullptr)
	{
		ArenaMatchClass = ARAArenaMatch::StaticClass();
		UE_LOG(LogArena, Warning, TEXT("No arena match class set, reverting to default"));
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	ArenaMatch = GetWorld()->SpawnActor<ARAArenaMatch>(ArenaMatchClass, SpawnInfo);
	ArenaMatch->OnArenaMatchStateChanged.AddDynamic(this, &ARAArenaGameMode::HandleOnArenaMatchStateChanged);
	ArenaMatch->OnArenaMatchTeamSizeChanged.AddDynamic(this, &ARAArenaGameMode::HandleOnArenaMatchTeamSizeChanged);
	ArenaMatch->OnArenaMatchTimeLimitChanged.AddDynamic(this, &ARAArenaGameMode::HandleOnArenaMatchTimeLimitChanged);
	ArenaMatch->EnableArenaMatch();

	ARAArenaGameState* ArenaGameState = GetGameState<ARAArenaGameState>();
	if (ArenaGameState != nullptr)
	{
		ArenaGameState->ArenaMatchState = ArenaMatch->GetArenaMatchState();
	}
}

void ARAArenaGameMode::Killed(AController* KillerController, AController* VictimController, APawn* VictimPawn)
{
	Super::Killed(KillerController, VictimController, VictimPawn);

	ArenaMatch->Killed(KillerController, VictimController, VictimPawn);
}

void ARAArenaGameMode::HandleOnArenaMatchStateChanged(ARAArenaMatch* InArenaMatch, FName OldArenaMatchStateName, FName NewArenaMatchStateName)
{
	if (OldArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
	{
		// Eject all losers from the match and restart them, but winners stay
		TArray<AController*> ArenaMatchLoserControllers;
		InArenaMatch->GetArenaMatchLoserControllersCopy(ArenaMatchLoserControllers);
		InArenaMatch->RemoveControllersFromArenaMatch(ArenaMatchLoserControllers);
	}
	
	if (NewArenaMatchStateName == EArenaMatchStateName::ArenaIdle)
	{
		CheckAndTryToStartArenaMatch();
	}
}

void ARAArenaGameMode::HandleOnArenaMatchTeamSizeChanged(ARAArenaMatch* InArenaMatch, int32 NewArenaMatchTeamSize)
{
	if (InArenaMatch->CheckIsAvailableToStartNewArenaMatch())
	{
		CheckAndTryToStartArenaMatch();
	}

	// Broadcast message
	int32 Switch = URAArenaGameMessage::MakeSwitch(URAArenaGameMessage::Index::ArenaMatchTeamSize, NewArenaMatchTeamSize);
	ARAArenaMatchState* InArenaMatchState = InArenaMatch->GetArenaMatchState();
	BroadcastLocalized(this, URAArenaGameMessage::StaticClass(), Switch, nullptr, nullptr, InArenaMatchState);
}

void ARAArenaGameMode::HandleOnArenaMatchTimeLimitChanged(ARAArenaMatch* InArenaMatch, float NewArenaMatchTimeLimitSeconds)
{
	// Broadcast message
	int32 NewArenaMatchTimeLimitMinutes = FMath::TruncToInt(NewArenaMatchTimeLimitSeconds / 60.0f);
	int32 Switch = URAArenaGameMessage::MakeSwitch(URAArenaGameMessage::Index::ArenaMatchTimeLimit, NewArenaMatchTimeLimitMinutes);
	ARAArenaMatchState* InArenaMatchState = InArenaMatch->GetArenaMatchState();
	BroadcastLocalized(this, URAArenaGameMessage::StaticClass(), Switch, nullptr, nullptr, InArenaMatchState);
}

// TODO: Offload to arena match
// right now, this isn't doing anything
int32 ARAArenaGameMode::CalculateMaximumLevelSupportedArenaTeamSize()
{
	int32 NumChallengerStarts = 0;
	int32 NumChampionsStarts = 0;

	for (TActorIterator<ARAArenaPlayerStart> It(GetWorld()); It; ++It)
	{
		switch (It->GetTeamIndex())
		{
			case 0: ++NumChallengerStarts; break;
			case 1: ++NumChampionsStarts; break;
		}
	}

	int32 Result = FMath::Min(NumChallengerStarts, NumChampionsStarts);
	return Result;
}

void ARAArenaGameMode::StartMatch()
{
	Super::StartMatch();

	//SetArenaMatchState(ArenaMatchState::ArenaIdle);
}

void ARAArenaGameMode::EndMatch()
{
	//SetArenaMatchState(ArenaMatchState::ArenaInactive);

	Super::EndMatch();
}

bool ARAArenaGameMode::CheckControllerCanEnterQueue(AController* Controller)
{
	if (Controller == nullptr || ControllerQueue.Contains(Controller))
	{
		return false;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr)
	{
		return false;
	}

	// Non-fighter players are placed on spectator team, only they can enter queue
	ARATeamInfo* PlayerTeam = PlayerState->GetTeam();
	if (PlayerTeam != nullptr && PlayerTeam->GetTeamIndex() != EArenaTeamIndex::TeamIndexSpectators)
	{
		return false;
	}

	return true;
}

void ARAArenaGameMode::OnQueueZoneOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn == nullptr)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!CheckControllerCanEnterQueue(Controller))
	{
		return;
	}

	AddControllerToQueue(Controller);

	K2_OnQueueZoneOverlapBegin(OverlappedActor, OtherActor);
}

void ARAArenaGameMode::AddControllerToQueue(AController* Controller)
{
	if (ControllerQueue.Contains(Controller))
	{
		return;
	}

	ControllerQueue.Add(Controller);
	UpdateArenaGameStatePlayerQueue();

	K2_PostAddControllerToQueue(Controller);

	// See if the match can start
	CheckAndTryToStartArenaMatch();
}

void ARAArenaGameMode::UpdateArenaGameStatePlayerQueue()
{
	ARAArenaGameState* ArenaGameState = GetGameState<ARAArenaGameState>();
	if (ArenaGameState == nullptr)
	{
		return;
	}

	TArray<ARAPlayerState*> ArenaPlayerQueue;
	for (auto It : ControllerQueue)
	{
		ARAPlayerState* PlayerState = It->GetPlayerState<ARAPlayerState>();
		if (PlayerState == nullptr)
		{
			continue;
		}

		ArenaPlayerQueue.Add(PlayerState);
	}

	ArenaGameState->UpdateArenaPlayerQueue(ArenaPlayerQueue);
}

void ARAArenaGameMode::OnQueueZoneOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn == nullptr)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (Controller == nullptr)
	{
		return;
	}

	RemoveControllerFromQueue(Controller);

	K2_OnQueueZoneOverlapEnd(OverlappedActor, OtherActor);
}

void ARAArenaGameMode::RemoveControllerFromQueue(AController* Controller)
{
	if (!ControllerQueue.Contains(Controller))
	{
		return;
	}

	ControllerQueue.Remove(Controller);
	UpdateArenaGameStatePlayerQueue();

	if (ArenaMatch->ContainsPendingController(Controller))
	{
		ArenaMatch->AbortArenaMatch();
	}

	K2_PostRemoveControllerFromQueue(Controller);
}

void ARAArenaGameMode::CheckAndTryToStartArenaMatch()
{
	if (ArenaMatch == nullptr || !ArenaMatch->CheckIsAvailableToStartNewArenaMatch())
	{
		return;
	}

	int32 NumRequiredPlayers = ArenaMatch->CalculateAdditionalPlayersRequiredToStart();
	if (ControllerQueue.Num() < NumRequiredPlayers)
	{
		return;
	}

	TArray<AController*> ControllersToAdd;
	for (int32 i = 0; i < NumRequiredPlayers; ++i)
	{
		ControllersToAdd.Add(ControllerQueue[i]);
	}

	ArenaMatch->AddControllersToArenaMatch(ControllersToAdd);
	if (ArenaMatch->CalculateAdditionalPlayersRequiredToStart() == 0)
	{
		ArenaMatch->StartArenaMatch();

		ARAArenaMatchState* ArenaMatchState = ArenaMatch->GetArenaMatchState();
		// Broadcast message
		int32 Switch = URAArenaGameMessage::MakeSwitch(URAArenaGameMessage::Index::YourTurnToFight, 0);
		for (auto It : ControllersToAdd)
		{
			ARAPlayerController* PlayerController = Cast<ARAPlayerController>(It);
			if (PlayerController != nullptr)
			{
				PlayerController->ClientReceiveLocalizedMessage(URAArenaGameMessage::StaticClass(), Switch, nullptr, nullptr, ArenaMatchState);
			}
		}
	}
}

AActor* ARAArenaGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Choose a player start
	APlayerStart* FoundPlayerStart = nullptr;
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;
	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		// Ignore ArenaPlayerStarts
		if (PlayerStart->IsA<ARAArenaPlayerStart>())
		{
			continue;
		}

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		else
		{
			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				UnOccupiedStartPoints.Add(PlayerStart);
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				OccupiedStartPoints.Add(PlayerStart);
			}
		}
	}
	if (FoundPlayerStart == nullptr)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}
	return FoundPlayerStart;
}

bool ARAArenaGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	// Dead team players cannot restart until match end
	if (Player != nullptr)
	{
		ARAArenaPlayerState* PlayerState = Player->GetPlayerState<ARAArenaPlayerState>();
		if (PlayerState != nullptr)
		{
			ARAArenaMatchState* ArenaMatchState = PlayerState->GetArenaMatchState();
			if (ArenaMatchState != nullptr && ArenaMatchState->GetArenaMatchStateName() == EArenaMatchStateName::ArenaMatchInProgress)
			{
				return false;
			}
		}
	}

	return Super::PlayerCanRestart_Implementation(Player);
}

float ARAArenaGameMode::CalculateDesiredDamageModification(APawn* VictimPawn, APawn* InstigatorPawn, float InputDamage)
{
	// Disable all damage outside of the arena
	if (VictimPawn == nullptr || InstigatorPawn == nullptr || ArenaMatch == nullptr)
	{
		return 0.0f;
	}

	AController* VictimController = VictimPawn->GetController<AController>();
	AController* InstigatorController = InstigatorPawn->GetController<AController>();
	if (VictimController == nullptr || InstigatorController == nullptr)
	{
		return 0.0f;
	}

	// Let arena matches determine damages individually
	if(ArenaMatch->ContainsController(VictimController) && ArenaMatch->ContainsController(InstigatorController))
	{
		return ArenaMatch->CalculateDesiredDamageModification(VictimPawn, InstigatorPawn, InputDamage);
	}
	return 0.0f;
}

void ARAArenaGameMode::RconAuthorized_TeamSize_Implementation(ARAPlayerController* Controller, int32 NewTeamSize)
{
	if (ArenaMatch != nullptr)
	{
		ArenaMatch->SetArenaMatchTeamSize(NewTeamSize);
	}
}

void ARAArenaGameMode::RconAuthorized_TimeLimit_Implementation(ARAPlayerController* Controller, float NewTimeLimitMinutes)
{
	if (ArenaMatch != nullptr)
	{
		float NewTimeLimitSeconds = NewTimeLimitMinutes * 60.0f;
		ArenaMatch->SetArenaMatchTimeLimit(NewTimeLimitSeconds);

	}
}