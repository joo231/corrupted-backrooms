#pragma once

#include "CoreMinimal.h"
#include "CorruptedBackroomsGameModeBase.h"
#include "CBArrivalGameMode.generated.h"

UCLASS()
class CORRUPTEDBACKROOMS_API ACBArrivalGameMode : public ACorruptedBackroomsGameModeBase
{
	GENERATED_BODY()

public:
	ACBArrivalGameMode();

	virtual void StartPlay() override;
};
