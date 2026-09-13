#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CBPlayerController.generated.h"

class UCBMainMenuWidget;
class UCBLoadingWidget;
class UCBPromptWidget;
class ACBTrain;

UCLASS()
class CORRUPTEDBACKROOMS_API ACBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACBPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void StartGameFromMenu();
	void HandleInteract();
	void ShowLoadingScreen();

	void SetMouseSensitivity(float InSensitivity) { MouseSensitivity = FMath::Clamp(InSensitivity, 0.2f, 2.5f); }
	float GetMouseSensitivity() const { return MouseSensitivity; }

	void SetMenuCamera(AActor* CameraActor);

protected:
	void RefreshPrompt();

	UPROPERTY()
	UCBMainMenuWidget* MenuWidget = nullptr;

	UPROPERTY()
	UCBLoadingWidget* LoadingWidget = nullptr;

	UPROPERTY()
	UCBPromptWidget* PromptWidget = nullptr;

	UPROPERTY()
	AActor* MenuCamera = nullptr;

	float MouseSensitivity = 1.f;
	bool bInMenu = true;
};
