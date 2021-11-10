#pragma once

#include "CoreMinimal.h"
#include "RASpawnerState.generated.h"

UCLASS(DefaultToInstanced, EditInlineNew, CustomConstructor, Within=RASpawner)
class URASpawnerState : public UObject
{
	GENERATED_BODY()

	friend class ARASpawner;

protected:
	class ARASpawner* GetSpawner();

	virtual void BeginState();
	virtual void EndState();

public:
	URASpawnerState(const FObjectInitializer& ObjectInitializer);
};