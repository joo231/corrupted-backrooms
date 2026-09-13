#include "World/CBTrain.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/CBPlayerCharacter.h"
#include "Player/CBPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ACBTrain::ACBTrain()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	UStaticMesh* Cube = CubeMesh.Succeeded() ? CubeMesh.Object : nullptr;
	UStaticMesh* Cylinder = CylinderMesh.Succeeded() ? CylinderMesh.Object : nullptr;
	UMaterialInterface* BaseMat = ShapeMat.Succeeded() ? ShapeMat.Object : nullptr;

	UMaterialInstanceDynamic* BodyMat = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this, TEXT("BodyMat")) : nullptr;
	UMaterialInstanceDynamic* RoofMat = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this, TEXT("RoofMat")) : nullptr;
	UMaterialInstanceDynamic* MetalMat = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this, TEXT("MetalMat")) : nullptr;
	UMaterialInstanceDynamic* GlassMat = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this, TEXT("GlassMat")) : nullptr;
	UMaterialInstanceDynamic* TrimMat = BaseMat ? UMaterialInstanceDynamic::Create(BaseMat, this, TEXT("TrimMat")) : nullptr;
	if (BodyMat)
	{
		BodyMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.93f, 0.94f, 0.96f));
	}
	if (RoofMat)
	{
		RoofMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.18f, 0.22f, 0.28f));
	}
	if (MetalMat)
	{
		MetalMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.09f, 0.1f));
	}
	if (GlassMat)
	{
		GlassMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 0.28f, 0.42f));
	}
	if (TrimMat)
	{
		TrimMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.75f, 0.12f, 0.12f));
	}

	// Hollow carriage: floor, roof, walls with a door gap on -Y.
	MakePart(TEXT("Floor"), Cube, FVector(0.f, 0.f, 48.f), FVector(22.f, 3.1f, 0.12f), BodyMat);
	MakePart(TEXT("Roof"), Cube, FVector(0.f, 0.f, 330.f), FVector(22.2f, 3.3f, 0.12f), RoofMat);
	MakePart(TEXT("WallPosY"), Cube, FVector(0.f, 155.f, 190.f), FVector(22.f, 0.12f, 2.7f), BodyMat);
	MakePart(TEXT("WallNegY_A"), Cube, FVector(-750.f, -155.f, 190.f), FVector(7.f, 0.12f, 2.7f), BodyMat);
	MakePart(TEXT("WallNegY_B"), Cube, FVector(650.f, -155.f, 190.f), FVector(9.f, 0.12f, 2.7f), BodyMat);
	MakePart(TEXT("DoorHeader"), Cube, FVector(-120.f, -155.f, 300.f), FVector(3.6f, 0.12f, 0.55f), BodyMat);
	MakePart(TEXT("FrontWall"), Cube, FVector(1100.f, 0.f, 190.f), FVector(0.12f, 3.1f, 2.7f), BodyMat);
	MakePart(TEXT("BackWall"), Cube, FVector(-1100.f, 0.f, 190.f), FVector(0.12f, 3.1f, 2.7f), BodyMat);
	MakePart(TEXT("Stripe"), Cube, FVector(0.f, 0.f, 250.f), FVector(22.2f, 3.32f, 0.12f), TrimMat, false);

	for (int32 i = 0; i < 5; ++i)
	{
		const float X = -800.f + i * 400.f;
		MakePart(*FString::Printf(TEXT("Window_%d"), i), Cube, FVector(X, 158.f, 210.f), FVector(1.6f, 0.06f, 0.9f), GlassMat, false);
		MakePart(*FString::Printf(TEXT("WindowInner_%d"), i), Cube, FVector(X, -158.f, 210.f), FVector(1.6f, 0.06f, 0.9f), GlassMat, false);
	}

	for (int32 i = 0; i < 8; ++i)
	{
		const float X = -900.f + i * 260.f;
		UStaticMeshComponent* WheelL = MakePart(*FString::Printf(TEXT("WheelL_%d"), i), Cylinder, FVector(X, 110.f, 35.f), FVector(0.9f, 0.35f, 0.9f), MetalMat);
		UStaticMeshComponent* WheelR = MakePart(*FString::Printf(TEXT("WheelR_%d"), i), Cylinder, FVector(X, -110.f, 35.f), FVector(0.9f, 0.35f, 0.9f), MetalMat);
		if (WheelL)
		{
			WheelL->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		}
		if (WheelR)
		{
			WheelR->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		}
	}

	MakePart(TEXT("Chassis"), Cube, FVector(0.f, 0.f, 28.f), FVector(21.5f, 2.6f, 0.18f), MetalMat);
	MakePart(TEXT("Nose"), Cube, FVector(1180.f, 0.f, 160.f), FVector(1.4f, 2.6f, 2.2f), BodyMat);

	DoorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorTrigger"));
	DoorTrigger->SetupAttachment(Root);
	DoorTrigger->SetRelativeLocation(FVector(-120.f, -230.f, 140.f));
	DoorTrigger->SetBoxExtent(FVector(160.f, 90.f, 130.f));
	DoorTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DoorTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	DoorTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DoorTrigger->SetGenerateOverlapEvents(true);

	CabinTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CabinTrigger"));
	CabinTrigger->SetupAttachment(Root);
	CabinTrigger->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	CabinTrigger->SetBoxExtent(FVector(1000.f, 120.f, 120.f));
	CabinTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CabinTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CabinTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CabinTrigger->SetGenerateOverlapEvents(true);
}

void ACBTrain::BeginPlay()
{
	Super::BeginPlay();
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

UStaticMeshComponent* ACBTrain::MakePart(const FName& Name, UStaticMesh* Mesh, const FVector& RelativeLocation, const FVector& RelativeScale, UMaterialInterface* Material, bool bCollision)
{
	UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Comp->SetupAttachment(Root);
	Comp->SetRelativeLocation(RelativeLocation);
	Comp->SetRelativeScale3D(RelativeScale);
	if (Mesh)
	{
		Comp->SetStaticMesh(Mesh);
	}
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
	return Comp;
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
	const FVector InsideLocation = GetActorTransform().TransformPosition(FVector(-200.f, 0.f, 150.f));
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
