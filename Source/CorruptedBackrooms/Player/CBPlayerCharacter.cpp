#include "CBPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CBPlayerController.h"

ACBPlayerCharacter::ACBPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 92.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 480.f;
	GetCharacterMovement()->JumpZVelocity = 420.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxStepHeight = 46.f;
	GetCharacterMovement()->SetWalkableFloorAngle(60.f);
	GetCharacterMovement()->bCanWalkOffLedges = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
}

void ACBPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACBPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACBPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACBPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACBPlayerCharacter::LookUp);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ACBPlayerCharacter::OnInteract);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACBPlayerCharacter::OnJumpPressed);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
}

void ACBPlayerCharacter::SetMovementLocked(bool bLocked)
{
	bMovementLocked = bLocked;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (bLocked)
		{
			Movement->DisableMovement();
		}
		else
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
}

void ACBPlayerCharacter::MoveForward(float Value)
{
	if (bMovementLocked || Value == 0.f)
	{
		return;
	}
	AddMovementInput(GetActorForwardVector(), Value);
}

void ACBPlayerCharacter::MoveRight(float Value)
{
	if (bMovementLocked || Value == 0.f)
	{
		return;
	}
	AddMovementInput(GetActorRightVector(), Value);
}

void ACBPlayerCharacter::Turn(float Value)
{
	if (bMovementLocked)
	{
		return;
	}

	float Sensitivity = 1.f;
	if (const ACBPlayerController* PC = Cast<ACBPlayerController>(Controller))
	{
		Sensitivity = PC->GetMouseSensitivity();
	}
	AddControllerYawInput(Value * Sensitivity);
}

void ACBPlayerCharacter::LookUp(float Value)
{
	if (bMovementLocked)
	{
		return;
	}

	float Sensitivity = 1.f;
	if (const ACBPlayerController* PC = Cast<ACBPlayerController>(Controller))
	{
		Sensitivity = PC->GetMouseSensitivity();
	}
	AddControllerPitchInput(Value * Sensitivity);
}

void ACBPlayerCharacter::OnInteract()
{
	if (ACBPlayerController* PC = Cast<ACBPlayerController>(Controller))
	{
		PC->HandleInteract();
	}
}

void ACBPlayerCharacter::OnJumpPressed()
{
	if (!bMovementLocked)
	{
		Jump();
	}
}
