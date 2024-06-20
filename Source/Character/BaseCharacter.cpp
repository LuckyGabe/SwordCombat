// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "BaseCharacter.h"

#include "Combat/SlashingDamageType.h"



#include "Sound/SoundCue.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraSystem.h"


ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
}

#pragma region Inherited
// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->Init(MaxHealth);
	HealthComponent->OnActorDeath().BindUObject(this, &ABaseCharacter::OnCharacterDeath);
	UpdateHealthWidgetEvent();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead()) { return 0.f; }

	if (bIsDodging)
	{
		// Ignore damage
		return 0.f;
	}

	//Check if damage is blockable. There might be some cases that the character can not block damage (for example spell damage).
	if (bIsBlocking && IsDamageBlockable(DamageEvent))
	{
		ABaseCharacter* CauserCharacter = Cast<ABaseCharacter>(DamageCauser);

		if (!IsValid(CauserCharacter))
		{
			DamageBlockedEvent();
			return 0.f;
		}

		if (AttackBlockedSoundCue)
		{
			UGameplayStatics::PlaySoundAtLocation(this, AttackBlockedSoundCue, GetActorLocation());
		}

		DamageBlockedEvent();
		CauserCharacter->OnAttackBlocked();
		return 0.f;

	}


	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Now, delegate the damage to the HealthComponent
	if (HealthComponent)
	{
		HealthComponent->ApplyDamage(ActualDamage, DamageCauser);
	}

	UpdateHealthWidgetEvent();

	if (IsDead()) { return 0.f; }

	if (DamageReceivedSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DamageReceivedSoundCue, GetActorLocation());
	}

	OnDamageReceived();


	return ActualDamage;
}

ETeamAttitude::Type ABaseCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		if (TeamId == OtherTeamAgent->GetGenericTeamId())
			return ETeamAttitude::Friendly;
		else
			return ETeamAttitude::Hostile;
	}

	return ETeamAttitude::Neutral;
}

#pragma endregion Inherited


#pragma region Melee Combat
bool ABaseCharacter::IsPlayingAnyMontage() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			// Returns true if any montage is currently playing
			return AnimInstance->IsAnyMontagePlaying();
		}
	}
	return false; // No montages are playing or anim instance is not available
}

#pragma region Attack

void ABaseCharacter::StartAttackLineTraceTimer()
{
	if (SwordSlashSoundCue)
	{
		UGameplayStatics::SpawnSoundAttached(SwordSlashSoundCue, GetSwordMesh(), TEXT("SwordEnd_Socket"));
	}
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABaseCharacter::PerformAttackTrace, 0.02f, true);
}

void ABaseCharacter::StopAttackLineTraceTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
}

void ABaseCharacter::OnAttackFinished()
{
	if (!bIsAttacking)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
	}

	AttackedActorsArray.Empty();
	StopAttackLineTraceTimer();
}

void ABaseCharacter::OnSwordsCross()
{
	USkeletalMeshComponent* SwordMesh = GetSwordMesh();

	FVector SwordEndLocation = SwordMesh->GetSocketLocation("SwordEnd_Socket");
	FVector SwordStartLocation = SwordMesh->GetSocketLocation("SwordStart_Socket");

	FVector MiddleLocation = (SwordEndLocation + SwordStartLocation) / 2;

	if (SwordImpactEmmiter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SwordImpactEmmiter, MiddleLocation);
	}

	if (SwordHitSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwordHitSoundCue, MiddleLocation);
	}

}

void ABaseCharacter::OnSwordSwing()
{
	if (AttackGroanSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackGroanSoundCue, GetActorLocation());
	}
	if (SwordSlashSoundCue)
	{
		UGameplayStatics::SpawnSoundAttached(SwordSlashSoundCue, GetSwordMesh(), TEXT("SwordEnd_Socket"));
	}
}

void ABaseCharacter::PlayAttackMontage()
{
	PlayAttackMontageEvent();
}

void ABaseCharacter::PerformAttackTrace()
{
	USkeletalMeshComponent* SwordMesh = GetSwordMesh();
	if (!SwordMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("SwordMesh is nullptr! AKnight::Attack"));
		return;
	}

	FVector StartLocation = SwordMesh->GetSocketLocation("SwordStart_Socket");
	FVector EndLocation = SwordMesh->GetSocketLocation("SwordEnd_Socket");

	// Define the radius and half-height of the capsule
	float CapsuleRadius = 7.0f;
	float CapsuleHalfHeight = FVector::Dist(StartLocation, EndLocation) / 2.0f;

	// Calculate the center of the capsule
	FVector CapsuleCenter = (StartLocation + EndLocation) / 2.0f;

	// Calculate the rotation of the capsule based on the start and end points
	FQuat CapsuleRotation = SwordMesh->GetSocketQuaternion("SwordStart_Socket");

	// Define the collision shape
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	// Sweep for multiple hits
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the actor performing the attack

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		CapsuleCenter,
		CapsuleCenter, // No movement, single frame trace
		CapsuleRotation,
		ECC_GameTraceChannel1,
		CapsuleShape,
		QueryParams
	);

	////// Debug draw the capsule
	//DrawDebugCapsule(
	//	GetWorld(),
	//	CapsuleCenter,
	//	CapsuleHalfHeight,
	//	CapsuleRadius,
	//	CapsuleRotation,
	//	FColor::Red, // Color of the capsule
	//	false, // Persistent lines
	//	3.0f // Lifetime of the line
	//);


	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!IsValid(HitActor) || AttackedActorsArray.Contains(HitActor)) { continue; }

			// Check if the hit actor is a character
			ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(HitActor);
			if (!IsValid(HitCharacter))
			{
				continue;
			}

			if (SwordImpactEmmiter)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SwordImpactEmmiter, HitResult.Location);
			}
			if (bBackstab)
			{
				bBackstab = false;
				// Apply damage to the hit actor
				HitCharacter->TakeDamage(999.f, FDamageEvent(USlashingDamageType::StaticClass()), GetController(), this);
				return;
			}
			// Apply damage to the hit actor
			HitCharacter->TakeDamage(BaseDamage, FDamageEvent(USlashingDamageType::StaticClass()), GetController(), this);

			// Add the attacked actor to an array to prevent multiple damage during one swing/slash
			AttackedActorsArray.AddUnique(HitActor);

			UE_LOG(LogTemp, Warning, TEXT("HitActor name: %s, BaseDamage: %i"), *HitActor->GetName(), BaseDamage);
		}

	}

}

bool ABaseCharacter::Attack()
{
	if (IsPlayingAnyMontage() && !bIsBlocking) { return false; }
	bIsAttacking = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedAttacking;
	PlayAttackMontage();
	return true;
}

void ABaseCharacter::OnSlashAttackFinished()
{
	AttackedActorsArray.Empty();
	OnSwordSwing();
};

#pragma region Blocking
void ABaseCharacter::StopBlocking()
{
	bIsBlocking = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
	StopBlockingMontageEvent();
}

bool ABaseCharacter::IsDamageBlockable(FDamageEvent DamageEvent)
{
	if (DamageEvent.DamageTypeClass == USlashingDamageType::StaticClass())
	{
		return true;
	}
	return false;
}

void ABaseCharacter::OnAttackBlocked()
{
	bIsAttacking = false;
	OnAttackFinished();

	GetCharacterMovement()->MaxWalkSpeed = 0;

	FVector LaunchVelocity = -GetActorForwardVector() * OnDamageStepBackForce;

	LaunchCharacter(LaunchVelocity, true, false);

	OnAttackBlockedEvent();
}

void ABaseCharacter::OnDamageReceived()
{
	OnAttackFinished();

	if (bIsBlocking)
	{
		StopBlocking();
	}

	FVector LaunchVelocity = -GetActorForwardVector() * OnDamageStepBackForce;

	LaunchCharacter(LaunchVelocity, true, false);

	OnDamageReceivedEvent();
}

void ABaseCharacter::OnCharacterDeath()
{
	HealthComponent->OnActorDeath().Unbind();
	bIsDead = true;

	PlayDyingMontageEvent();

	//Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	// Disable movement
	GetCharacterMovement()->DisableMovement();


	//Detach from controller
	if (Controller != NULL)
	{
		Controller->UnPossess();
	}

}

#pragma endregion Blocking

#pragma endregion Melee Combat

void ABaseCharacter::PlayFootstepEffects(bool bRightFootstep)
{
	if (FootstepEmitter)
	{
		if (bRightFootstep)
		{
			FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("RightFootSocket"));
			SpawnFootstepEmitterEvent(SpawnLocation);
		}
		else
		{
			FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("LeftFootSocket"));
			SpawnFootstepEmitterEvent(SpawnLocation);
		}
	}


}

