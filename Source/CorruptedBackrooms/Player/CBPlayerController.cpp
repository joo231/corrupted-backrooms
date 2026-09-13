#include "Player/CBPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"
#include "UI/CBLoadingWidget.h"
#include "UI/CBMainMenuWidget.h"
#include "UI/CBPromptWidget.h"
#include "World/CBHubGameMode.h"
#include "World/CBTrain.h"

ACBPlayerController::ACBPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ACBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PromptWidget = CreateWidget<UCBPromptWidget>(this, UCBPromptWidget::StaticClass());
	if (PromptWidget)
	{
		PromptWidget->AddToViewport(20);
		PromptWidget->SetPromptVisible(false);
	}

	if (GetWorld() && GetWorld()->GetAuthGameMode<ACBHubGameMode>())
	{
		bInMenu = true;
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		MenuWidget = CreateWidget<UCBMainMenuWidget>(this, UCBMainMenuWidget::StaticClass());
		if (MenuWidget)
		{
			MenuWidget->SetOwnerController(this);
			MenuWidget->AddToViewport(10);
			InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		if (MenuCamera)
		{
			SetViewTarget(MenuCamera);
		}
	}
	else
	{
		bInMenu = false;
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}

void ACBPlayerController::SetMenuCamera(AActor* CameraActor)
{
	MenuCamera = CameraActor;
	if (bInMenu && MenuCamera)
	{
		SetViewTarget(MenuCamera);
	}
}

void ACBPlayerController::StartGameFromMenu()
{
	if (!bInMenu)
	{
		return;
	}
	bInMenu = false;

	if (MenuWidget)
	{
		MenuWidget->RemoveFromParent();
		MenuWidget = nullptr;
	}

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (ACBHubGameMode* GM = GetWorld()->GetAuthGameMode<ACBHubGameMode>())
	{
		GM->StartExploration();
	}
}

void ACBPlayerController::HandleInteract()
{
	if (bInMenu)
	{
		return;
	}

	if (ACBHubGameMode* GM = GetWorld()->GetAuthGameMode<ACBHubGameMode>())
	{
		if (ACBTrain* Train = GM->GetTrain())
		{
			Train->TryInteract(GetPawn());
		}
	}
}

void ACBPlayerController::ShowLoadingScreen()
{
	if (!LoadingWidget)
	{
		LoadingWidget = CreateWidget<UCBLoadingWidget>(this, UCBLoadingWidget::StaticClass());
		if (LoadingWidget)
		{
			LoadingWidget->AddToViewport(50);
		}
	}

	if (PromptWidget)
	{
		PromptWidget->SetPromptVisible(false);
	}
}

void ACBPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshPrompt();
}

void ACBPlayerController::RefreshPrompt()
{
	if (!PromptWidget || bInMenu || LoadingWidget)
	{
		return;
	}

	FText Prompt = FText::GetEmpty();
	if (ACBHubGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ACBHubGameMode>() : nullptr)
	{
		if (ACBTrain* Train = GM->GetTrain())
		{
			Prompt = Train->GetPromptText();
		}
	}

	const bool bHasPrompt = !Prompt.IsEmpty();
	PromptWidget->SetPrompt(Prompt);
	PromptWidget->SetPromptVisible(bHasPrompt);
}
