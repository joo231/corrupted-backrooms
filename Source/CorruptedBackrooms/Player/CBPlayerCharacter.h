#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CBPlayerCharacter.generated.h"

class UCameraComponent;

UCLASS()
class CORRUPTEDBACKROOMS_API ACBPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACBPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void SetMovementLocked(bool bLocked);

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void OnInteract();
	void OnJumpPressed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	bool bMovementLocked = false;
};
