#include "World/CBHubGameMode.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"
#include "Player/CBPlayerController.h"
#include "World/CBTrain.h"
#include "World/CBWorldBuilder.h"

ACBHubGameMode::ACBHubGameMode()
{
	PlayerControllerClass = ACBPlayerController::StaticClass();
	DefaultPawnClass = ACBPlayerCharacter::StaticClass();
	bStartPlayersAsSpectators = true;
}

void ACBHubGameMode::StartPlay()
{
	CBWorldBuilder::ClearTemplateActors(GetWorld());
	const FCBHubBuildResult Hub = CBWorldBuilder::BuildHub(GetWorld());
	Train = Hub.Train;
	MenuCamera = Hub.MenuCamera;
	Super::StartPlay();

	if (ACBPlayerController* PC = Cast<ACBPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->SetMenuCamera(MenuCamera);
	}
}

void ACBHubGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!bGameStarted)
	{
		if (ACBPlayerController* PC = Cast<ACBPlayerController>(NewPlayer))
		{
			PC->SetMenuCamera(MenuCamera);
		}
		return;
	}

	Super::RestartPlayer(NewPlayer);
}

void ACBHubGameMode::StartExploration()
{
	if (bGameStarted)
	{
		return;
	}
	bGameStarted = true;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	PC->ChangeState(NAME_Playing);
	RestartPlayer(PC);
}
