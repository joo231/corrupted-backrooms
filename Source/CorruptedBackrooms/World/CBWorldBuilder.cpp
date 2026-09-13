#include "World/CBWorldBuilder.h"

#include "Camera/CameraActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "World/CBTrain.h"

namespace
{
	AStaticMeshActor* SpawnMesh(UWorld* World, UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Location, const FRotator& Rotation, const FVector& Scale, bool bCollision = true)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, Params);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->Tags.Add(TEXT("CBBuilt"));
		UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetStaticMesh(Mesh);
		Actor->SetActorScale3D(Scale);
		if (Material)
		{
			Comp->SetMaterial(0, Material);
		}
		if (bCollision)
		{
			Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Comp->SetCollisionResponseToAllChannels(ECR_Block);
		}
		else
		{
			Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		return Actor;
	}

	UMaterialInstanceDynamic* MakeColor(UObject* Outer, UMaterialInterface* Base, const FLinearColor& Color)
	{
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		Mid->SetVectorParameterValue(TEXT("Color"), Color);
		Mid->SetVectorParameterValue(TEXT("BaseColor"), Color);
		return Mid;
	}
}

void CBWorldBuilder::ClearTemplateActors(UWorld* World)
{
	if (!World)
	{
		return;
	}

	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AGameModeBase>() || Actor->IsA<AGameStateBase>() || Actor->IsA<APlayerController>()
			|| Actor->IsA<APlayerState>() || Actor->IsA<AHUD>() || Actor->IsA<AWorldSettings>())
		{
			continue;
		}
		ToDestroy.Add(Actor);
	}

	for (AActor* Actor : ToDestroy)
	{
		Actor->Destroy();
	}
}

FCBHubBuildResult CBWorldBuilder::BuildHub(UWorld* World)
{
	FCBHubBuildResult Result;
	if (!World)
	{
		return Result;
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* SkySphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere"));
	UMaterialInterface* ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* WhiteGrid = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid"));
	UMaterialInterface* SkyClouds = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/M_Sky_Panning_Clouds2.M_Sky_Panning_Clouds2"));
	if (!SkyClouds)
	{
		SkyClouds = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/M_Sky_Panning_Clouds.M_Sky_Panning_Clouds"));
	}
	UMaterialInterface* CloudVolume = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud.m_SimpleVolumetricCloud"));

	UMaterialInterface* Marble = WhiteGrid ? WhiteGrid : ShapeMat;
	UMaterialInstanceDynamic* MarbleEdge = MakeColor(World, ShapeMat, FLinearColor(0.86f, 0.87f, 0.9f));
	UMaterialInstanceDynamic* RailMat = MakeColor(World, ShapeMat, FLinearColor(0.12f, 0.13f, 0.14f));
	UMaterialInstanceDynamic* SleeperMat = MakeColor(World, ShapeMat, FLinearColor(0.22f, 0.16f, 0.12f));
	UMaterialInstanceDynamic* ColumnMat = MakeColor(World, ShapeMat, FLinearColor(0.94f, 0.94f, 0.96f));

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-28.f, 40.f, 0.f), Params);
	if (Sun)
	{
		if (UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			SunComp->SetMobility(EComponentMobility::Movable);
			SunComp->SetIntensity(11.f);
			SunComp->SetLightColor(FLinearColor(1.f, 0.96f, 0.88f));
			SunComp->SetAtmosphereSunLight(true);
			SunComp->bUsedAsAtmosphereSunLight = true;
			SunComp->SetCastShadows(true);
		}
	}

	World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Params);

	ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (SkyLight && SkyLight->GetLightComponent())
	{
		SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
		SkyLight->GetLightComponent()->SetIntensity(1.15f);
		SkyLight->GetLightComponent()->RecaptureSky();
	}

	AVolumetricCloud* Clouds = World->SpawnActor<AVolumetricCloud>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (Clouds)
	{
		if (UVolumetricCloudComponent* CloudComp = Clouds->FindComponentByClass<UVolumetricCloudComponent>())
		{
			if (CloudVolume)
			{
				CloudComp->Material = CloudVolume;
				CloudComp->MarkRenderStateDirty();
			}
			CloudComp->LayerBottomAltitude = 3.5f;
			CloudComp->LayerHeight = 8.f;
		}
	}

	AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);
	if (Fog)
	{
		if (UExponentialHeightFogComponent* FogComp = Fog->FindComponentByClass<UExponentialHeightFogComponent>())
		{
			FogComp->SetFogDensity(0.012f);
			FogComp->SetFogHeightFalloff(0.08f);
			FogComp->SetFogInscatteringColor(FLinearColor(0.45f, 0.62f, 0.9f));
			FogComp->SetVolumetricFog(true);
			FogComp->SetVolumetricFogExtinctionScale(0.4f);
		}
	}

	if (SkySphere)
	{
		AStaticMeshActor* Sky = SpawnMesh(World, SkySphere, SkyClouds, FVector::ZeroVector, FRotator::ZeroRotator, FVector(400.f), false);
		if (Sky)
		{
			Sky->GetStaticMeshComponent()->SetCastShadow(false);
		}
	}

	APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(Params);
	if (Volume)
	{
		Volume->bUnbound = true;
		Volume->Settings.bOverride_AutoExposureMinBrightness = true;
		Volume->Settings.bOverride_AutoExposureMaxBrightness = true;
		Volume->Settings.AutoExposureMinBrightness = 1.0f;
		Volume->Settings.AutoExposureMaxBrightness = 1.0f;
		Volume->Settings.bOverride_AutoExposureBias = true;
		Volume->Settings.AutoExposureBias = 0.35f;
		Volume->Settings.bOverride_BloomIntensity = true;
		Volume->Settings.BloomIntensity = 0.35f;
	}

	// Marble platform in the sky.
	SpawnMesh(World, Cube, Marble, FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, FVector(96.f, 32.f, 0.8f));
	SpawnMesh(World, Cube, MarbleEdge, FVector(0.f, 0.f, 42.f), FRotator::ZeroRotator, FVector(97.f, 33.f, 0.08f));
	SpawnMesh(World, Cube, MarbleEdge, FVector(0.f, 0.f, -50.f), FRotator::ZeroRotator, FVector(94.f, 30.f, 0.4f));

	const FVector ColumnOffsets[] = {
		FVector(-4300.f, -1400.f, 280.f),
		FVector(4300.f, -1400.f, 280.f),
		FVector(-4300.f, 1400.f, 280.f),
		FVector(4300.f, 1400.f, 280.f)
	};
	for (const FVector& Offset : ColumnOffsets)
	{
		SpawnMesh(World, Cube, ColumnMat, Offset, FRotator::ZeroRotator, FVector(1.6f, 1.6f, 5.2f));
		SpawnMesh(World, Cube, MarbleEdge, Offset + FVector(0.f, 0.f, 280.f), FRotator::ZeroRotator, FVector(2.2f, 2.2f, 0.25f));
	}

	// Rails under the train.
	SpawnMesh(World, Cube, RailMat, FVector(200.f, 110.f, 46.f), FRotator::ZeroRotator, FVector(70.f, 0.12f, 0.08f));
	SpawnMesh(World, Cube, RailMat, FVector(200.f, -110.f, 46.f), FRotator::ZeroRotator, FVector(70.f, 0.12f, 0.08f));
	for (int32 i = 0; i < 28; ++i)
	{
		const float X = -2400.f + i * 200.f;
		SpawnMesh(World, Cube, SleeperMat, FVector(X, 0.f, 42.f), FRotator::ZeroRotator, FVector(0.28f, 3.2f, 0.08f));
	}

	if (Cylinder)
	{
		SpawnMesh(World, Cylinder, ColumnMat, FVector(-3200.f, -900.f, 90.f), FRotator::ZeroRotator, FVector(1.2f, 1.2f, 0.2f));
		SpawnMesh(World, Cylinder, ColumnMat, FVector(3200.f, 900.f, 90.f), FRotator::ZeroRotator, FVector(1.2f, 1.2f, 0.2f));
	}

	if (Plane)
	{
		SpawnMesh(World, Plane, Marble, FVector(0.f, 0.f, 41.f), FRotator::ZeroRotator, FVector(24.f, 8.f, 1.f), false);
	}

	Result.Train = World->SpawnActor<ACBTrain>(FVector(200.f, 0.f, 0.f), FRotator::ZeroRotator, Params);

	APlayerStart* Start = World->SpawnActor<APlayerStart>(FVector(0.f, -900.f, 150.f), FRotator(0.f, 90.f, 0.f), Params);
	Result.PlayerStart = Start;

	ACameraActor* Camera = World->SpawnActor<ACameraActor>(FVector(-2200.f, -2100.f, 720.f), FRotator(-12.f, 40.f, 0.f), Params);
	Result.MenuCamera = Camera;

	if (UClass* SkyBP = LoadClass<AActor>(nullptr, TEXT("/Engine/EngineSky/BP_Sky_Sphere.BP_Sky_Sphere_C")))
	{
		World->SpawnActor<AActor>(SkyBP, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	}

	return Result;
}

void CBWorldBuilder::BuildArrival(UWorld* World)
{
	if (!World)
	{
		return;
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* WallMat = MakeColor(World, ShapeMat, FLinearColor(0.72f, 0.68f, 0.42f));
	UMaterialInstanceDynamic* FloorMat = MakeColor(World, ShapeMat, FLinearColor(0.55f, 0.5f, 0.32f));
	UMaterialInstanceDynamic* CeilingMat = MakeColor(World, ShapeMat, FLinearColor(0.78f, 0.74f, 0.55f));

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADirectionalLight* Light = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-70.f, 20.f, 0.f), Params);
	if (Light)
	{
		if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Light->GetLightComponent()))
		{
			LightComp->SetIntensity(4.f);
			LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.75f));
		}
	}

	ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(Params);
	if (SkyLight && SkyLight->GetLightComponent())
	{
		SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
		SkyLight->GetLightComponent()->SetIntensity(0.6f);
		SkyLight->GetLightComponent()->RecaptureSky();
	}

	// Simple first location: a long liminal hall the train "arrives" into.
	SpawnMesh(World, Cube, FloorMat, FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, FVector(80.f, 12.f, 0.4f));
	SpawnMesh(World, Cube, CeilingMat, FVector(0.f, 0.f, 420.f), FRotator::ZeroRotator, FVector(80.f, 12.f, 0.3f));
	SpawnMesh(World, Cube, WallMat, FVector(0.f, 620.f, 210.f), FRotator::ZeroRotator, FVector(80.f, 0.4f, 4.2f));
	SpawnMesh(World, Cube, WallMat, FVector(0.f, -620.f, 210.f), FRotator::ZeroRotator, FVector(80.f, 0.4f, 4.2f));
	SpawnMesh(World, Cube, WallMat, FVector(-4000.f, 0.f, 210.f), FRotator::ZeroRotator, FVector(0.4f, 12.f, 4.2f));
	SpawnMesh(World, Cube, WallMat, FVector(4000.f, 0.f, 210.f), FRotator::ZeroRotator, FVector(0.4f, 12.f, 4.2f));

	World->SpawnActor<APlayerStart>(FVector(-3200.f, 0.f, 140.f), FRotator(0.f, 0.f, 0.f), Params);
}
