#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CBTrain.generated.h"

class UBoxComponent;
class ACBPlayerCharacter;

UCLASS()
class CORRUPTEDBACKROOMS_API ACBTrain : public AActor
{
	GENERATED_BODY()

public:
	ACBTrain();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	bool TryInteract(APawn* Pawn);
	FText GetPromptText() const;
	bool IsPlayerInside() const { return bPlayerInside; }

protected:
	UStaticMeshComponent* MakePart(const FName& Name, UStaticMesh* Mesh, const FVector& RelativeLocation, const FVector& RelativeScale, UMaterialInterface* Material, bool bCollision = true);

	void BoardPlayer(APawn* Pawn);
	void BeginDeparture();

	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* DoorTrigger;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* CabinTrigger;

	UPROPERTY()
	APawn* OverlappingPawn = nullptr;

	bool bPlayerInside = false;
	bool bDeparting = false;
	float DepartTime = 0.f;
};
