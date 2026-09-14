#include "World/CBTrain.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"
#include "Player/CBPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ACBTrain::ACBTrain()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	LocoComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Loco"));
	LocoComp->SetupAttachment(Root);
	LocoComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarAComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarA"));
	CarAComp->SetupAttachment(Root);
	CarAComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarBComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarB"));
	CarBComp->SetupAttachment(Root);
	CarBComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	RideFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RideFloor"));
	RideFloor->SetupAttachment(Root);
	RideFloor->SetRelativeLocation(FVector(0.f, 0.f, 42.f));
	RideFloor->SetRelativeScale3D(FVector(23.2f, 3.4f, 0.1f));
	RideFloor->SetVisibility(false);
	RideFloor->SetHiddenInGame(true);
	RideFloor->SetCastShadow(false);
	RideFloor->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RideFloor->SetCollisionResponseToAllChannels(ECR_Block);
	if (CubeMesh.Succeeded())
	{
		RideFloor->SetStaticMesh(CubeMesh.Object);
	}

	DoorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorTrigger"));
	DoorTrigger->SetupAttachment(Root);
	DoorTrigger->SetRelativeLocation(FVector(0.f, -240.f, 140.f));
	DoorTrigger->SetBoxExtent(FVector(220.f, 110.f, 130.f));
	DoorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DoorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	DoorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DoorTrigger->SetGenerateOverlapEvents(true);

	CabinTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CabinTrigger"));
	CabinTrigger->SetupAttachment(Root);
	CabinTrigger->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	CabinTrigger->SetBoxExtent(FVector(1100.f, 130.f, 130.f));
	CabinTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CabinTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CabinTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CabinTrigger->SetGenerateOverlapEvents(true);
}

void ACBTrain::ApplyCarMesh(UStaticMeshComponent* Comp, UStaticMesh* Mesh, float X)
{
	if (!Comp || !Mesh)
	{
		return;
	}

	const float CarLength = 760.f;
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const bool bYaw90 = Bounds.BoxExtent.Y > Bounds.BoxExtent.X * 1.05f;
	const float Length = (bYaw90 ? Bounds.BoxExtent.Y : Bounds.BoxExtent.X) * 2.f;
	const float Scale = Length > 1.f ? (CarLength / Length) : 8.f;
	const float Bottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
	Comp->SetStaticMesh(Mesh);
	Comp->SetRelativeLocation(FVector(X, 0.f, 46.f - Bottom * Scale));
	Comp->SetRelativeScale3D(FVector(Scale));
	Comp->SetRelativeRotation(bYaw90 ? FRotator(0.f, 90.f, 0.f) : FRotator::ZeroRotator);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACBTrain::SetupTrainVisuals()
{
	UStaticMesh* LocoMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Meshes/train_locomotive.train_locomotive"));
	UStaticMesh* CarMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Meshes/train_carriage.train_carriage"));
	UStaticMesh* TailMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Meshes/train_tail.train_tail"));
	if (!LocoMesh)
	{
		return;
	}
	if (!CarMesh)
	{
		CarMesh = LocoMesh;
	}
	if (!TailMesh)
	{
		TailMesh = CarMesh;
	}

	const float CarLength = 760.f;
	ApplyCarMesh(LocoComp, LocoMesh, CarLength);
	ApplyCarMesh(CarAComp, CarMesh, 0.f);
	ApplyCarMesh(CarBComp, TailMesh, -CarLength);
}

void ACBTrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupTrainVisuals();
}

void ACBTrain::BeginPlay()
{
	Super::BeginPlay();
	SetupTrainVisuals();
	DoorTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACBTrain::OnTriggerBegin);
	DoorTrigger->OnComponentEndOverlap.AddDynamic(this, &ACBTrain::OnTriggerEnd);
	CabinTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACBTrain::OnTriggerBegin);
	CabinTrigger->OnComponentEndOverlap.AddDynamic(this, &ACBTrain::OnTriggerEnd);
}

void ACBTrain::OnTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		OverlappingPawn = Cast<APawn>(OtherActor);
	}
}

void ACBTrain::OnTriggerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OverlappingPawn && !bPlayerInside)
	{
		OverlappingPawn = nullptr;
	}
}

void ACBTrain::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (OtherActor && OtherActor->IsA<APawn>())
	{
		OverlappingPawn = Cast<APawn>(OtherActor);
	}
}

void ACBTrain::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	if (OtherActor == OverlappingPawn && !bPlayerInside)
	{
		OverlappingPawn = nullptr;
	}
}

FText ACBTrain::GetPromptText() const
{
	if (bDeparting)
	{
		return FText::GetEmpty();
	}
	if (bPlayerInside)
	{
		return FText::FromString(TEXT("Нажмите E, чтобы отправиться"));
	}
	if (OverlappingPawn)
	{
		return FText::FromString(TEXT("Нажмите E, чтобы сесть в поезд"));
	}
	return FText::GetEmpty();
}

bool ACBTrain::TryInteract(APawn* Pawn)
{
	if (bDeparting || !Pawn)
	{
		return false;
	}

	if (!bPlayerInside)
	{
		if (OverlappingPawn == Pawn)
		{
			BoardPlayer(Pawn);
			return true;
		}
		return false;
	}

	BeginDeparture();
	return true;
}

void ACBTrain::BoardPlayer(APawn* Pawn)
{
	bPlayerInside = true;
	OverlappingPawn = Pawn;
	const FVector InsideLocation = GetActorTransform().TransformPosition(FVector(-200.f, 0.f, 110.f));
	Pawn->SetActorLocation(InsideLocation, false, nullptr, ETeleportType::TeleportPhysics);
	Pawn->SetActorRotation(GetActorRotation());
	if (AController* Controller = Pawn->GetController())
	{
		Controller->SetControlRotation(GetActorRotation());
	}
}

void ACBTrain::BeginDeparture()
{
	if (bDeparting)
	{
		return;
	}
	bDeparting = true;

	if (ACBPlayerCharacter* Character = Cast<ACBPlayerCharacter>(OverlappingPawn))
	{
		Character->SetMovementLocked(true);
		Character->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (ACBPlayerController* PC = Cast<ACBPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowLoadingScreen();
	}

	DepartTime = 0.f;
}

void ACBTrain::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bDeparting)
	{
		return;
	}

	AddActorWorldOffset(FVector(420.f * DeltaSeconds, 0.f, 0.f));
	DepartTime += DeltaSeconds;
	if (DepartTime >= 2.4f)
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Engine/Maps/Templates/TimeOfDay_Default")), true,
			TEXT("?game=/Script/CorruptedBackrooms.CBArrivalGameMode"));
	}
}
