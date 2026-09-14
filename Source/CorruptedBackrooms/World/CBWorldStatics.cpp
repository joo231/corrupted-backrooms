#include "World/CBWorldStatics.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "World/CBHubGameMode.h"
#include "World/CBWorldBuilder.h"

int32 UCBWorldStatics::RebuildHub(UObject* WorldContextObject)
{
	UWorld* World = Cast<UWorld>(WorldContextObject);
	if (!World && WorldContextObject)
	{
		World = WorldContextObject->GetWorld();
	}
	if (!World)
	{
		return 0;
	}

	CBWorldBuilder::ClearTemplateActors(World);
	CBWorldBuilder::BuildHub(World);
	if (AWorldSettings* Settings = World->GetWorldSettings())
	{
		Settings->DefaultGameMode = ACBHubGameMode::StaticClass();
	}

	int32 Count = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		++Count;
	}
	return Count;
}
