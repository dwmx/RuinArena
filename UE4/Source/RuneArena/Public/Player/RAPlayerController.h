// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/DateTime.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectTypes.h"
#include "RAPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnHudContextChangeSignature, class APawn*, NewPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientReceivePlayerState, class APlayerState*, NewPlayerState);

UCLASS()
class RUNEARENA_API ARAPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    class ARACharacter* RACharacter;

    class APlayerStart* MostRecentlyChosenPlayerStart;

public:
    ARAPlayerController(const class FObjectInitializer& ObjectInitializer);

    /** Fired when the controller wishes to change which pawn it's hud is displaying for */
    UPROPERTY(BlueprintAssignable, Category = "Player|HUD")
    FOnPawnHudContextChangeSignature OnPawnHudContextChange;

    UPROPERTY(BlueprintAssignable, Category = "Player")
    FOnClientReceivePlayerState OnClientReceivePlayerState;

    class APlayerStart* GetMostRecentlyChosenPlayerStart() { return MostRecentlyChosenPlayerStart; }
    void SetMostRecentlyChosenPlayerStart(class APlayerStart* PlayerStart) { MostRecentlyChosenPlayerStart = PlayerStart; }

protected:
    /** Overridden to broadcast delegate which HUD listens for */
    virtual void OnRep_PlayerState() override;

    /** Overridden to send team info updates to possessed character */
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bShowScores;

    virtual void BeginPlay() override;

    /** The root widget to be displayed as the main menu for this controller */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UserInterface")
    TSubclassOf<class UUserWidget> MainMenuRootWidgetClass;
    class UUserWidget* MainMenuRootWidget;

    UPROPERTY(BlueprintReadOnly, Category = Dodging)
    float LastTapForwardTime;

    UPROPERTY(BlueprintReadOnly, Category = Dodging)
    float LastTapBackwardTime;

    UPROPERTY(BlueprintReadOnly, Category = Dodging)
    float LastTapRightTime;

    UPROPERTY(BlueprintReadOnly, Category = Dodging)
    float LastTapLeftTime;

    UPROPERTY(BlueprintReadOnly, Category = Dodging)
    float MaxDoubleTapTime;

    /** Core movement */
    virtual void MoveForward(float Value);
    virtual void MoveBackward(float Value);
    virtual void MoveRight(float Value);
    virtual void MoveLeft(float Value);
    virtual void MoveUp(float Value);
    virtual void MoveDown(float Value);

    /** Double tap recognition for dodging */
    void OnTapForward();
    void OnTapBackward();
    void OnTapRight();
    void OnTapLeft();

    void OnTapForwardRelease();
    void OnTapBackwardRelease();
    void OnTapRightRelease();
    void OnTapLeftRelease();

    void CheckDoubleTap(float LastTapTime, bool bForward, bool bBackward, bool bRight, bool bLeft);

    /** Secondary movement */
    virtual void HandleJumpActionPressed();
    virtual void HandleJumpActionReleased();

    virtual void HandleCrouchActionPressed();
    virtual void HandleCrouchActionReleased();

    /** Combat */
    virtual void HandleAttackActionPressed();
    virtual void HandleAttackActionReleased();

    virtual void HandleThrowActionPressed();
    virtual void HandleThrowActionReleased();

    /** Interaction */
    virtual void HandleUseActionPressed();
    virtual void HandleUseActionReleased();

    virtual void HandleSelectInventory0ActionPressed();
    virtual void HandleSelectInventory0ActionReleased();
    virtual void HandleSelectInventory1ActionPressed();
    virtual void HandleSelectInventory1ActionReleased();
    virtual void HandleSelectInventory2ActionPressed();
    virtual void HandleSelectInventory2ActionReleased();
    virtual void HandleSelectInventory3ActionPressed();
    virtual void HandleSelectInventory3ActionReleased();
    virtual void HandleSelectInventory4ActionPressed();
    virtual void HandleSelectInventory4ActionReleased();

    virtual void HandleSelectInventoryActionPressed(int InventoryCode);
    virtual void HandleSelectInventoryActionReleased(int InventoryCode);

    /** Camera controls */
    void HandleCameraInActionPressed();
    void HandleCameraInActionReleased();
    void HandleCameraOutActionPressed();
    void HandleCameraOutActionReleased();

    /** HUD controls */
    void HandleActionShowScoresPressed();
    void HandleActionShowScoresReleased();

    void HandleActionSayPressed();
    void HandleActionSayReleased();

    void HandleActionTeamSayPressed();
    void HandleActionTeamSayReleased();

public:
    virtual void SetPawn(APawn* InPawn) override;

    /** Helper function which returns this controllers team, based on PlayerState */
    UFUNCTION(BlueprintCallable)
    const uint8 GetTeamIndex();

    /** Returns true if this controller's character is dead */
    UFUNCTION(BlueprintCallable)
    bool CheckIsDead();

protected:
	virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintImplementableEvent)
    void OnClientPossess(APawn* P);

    virtual void AcknowledgePossession(APawn* P) override;

    /** Event handler bound to character's OnTakenDamage delegate */
    UFUNCTION()
    virtual void HandleOnCharacterTakenDamage(class AActor* InstigatorActor, float DamageAmount);
    /** Client RPC on damage invocations */
    UFUNCTION(Client, Unreliable)
    void ClientNotifyCharacterTakenDamage(class AActor* InstigatorActor, float DamageAmount);
    void SendCharacterTakenDamageEventToHUD(class AActor* InstigatorActor, float DamageAmount);

    /** Event handler bound to character's OnGivenDamage delegate */
    UFUNCTION()
    virtual void HandleOnCharacterGivenDamage(class AActor* VictimActor, float DamageAmount);
    /** Client RPC on damage output */
    UFUNCTION(Client, Unreliable)
    void ClientNotifyCharacterGivenDamage(class AActor* VictimActor, float DamageAmount);
    void SendCharacterGivenDamageEventToHUD(class AActor* VictimActor, float DamageAmount);

    /** Client commands */
public:
    UFUNCTION(Exec, BlueprintCallable)
    virtual void Say(FString Message);

    UFUNCTION(Exec, BlueprintCallable)
    virtual void TeamSay(FString Message);

    UFUNCTION(Exec, BlueprintCallable)
    virtual void ChangeTeam(uint8 TeamIndex = 255);

    UFUNCTION(Exec, BlueprintCallable)
    virtual void Suicide();

protected:
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerSuicide();

    virtual bool CheckSayMessageAllowed(const FString& Message);

    UFUNCTION(Server, Reliable)
    virtual void ServerSay(const FString& Message, bool bTeamMessage = false);

public:
    /** Receiving point for GameMode's Broadcast */
    UFUNCTION(Client, Reliable)
    virtual void ClientBroadcast(class ARAPlayerState* SendingPlayerState, const FString& Message, FName Type);

    /** Receiving point for incoming chat messages */
    UFUNCTION(Client, Reliable)
    virtual void ClientSay(class ARAPlayerState* SendingPlayerState, const FString& Message);

    virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime) override;

protected:
    /** ADMIN INTERFACE */
    /** These commands may need to be moved somewhere else later */
    UFUNCTION(Exec)
    virtual void RconAuth(FString RconPassword);

    UFUNCTION(Server, Reliable)
    virtual void ServerRconAuth(const FString& RconPassword);

    UFUNCTION(Exec)
    virtual void RconNormal();

    UFUNCTION(Server, Reliable)
    virtual void ServerRconNormal();

    UFUNCTION(Exec)
    virtual void TeamSize(int32 NewTeamSize);
    UFUNCTION(Server, Reliable)
    virtual void ServerTeamSize(int32 NewTeamSize);

    UFUNCTION(Exec)
    virtual void TimeLimit(float NewtimeLimitMinutes);
    UFUNCTION(Server, Reliable)
    virtual void ServerTimeLimit(float NewtimeLimitMinutes);
    /** END ADMIN INTERFACE */
};
