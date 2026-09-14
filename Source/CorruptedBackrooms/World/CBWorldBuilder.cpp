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
#include "World/CBStairStream.h"
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
		Actor->SetFlags(RF_Transactional);
		Actor->ClearFlags(RF_Transient);
		UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
		Comp->SetMobility(EComponentMobility::Static);
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

	AStaticMeshActor* SpawnProp(UWorld* World, UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& FloorPos, const FRotator& Rotation, float DesiredHeight)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		const float Height = FMath::Max(1.f, Bounds.BoxExtent.Z * 2.f);
		const float Scale = DesiredHeight / Height;
		FVector Location = FloorPos;
		const float Bottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
		Location.Z -= Bottom * Scale;
		return SpawnMesh(World, Mesh, Material, Location, Rotation, FVector(Scale), true);
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

FCBHubBuildResult CBWorldBuilder::FindHub(UWorld* World)
{
	FCBHubBuildResult Result;
	if (!World)
	{
		return Result;
	}

	for (TActorIterator<ACBTrain> It(World); It; ++It)
	{
		Result.Train = *It;
		break;
	}

	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		if (It->ActorHasTag(TEXT("CBMenuCamera")))
		{
			Result.MenuCamera = *It;
			break;
		}
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		Result.PlayerStart = *It;
		break;
	}

	return Result;
}

FCBHubBuildResult CBWorldBuilder::BuildHub(UWorld* World)
{
	FCBHubBuildResult Result;
	if (!World)
	{
		return Result;
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* SkySphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere"));
	UMaterialInterface* ShapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* SkyClouds = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/M_Sky_Panning_Clouds2.M_Sky_Panning_Clouds2"));
	if (!SkyClouds)
	{
		SkyClouds = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/M_Sky_Panning_Clouds.M_Sky_Panning_Clouds"));
	}
	UMaterialInterface* CloudVolume = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud.m_SimpleVolumetricCloud"));

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AWorldSettings* Settings = World->GetWorldSettings())
	{
		Settings->KillZ = -100000.f;
		Settings->bEnableWorldBoundsChecks = false;
	}

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
			FogComp->SetFogDensity(0.022f);
			FogComp->SetFogHeightFalloff(0.045f);
			FogComp->SetFogInscatteringColor(FLinearColor(0.62f, 0.72f, 0.88f));
			FogComp->SetVolumetricFog(true);
			FogComp->SetVolumetricFogExtinctionScale(0.7f);
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
		Volume->Settings.BloomIntensity = 0.55f;
		Volume->Settings.bOverride_IndirectLightingIntensity = true;
		Volume->Settings.IndirectLightingIntensity = 1.15f;
	}

	auto LoadMesh = [](const TCHAR* Path) -> UStaticMesh*
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	};
	auto LoadMat = [](const TCHAR* Path) -> UMaterialInterface*
	{
		return LoadObject<UMaterialInterface>(nullptr, Path);
	};

	UStaticMesh* FloorMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanFloor.SM_RomanFloor"));
	UStaticMesh* ColumnMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanColumn.SM_RomanColumn"));
	UStaticMesh* EntablatureMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanEntablature.SM_RomanEntablature"));
	UStaticMesh* PedestalMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanPedestal.SM_RomanPedestal"));
	UStaticMesh* BalusterMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanBaluster.SM_RomanBaluster"));
	UStaticMesh* StepMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanStep.SM_RomanStep"));
	UStaticMesh* RoofMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanRoof.SM_RomanRoof"));
	UStaticMesh* PedimentMesh = LoadMesh(TEXT("/Game/Art/Kit/SM_RomanPediment.SM_RomanPediment"));
	UStaticMesh* BustMesh = LoadMesh(TEXT("/Game/Art/Meshes/marble_bust_01.marble_bust_01"));
	UStaticMesh* HorseMesh = LoadMesh(TEXT("/Game/Art/Meshes/horse_statue_01.horse_statue_01"));
	UStaticMesh* LionMesh = LoadMesh(TEXT("/Game/Art/Meshes/lion_head.lion_head"));
	UStaticMesh* HorseHeadMesh = LoadMesh(TEXT("/Game/Art/Meshes/horse_head.horse_head"));
	UStaticMesh* BullHeadMesh = LoadMesh(TEXT("/Game/Art/Meshes/bull_head.bull_head"));
	UStaticMesh* VaseAMesh = LoadMesh(TEXT("/Game/Art/Meshes/antique_ceramic_vase_01.antique_ceramic_vase_01"));
	UStaticMesh* VaseBMesh = LoadMesh(TEXT("/Game/Art/Meshes/ceramic_vase_02.ceramic_vase_02"));
	UStaticMesh* BrassVaseMesh = LoadMesh(TEXT("/Game/Art/Meshes/brass_vase_01.brass_vase_01"));
	UMaterialInterface* RomanMarble = LoadMat(TEXT("/Game/Art/Materials/M_RomanMarble.M_RomanMarble"));
	UMaterialInterface* BustMat = LoadMat(TEXT("/Game/Art/Materials/M_marble_bust_01.M_marble_bust_01"));
	UMaterialInterface* HorseMat = LoadMat(TEXT("/Game/Art/Materials/M_horse_statue_01.M_horse_statue_01"));
	UMaterialInterface* LionMat = LoadMat(TEXT("/Game/Art/Materials/M_lion_head.M_lion_head"));
	UMaterialInterface* HorseHeadMat = LoadMat(TEXT("/Game/Art/Materials/M_horse_head.M_horse_head"));
	UMaterialInterface* BullHeadMat = LoadMat(TEXT("/Game/Art/Materials/M_bull_head.M_bull_head"));
	UMaterialInterface* VaseAMat = LoadMat(TEXT("/Game/Art/Materials/M_antique_ceramic_vase_01.M_antique_ceramic_vase_01"));
	UMaterialInterface* VaseBMat = LoadMat(TEXT("/Game/Art/Materials/M_ceramic_vase_02.M_ceramic_vase_02"));
	UMaterialInterface* BrassVaseMat = LoadMat(TEXT("/Game/Art/Materials/M_brass_vase_01.M_brass_vase_01"));
	if (!RomanMarble)
	{
		RomanMarble = LoadMat(TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid"));
	}

	const float FloorZ = 0.f;
	if (FloorMesh)
	{
		SpawnMesh(World, FloorMesh, RomanMarble, FVector(0.f, 0.f, FloorZ), FRotator::ZeroRotator, FVector(1.f));
	}
	else if (Cube)
	{
		SpawnMesh(World, Cube, RomanMarble, FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, FVector(80.f, 22.f, 0.6f));
	}

	if (StepMesh)
	{
		SpawnMesh(World, StepMesh, RomanMarble, FVector(0.f, -1220.f, 0.f), FRotator::ZeroRotator, FVector(1.f));
		SpawnMesh(World, StepMesh, RomanMarble, FVector(0.f, 1220.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(1.f));
	}

	const float ColY[2] = { -980.f, 980.f };
	const float ColStartX = -3600.f;
	const float ColStepX = 450.f;
	const int32 ColCount = 17;
	const float ColumnTopZ = 536.f;

	if (ColumnMesh)
	{
		for (float Y : ColY)
		{
			for (int32 i = 0; i < ColCount; ++i)
			{
				const float X = ColStartX + i * ColStepX;
				SpawnMesh(World, ColumnMesh, RomanMarble, FVector(X, Y, FloorZ + 30.f), FRotator::ZeroRotator, FVector(1.f));
			}
		}
	}

	if (EntablatureMesh)
	{
		for (float Y : ColY)
		{
			for (int32 i = 0; i < ColCount - 1; ++i)
			{
				const float X = ColStartX + (i + 0.5f) * ColStepX;
				SpawnMesh(World, EntablatureMesh, RomanMarble, FVector(X, Y, FloorZ + 30.f + ColumnTopZ), FRotator::ZeroRotator, FVector(1.f));
			}
		}
	}

	if (BalusterMesh)
	{
		for (float Y : ColY)
		{
			for (int32 i = 0; i < 48; ++i)
			{
				const float X = -3400.f + i * 150.f;
				SpawnMesh(World, BalusterMesh, RomanMarble, FVector(X, Y + FMath::Sign(Y) * 80.f, FloorZ + 30.f), FRotator::ZeroRotator, FVector(1.f));
			}
		}
	}

	const float DeckZ = FloorZ + 30.f;
	if (RoofMesh)
	{
		for (int32 i = 0; i < ColCount - 1; ++i)
		{
			const float X = ColStartX + (i + 0.5f) * ColStepX;
			SpawnMesh(World, RoofMesh, RomanMarble, FVector(X, 0.f, DeckZ + ColumnTopZ + 72.f), FRotator::ZeroRotator, FVector(1.f));
		}
	}
	if (PedimentMesh)
	{
		SpawnMesh(World, PedimentMesh, RomanMarble, FVector(ColStartX - 40.f, 0.f, DeckZ + ColumnTopZ + 40.f), FRotator(0.f, 90.f, 0.f), FVector(3.6f, 1.f, 1.6f));
		SpawnMesh(World, PedimentMesh, RomanMarble, FVector(ColStartX + (ColCount - 1) * ColStepX + 40.f, 0.f, DeckZ + ColumnTopZ + 40.f), FRotator(0.f, -90.f, 0.f), FVector(3.6f, 1.f, 1.6f));
	}
	if (PedestalMesh && BustMesh)
	{
		const float BustXs[] = { -2700.f, -900.f, 900.f, 2700.f };
		for (float X : BustXs)
		{
			SpawnMesh(World, PedestalMesh, RomanMarble, FVector(X, -700.f, DeckZ), FRotator::ZeroRotator, FVector(1.f));
			SpawnProp(World, BustMesh, BustMat, FVector(X, -700.f, DeckZ + 110.f), FRotator(0.f, 180.f, 0.f), 95.f);
			SpawnMesh(World, PedestalMesh, RomanMarble, FVector(X, 700.f, DeckZ), FRotator::ZeroRotator, FVector(1.f));
			SpawnProp(World, BustMesh, BustMat, FVector(X, 700.f, DeckZ + 110.f), FRotator::ZeroRotator, 95.f);
		}
	}

	if (HorseMesh)
	{
		SpawnProp(World, HorseMesh, HorseMat, FVector(3400.f, 0.f, DeckZ), FRotator(0.f, -90.f, 0.f), 220.f);
	}

	if (HorseHeadMesh)
	{
		SpawnProp(World, HorseHeadMesh, HorseHeadMat, FVector(-3200.f, -620.f, DeckZ), FRotator(0.f, 180.f, 0.f), 110.f);
		SpawnProp(World, HorseHeadMesh, HorseHeadMat, FVector(-3200.f, 620.f, DeckZ), FRotator::ZeroRotator, 110.f);
	}
	if (BullHeadMesh)
	{
		SpawnProp(World, BullHeadMesh, BullHeadMat, FVector(3200.f, -620.f, DeckZ), FRotator(0.f, 180.f, 0.f), 110.f);
		SpawnProp(World, BullHeadMesh, BullHeadMat, FVector(3200.f, 620.f, DeckZ), FRotator::ZeroRotator, 110.f);
	}

	if (VaseAMesh)
	{
		SpawnProp(World, VaseAMesh, VaseAMat, FVector(-3800.f, -800.f, DeckZ), FRotator::ZeroRotator, 90.f);
		SpawnProp(World, VaseAMesh, VaseAMat, FVector(-3800.f, 800.f, DeckZ), FRotator::ZeroRotator, 90.f);
	}
	if (VaseBMesh)
	{
		SpawnProp(World, VaseBMesh, VaseBMat, FVector(3800.f, -800.f, DeckZ), FRotator::ZeroRotator, 80.f);
		SpawnProp(World, VaseBMesh, VaseBMat, FVector(3800.f, 800.f, DeckZ), FRotator::ZeroRotator, 80.f);
	}
	if (BrassVaseMesh)
	{
		SpawnProp(World, BrassVaseMesh, BrassVaseMat, FVector(-1200.f, -620.f, DeckZ), FRotator::ZeroRotator, 120.f);
		SpawnProp(World, BrassVaseMesh, BrassVaseMat, FVector(-1200.f, 620.f, DeckZ), FRotator::ZeroRotator, 120.f);
		SpawnProp(World, BrassVaseMesh, BrassVaseMat, FVector(1200.f, -620.f, DeckZ), FRotator::ZeroRotator, 120.f);
		SpawnProp(World, BrassVaseMesh, BrassVaseMat, FVector(1200.f, 620.f, DeckZ), FRotator::ZeroRotator, 120.f);
	}

	if (LionMesh)
	{
		SpawnProp(World, LionMesh, LionMat, FVector(-200.f, -980.f, DeckZ + ColumnTopZ + 40.f), FRotator(0.f, 90.f, 0.f), 70.f);
		SpawnProp(World, LionMesh, LionMat, FVector(600.f, 980.f, DeckZ + ColumnTopZ + 40.f), FRotator(0.f, -90.f, 0.f), 70.f);
	}

	UStaticMesh* TrackMesh = LoadMesh(TEXT("/Game/Art/Meshes/train_track.train_track"));
	UMaterialInterface* TrackMat = RomanMarble;
	if (TrackMesh)
	{
		const FBoxSphereBounds TrackBounds = TrackMesh->GetBounds();
		const bool bTrackYaw90 = TrackBounds.BoxExtent.Y > TrackBounds.BoxExtent.X * 1.05f;
		const float TrackLen = (bTrackYaw90 ? TrackBounds.BoxExtent.Y : TrackBounds.BoxExtent.X) * 2.f;
		const float TrackScale = TrackLen > 1.f ? (200.f / TrackLen) : 1.f;
		const float TrackBottom = TrackBounds.Origin.Z - TrackBounds.BoxExtent.Z;
		const FRotator TrackRot = bTrackYaw90 ? FRotator(0.f, 90.f, 0.f) : FRotator::ZeroRotator;
		for (int32 i = 0; i < 36; ++i)
		{
			const float X = -2500.f + i * 200.f;
			SpawnMesh(World, TrackMesh, nullptr, FVector(X, 0.f, 38.f - TrackBottom * TrackScale), TrackRot, FVector(TrackScale));
		}
	}
	else if (Cube && TrackMat)
	{
		SpawnMesh(World, Cube, TrackMat, FVector(200.f, 0.f, 36.f), FRotator::ZeroRotator, FVector(70.f, 3.8f, 0.1f));
		SpawnMesh(World, Cube, TrackMat, FVector(200.f, 110.f, 50.f), FRotator::ZeroRotator, FVector(70.f, 0.38f, 0.2f));
		SpawnMesh(World, Cube, TrackMat, FVector(200.f, -110.f, 50.f), FRotator::ZeroRotator, FVector(70.f, 0.38f, 0.2f));
		for (int32 i = 0; i < 28; ++i)
		{
			const float X = -2400.f + i * 200.f;
			SpawnMesh(World, Cube, TrackMat, FVector(X, 0.f, 42.f), FRotator::ZeroRotator, FVector(0.5f, 3.6f, 0.12f));
		}
	}

	Result.Train = World->SpawnActor<ACBTrain>(FVector(200.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
	if (Result.Train)
	{
		Result.Train->Tags.Add(TEXT("CBBuilt"));
		Result.Train->SetFlags(RF_Transactional);
	}

	auto SpawnStairs = [&](const FVector& Location, const FRotator& Rotation)
	{
		ACBStairStream* Stairs = World->SpawnActor<ACBStairStream>(Location, Rotation, Params);
		if (Stairs)
		{
			Stairs->Tags.Add(TEXT("CBBuilt"));
			Stairs->SetFlags(RF_Transactional);
		}
	};
	SpawnStairs(FVector(0.f, -1340.f, 28.f), FRotator::ZeroRotator);
	SpawnStairs(FVector(0.f, 1340.f, 28.f), FRotator(0.f, 180.f, 0.f));
	SpawnStairs(FVector(-4080.f, 0.f, 28.f), FRotator(0.f, -90.f, 0.f));
	SpawnStairs(FVector(4080.f, 0.f, 28.f), FRotator(0.f, 90.f, 0.f));

	if (PedestalMesh)
	{
		const float NewelZ = DeckZ;
		SpawnMesh(World, PedestalMesh, RomanMarble, FVector(-480.f, -1320.f, NewelZ), FRotator::ZeroRotator, FVector(1.f));
		SpawnMesh(World, PedestalMesh, RomanMarble, FVector(480.f, -1320.f, NewelZ), FRotator::ZeroRotator, FVector(1.f));
		SpawnMesh(World, PedestalMesh, RomanMarble, FVector(-480.f, 1320.f, NewelZ), FRotator::ZeroRotator, FVector(1.f));
		SpawnMesh(World, PedestalMesh, RomanMarble, FVector(480.f, 1320.f, NewelZ), FRotator::ZeroRotator, FVector(1.f));
	}

	APlayerStart* Start = World->SpawnActor<APlayerStart>(FVector(0.f, -720.f, 160.f), FRotator(0.f, 90.f, 0.f), Params);
	if (Start)
	{
		Start->Tags.Add(TEXT("CBBuilt"));
	}
	Result.PlayerStart = Start;

	ACameraActor* Camera = World->SpawnActor<ACameraActor>(FVector(-2400.f, -2400.f, 820.f), FRotator(-10.f, 42.f, 0.f), Params);
	if (Camera)
	{
		Camera->Tags.Add(TEXT("CBBuilt"));
		Camera->Tags.Add(TEXT("CBMenuCamera"));
		Camera->SetFlags(RF_Transactional);
	}
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
