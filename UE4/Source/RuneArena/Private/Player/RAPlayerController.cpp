// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/RAPlayerController.h"
#include "Player/RAPlayerState.h"
#include "Player/Messaging/RAChatMessage.h"
#include "GameModes/RAHUDBase.h"
#include "GameModes/RAGameModeBase.h"
#include "GameModes/RATeamInfo.h"
#include "Characters/RACharacter.h"
#include "Characters/Abilities/RAAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/RAAttributeSetBase.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"

static const float DOUBLE_TAP_RESET_TIME = -10.0f;

ARAPlayerController::ARAPlayerController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    LastTapForwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapBackwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapRightTime = DOUBLE_TAP_RESET_TIME;
    LastTapLeftTime = DOUBLE_TAP_RESET_TIME;
    //bIsHoldingDodge = false;
    MaxDoubleTapTime = 0.3f;
}

void ARAPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalPlayerController() && IsValid(MainMenuRootWidgetClass))
    {
        MainMenuRootWidget = CreateWidget<UUserWidget>(this, MainMenuRootWidgetClass, FName(TEXT("MainMenuWidget")));
        if (MainMenuRootWidget != nullptr)
        {
            MainMenuRootWidget->SetVisibility(ESlateVisibility::Hidden);
            MainMenuRootWidget->AddToViewport();
        }
    }
}

void ARAPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    ARACharacter* CharacterTemp = GetPawn<ARACharacter>();
    if (CharacterTemp != nullptr)
    {
        ARAPlayerState* PlayerStateTemp = GetPlayerState<ARAPlayerState>();
        if (PlayerStateTemp != nullptr)
        {
            CharacterTemp->SetOwnerTeamInfo(PlayerStateTemp->GetTeam());
        }

        CharacterTemp->OnDamageTaken.AddDynamic(this, &ARAPlayerController::HandleOnCharacterTakenDamage);
        CharacterTemp->OnDamageGiven.AddDynamic(this, &ARAPlayerController::HandleOnCharacterGivenDamage);
    }
}

void ARAPlayerController::OnUnPossess()
{
    ARACharacter* CharacterTemp = GetPawn<ARACharacter>();
    if (CharacterTemp != nullptr)
    {
        ARAPlayerState* PlayerStateTemp = GetPlayerState<ARAPlayerState>();
        if (PlayerStateTemp != nullptr)
        {
            CharacterTemp->SetOwnerTeamInfo(nullptr);
        }

        CharacterTemp->OnDamageTaken.RemoveAll(this);
        CharacterTemp->OnDamageGiven.RemoveAll(this);
    }

    Super::OnUnPossess();
}

void ARAPlayerController::HandleOnCharacterTakenDamage(AActor* InstigatorActor, float DamageAmount)
{
    if (IsLocalPlayerController())
    {
        SendCharacterTakenDamageEventToHUD(InstigatorActor, DamageAmount);
    }
    else
    {
        ClientNotifyCharacterTakenDamage(InstigatorActor, DamageAmount);
    }
}

void ARAPlayerController::ClientNotifyCharacterTakenDamage_Implementation(AActor* InstigatorActor, float DamageAmount)
{
    SendCharacterTakenDamageEventToHUD(InstigatorActor, DamageAmount);
}

void ARAPlayerController::SendCharacterTakenDamageEventToHUD(AActor* InstigatorActor, float DamageAmount)
{
    ARAHUDBase* TempHUD = GetHUD<ARAHUDBase>();
    if (TempHUD != nullptr)
    {
        TempHUD->ReceiveCharacterTakenDamage(InstigatorActor, DamageAmount);
    }
}

void ARAPlayerController::HandleOnCharacterGivenDamage(AActor* VictimActor, float DamageAmount)
{
    if (IsLocalPlayerController())
    {
        SendCharacterGivenDamageEventToHUD(VictimActor, DamageAmount);
    }
    else
    {
        ClientNotifyCharacterGivenDamage(VictimActor, DamageAmount);
    }
}

void ARAPlayerController::ClientNotifyCharacterGivenDamage_Implementation(AActor* VictimActor, float DamageAmount)
{
    SendCharacterGivenDamageEventToHUD(VictimActor, DamageAmount);
}

void ARAPlayerController::SendCharacterGivenDamageEventToHUD(AActor* VictimActor, float DamageAmount)
{
    ARAHUDBase* TempHUD = GetHUD<ARAHUDBase>();
    if (TempHUD != nullptr)
    {
        TempHUD->ReceiveCharacterGivenDamage(VictimActor, DamageAmount);
    }
}

void ARAPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    //// Main menu
    //InputComponent->BindAction("ShowMenu", IE_Pressed, this, &ARAPlayerController::HandleActionShowMenuPressed);
    //InputComponent->BindAction("ShowMenu", IE_Released, this, &ARAPlayerController::HandleActionShowMenuReleased);

    // Core movement
    InputComponent->BindAxis("MoveForward", this, &ARAPlayerController::MoveForward);
    InputComponent->BindAxis("MoveBackward", this, &ARAPlayerController::MoveBackward);
    InputComponent->BindAxis("MoveRight", this, &ARAPlayerController::MoveRight);
    InputComponent->BindAxis("MoveLeft", this, &ARAPlayerController::MoveLeft);
    InputComponent->BindAxis("MoveUp", this, &ARAPlayerController::MoveUp);
    InputComponent->BindAxis("MoveDown", this, &ARAPlayerController::MoveDown);

    // Dodging
    InputComponent->BindAction("TapForward", IE_Pressed, this, &ARAPlayerController::OnTapForward);
    InputComponent->BindAction("TapBackward", IE_Pressed, this, &ARAPlayerController::OnTapBackward);
    InputComponent->BindAction("TapRight", IE_Pressed, this, &ARAPlayerController::OnTapRight);
    InputComponent->BindAction("TapLeft", IE_Pressed, this, &ARAPlayerController::OnTapLeft);

    InputComponent->BindAction("TapForward", IE_Released, this, &ARAPlayerController::OnTapForwardRelease);
    InputComponent->BindAction("TapBackward", IE_Released, this, &ARAPlayerController::OnTapBackwardRelease);
    InputComponent->BindAction("TapRight", IE_Released, this, &ARAPlayerController::OnTapRightRelease);
    InputComponent->BindAction("TapLeft", IE_Released, this, &ARAPlayerController::OnTapLeftRelease);

    // Orientation
    InputComponent->BindAxis("Turn", this, &APlayerController::AddYawInput);
    InputComponent->BindAxis("LookUp", this, &APlayerController::AddPitchInput);

    // Secondary movement
    InputComponent->BindAction("Jump", IE_Pressed, this, &ARAPlayerController::HandleJumpActionPressed);
    InputComponent->BindAction("Jump", IE_Released, this, &ARAPlayerController::HandleJumpActionReleased);

    InputComponent->BindAction("Crouch", IE_Pressed, this, &ARAPlayerController::HandleCrouchActionPressed);
    InputComponent->BindAction("Crouch", IE_Released, this, &ARAPlayerController::HandleCrouchActionReleased);

    // Combat
    InputComponent->BindAction("Attack", IE_Pressed, this, &ARAPlayerController::HandleAttackActionPressed);
    InputComponent->BindAction("Attack", IE_Released, this, &ARAPlayerController::HandleAttackActionReleased);

    InputComponent->BindAction("Throw", IE_Pressed, this, &ARAPlayerController::HandleThrowActionPressed);
    InputComponent->BindAction("Throw", IE_Released, this, &ARAPlayerController::HandleThrowActionReleased);

    // Interaction
    InputComponent->BindAction("Use", IE_Pressed, this, &ARAPlayerController::HandleUseActionPressed);
    InputComponent->BindAction("Use", IE_Released, this, &ARAPlayerController::HandleUseActionReleased);

    InputComponent->BindAction("SelectInventory0", IE_Pressed, this, &ARAPlayerController::HandleSelectInventory0ActionPressed);
    InputComponent->BindAction("SelectInventory0", IE_Released, this, &ARAPlayerController::HandleSelectInventory0ActionReleased);
    InputComponent->BindAction("SelectInventory1", IE_Pressed, this, &ARAPlayerController::HandleSelectInventory1ActionPressed);
    InputComponent->BindAction("SelectInventory1", IE_Released, this, &ARAPlayerController::HandleSelectInventory1ActionReleased);
    InputComponent->BindAction("SelectInventory2", IE_Pressed, this, &ARAPlayerController::HandleSelectInventory2ActionPressed);
    InputComponent->BindAction("SelectInventory2", IE_Released, this, &ARAPlayerController::HandleSelectInventory2ActionReleased);
    InputComponent->BindAction("SelectInventory3", IE_Pressed, this, &ARAPlayerController::HandleSelectInventory3ActionPressed);
    InputComponent->BindAction("SelectInventory3", IE_Released, this, &ARAPlayerController::HandleSelectInventory3ActionReleased);
    InputComponent->BindAction("SelectInventory4", IE_Pressed, this, &ARAPlayerController::HandleSelectInventory4ActionPressed);
    InputComponent->BindAction("SelectInventory4", IE_Released, this, &ARAPlayerController::HandleSelectInventory4ActionReleased);

    // Camera
    InputComponent->BindAction("CameraIn", IE_Pressed, this, &ARAPlayerController::HandleCameraInActionPressed);
    InputComponent->BindAction("CameraIn", IE_Released, this, &ARAPlayerController::HandleCameraInActionReleased);
    InputComponent->BindAction("CameraOut", IE_Pressed, this, &ARAPlayerController::HandleCameraOutActionPressed);
    InputComponent->BindAction("CameraOut", IE_Released, this, &ARAPlayerController::HandleCameraOutActionReleased);

    // HUD
    InputComponent->BindAction("ShowScores", IE_Pressed, this, &ARAPlayerController::HandleActionShowScoresPressed);
    InputComponent->BindAction("ShowScores", IE_Released, this, &ARAPlayerController::HandleActionShowScoresReleased);

    InputComponent->BindAction("Say", IE_Pressed, this, &ARAPlayerController::HandleActionSayPressed);
    InputComponent->BindAction("Say", IE_Released, this, &ARAPlayerController::HandleActionSayReleased);
    InputComponent->BindAction("TeamSay", IE_Pressed, this, &ARAPlayerController::HandleActionTeamSayPressed);
    InputComponent->BindAction("TeamSay", IE_Released, this, &ARAPlayerController::HandleActionTeamSayReleased);
}

void ARAPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (OnClientReceivePlayerState.IsBound())
    {
        OnClientReceivePlayerState.Broadcast(PlayerState);
    }
}

bool ARAPlayerController::CheckIsDead()
{
    ARACharacter* TempCharacter = Cast<ARACharacter>(GetCharacter());
    if (TempCharacter == nullptr)
    {
        return true;
    }

    return TempCharacter->CheckIsDead();
}

const uint8 ARAPlayerController::GetTeamIndex()
{
    ARAPlayerState* TempPlayerState = GetPlayerState<ARAPlayerState>();
    if (TempPlayerState != nullptr)
    {
        ARATeamInfo* TempTeamInfo = TempPlayerState->GetTeam();
        if (TempTeamInfo != nullptr)
        {
            return TempTeamInfo->GetTeamIndex();
        }
    }
    return 255;
}

//void ARAPlayerController::ClientSetUserWidget_Implementation(TSubclassOf<class UUserWidget> NewUserWidgetClass)
//{
//    UserWidgetClass = NewUserWidgetClass;
//    CreateHUD();
//}

//void ARAPlayerController::CreateHUD()
//{
//    if (UserWidget != nullptr)
//    {
//        return;
//    }
//
//    if (!IsLocalPlayerController() || UserWidgetClass == nullptr)
//    {
//        return;
//    }
//
//    UserWidget = CreateWidget<UUserWidget>(this, UserWidgetClass);
//    UserWidget->AddToViewport();
//}

void ARAPlayerController::SetPawn(APawn* InPawn)
{
    if (InPawn == NULL)
    {
        // Attempt to move the PC to the current camera location if no pawn was specified
        const FVector NewLocation = (PlayerCameraManager != NULL) ? PlayerCameraManager->GetCameraLocation() : GetSpawnLocation();
        SetSpawnLocation(NewLocation);
    }

    AController::SetPawn(InPawn);
    RACharacter = Cast<ARACharacter>(InPawn);
}

void ARAPlayerController::MoveForward(float Value)
{
    if (Value != 0.0f && RACharacter != NULL)
    {
        RACharacter->MoveForward(Value);
    }
}

void ARAPlayerController::MoveBackward(float Value)
{
    MoveForward(Value * -1.0f);
}

void ARAPlayerController::MoveRight(float Value)
{
    if (Value != 0.0f && RACharacter != NULL)
    {
        RACharacter->MoveRight(Value);
    }
}

void ARAPlayerController::MoveLeft(float Value)
{
    MoveRight(Value * -1.0f);
}

void ARAPlayerController::MoveUp(float Value)
{
    if (Value != 0.0f && RACharacter != NULL)
    {
        RACharacter->MoveUp(Value);
    }
}

void ARAPlayerController::MoveDown(float Value)
{
    MoveUp(Value * -1.0f);
}

void ARAPlayerController::OnTapForward()
{
    LastTapBackwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapRightTime = DOUBLE_TAP_RESET_TIME;
    LastTapLeftTime = DOUBLE_TAP_RESET_TIME;
    CheckDoubleTap(LastTapForwardTime, true, false, false, false);
    LastTapForwardTime = GetWorld()->GetRealTimeSeconds();
}

void ARAPlayerController::OnTapBackward()
{
    LastTapForwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapRightTime = DOUBLE_TAP_RESET_TIME;
    LastTapLeftTime = DOUBLE_TAP_RESET_TIME;
    CheckDoubleTap(LastTapBackwardTime, false, true, false, false);
    LastTapBackwardTime = GetWorld()->GetRealTimeSeconds();
}

void ARAPlayerController::OnTapRight()
{
    LastTapForwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapBackwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapLeftTime = DOUBLE_TAP_RESET_TIME;
    CheckDoubleTap(LastTapRightTime, false, false, true, false);
    LastTapRightTime = GetWorld()->GetRealTimeSeconds();
}

void ARAPlayerController::OnTapLeft()
{
    LastTapForwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapBackwardTime = DOUBLE_TAP_RESET_TIME;
    LastTapRightTime = DOUBLE_TAP_RESET_TIME;
    CheckDoubleTap(LastTapLeftTime, false, false, false, true);
    LastTapLeftTime = GetWorld()->GetRealTimeSeconds();
}

void ARAPlayerController::OnTapForwardRelease()
{}

void ARAPlayerController::OnTapBackwardRelease()
{}

void ARAPlayerController::OnTapRightRelease()
{}

void ARAPlayerController::OnTapLeftRelease()
{}

void ARAPlayerController::CheckDoubleTap(float LastTapTime, bool bForward, bool bBackward, bool bRight, bool bLeft)
{
    if (RACharacter != NULL && !IsMoveInputIgnored())
    {
        if (GetWorld()->GetRealTimeSeconds() - LastTapTime < MaxDoubleTapTime)
        {
            RACharacter->DoubleTap(bForward, bBackward, bRight, bLeft);
        }
    }
}

void ARAPlayerController::HandleCameraInActionPressed()
{
    if (RACharacter != NULL)
    {
        RACharacter->ControllerCameraInActionPressed();
    }
}

void ARAPlayerController::HandleCameraInActionReleased()
{
    if (RACharacter != NULL)
    {
        RACharacter->ControllerCameraInActionReleased();
    }
}

void ARAPlayerController::HandleCameraOutActionPressed()
{
    if (RACharacter != NULL)
    {
        RACharacter->ControllerCameraOutActionPressed();
    }
}

void ARAPlayerController::HandleCameraOutActionReleased()
{
    if (RACharacter != NULL)
    {
        RACharacter->ControllerCameraOutActionReleased();
    }
}

void ARAPlayerController::HandleActionShowScoresPressed()
{
    bShowScores = true;
    ARAHUDBase* RAHUDBase = GetHUD<ARAHUDBase>();
    if (RAHUDBase != nullptr)
    {
        RAHUDBase->ReceiveShowScores(bShowScores);
    }
}

void ARAPlayerController::HandleActionShowScoresReleased()
{
    bShowScores = false;
    ARAHUDBase* RAHUDBase = GetHUD<ARAHUDBase>();
    if (RAHUDBase != nullptr)
    {
        RAHUDBase->ReceiveShowScores(bShowScores);
    }
}

void ARAPlayerController::HandleActionSayPressed()
{
    ARAHUDBase* RAHUDBase = GetHUD<ARAHUDBase>();
    if (RAHUDBase != nullptr)
    {
        RAHUDBase->ReceiveActionSay();
    }
}

void ARAPlayerController::HandleActionSayReleased()
{}

void ARAPlayerController::HandleActionTeamSayPressed()
{
    HandleActionSayPressed();
}

void ARAPlayerController::HandleActionTeamSayReleased()
{}

void ARAPlayerController::HandleJumpActionPressed()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerJumpActionPressed();
    }
}

void ARAPlayerController::HandleJumpActionReleased()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerJumpActionReleased();
    }
}

void ARAPlayerController::HandleCrouchActionPressed()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerCrouchActionPressed();
    }
}

void ARAPlayerController::HandleCrouchActionReleased()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerCrouchActionReleased();
    }
}

void ARAPlayerController::HandleAttackActionPressed()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerAttackActionPressed();
    }
    else
    {
        ServerRestartPlayer();
    }
}

void ARAPlayerController::HandleAttackActionReleased()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerAttackActionReleased();
    }
}

void ARAPlayerController::HandleThrowActionPressed()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerThrowActionPressed();
    }
}

void ARAPlayerController::HandleThrowActionReleased()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerThrowActionReleased();
    }
}

void ARAPlayerController::HandleUseActionPressed()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerUseActionPressed();
    }
}

void ARAPlayerController::HandleUseActionReleased()
{
    if (RACharacter != nullptr)
    {
        RACharacter->ControllerUseActionReleased();
    }
}

void ARAPlayerController::HandleSelectInventory0ActionPressed()   { HandleSelectInventoryActionPressed(0); }
void ARAPlayerController::HandleSelectInventory0ActionReleased()  { HandleSelectInventoryActionReleased(0); }
void ARAPlayerController::HandleSelectInventory1ActionPressed()   { HandleSelectInventoryActionPressed(1); }
void ARAPlayerController::HandleSelectInventory1ActionReleased()  { HandleSelectInventoryActionReleased(1); }
void ARAPlayerController::HandleSelectInventory2ActionPressed()   { HandleSelectInventoryActionPressed(2); }
void ARAPlayerController::HandleSelectInventory2ActionReleased()  { HandleSelectInventoryActionReleased(2); }
void ARAPlayerController::HandleSelectInventory3ActionPressed()   { HandleSelectInventoryActionPressed(3); }
void ARAPlayerController::HandleSelectInventory3ActionReleased()  { HandleSelectInventoryActionReleased(3); }
void ARAPlayerController::HandleSelectInventory4ActionPressed()   { HandleSelectInventoryActionPressed(4); }
void ARAPlayerController::HandleSelectInventory4ActionReleased()  { HandleSelectInventoryActionReleased(4); }

void ARAPlayerController::HandleSelectInventoryActionPressed(int InventoryCode)
{
    if (RACharacter != nullptr)
    {
        RACharacter->SelectInventory(InventoryCode);
    }
}

void ARAPlayerController::HandleSelectInventoryActionReleased(int InventoryCode)
{
}

void ARAPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    if (GetLocalRole() != ROLE_Authority)
    {
        OnClientPossess(P);
    }

    ARACharacter* CharacterTemp = Cast<ARACharacter>(P);
    if (CharacterTemp != nullptr)
    {
        UAbilitySystemComponent* ASC = nullptr;
        ASC = CharacterTemp->GetAbilitySystemComponent();
        if (ASC != nullptr)
        {
            ASC->InitAbilityActorInfo(CharacterTemp, CharacterTemp);
        }
    }

    if (OnPawnHudContextChange.IsBound())
    {
        OnPawnHudContextChange.Broadcast(P);
    }
}

void ARAPlayerController::ChangeTeam(uint8 TeamIndex)
{
    ARAPlayerState* RAPlayerState = GetPlayerState<ARAPlayerState>();
    if (RAPlayerState != nullptr)
    {
        RAPlayerState->Server_RequestChangeTeam(TeamIndex);
    }
}

void ARAPlayerController::Suicide()
{
    ServerSuicide();
}

void ARAPlayerController::ServerSuicide_Implementation()
{
    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();

    if (GameMode != nullptr && GameMode->AllowSuicideBy(this))
    {
        ARACharacter* OwnedCharacter = GetPawn<ARACharacter>();
        if (OwnedCharacter != nullptr)
        {
            OwnedCharacter->Died(this);
        }
    }
}

bool ARAPlayerController::ServerSuicide_Validate()
{
    return true;
}

const int32 MAX_CHAT_TEXT_SIZE = 384;

bool ARAPlayerController::CheckSayMessageAllowed(const FString& Message)
{
    if (Message.IsEmpty())
    {
        return false;
    }

    return true;
}

void ARAPlayerController::Say(FString Message)
{
    FString TrimmedMessage = Message.TrimStartAndEnd();
    TrimmedMessage = TrimmedMessage.Left(MAX_CHAT_TEXT_SIZE);
    if (CheckSayMessageAllowed(TrimmedMessage))
    {
        ServerSay(TrimmedMessage, false);
    }
}

void ARAPlayerController::TeamSay(FString Message)
{
    FString TrimmedMessage = Message.TrimStartAndEnd();
    TrimmedMessage = TrimmedMessage.Left(MAX_CHAT_TEXT_SIZE);
    if (CheckSayMessageAllowed(TrimmedMessage))
    {
        ServerSay(TrimmedMessage, true);
    }
}

void ARAPlayerController::ServerSay_Implementation(const FString& Message, bool bTeamMessage)
{
    if(!CheckSayMessageAllowed(Message))
    {
        return;
    }

    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->PlayerSay(this, Message, bTeamMessage);
    }
}

void ARAPlayerController::ClientSay_Implementation(ARAPlayerState* SendingPlayerState, const FString& Message)
{
    FClientReceiveData ClientReceiveData;
    ClientReceiveData.LocalPC = this;
    ClientReceiveData.MessageString = Message;
    ClientReceiveData.RelatedPlayerState_1 = SendingPlayerState;

    URAChatMessage::StaticClass()->GetDefaultObject<URAChatMessage>()->ClientReceiveChat(ClientReceiveData);
}

void ARAPlayerController::ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime)
{
    ClientSay(Cast<ARAPlayerState>(SenderPlayerState), S);
}

void ARAPlayerController::ClientBroadcast_Implementation(ARAPlayerState* SendingPlayerState, const FString& Message, FName Type)
{
    FClientReceiveData ClientReceiveData;
    ClientReceiveData.LocalPC = this;
    ClientReceiveData.MessageString = Message;
    ClientReceiveData.MessageType = Type;
    ClientReceiveData.RelatedPlayerState_1 = SendingPlayerState;

    URALocalMessage::StaticClass()->GetDefaultObject<URALocalMessage>()->ClientReceive(ClientReceiveData);
}

/** ADMIN INTERFACE */
void ARAPlayerController::RconAuth(FString RconPassword)
{
    ServerRconAuth(RconPassword);
}

void ARAPlayerController::ServerRconAuth_Implementation(const FString& RconPassword)
{
    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->RconAuth(this, RconPassword);
    }
}

void ARAPlayerController::RconNormal()
{
    ServerRconNormal();
}

void ARAPlayerController::ServerRconNormal_Implementation()
{
    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->RconNormal(this);
    }
}

void ARAPlayerController::TeamSize(int32 NewTeamSize)
{
    ServerTeamSize(NewTeamSize);
}

void ARAPlayerController::ServerTeamSize_Implementation(int32 NewTeamSize)
{
    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->Rcon_TeamSize(this, NewTeamSize);
    }
}

void ARAPlayerController::TimeLimit(float NewtimeLimitMinutes)
{
    ServerTimeLimit(NewtimeLimitMinutes);
}

void ARAPlayerController::ServerTimeLimit_Implementation(float NewtimeLimitMinutes)
{
    ARAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ARAGameModeBase>();
    if (GameMode != nullptr)
    {
        GameMode->Rcon_TimeLimit(this, NewtimeLimitMinutes);
    }
}
/** END ADMIN INTERFACE */