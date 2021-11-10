// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ERAAbilityInputID : uint8
{
	None	UMETA(DisplayName = "None"),
	Confirm	UMETA(DisplayName = "Confirm"),
	Cancel	UMETA(DisplayName = "Cancel"),
	Attack	UMETA(DisplayName = "Attack"),
	Use		UMETA(DisplayName = "Use"),
	Jump	UMETA(DisplayName = "Jump")
};