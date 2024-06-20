// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "Animation/MeleeCharacterAnimInstance.h"

#include "Character/BaseCharacter.h"
#include "Character/Knight.h"

#include "Components/StaticMeshComponent.h"


void UMeleeCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}


	// Update Speed
	Speed = Character->GetVelocity().Length();

	// Get rotation angle
	FVector InversedTransformDirection = Character->GetActorTransform().InverseTransformVector(Character->GetVelocity());
	Angle = InversedTransformDirection.Rotation().Yaw;

	bIsBlocking = Character->GetIsBlocking();

}

void UMeleeCharacterAnimInstance::OnSwordSlashFinish() const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnSwordSlashFinish"));
		return;
	}

	Character->StopAttackLineTraceTimer();
	Character->OnSlashAttackFinished();
}

void UMeleeCharacterAnimInstance::OnSwordSlashStart() const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnSwordSlashStart"));
		return;
	}

	Character->StartAttackLineTraceTimer();

	FVector ForwardLaunchVector = Character->GetActorForwardVector() * OnSlashLaunchForwardForce;
	Character->LaunchCharacter(ForwardLaunchVector, true, false);
}

void UMeleeCharacterAnimInstance::OnAttackFinished() const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnAttackFinished"));
		return;
	}

	Character->OnAttackFinished();

}

void UMeleeCharacterAnimInstance::OnStepForward(float LaunchForce) const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnStepForward"));
		return;
	}
	FVector ForwardLaunchVector = Character->GetActorForwardVector() * LaunchForce;
	Character->LaunchCharacter(ForwardLaunchVector, true, false);
}

void UMeleeCharacterAnimInstance::OnSwordsCross() const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnSwordsCross"));
		return;
	}

	Character->OnSwordsCross();
}

void UMeleeCharacterAnimInstance::OnSwordSwing() const
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnSwordSwing"));
		return;
	}

	Character->OnSwordSwing();
}


void UMeleeCharacterAnimInstance::OnFootstep(bool bRightFootstep) const
{

	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnFootstep"));
		return;
	}

	Character->PlayFootstepEffects(bRightFootstep);

}

void UMeleeCharacterAnimInstance::OnPauseMontage()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast the BaseCharacter! UMeleeCharacterAnimInstance::OnFootstep"));
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();

	if (!IsValid(AnimInstance))
	{
		return;
	}

	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();

	if (!IsValid(CurrentMontage))
	{
		return;
	}

	AnimInstance->Montage_Pause(CurrentMontage);

	FTimerHandle TempTimerHandle;

	// Bind the wrapper function and set the timer
	GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, FTimerDelegate::CreateUObject(this, &UMeleeCharacterAnimInstance::ResumeMontage, AnimInstance, CurrentMontage), 0.2f, false);

}

void UMeleeCharacterAnimInstance::ResumeMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage)
{
	AnimInstance->Montage_Resume(Montage);
}
