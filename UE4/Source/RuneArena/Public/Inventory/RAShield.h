// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RANewInventory.h"
#include "RAShield.generated.h"

UCLASS()
class RUNEARENA_API ARAShield : public ARANewInventory
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ARAShield(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
