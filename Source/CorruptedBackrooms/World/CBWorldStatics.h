#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBWorldStatics.generated.h"

UCLASS()
class CORRUPTEDBACKROOMS_API UCBWorldStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CB")
	static int32 RebuildHub(UObject* WorldContextObject);
};
