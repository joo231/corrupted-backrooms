#include "World/CBStairStream.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

ACBStairStream::ACBStairStream()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 0.05f;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	GuideLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GuideLight"));
	GuideLight->SetupAttachment(Root);
	GuideLight->SetIntensity(1800.f);
	GuideLight->SetAttenuationRadius(1100.f);
	GuideLight->SetLightColor(FLinearColor(1.f, 0.78f, 0.52f));
	GuideLight->SetCastShadows(false);
	GuideLight->SetVisibility(false);
}

void ACBStairStream::BeginPlay()
{
	Super::BeginPlay();
	Path.Empty();
	CachedCenter = INDEX_NONE;
	CachedRadius = INDEX_NONE;
	UpdateChunks(GetActorLocation());
}

void ACBStairStream::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto& Pair : LoadedChunks)
	{
		if (AStaticMeshActor* Actor = Pair.Value.Get())
		{
			Actor->Destroy();
		}
	}
	LoadedChunks.Empty();
	for (AStaticMeshActor* Actor : Pool)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	Pool.Empty();
	Path.Empty();
	Super::EndPlay(EndPlayReason);
}

void ACBStairStream::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Sample = GetActorLocation();
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Sample = Pawn->GetActorLocation();
	}
	UpdateChunks(Sample);
}

bool ACBStairStream::ChunkGoesUp(int32 Index) const
{
	const uint32 Hash = StreamSeed ^ (static_cast<uint32>(Index + 1) * 2654435761u);
	return static_cast<int32>(Hash % 100u) < UpChancePercent;
}

int32 ACBStairStream::ChunkTurnYaw(int32 Index) const
{
	const uint32 Hash = (StreamSeed * 0x9E3779B9u) ^ (static_cast<uint32>(Index / 2 + 3) * 974711u);
	return (Hash & 1u) ? 90 : -90;
}

FTransform ACBStairStream::MakeChunkTransform(const FCBStairChunkDesc& Desc) const
{
	return FTransform(FRotator(0.f, Desc.Yaw, 0.f), Desc.Location);
}

FVector ACBStairStream::ChunkEnd(const FCBStairChunkDesc& Desc) const
{
	const FRotator Travel(0.f, Desc.Yaw, 0.f);
	const float ZDelta = Desc.bGoesUp ? ChunkDrop : -ChunkDrop;
	return Desc.Location + Travel.RotateVector(FVector(0.f, ChunkAlong, ZDelta));
}

void ACBStairStream::EnsurePath(int32 MaxIndex)
{
	if (MaxIndex < 0)
	{
		return;
	}

	if (Path.Num() == 0)
	{
		StreamSeed = GetTypeHash(GetActorLocation()) ^ GetTypeHash(GetActorRotation().Yaw);
		PathYaw = FMath::RoundToInt(GetActorRotation().Yaw);
	}

	while (Path.Num() <= MaxIndex)
	{
		const int32 Index = Path.Num();
		if (Index > 0 && (Index % 2) == 0)
		{
			PathYaw += ChunkTurnYaw(Index);
		}

		FCBStairChunkDesc Desc;
		Desc.Yaw = static_cast<float>(PathYaw);
		Desc.bGoesUp = ChunkGoesUp(Index);
		Desc.Location = (Index == 0) ? GetActorLocation() : ChunkEnd(Path[Index - 1]);
		Path.Add(Desc);
	}
}

int32 ACBStairStream::FindNearestIndex(const FVector& WorldPos)
{
	const int32 Guess = (CachedCenter == INDEX_NONE) ? 0 : CachedCenter;
	EnsurePath(Guess + KeepRadius + 8);

	int32 From = FMath::Max(0, Guess - KeepRadius - 4);
	int32 To = Guess + KeepRadius + 8;
	EnsurePath(To);

	int32 Best = Guess;
	float BestDistSq = TNumericLimits<float>::Max();
	for (int32 Index = From; Index <= To && Index < Path.Num(); ++Index)
	{
		const float StartDistSq = FVector::DistSquared(WorldPos, Path[Index].Location);
		const float EndDistSq = FVector::DistSquared(WorldPos, ChunkEnd(Path[Index]));
		const float DistSq = FMath::Min(StartDistSq, EndDistSq);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Index;
		}
	}
	return Best;
}

AStaticMeshActor* ACBStairStream::TakeChunkActor(UWorld* World)
{
	while (Pool.Num() > 0)
	{
		AStaticMeshActor* Actor = Pool.Pop(false);
		if (IsValid(Actor))
		{
			return Actor;
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags = RF_Transient;
	Params.Owner = this;
	AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Params);
	if (!Actor)
	{
		return nullptr;
	}

	Actor->SetFlags(RF_Transient);
	UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	return Actor;
}

void ACBStairStream::UpdateChunks(const FVector& SampleWorld)
{
	if (!DownMesh)
	{
		DownMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Kit/SM_RomanStairChunk.SM_RomanStairChunk"));
		UpMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Kit/SM_RomanStairUp.SM_RomanStairUp"));
		ChunkMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_RomanMarble.M_RomanMarble"));
		if (!DownMesh)
		{
			return;
		}
		if (!UpMesh)
		{
			UpMesh = DownMesh;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Radius = KeepRadius;
	int32 Center = 1;
	if (UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Center = FindNearestIndex(SampleWorld);
		const float DistToPath = (Path.IsValidIndex(Center))
			? FMath::Sqrt(FMath::Min(
				FVector::DistSquared(SampleWorld, Path[Center].Location),
				FVector::DistSquared(SampleWorld, ChunkEnd(Path[Center]))))
			: TNumericLimits<float>::Max();
		if (GuideLight)
		{
			const bool bNear = DistToPath < 1800.f;
			GuideLight->SetVisibility(bNear);
			if (bNear)
			{
				GuideLight->SetWorldLocation(SampleWorld + FVector(0.f, 0.f, 90.f));
			}
		}
		if (Center <= 1 && DistToPath > 2800.f)
		{
			Radius = 2;
		}
	}
	else
	{
		EnsurePath(KeepRadius);
	}

	Center = FMath::Max(0, Center);

	if (Center == CachedCenter && Radius == CachedRadius)
	{
		return;
	}
	CachedCenter = Center;
	CachedRadius = Radius;

	const int32 MinIndex = FMath::Max(0, Center - Radius);
	const int32 MaxIndex = Center + Radius;
	EnsurePath(MaxIndex);

	TSet<int32> Needed;
	Needed.Reserve(MaxIndex - MinIndex + 1);
	for (int32 Index = MinIndex; Index <= MaxIndex; ++Index)
	{
		Needed.Add(Index);
	}

	for (auto It = LoadedChunks.CreateIterator(); It; ++It)
	{
		if (!Needed.Contains(It.Key()))
		{
			if (AStaticMeshActor* Actor = It.Value().Get())
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				if (Pool.Num() < 32)
				{
					Pool.Add(Actor);
				}
				else
				{
					Actor->Destroy();
				}
			}
			It.RemoveCurrent();
		}
	}

	for (int32 Index : Needed)
	{
		if (LoadedChunks.Contains(Index) && LoadedChunks[Index].IsValid())
		{
			continue;
		}
		if (!Path.IsValidIndex(Index))
		{
			continue;
		}

		AStaticMeshActor* Actor = TakeChunkActor(World);
		if (!Actor)
		{
			continue;
		}

		const FCBStairChunkDesc& Desc = Path[Index];
		UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
		UStaticMesh* Mesh = Desc.bGoesUp ? UpMesh : DownMesh;
		if (Comp && Mesh && Comp->GetStaticMesh() != Mesh)
		{
			Comp->SetStaticMesh(Mesh);
		}
		if (Comp && ChunkMaterial)
		{
			Comp->SetMaterial(0, ChunkMaterial);
		}

		Actor->SetActorTransform(MakeChunkTransform(Desc));
		Actor->SetActorHiddenInGame(false);
		Actor->SetActorEnableCollision(true);
		LoadedChunks.Add(Index, Actor);
	}
}
