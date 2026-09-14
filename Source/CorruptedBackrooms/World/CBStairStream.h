#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CBStairStream.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UPointLightComponent;
class AStaticMeshActor;

struct FCBStairChunkDesc
{
	FVector Location = FVector::ZeroVector;
	float Yaw = 0.f;
	bool bGoesUp = false;
};

UCLASS()
class CORRUPTEDBACKROOMS_API ACBStairStream : public AActor
{
	GENERATED_BODY()

public:
	ACBStairStream();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	UPROPERTY(EditAnywhere, Category = "Stairs")
	int32 KeepRadius = 6;

	UPROPERTY(EditAnywhere, Category = "Stairs")
	int32 UpChancePercent = 76;

protected:
	void EnsurePath(int32 MaxIndex);
	int32 FindNearestIndex(const FVector& WorldPos);
	void UpdateChunks(const FVector& SampleWorld);
	AStaticMeshActor* TakeChunkActor(UWorld* World);
	bool ChunkGoesUp(int32 Index) const;
	int32 ChunkTurnYaw(int32 Index) const;
	FTransform MakeChunkTransform(const FCBStairChunkDesc& Desc) const;
	FVector ChunkEnd(const FCBStairChunkDesc& Desc) const;

	UPROPERTY()
	UStaticMesh* DownMesh = nullptr;

	UPROPERTY()
	UStaticMesh* UpMesh = nullptr;

	UPROPERTY()
	UMaterialInterface* ChunkMaterial = nullptr;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* GuideLight = nullptr;

	TMap<int32, TWeakObjectPtr<AStaticMeshActor>> LoadedChunks;
	TArray<AStaticMeshActor*> Pool;
	TArray<FCBStairChunkDesc> Path;

	float ChunkAlong = -400.f;
	float ChunkDrop = 200.f;
	int32 PathYaw = 0;
	uint32 StreamSeed = 0;
	int32 CachedCenter = INDEX_NONE;
	int32 CachedRadius = INDEX_NONE;
};
