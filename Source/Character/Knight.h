// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Knight.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AMeleeEnemy;

UCLASS()
class SWORDCOMBATSYSTEM_API AKnight : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AKnight();

#pragma region Player Character
protected:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
	bool IsInAir();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnJumpEvent();

private:
	void MoveRight(float Value);
	void MoveForward(float Value);
	void Turn(float Rate);
	void LookUp(float Rate);
	void StartJump();
	void StopJump();
	void OnLeftMouseButtonReleased();
	void PerformDodge();
	void EndDodge();
	void ExecuteDodgeMovement();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCamera;

	// Rate at which the character turns
	UPROPERTY(EditAnywhere, Category = "Stats|Camera")
	float BaseTurnRate;

	// Rate at which the character looks up/down
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Camera")
	float BaseLookUpRate;

#pragma endregion Player Character

#pragma region Sword Combat
public:
	virtual void OnSlashAttackFinished() override;

	virtual void OnAttackFinished() override;
protected:

	virtual void PlayAttackMontage() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnSlashAttackFinishedEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayBackstabMontageEvent();

	bool CheckForBackstab(float MaxDistance, AMeleeEnemy*& outFoundEnemy);

	virtual USkeletalMeshComponent* GetSwordMesh() override { return GetMesh(); }

private:

	virtual void StartBlocking() override;

	void BackstabEnemy(AMeleeEnemy* Enemy);
/*
* Function used just for binding. To perform attack use PerformAttack function which returns if the attack was initiated succesfully.
*/
	 void PerformAttack();

protected:

	UPROPERTY(EditAnywhere, Category = "Stats")
	float DodgeDistance = 800.f;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DodgeAnimMontage;

	FTimerHandle DodgeTimerHandle;

	// Set a timer to delay the launch character movement
	FTimerHandle DodgeMovementDelayTimerHandle;

#pragma endregion Sword Combat

};
