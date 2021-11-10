// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/RAShield.h"

// Sets default values
ARAShield::ARAShield(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//InventoryType = ERAInventoryType::Defense;
}

// Called when the game starts or when spawned
void ARAShield::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARAShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}