#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CBPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class CORRUPTEDBACKROOMS_API UCBPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPrompt(const FText& InText);
	void SetPromptVisible(bool bVisible);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	UTextBlock* PromptText = nullptr;

	bool bLayoutBuilt = false;
};
