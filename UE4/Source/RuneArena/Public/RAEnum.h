#pragma once

/** Five-way directional enum used throughout game classes */
UENUM(BlueprintType)
enum class ERADirection : uint8
{
	Neutral	UMETA(DisplayName = "Neutral"),
	Front	UMETA(DisplayName = "Front"),
	Back	UMETA(DisplayName = "Back"),
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right")
};

///** General movement state enum used throughout game classes */
//UENUM(BlueprintType)
//enum class ERACharacterState : uint8
//{
//	Neutral		UMETA(DisplayName = "Neutral"),
//	Crouching	UMETA(DisplayName = "Crouching"),
//	Falling		UMETA(DisplayName = "Falling"),
//	Dodging		UMETA(DisplayName = "Dodging")
//};

/** Equipable hand enum used extensively throughout item and inventory interaction */
UENUM(BlueprintType)
enum class ERAEquipableHand : uint8
{
	MainHand	UMETA(DisplayName = "Main-Hand"),
	OffHand		UMETA(DisplayName = "Off-Hand"),
	Defense		UMETA(DisplayName = "Defense")
};