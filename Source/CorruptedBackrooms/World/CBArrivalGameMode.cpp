#include "World/CBArrivalGameMode.h"

#include "Player/CBPlayerCharacter.h"
#include "Player/CBPlayerController.h"
#include "World/CBWorldBuilder.h"

ACBArrivalGameMode::ACBArrivalGameMode()
{
	PlayerControllerClass = ACBPlayerController::StaticClass();
	DefaultPawnClass = ACBPlayerCharacter::StaticClass();
	bStartPlayersAsSpectators = false;
}

void ACBArrivalGameMode::StartPlay()
{
	CBWorldBuilder::ClearTemplateActors(GetWorld());
	CBWorldBuilder::BuildArrival(GetWorld());
	Super::StartPlay();
}
