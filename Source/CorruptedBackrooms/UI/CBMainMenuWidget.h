#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CBMainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class UWidget;
class ACBPlayerController;

UCLASS()
class CORRUPTEDBACKROOMS_API UCBMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOwnerController(ACBPlayerController* InController);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnSensitivityChanged(float Value);

private:
	void BuildLayout();
	UButton* MakeMenuButton(const FName& Name, const FString& Label, const FLinearColor& Color);

	UPROPERTY()
	UWidget* MainPanel = nullptr;

	UPROPERTY()
	UWidget* SettingsPanel = nullptr;

	UPROPERTY()
	UTextBlock* SensitivityValueText = nullptr;

	TWeakObjectPtr<ACBPlayerController> OwnerController;
	bool bLayoutBuilt = false;
};
