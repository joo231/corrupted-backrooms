#pragma once

#include "CoreMinimal.h"
#include "CorruptedBackroomsGameModeBase.h"
#include "CBHubGameMode.generated.h"

class ACBTrain;

UCLASS()
class CORRUPTEDBACKROOMS_API ACBHubGameMode : public ACorruptedBackroomsGameModeBase
{
	GENERATED_BODY()

public:
	ACBHubGameMode();

	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void StartExploration();
	ACBTrain* GetTrain() const { return Train; }

protected:
	UPROPERTY()
	ACBTrain* Train = nullptr;

	UPROPERTY()
	AActor* MenuCamera = nullptr;

	bool bGameStarted = false;
};
