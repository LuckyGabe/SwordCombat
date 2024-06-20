// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MeleeCharacterAnimInstance.generated.h"


class UAnimMontage;

UCLASS()
class SWORDCOMBATSYSTEM_API UMeleeCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSwordSlashFinish() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSwordSlashStart() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackFinished() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnStepForward(float LaunchForce) const;


	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSwordsCross() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnSwordSwing() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void OnFootstep(bool bRightFootstep) const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnPauseMontage();

private:
	void ResumeMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
	float Angle;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Combat")
	bool bIsBlocking;

	UPROPERTY(EditAnywhere, Category = "Character|Combat")
	float OnSlashLaunchForwardForce;

};
