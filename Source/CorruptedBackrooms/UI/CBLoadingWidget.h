#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CBLoadingWidget.generated.h"

UCLASS()
class CORRUPTEDBACKROOMS_API UCBLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	bool bLayoutBuilt = false;
};
