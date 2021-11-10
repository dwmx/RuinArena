#include "GameModes/Arena/RAArenaMatch.h"
#include "GameModes/Arena/RAArena.h"
#include "GameModes/Arena/RAArenaGameMode.h"
#include "GameModes/Arena/RAArenaMatchState.h"
#include "GameModes/Arena/RAArenaMatchVolume.h"
#include "GameModes/Arena/RAArenaPlayerController.h"
#include "GameModes/Arena/RAArenaPlayerState.h"
#include "GameModes/Arena/RAArenaPlayerStart.h"
#include "Inventory/RANewInventory.h"
#include "Characters/RACharacter.h"
#include "EngineUtils.h"

ARAArenaMatch::ARAArenaMatch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = false;
	bNetLoadOnClient = false;
	ArenaMatchStateName = EArenaMatchStateName::ArenaInactive;
	ArenaMatchStateClass = ARAArenaMatchState::StaticClass();
	ArenaMatchState = nullptr;
	ArenaMatchPreMatchTimeSeconds = 5.0f;
	ArenaMatchTimeLimitSeconds = 120.0f;
	ArenaMatchTimeLimitSecondsMaximum = 1200.0f; // 20 minutes
	ArenaMatchTimeLimitSecondsMinimum = 20.0f; // 20 seconds
	ArenaMatchPostMatchTimeSeconds = 5.0f;
	ArenaMatchTeamSize = 1;
	ArenaMatchTeamSizePendingChange = 1;
	ArenaMatchTeamSizeMaximum = 8;
	ArenaMatchTeamSizeMinimum = 1;
}

void ARAArenaMatch::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;

	if (ArenaMatchStateClass == nullptr)
	{
		ArenaMatchStateClass = ARAArenaMatchState::StaticClass();
	}

	UWorld* World = GetWorld();
	ArenaMatchState = World->SpawnActor<ARAArenaMatchState>(ArenaMatchStateClass, SpawnInfo);
	InitArenaMatchState();
}

void ARAArenaMatch::InitArenaMatchState()
{
	if (ArenaMatchState == nullptr)
	{
		UE_LOG(LogArena, Warning, TEXT("Failed to initialize ArenaMatchState"));
		return;
	}

	ArenaMatchState->SetArenaMatchStateName(ArenaMatchStateName);
	ArenaMatchState->SetArenaMatchTeamSize(ArenaMatchTeamSize);
}

void ARAArenaMatch::BeginDestroy()
{
	if (ArenaMatchState != nullptr)
	{
		ArenaMatchState->Destroy();
	}

	Super::BeginDestroy();
}

void ARAArenaMatch::SetArenaMatchStateTimer(float TimeSeconds)
{
	if (TimeSeconds != 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ArenaMatchStateTimerHandle, this, &ARAArenaMatch::HandleArenaMatchStateTimer, TimeSeconds, false);
	}

	ArenaMatchState->SetArenaMatchStateTimer(TimeSeconds, TimeSeconds);
}

void ARAArenaMatch::HandleArenaMatchStateTimer()
{
	FName NextArenaMatchState = ArenaMatchStateName;

	if (ArenaMatchStateName == EArenaMatchStateName::ArenaPreMatch)
	{
		NextArenaMatchState = EArenaMatchStateName::ArenaMatchInProgress;
	}
	else if (ArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
	{
		// Arena match timed out
		TryProcessMatchEndCondition(true);

		NextArenaMatchState = EArenaMatchStateName::ArenaPostMatch;
	}
	else if (ArenaMatchStateName == EArenaMatchStateName::ArenaPostMatch)
	{
		NextArenaMatchState = EArenaMatchStateName::ArenaIdle;
	}

	SetArenaMatchStateName(NextArenaMatchState);
}

void ARAArenaMatch::SetArenaMatchStateName(FName NewArenaMatchStateName)
{
	if (ArenaMatchStateName == NewArenaMatchStateName)
	{
		return;
	}

	FName OldArenaMatchStateName = ArenaMatchStateName;
	ArenaMatchStateName = NewArenaMatchStateName;

	if (ArenaMatchState != nullptr)
	{
		ArenaMatchState->SetArenaMatchStateName(NewArenaMatchStateName);
	}

	OnArenaMatchStateNameSet(OldArenaMatchStateName, NewArenaMatchStateName);

	UE_LOG(LogArena, Log, TEXT("ArenaMatch state transition (%s) -> (%s)"), *OldArenaMatchStateName.ToString(), *NewArenaMatchStateName.ToString());

	K2_OnArenaMatchStateNameSet(OldArenaMatchStateName, NewArenaMatchStateName);
}

void ARAArenaMatch::OnArenaMatchStateNameSet(FName OldArenaMatchStateName, FName NewArenaMatchStateName)
{
	GetWorld()->GetTimerManager().ClearTimer(ArenaMatchStateTimerHandle);

	// Broadcast first, so that arena game mode has an opportunity to eject losers
	if (OnArenaMatchStateChanged.IsBound())
	{
		OnArenaMatchStateChanged.Broadcast(this, OldArenaMatchStateName, NewArenaMatchStateName);
	}

	if (OldArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
	{
		RestartPlayersInArena();
	}

	if (NewArenaMatchStateName == EArenaMatchStateName::ArenaIdle
	||	NewArenaMatchStateName == EArenaMatchStateName::ArenaInactive)
	{
		SetArenaMatchStateTimer(0.0f);
		SetArenaMatchTeamSize(ArenaMatchTeamSizePendingChange);
	}
	else if (NewArenaMatchStateName == EArenaMatchStateName::ArenaPreMatch)
	{
		SetArenaMatchStateTimer(ArenaMatchPreMatchTimeSeconds);
	}
	else if (NewArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
	{
		SetArenaMatchStateTimer(ArenaMatchTimeLimitSeconds);

		CommitPendingControllers();
		CleanUpArena();
		SpawnPlayersInArena();
	}
	else if (NewArenaMatchStateName == EArenaMatchStateName::ArenaPostMatch)
	{
		SetArenaMatchStateTimer(ArenaMatchPostMatchTimeSeconds);
	}
}

void ARAArenaMatch::CommitPendingControllers()
{
	// Commit pending controllers to this match
	for (auto It : ArenaMatchPendingControllers)
	{
		ArenaMatchControllers.Add(It);

		// Save inventories
		ARAArenaPlayerController* ArenaController = Cast<ARAArenaPlayerController>(It);
		if (ArenaController != nullptr)
		{
			ArenaController->SaveInventoryClasses();
		}
	}
	ArenaMatchPendingControllers.Empty();
}

void ARAArenaMatch::CleanUpArena()
{
	for (TActorIterator<ARAArenaMatchVolume> It(GetWorld()); It; ++It)
	{
		It->CleaUpArenaMatchVolume();
	}
}

bool ARAArenaMatch::CheckIsAvailableToStartNewArenaMatch()
{
	return ArenaMatchStateName == EArenaMatchStateName::ArenaIdle;
}

int32 ARAArenaMatch::CalculateAdditionalPlayersRequiredToStart()
{
	return (ArenaMatchTeamSize * 2) - (ArenaMatchControllers.Num() + ArenaMatchPendingControllers.Num());
}

void ARAArenaMatch::SetArenaMatchTeamSize(int32 NewArenaMatchTeamSize)
{
	NewArenaMatchTeamSize = FMath::Clamp<int32>(NewArenaMatchTeamSize, ArenaMatchTeamSizeMinimum, ArenaMatchTeamSizeMaximum);
	if (ArenaMatchTeamSize == NewArenaMatchTeamSize)
	{
		return;
	}

	// If there's a match in progress, set a pending team size change
	if (ArenaMatchStateName == EArenaMatchStateName::ArenaIdle || ArenaMatchStateName == EArenaMatchStateName::ArenaInactive)
	{
		ArenaMatchTeamSize = NewArenaMatchTeamSize;
		ArenaMatchTeamSizePendingChange = NewArenaMatchTeamSize;
		if (OnArenaMatchTeamSizeChanged.IsBound())
		{
			OnArenaMatchTeamSizeChanged.Broadcast(this, ArenaMatchTeamSize);
		}

		if (ArenaMatchState != nullptr)
		{
			ArenaMatchState->SetArenaMatchTeamSize(ArenaMatchTeamSize);
		}
	}
	else
	{
		ArenaMatchTeamSizePendingChange = NewArenaMatchTeamSize;
	}
}

void ARAArenaMatch::SetArenaMatchTimeLimit(float NewArenaMatchTimeLimitSeconds)
{
	NewArenaMatchTimeLimitSeconds = FMath::Clamp<float>(NewArenaMatchTimeLimitSeconds, ArenaMatchTimeLimitSecondsMinimum, ArenaMatchTimeLimitSecondsMaximum);
	ArenaMatchTimeLimitSeconds = NewArenaMatchTimeLimitSeconds;

	if (OnArenaMatchTimeLimitChanged.IsBound())
	{
		OnArenaMatchTimeLimitChanged.Broadcast(this, ArenaMatchTimeLimitSeconds);
	}
}

void ARAArenaMatch::GetArenaMatchCommittedControllersCopy(TArray<AController*>& OutControllers)
{
	OutControllers.Empty();
	for (auto It : ArenaMatchControllers)
	{
		OutControllers.Add(It);
	}
}

void ARAArenaMatch::GetArenaMatchPendingControllersCopy(TArray<AController*>& OutControllers)
{
	OutControllers.Empty();
	for (auto It : ArenaMatchPendingControllers)
	{
		OutControllers.Add(It);
	}
}

void ARAArenaMatch::GetArenaMatchAllControllersCopy(TArray<AController*>& OutControllers)
{
	OutControllers.Empty();
	for (auto It : ArenaMatchPendingControllers)
	{
		OutControllers.Add(It);
	}
	for (auto It : ArenaMatchControllers)
	{
		OutControllers.Add(It);
	}
}

void ARAArenaMatch::GetArenaMatchWinnerControllersCopy(TArray<AController*>& OutControllers)
{
	OutControllers.Empty();
	for (auto It : ArenaMatchMostRecentWinnerControllers)
	{
		OutControllers.Add(It);
	}
}

void ARAArenaMatch::GetArenaMatchLoserControllersCopy(TArray<AController*>& OutControllers)
{
	OutControllers.Empty();
	for (auto It : ArenaMatchMostRecentLoserControllers)
	{
		OutControllers.Add(It);
	}
}

void ARAArenaMatch::AddControllersToArenaMatch(const TArray<AController*>& Controllers)
{
	ARAArenaGameMode* GameMode = GetWorld()->GetAuthGameMode<ARAArenaGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	int32 NumChampions = 0;
	int32 NumChallengers = 0;
	for (auto It : ArenaMatchControllers)
	{
		ARAPlayerController* Controller = Cast<ARAPlayerController>(It);
		if (Controller == nullptr)
		{
			continue;
		}

		if (Controller->GetTeamIndex() == EArenaTeamIndex::TeamIndexChampions)
		{
			++NumChampions;
		}
		else if (Controller->GetTeamIndex() == EArenaTeamIndex::TeamIndexChallengers)
		{
			++NumChallengers;
		}
	}

	// Temp copy to iterate over all players that were added
	TArray<AController*> AddedControllers;

	// Fill champions team first
	int32 NumChampionsRequired = ArenaMatchTeamSize - NumChampions;
	int32 i = 0;
	for (; i < NumChampionsRequired; ++i)
	{
		if (i >= Controllers.Num())
		{
			break;
		}

		GameMode->ChangeTeam(Controllers[i], EArenaTeamIndex::TeamIndexChampions);
		ArenaMatchPendingControllers.Add(Controllers[i]);
		AddedControllers.Add(Controllers[i]);
	}

	// Fill challengers team second
	int32 NumChallengersRequired = ArenaMatchTeamSize - NumChallengers;
	int32 j = 0;
	for (; j < NumChallengersRequired; ++j)
	{
		int32 Index = i + j;
		if (Index > Controllers.Num())
		{
			break;
		}

		GameMode->ChangeTeam(Controllers[Index], EArenaTeamIndex::TeamIndexChallengers);
		ArenaMatchPendingControllers.Add(Controllers[Index]);
		AddedControllers.Add(Controllers[Index]);
	}

	// Post add
	for (auto It : AddedControllers)
	{
		PostAddControllerToArenaMatch(It);
	}

	// Net update of arena match players
	UpdateArenaMatchStateArenaPlayers();
}

void ARAArenaMatch::PostAddControllerToArenaMatch(AController* Controller)
{
	if (Controller == nullptr)
	{
		return;
	}
}

void ARAArenaMatch::RemoveControllersFromArenaMatch(const TArray<AController*>& Controllers)
{
	ARAArenaGameMode* GameMode = GetWorld()->GetAuthGameMode<ARAArenaGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	for (auto It : Controllers)
	{
		if (!ArenaMatchControllers.Contains(It) && !ArenaMatchPendingControllers.Contains(It))
		{
			continue;
		}
		
		GameMode->ChangeTeam(It, EArenaTeamIndex::TeamIndexSpectators);
		
		ArenaMatchControllers.Remove(It);
		ArenaMatchPendingControllers.Remove(It);
		
		PostRemoveControllerFromArenaMatch(It);
	}

	// Net update of arena match players
	UpdateArenaMatchStateArenaPlayers();
}

void ARAArenaMatch::PostRemoveControllerFromArenaMatch(AController* Controller)
{
	if (Controller == nullptr)
	{
		return;
	}

	ARAArenaPlayerController* ArenaController = Cast<ARAArenaPlayerController>(Controller);
	if (ArenaController != nullptr)
	{
		ArenaController->ClearSavedInventoryClasses();
	}
}

void ARAArenaMatch::RemoveAllControllersFromArenaMatch()
{
	TArray<AController*> ControllersToRemove;

	for (auto It : ArenaMatchControllers)
	{
		ControllersToRemove.Add(It);
	}

	for (auto It : ArenaMatchPendingControllers)
	{
		ControllersToRemove.Add(It);
	}

	RemoveControllersFromArenaMatch(ControllersToRemove);
}

void ARAArenaMatch::UpdateArenaMatchStateArenaPlayers()
{
	if (ArenaMatchState == nullptr)
	{
		return;
	}

	TArray<APlayerState*> ArenaMatchPlayers;

	for (auto It : ArenaMatchControllers)
	{
		APlayerState* PlayerState = It->GetPlayerState<APlayerState>();
		if (PlayerState != nullptr)
		{
			ArenaMatchPlayers.Add(PlayerState);
		}
	}

	for (auto It : ArenaMatchPendingControllers)
	{
		APlayerState* PlayerState = It->GetPlayerState<APlayerState>();
		if (PlayerState != nullptr)
		{
			ArenaMatchPlayers.Add(PlayerState);
		}
	}

	ArenaMatchState->UpdateArenaMatchPlayers(ArenaMatchPlayers);
}

void ARAArenaMatch::EnableArenaMatch()
{
	if (ArenaMatchStateName != EArenaMatchStateName::ArenaInactive)
	{
		return;
	}
	SetArenaMatchStateName(EArenaMatchStateName::ArenaIdle);
}

void ARAArenaMatch::StartArenaMatch()
{
	SetArenaMatchStateName(EArenaMatchStateName::ArenaPreMatch);
}

void ARAArenaMatch::AbortArenaMatch()
{
	// Can only abort during prematch
	if (ArenaMatchStateName != EArenaMatchStateName::ArenaPreMatch)
	{
		return;
	}

	TArray<AController*> ArenaMatchPendingControllersCopy;
	for (auto It : ArenaMatchPendingControllers)
	{
		ArenaMatchPendingControllersCopy.Add(It);
	}
	RemoveControllersFromArenaMatch(ArenaMatchPendingControllersCopy);

	SetArenaMatchStateName(EArenaMatchStateName::ArenaIdle);
}

void ARAArenaMatch::SpawnPlayersInArena()
{
	ARAArenaGameMode* GameMode = GetWorld()->GetAuthGameMode<ARAArenaGameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	TArray<ARAArenaPlayerStart*> ChampionsStarts;
	TArray<ARAArenaPlayerStart*> ChallengersStarts;

	for (TActorIterator<ARAArenaPlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->GetTeamIndex() == EArenaTeamIndex::TeamIndexChampions)
		{
			ChampionsStarts.Add(*It);
		}
		else if (It->GetTeamIndex() == EArenaTeamIndex::TeamIndexChallengers)
		{
			ChallengersStarts.Add(*It);
		}
	}

	int32 ChampionsStartIndex = 0;
	int32 ChallengersStartIndex = 0;
	for (auto It : ArenaMatchControllers)
	{
		ARAPlayerController* RAPlayerController = Cast<ARAPlayerController>(It);
		if (RAPlayerController != nullptr)
		{
			ARAArenaPlayerStart* PlayerStart = nullptr;
			int32 TeamIndex = RAPlayerController->GetTeamIndex();
			if (TeamIndex == EArenaTeamIndex::TeamIndexChampions)
			{
				PlayerStart = ChampionsStarts[ChampionsStartIndex % ChampionsStarts.Num()];
				++ChampionsStartIndex;
			}
			else if (TeamIndex == EArenaTeamIndex::TeamIndexChallengers)
			{
				PlayerStart = ChallengersStarts[ChallengersStartIndex % ChallengersStarts.Num()];
				++ChallengersStartIndex;
			}

			APawn* PlayerPawn = RAPlayerController->GetPawn();
			if (PlayerPawn != nullptr)
			{
				PlayerPawn->Reset();
			}

			GameMode->RestartPlayerAtPlayerStart(RAPlayerController, PlayerStart);
			GiveControllerInventory(RAPlayerController);
		}
	}
}

void ARAArenaMatch::GiveControllerInventory(AController* Controller)
{
	ARAArenaPlayerController* ArenaController = Cast<ARAArenaPlayerController>(Controller);
	if (ArenaController == nullptr)
	{
		return;
	}

	ARACharacter* Character = Cast<ARACharacter>(ArenaController->GetCharacter());
	if (Character == nullptr)
	{
		return;
	}

	Character->ClearInventory(ERACharacterInventoryReleasePolicy::ReleaseAndDestroy);

	TArray<TSubclassOf<ARANewInventory>> InventoryClassesCopy;
	ArenaController->GetSavedInventoryClassesCopy(InventoryClassesCopy);
	UClass* WeaponClass = ArenaController->GetSavedWeaponClass();

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// Grant all inventory
		FActorSpawnParameters SpawnParams;
		for (auto It : InventoryClassesCopy)
		{
			ARANewInventory* SpawnedInventory = World->SpawnActor<ARANewInventory>(It, SpawnParams);
			Character->AcquireInventory(SpawnedInventory, ERACharacterInventoryAcquirePolicy::AcquireAndStow);
		}

		// Equip the saved weapon if there was one
		if (IsValid(WeaponClass))
		{
			for (TRAInventoryIterator<ARANewInventory> It(Character); It; ++It)
			{
				if (It->GetClass() == WeaponClass)
				{
					Character->EquipInventory(*It);
					break;
				}
			}
		}
	}
}

void ARAArenaMatch::RestartPlayersInArena()
{
	ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
	if (GameMode == nullptr)
	{
		return;
	}

	TArray<ARAArenaPlayerStart*> ChampionsStarts;
	TArray<ARAArenaPlayerStart*> ChallengersStarts;

	for (TActorIterator<ARAArenaPlayerStart> It(GetWorld()); It; ++It)
	{
		uint8 TeamIndex = It->GetTeamIndex();
		if (TeamIndex == EArenaTeamIndex::TeamIndexChampions)
		{
			ChampionsStarts.Add(*It);
		}
		else if (TeamIndex == EArenaTeamIndex::TeamIndexChallengers)
		{
			ChallengersStarts.Add(*It);
		}
	}

	int32 ChampionsIndex = 0;
	int32 ChallengersIndex = 0;

	for (auto It : ArenaMatchControllers)
	{
		ARAPlayerController* Controller = Cast<ARAPlayerController>(It);
		if (Controller == nullptr || !Controller->CheckIsDead())
		{
			continue;
		}

		ARAArenaPlayerStart* PlayerStart = nullptr;
		uint8 TeamIndex = Controller->GetTeamIndex();
		if (TeamIndex == EArenaTeamIndex::TeamIndexChampions && ChampionsStarts.Num() > 0)
		{
			PlayerStart = ChampionsStarts[ChampionsIndex % ChampionsStarts.Num()];
			++ChampionsIndex;
		}
		else if (TeamIndex == EArenaTeamIndex::TeamIndexChallengers && ChallengersStarts.Num() > 0)
		{
			PlayerStart = ChallengersStarts[ChallengersIndex % ChallengersStarts.Num()];
			++ChallengersIndex;
		}

		if (PlayerStart != nullptr)
		{
			GameMode->RestartPlayerAtPlayerStart(It, PlayerStart);
		}
	}
}

void ARAArenaMatch::Killed(AController* KillerController, AController* VictimController, APawn* VictimPawn)
{
	if (ArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
	{
		if (ArenaMatchControllers.Contains(VictimController))
		{
			if (TryProcessMatchEndCondition(false))
			{
				SetArenaMatchStateName(EArenaMatchStateName::ArenaPostMatch);
			}
		}
	}
	else
	{
		TArray<AController*> ControllersToRemove;
		ControllersToRemove.Add(VictimController);
		RemoveControllersFromArenaMatch(ControllersToRemove);
	}
}

bool ARAArenaMatch::ContainsController(const AController* Controller)
{
	return ArenaMatchControllers.Contains(Controller);
}

bool ARAArenaMatch::ContainsPendingController(const AController* Controller)
{
	return ArenaMatchPendingControllers.Contains(Controller);
}

float ARAArenaMatch::CalculateDesiredDamageModification(APawn* VictimPawn, APawn* InstigatorPawn, float InputDamage)
{
	if (VictimPawn == nullptr || InstigatorPawn == nullptr)
	{
		return 0.0f;
	}

	AController* VictimController = VictimPawn->GetController<AController>();
	AController* InstigatorController = InstigatorPawn->GetController<AController>();
	if (VictimController == nullptr || InstigatorController == nullptr)
	{
		return 0.0f;
	}

	// Team damaging is managed by game mode, so this only needs to account for match state
	if (ArenaMatchControllers.Contains(VictimController) && ArenaMatchControllers.Contains(InstigatorController))
	{
		if (ArenaMatchStateName == EArenaMatchStateName::ArenaMatchInProgress)
		{
			return InputDamage;
		}
		else
		{
			return 0.0f;
		}
	}

	return 0.0f;
}

bool ARAArenaMatch::TryProcessMatchEndCondition(bool bMatchTimedOut)
{
	int32 ChampionsAlive = 0;
	float CumulativeChampionsHealth = 0.0f;

	int32 ChallengersAlive = 0;
	float CumulativeChallengersHealth = 0.0f;

	for (auto It : ArenaMatchControllers)
	{
		ARAPlayerController* TempController = Cast<ARAPlayerController>(It);
		if (TempController == nullptr)
		{
			continue;
		}

		if (TempController->CheckIsDead())
		{
			continue;
		}

		ARACharacter* TempCharacter = Cast<ARACharacter>(TempController->GetCharacter());

		uint8 TempTeamIndex = TempController->GetTeamIndex();
		if (TempTeamIndex == EArenaTeamIndex::TeamIndexChampions)
		{
			++ChampionsAlive;
			if (TempCharacter != nullptr)
			{
				CumulativeChampionsHealth += TempCharacter->GetHealth();
			}
		}
		else if (TempTeamIndex == EArenaTeamIndex::TeamIndexChallengers)
		{
			++ChallengersAlive;
			if (TempCharacter != nullptr)
			{
				CumulativeChallengersHealth += TempCharacter->GetHealth();
			}
		}
	}

	// Determine winning team index
	uint8 WinningTeamIndex = 255;

	if ((ChampionsAlive + ChallengersAlive) == 0)
	{
		WinningTeamIndex = 255; // Draw
	}
	else if (ChampionsAlive == 0)
	{
		WinningTeamIndex = EArenaTeamIndex::TeamIndexChallengers;
	}
	else if (ChallengersAlive == 0)
	{
		WinningTeamIndex = EArenaTeamIndex::TeamIndexChampions;
	}
	
	// Match time out condition
	if (WinningTeamIndex == 255)
	{
		if (bMatchTimedOut)
		{
			if (CumulativeChampionsHealth > CumulativeChallengersHealth)
			{
				WinningTeamIndex = EArenaTeamIndex::TeamIndexChampions;
			}
			else if (CumulativeChallengersHealth > CumulativeChampionsHealth)
			{
				WinningTeamIndex = EArenaTeamIndex::TeamIndexChallengers;
			}
		}
		else
		{
			return false; // Match is not over yet
		}
	}

	// Match end processing
	ProcessMatchEnd(WinningTeamIndex);
	AnnounceWinners(WinningTeamIndex);

	return true;
}

void ARAArenaMatch::ProcessMatchEnd(uint8 WinningTeamIndex)
{
	// Fill out arrays and swap teams if the challengers won
	bool bSwapTeams = false;
	ARAArenaGameMode* GameMode = GetWorld()->GetAuthGameMode<ARAArenaGameMode>();
	if (GameMode != nullptr && WinningTeamIndex == EArenaTeamIndex::TeamIndexChallengers)
	{
		bSwapTeams = true;
	}
	
	ArenaMatchMostRecentWinnerControllers.Empty();
	ArenaMatchMostRecentLoserControllers.Empty();

	for (auto It : ArenaMatchControllers)
	{
		// Fill out array
		ARAPlayerController* Controller = Cast<ARAPlayerController>(It);
		if (Controller == nullptr)
		{
			continue;
		}

		uint8 TeamIndex = Controller->GetTeamIndex();
		if (TeamIndex != WinningTeamIndex)
		{
			ArenaMatchMostRecentLoserControllers.Add(It);
		}
		else
		{
			ArenaMatchMostRecentWinnerControllers.Add(It);
		}

		// Swap teams
		if (bSwapTeams)
		{
			uint8 NewTeamIndex = TeamIndex;
			if (TeamIndex == EArenaTeamIndex::TeamIndexChallengers)
			{
				NewTeamIndex = EArenaTeamIndex::TeamIndexChampions;
			}
			else
			{
				NewTeamIndex = EArenaTeamIndex::TeamIndexChallengers;
			}

			GameMode->ChangeTeam(It, NewTeamIndex);
		}
	}
}

void ARAArenaMatch::AnnounceWinners(uint8 WinningTeamIndex)
{
	if (WinningTeamIndex == EArenaTeamIndex::TeamIndexChampions)
	{
		UE_LOG(LogArena, Log, TEXT("CHAMPIONS WIN"));
	}
	else if (WinningTeamIndex == EArenaTeamIndex::TeamIndexChallengers)
	{
		UE_LOG(LogArena, Log, TEXT("CHALLENGERS WIN"));
	}
	else if (WinningTeamIndex == 255)
	{
		UE_LOG(LogArena, Log, TEXT("DRAW"));
	}
}