#include "GameModes/RAGameModeBase.h"
#include "GameModes/RATeamInfo.h"
#include "GameModes/RAPlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "Player/RAPlayerState.h"
#include "Player/RAPlayerController.h"
#include "Player/Messaging/RAKilledMessage.h"
#include "Player/Messaging/RAGameMessage.h"
#include "Characters/RACharacter.h"
#include "Inventory/RANewInventory.h"
#include "GameFramework/SpectatorPawn.h"
#include "EngineUtils.h"

ARAGameModeBase::ARAGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPlayerName = FText::FromString(FString::Printf(TEXT("Player")));
	PlayerRespawnTimeSeconds = 5.0f;
	KilledMessageClass = URAKilledMessage::StaticClass();
	TeamClass = ARATeamInfo::StaticClass();
	bTeamGame = false;
	NumTeams = 2;
	bAllowPlayerTeamChangeRequests = true;
	FriendlyFireMultiplier = 0.0f;
}

#if WITH_EDITOR
void ARAGameModeBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Ensure default weapon is also included in default inventory array
	if (PropertyChangedEvent.Property != nullptr && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ARAGameModeBase, DefaultCharacterWeaponClass))
	{
		if (IsValid(DefaultCharacterWeaponClass))
		{
			DefaultCharacterInventoryClasses.AddUnique(DefaultCharacterWeaponClass);
		}
	}
}
#endif

void ARAGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Init teams
	if (bTeamGame)
	{
		for (uint8 i = 0; i < NumTeams; ++i)
		{
			ARATeamInfo* NewTeamInfo = GetWorld()->SpawnActor<ARATeamInfo>(TeamClass);
			NewTeamInfo->SetTeamIndex(i);

			if (TeamColors.IsValidIndex(i))
			{
				NewTeamInfo->SetTeamColor(TeamColors[i]);
			}

			if (TeamNames.IsValidIndex(i))
			{
				NewTeamInfo->SetTeamName(TeamNames[i]);
			}

			TeamInfos.Add(NewTeamInfo);
			checkSlow(TeamInfos[i] == NewTeamInfo);
		}

		UE_LOG(LogGameMode, Log, TEXT("GameMode initialized with %d teams"), TeamInfos.Num());
	}
}

void ARAGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UE_LOG(LogGameMode, Log, TEXT("PreLogin: %s : %s"), *Address, *Options);
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void ARAGameModeBase::PostLogin(class APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 Switch = URAGameMessage::MakeSwitch(URAGameMessage::Index::PlayerJoined, 0);
	BroadcastLocalized(this, URAGameMessage::StaticClass(), Switch, NewPlayer->GetPlayerState<APlayerState>());
}

void ARAGameModeBase::Logout(AController* Exiting)
{
	int32 Switch = URAGameMessage::MakeSwitch(URAGameMessage::Index::PlayerLeft, 0);
	BroadcastLocalized(this, URAGameMessage::StaticClass(), Switch, Exiting->GetPlayerState<APlayerState>());
	Super::Logout(Exiting);
}

AActor* ARAGameModeBase::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	UWorld* World = GetWorld();

	// If incoming start is specified, then just use it
	if (!IncomingName.IsEmpty())
	{
		const FName IncomingPlayerStartTag = FName(*IncomingName);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (Start && Start->PlayerStartTag == IncomingPlayerStartTag)
			{
				return Start;
			}
		}
	}

	// Find player start, ignoring any spot saved on the player
	AActor* BestStart = ChoosePlayerStart(Player);
	if (BestStart == nullptr)
	{
		// No player start found
		UE_LOG(LogGameMode, Log, TEXT("FindPlayerStart: PATHS NOT DEFINED or NO PLAYERSTART with positive rating"));

		// This is a bit odd, but there was a complex chunk of code that in the end always resulted in this, so we may as well just 
		// short cut it down to this.  Basically we are saying spawn at 0,0,0 if we didn't find a proper player start
		BestStart = World->GetWorldSettings();
	}

	return BestStart;
}

AActor* ARAGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
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

	// Prevent player from spawning at the same start multiple times in a row if possible
	APlayerStart* MostRecentlyChosenPlayerStart = nullptr;
	if (Cast<ARAPlayerController>(Player) != nullptr)
	{
		MostRecentlyChosenPlayerStart = Cast<ARAPlayerController>(Player)->GetMostRecentlyChosenPlayerStart();
	}
	if (UnOccupiedStartPoints.Num() > 1 && MostRecentlyChosenPlayerStart != nullptr)
	{
		UnOccupiedStartPoints.Remove(MostRecentlyChosenPlayerStart);
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

const uint8 ARAGameModeBase::GetNumTeams()
{
	return TeamInfos.Num();
}

bool ARAGameModeBase::ChangeTeam(AController* Controller, uint8 NewTeamIndex, bool bBroadcast)
{
	if (!bTeamGame || Controller == nullptr)
	{
		return false;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr)
	{
		return false;
	}

	NewTeamIndex = NewTeamIndex % TeamInfos.Num();
	if (MovePlayerToTeam(Controller, PlayerState, NewTeamIndex))
	{
		return true;
	}

	return false;
}

bool ARAGameModeBase::MovePlayerToTeam(AController* Controller, class ARAPlayerState* PlayerState, uint8 NewTeamIndex)
{
	if (!TeamInfos.IsValidIndex(NewTeamIndex))
	{
		return false;
	}

	if (PlayerState->GetTeam() != nullptr && PlayerState->GetTeam()->GetTeamIndex() == NewTeamIndex)
	{
		return false;
	}

	TeamInfos[NewTeamIndex]->AddToTeam(Controller);
	PlayerState->ForceNetUpdate();
	return true;
}

ARATeamInfo* ARAGameModeBase::GetTeamFromIndex(uint8 TeamIndex)
{
	if (TeamInfos.IsValidIndex(TeamIndex))
	{
		return TeamInfos[TeamIndex];
	}
	else
	{
		return nullptr;
	}
}

float ARAGameModeBase::CalculateDesiredDamageModification(APawn* VictimPawn, APawn* InstigatorPawn, float InputDamage)
{
	return InputDamage;
}

bool ARAGameModeBase::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if (Player != nullptr)
	{
		ARAPlayerState* PlayerState = Player->GetPlayerState<ARAPlayerState>();
		if (PlayerState != nullptr)
		{
			if (PlayerState->GetRespawnTimeRemainingSeconds() > 0.0f)
			{
				return false;
			}
		}
	}

	return Super::PlayerCanRestart_Implementation(Player);
}

void ARAGameModeBase::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	if (!StartSpot)
	{
		UE_LOG(LogGameMode, Warning, TEXT("RestartPlayerAtPlayerStart: Player start not found"));
		return;
	}

	FRotator SpawnRotation = StartSpot->GetActorRotation();
	FVector SpawnLocation = StartSpot->GetActorLocation();

	UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart %s"), (NewPlayer && NewPlayer->PlayerState) ? *NewPlayer->PlayerState->GetPlayerName() : TEXT("Unknown"));

	if (MustSpectate(Cast<APlayerController>(NewPlayer)))
	{
		UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart: Tried to restart a spectator-only player!"));
		return;
	}

	if (NewPlayer->GetPawn() != nullptr)
	{
		// If we have an existing pawn, just use it's rotation
		//SpawnRotation = NewPlayer->GetPawn()->GetActorRotation();
	}
	else if (GetDefaultPawnClassForController(NewPlayer) != nullptr)
	{
		// Try to create a pawn to use of the default class for this player
		NewPlayer->SetPawn(SpawnDefaultPawnFor(NewPlayer, StartSpot));
	}

	if (NewPlayer->GetPawn() == nullptr)
	{
		NewPlayer->FailedToSpawnPawn();
	}
	else
	{
		// Tell the start spot it was used
		InitStartSpot(StartSpot, NewPlayer);

		// Save the start on the player so game mode can attempt to not use the same start next spawn
		if (Cast<ARAPlayerController>(NewPlayer) != nullptr)
		{
			Cast<ARAPlayerController>(NewPlayer)->SetMostRecentlyChosenPlayerStart(Cast<APlayerStart>(StartSpot));
		}

		NewPlayer->GetPawn()->TeleportTo(SpawnLocation, SpawnRotation);
		FinishRestartPlayer(NewPlayer, SpawnRotation);
	}
}

void ARAGameModeBase::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	NewPlayer->Possess(NewPlayer->GetPawn());

	// If the Pawn is destroyed as part of possession we have to abort
	if (NewPlayer->GetPawn() == nullptr)
	{
		NewPlayer->FailedToSpawnPawn();
	}
	else
	{
		// Set initial control rotation to starting rotation rotation
		//NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);
		NewPlayer->ClientSetRotation(StartRotation, true);

		FRotator NewControllerRot = StartRotation;
		NewControllerRot.Roll = 0.f;
		NewPlayer->SetControlRotation(NewControllerRot);

		// Inventory
		ARACharacter* Character = Cast<ARACharacter>(NewPlayer->GetPawn());
		if (Character != nullptr)
		{
			// Clear out anything the player may have in inventory
			Character->ClearInventory(ERACharacterInventoryReleasePolicy::ReleaseAndDestroy);

			// Provide default inventory
			for (auto It : DefaultCharacterInventoryClasses)
			{
				ARANewInventory* Inventory = Cast<ARANewInventory>(GetWorld()->SpawnActor(It));
				if (Inventory != nullptr)
				{
					Character->AcquireInventory(Inventory, ERACharacterInventoryAcquirePolicy::AcquireAndStow);
				}
			}

			// Equip default weapon if there is one
			if (IsValid(DefaultCharacterWeaponClass))
			{
				ARANewInventory* InventoryToEquip = nullptr;
				for (TRAInventoryIterator<ARANewInventory> It(Character); It; ++It)
				{
					if (It->GetClass() == DefaultCharacterWeaponClass)
					{
						InventoryToEquip = *It;
						break;
					}
				}

				if (InventoryToEquip != nullptr)
				{
					Character->EquipInventory(InventoryToEquip);
				}
				else
				{
					UE_LOG(LogGameMode, Warning, TEXT("Character was unable to equip default weapon"));
				}
			}
		}

		SetPlayerDefaults(NewPlayer->GetPawn());
		NewPlayer->GetPawn()->Reset();

		K2_OnRestartPlayer(NewPlayer);
	}
}

void ARAGameModeBase::Killed(AController* KillerController, AController* VictimController, APawn* VictimPawn)
{
	// TODO:
	// Update anything in the player state
	// Discard player inventory
	// Broadcast death message

	ARAPlayerController* RAKillerController = Cast<ARAPlayerController>(KillerController);
	ARAPlayerController* RAVictimController = Cast<ARAPlayerController>(VictimController);

	ARAPlayerState* RAVictimPlayerState = nullptr;
	if (RAVictimController != nullptr)
	{
		RAVictimPlayerState = RAVictimController->GetPlayerState<ARAPlayerState>();
	}

	// Update respawn timer on the dead player
	if (RAVictimPlayerState != nullptr)
	{
		RAVictimPlayerState->SetRespawnTimeRemainingSeconds(PlayerRespawnTimeSeconds);
	}

	// Check for team kill
	bool bTeamKill = false;
	if (bTeamGame)
	{
		if (RAKillerController != nullptr && RAVictimController != nullptr)
		{
			if (RAKillerController->GetTeamIndex() == RAVictimController->GetTeamIndex())
			{
				uint8 TeamIndex = RAKillerController->GetTeamIndex();
				if (TeamInfos.IsValidIndex(TeamIndex))
				{
					ARATeamInfo* Team = TeamInfos[TeamIndex];
					ScoreTeamKill(KillerController, VictimController, VictimPawn, Team);
					bTeamKill = true;
				}
			}
		}
	}

	if (!bTeamKill)
	{
		ScoreKill(KillerController, VictimController, VictimPawn);
	}

	BroadcastKilledMessage(KillerController, VictimController);
}

void ARAGameModeBase::BroadcastKilledMessage(AController* KillerController, AController* VictimController)
{
	if (KilledMessageClass == nullptr)
	{
		return;
	}

	BroadcastLocalized(this, KilledMessageClass, 0, KillerController->GetPlayerState<APlayerState>(), VictimController->GetPlayerState<APlayerState>(), nullptr);
}

void ARAGameModeBase::ScoreKill_Implementation(AController* KillerController, AController* VictimController, APawn* VictimPawn)
{
	ARAPlayerController* RAKillerController = Cast<ARAPlayerController>(KillerController);
	ARAPlayerState* RAKillerPlayerState = nullptr;
	if (RAKillerController != nullptr)
	{
		RAKillerPlayerState = RAKillerController->GetPlayerState<ARAPlayerState>();
		if (RAKillerPlayerState != nullptr)
		{
			RAKillerPlayerState->IncrementFrags();
		}
	}

	ARAPlayerController* RAVictimController = Cast<ARAPlayerController>(VictimController);
	ARAPlayerState* RAVictimPlayerState = nullptr;
	if (RAVictimController != nullptr)
	{
		RAVictimPlayerState = RAVictimController->GetPlayerState<ARAPlayerState>();
		if (RAVictimPlayerState != nullptr)
		{
			RAVictimPlayerState->IncrementDeaths();
		}
	}
}

void ARAGameModeBase::ScoreTeamKill_Implementation(AController* KillerController, AController* VictimController, APawn* VictimPawn, ARATeamInfo* Team)
{
	ARAPlayerController* RAKillerController = Cast<ARAPlayerController>(KillerController);
	ARAPlayerState* RAKillerPlayerState = nullptr;
	if (RAKillerController != nullptr)
	{
		RAKillerPlayerState = RAKillerController->GetPlayerState<ARAPlayerState>();
		if (RAKillerPlayerState != nullptr)
		{
			RAKillerPlayerState->IncrementFrags(-1.0f);
		}
	}

	ARAPlayerController* RAVictimController = Cast<ARAPlayerController>(VictimController);
	ARAPlayerState* RAVictimPlayerState = nullptr;
	if (RAVictimController != nullptr)
	{
		RAVictimPlayerState = RAVictimController->GetPlayerState<ARAPlayerState>();
		if (RAVictimPlayerState != nullptr)
		{
			RAVictimPlayerState->IncrementDeaths();
		}
	}
}

void ARAGameModeBase::Broadcast(AActor* Sender, const FString& Message, FName Type)
{
	APlayerState* SenderPlayerState = nullptr;
	if (Cast<APawn>(Sender) != nullptr)
	{
		SenderPlayerState = Cast<APawn>(Sender)->GetPlayerState();
	}
	else if (Cast<AController>(Sender) != nullptr)
	{
		SenderPlayerState = Cast<AController>(Sender)->PlayerState;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Iterator->Get())
		{
			ARAPlayerController* RAPC = Cast<ARAPlayerController>(PC);
			if (RAPC != nullptr)
			{
				RAPC->ClientBroadcast(Cast<ARAPlayerState>(SenderPlayerState), Message, Type);
			}
			else
			{
				PC->ClientTeamMessage(SenderPlayerState, Message, Type);
			}
		}
	}
}

void ARAGameModeBase::K2_Broadcast(AActor* Sender, const FString& Message, FName Type)
{
	Broadcast(Sender, Message, Type);
}

void ARAGameModeBase::K2_BroadcastLocalized(AActor* Sender, TSubclassOf<class ULocalMessage> Message, int32 Switch , APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	BroadcastLocalized(Sender, Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
}

/** Standard player controller interface */
void ARAGameModeBase::PlayerSay(ARAPlayerController* Controller, const FString& Message, bool bTeamMessage)
{
	if (Controller == nullptr || Message.IsEmpty())
	{
		return;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ARAPlayerController* ItController = Cast<ARAPlayerController>(*It);
		if (ItController == nullptr)
		{
			continue;
		}

		if (bTeamGame && bTeamMessage && Controller->GetTeamIndex() != ItController->GetTeamIndex())
		{
			continue;
		}

		ItController->ClientSay(PlayerState, Message);
	}
}

void ARAGameModeBase::ChangeName(AController* Other, const FString& S, bool bNameChange)
{
	if (Other && !S.IsEmpty())
	{
		// Change calling order so blueprints can get the current and new name
		K2_OnChangeName(Other, S, bNameChange);

		Other->PlayerState->SetPlayerName(S);
	}
}

/** ADMIN INTERFACE */
void ARAGameModeBase::RconAuth(ARAPlayerController* Controller, const FString& RconPassword)
{
	if (Controller == nullptr)
	{
		return;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr || PlayerState->GetIsRconAdmin())
	{
		return;
	}

#if WITH_EDITOR
	UE_LOG(LogGameMode, Log, TEXT("Rcon authorized user logged in: %s"), *PlayerState->GetPlayerName());
	PlayerState->SetIsRconAdmin(true);
	K2_OnRconAuthorized(Controller);
#else
	if(!RconAdminPassword.IsEmpty() && RconAdminPassword == RconPassword)
	{
		UE_LOG(LogGameMode, Log, TEXT("Rcon authorized user logged in: %s"), *PlayerState->GetPlayerName());
		PlayerState->SetIsRconAdmin(true);
		K2_OnRconAuthorized(Controller);
	}
#endif
}

void ARAGameModeBase::K2_OnRconAuthorized_Implementation(ARAPlayerController* Controller)
{
	if (Controller != nullptr)
	{
		int32 Switch = URAGameMessage::MakeSwitch(URAGameMessage::Index::RconAuthorized, 0);
		BroadcastLocalized(this, URAGameMessage::StaticClass(), Switch, Controller->GetPlayerState<ARAPlayerState>());
	}
}

void ARAGameModeBase::RconNormal(ARAPlayerController* Controller)
{
	if (Controller == nullptr)
	{
		return;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr || !PlayerState->GetIsRconAdmin())
	{
		return;
	}

	UE_LOG(LogGameMode, Log, TEXT("Rcon authorized user logged out: %s"), *PlayerState->GetPlayerName());
	PlayerState->SetIsRconAdmin(false);
	K2_OnRconNormal(Controller);
}

void ARAGameModeBase::K2_OnRconNormal_Implementation(ARAPlayerController* Controller)
{
	if (Controller != nullptr)
	{
		int32 Switch = URAGameMessage::MakeSwitch(URAGameMessage::Index::RconNormal, 0);
		BroadcastLocalized(this, URAGameMessage::StaticClass(), Switch, Controller->GetPlayerState<ARAPlayerState>());
	}
}

bool ARAGameModeBase::CheckIsRconAuthorized(ARAPlayerController* Controller)
{
	if (Controller == nullptr)
	{
		return false;
	}

	ARAPlayerState* PlayerState = Controller->GetPlayerState<ARAPlayerState>();
	if (PlayerState == nullptr)
	{
		return false;
	}

	return PlayerState->GetIsRconAdmin();
}

void ARAGameModeBase::Rcon_TeamSize(ARAPlayerController* Controller, int32 NewTeamSize)
{
	if (!CheckIsRconAuthorized(Controller))
	{
		return;
	}

	RconAuthorized_TeamSize(Controller, NewTeamSize);
	K2_RconAuthorized_PostTeamSize(Controller, NewTeamSize);
}

void ARAGameModeBase::RconAuthorized_TeamSize_Implementation(ARAPlayerController* Controller, int32 NewTeamSize)
{}

void ARAGameModeBase::Rcon_TimeLimit(ARAPlayerController* Controller, float NewtimeLimitMinutes)
{
	if (!CheckIsRconAuthorized(Controller))
	{
		return;
	}

	RconAuthorized_TimeLimit(Controller, NewtimeLimitMinutes);
	K2_RconAuthorized_PostTimeLimit(Controller, NewtimeLimitMinutes);
}

void ARAGameModeBase::RconAuthorized_TimeLimit_Implementation(ARAPlayerController* Controller, float NewtimeLimitMinutes)
{}
/** END ADMIN INTERFACE */