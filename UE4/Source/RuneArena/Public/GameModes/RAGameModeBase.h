// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RAGameModeBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class RUNEARENA_API ARAGameModeBase : public AGameMode
{
	GENERATED_BODY()
	using Super = AGameMode;

protected:
	/** Password required for rcon access */
	UPROPERTY(Config=Game)
	FString RconAdminPassword;

	/** The message class this game type will use for broadcasting killed messages */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classes|Messaging")
	TSubclassOf<class URAKilledMessage> KilledMessageClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class ARATeamInfo> TeamClass;

	/** All TeamInfo actors */
	UPROPERTY(BlueprintReadOnly, Category = "Teams")
	TArray<class ARATeamInfo*> TeamInfos;

	/** Colors for each team, expected to align with NumTeams */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	TArray<FLinearColor> TeamColors;

	/** Names for each team, expected to align with NumTeams */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	TArray<FText> TeamNames;

	/** How many teams to create */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	uint8 NumTeams;

	/** If false, all team info is ignored */
	UPROPERTY()
	bool bTeamGame;

	/** If disabled, ChangeTeam will deny calls made by player states via the player controller */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teams")
	bool bAllowPlayerTeamChangeRequests;

	/** Multiply damage values by this factor before applying to struck teammates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teams")
	float FriendlyFireMultiplier;

	bool MovePlayerToTeam(AController* Controller, class ARAPlayerState* PlayerState, uint8 NewTeamIndex);

	/** The inventory classes to be spawned and given to a character each time they restart */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game")
	TArray<TSubclassOf<class ARANewInventory>> DefaultCharacterInventoryClasses;

	/** The default weapon which will be equipped on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game")
	TSubclassOf<class ARANewInventory> DefaultCharacterWeaponClass;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
public:
	ARAGameModeBase(const FObjectInitializer& ObjectInitializer);

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(class APlayerController* NewPlayer) override;
	virtual void Logout(class AController* Exiting) override;

	/**
	*	Overridden to ignore saved player start and choose a random start each spawn.
	*	To override the actual player start selection, override AGameModeBase::ChoosePlayerStart.
	*/
	virtual class AActor* FindPlayerStart_Implementation(class AController* Player, const FString& IncomingName) override;

	/** Overridden to prevent spawning a player at the same spawn point twice in a row */
	virtual class AActor* ChoosePlayerStart_Implementation(class AController* Player) override;

	bool CheckIsTeamGame() { return bTeamGame; }
	
	float GetFriendlyFireMultiplier() { return FriendlyFireMultiplier; }

	/** Retrieve the number of teams that currently exist */
	UFUNCTION(BlueprintCallable, Category = "Teams")
	const uint8 GetNumTeams();

	UFUNCTION(BlueprintCallable, Category = "Teams")
	bool GetAllowPlayerTeamChangeRequests() { return bAllowPlayerTeamChangeRequests; };

	UFUNCTION(BlueprintCallable, Category = "Teams")
	virtual bool ChangeTeam(AController* Controller, uint8 NewTeamIndex = 255, bool bBroadcast = true);

	UFUNCTION(BlueprintCallable, Category = "Teams")
	class ARATeamInfo* GetTeamFromIndex(uint8 TeamIndex);

	/** Allows the game to modify damage values before they are applied */
	virtual float CalculateDesiredDamageModification(class APawn* VictimPawn, class APawn* InstigatorPawn, float InputDamage);

	/** Called by ARACharacter upon death */
	virtual void Killed(class AController* KillerController, class AController* VictimController, class APawn* VictimPawn);

protected:
	/** Overridden to reroute the message sent to client */
	virtual void Broadcast(class AActor* Sender, const FString& Message, FName Type = NAME_None) override;

	/** Exposed calling point of Broadcast with blueprints. Use BroadcastLocalized whenever possible */
	UFUNCTION(BlueprintCallable, Category = "GameMode", Meta = (DisplayName = "Broadcast"))
	void K2_Broadcast(class AActor* Sender, const FString& Message, FName Type = NAME_None);

	/** Exposed calling point of BroadcastLocalized with blueprints. Prefer this function over Broadcast. */
	UFUNCTION(BlueprintCallable, Category = "GameMode", Meta = (DisplayName = "BroadcastLocalized"))
	void K2_BroadcastLocalized(
		class AActor* Sender,
		TSubclassOf<class ULocalMessage> Message,
		int32 Switch = 0,
		class APlayerState* RelatedPlayerState_1 = nullptr,
		class APlayerState* RelatedPlayerState_2 = nullptr,
		class UObject* OptionalObject = nullptr);

	virtual void BroadcastKilledMessage(class AController* KillerController, class AController* VictimController);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameMode")
	float PlayerRespawnTimeSeconds;

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void ScoreKill(class AController* KillerController, class AController* VictimController, class APawn* VictimPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void ScoreTeamKill(class AController* KillerController, class AController* VictimController, class APawn* VictimPawn, class ARATeamInfo* Team);

	/** Overridden to use start location's rotation */
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

public:
	virtual bool AllowSuicideBy(class AController* Controller) { return true; }

	/** Overridden to implement respawn timer */
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;

	/** Overridden to use for player resetting without respawning */
	virtual void RestartPlayerAtPlayerStart(class AController* NewPlayer, class AActor* StartSpot) override;

	/** Overridden so that K2_OnNameChange can get the player's current name and new name */
	virtual void ChangeName(class AController* Other, const FString& S, bool bNameChange) override;

	/** Called from ARAPlayerController Say and TeamSay */
	virtual void PlayerSay(class ARAPlayerController* Controller, const FString& Message, bool bTeamMessage = false);

public:
	/** ADMIN INTERFACE */
	virtual void RconAuth(class ARAPlayerController* Controller, const FString& RconPassword);
	virtual void RconNormal(class ARAPlayerController* Controller);
	virtual bool CheckIsRconAuthorized(class ARAPlayerController* Controller);

	/** Fired immediately after the specified controller has gained admin rights */
	UFUNCTION(BlueprintNativeEvent, Category = "RconAdmin", Meta = (DisplayName = "OnRconAuthorized"))
	void K2_OnRconAuthorized(class ARAPlayerController* Controller);

	/** Fired immediately after the specified controller relinquishes admin rights */
	UFUNCTION(BlueprintNativeEvent, Category = "RconAdmin", Meta = (DisplayName = "OnRconNormal"))
	void K2_OnRconNormal(class ARAPlayerController* Controller);

	void Rcon_TeamSize(class ARAPlayerController* Controller, int32 NewTeamSize);
	void Rcon_TimeLimit(class ARAPlayerController* Controller, float NewtimeLimitMinutes);

protected:
	/** Allows subclassed gametypes to specialize their response to a team size command.
	*	Only implement this in blueprints if you want completely custom functionality, otherwise use the Post event.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "RconAdmin")
	void RconAuthorized_TeamSize(class ARAPlayerController* Controller, int32 NewTeamSize);
	UFUNCTION(BlueprintImplementableEvent, Category = "RconAdmin", Meta = (DisplayName = "RconAuthorizedPostTeamSize"))
	void K2_RconAuthorized_PostTeamSize(class ARAPlayerController* Controller, int32 NewTeamSize);

	/** Allows subclassed gametypes to specialize their response to a time limit command.
	*	Only implement this in blueprints if you want completely custom functionality, otherwise use the Post event.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "RconAdmin")
	void RconAuthorized_TimeLimit(class ARAPlayerController* Controller, float NewtimeLimitMinutes);
	UFUNCTION(BlueprintImplementableEvent, Category = "RconAdmin", Meta = (DisplayName="RconAuthorizedPostTimeLimit"))
	void K2_RconAuthorized_PostTimeLimit(class ARAPlayerController* Controller, float NewtimeLimitMinutes);
	/** END ADMIN INTERFACE */
};