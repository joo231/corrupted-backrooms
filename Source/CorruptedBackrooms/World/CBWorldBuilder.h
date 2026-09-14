#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;
class ACBTrain;

struct FCBHubBuildResult
{
	ACBTrain* Train = nullptr;
	AActor* MenuCamera = nullptr;
	AActor* PlayerStart = nullptr;
};

struct CBWorldBuilder
{
	static void ClearTemplateActors(UWorld* World);
	static FCBHubBuildResult FindHub(UWorld* World);
	static FCBHubBuildResult BuildHub(UWorld* World);
	static void BuildArrival(UWorld* World);
};
