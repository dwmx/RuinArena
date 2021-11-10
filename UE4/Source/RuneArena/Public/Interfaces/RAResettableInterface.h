#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RAResettableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URAResettableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 */
class RUNEARENA_API IRAResettableInterface
{
	GENERATED_BODY()

public:
	/** Perform whatever reset functionality is valid to the actor */
	virtual void Reset() = 0;
};