// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "Knight.h"

#include "Enemy/MeleeEnemy.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


AKnight::AKnight()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
	SpringArmComponent->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera to the end of the boom
	ThirdPersonCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Enable jumping.
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f; // Set the jump velocity
	GetCharacterMovement()->AirControl = 0.2f; // Set the amount of movement control in the air
	// Set the maximum speed in the air
	GetCharacterMovement()->MaxFlySpeed = 350.f; // or MaxWalkSpeed if you're using walking physics in the air

	// Set base turn and look up rates
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));


	// Set the player's team ID
	TeamId = FGenericTeamId(1);

}

#pragma region Player Character

void AKnight::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AKnight::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKnight::MoveRight);

	// Camera controls
	PlayerInputComponent->BindAxis("Turn", this, &AKnight::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AKnight::LookUp);

	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Pressed, this, &AKnight::PerformAttack);
	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Released, this, &AKnight::OnLeftMouseButtonReleased);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AKnight::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AKnight::StopJump);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AKnight::StartBlocking);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AKnight::StopBlocking);

	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AKnight::PerformDodge);
}

bool AKnight::IsInAir()
{
	return GetCharacterMovement()->IsFalling();
}

void AKnight::OnLeftMouseButtonReleased()
{
	//AttackCount = FMath::RandRange(0, MaxAttackCount);
	bIsAttacking = false;
}

void AKnight::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Determine which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AKnight::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Determine which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AKnight::Turn(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AKnight::LookUp(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AKnight::StartJump()
{
	if (IsInAir() || bIsDodging) { return; }

	StopAttackLineTraceTimer();
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed / 2;
	OnJumpEvent();
	Jump();
}

void AKnight::StopJump()
{
	StopJumping();
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
}

void AKnight::PerformDodge()
{
	if (!bIsDodging && !IsInAir()) // Implement this to check if dodging is possible at the moment
	{
		bIsDodging = true; // Prevent damage and re-triggering dodge
		StopAttackLineTraceTimer();
		GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed / 2;

		GetMesh()->GetAnimInstance()->Montage_Play(DodgeAnimMontage, 1.f);
		GetWorldTimerManager().SetTimer(DodgeTimerHandle, this, &AKnight::EndDodge, DodgeAnimMontage->GetPlayLength(), false);

		GetWorldTimerManager().SetTimer(DodgeMovementDelayTimerHandle, this, &AKnight::ExecuteDodgeMovement, DodgeAnimMontage->GetPlayLength() / 2, false);

		FVector DodgeDirection = GetActorForwardVector() * DodgeDistance / 2;
		LaunchCharacter(DodgeDirection, true, true);

	}
}

void AKnight::EndDodge()
{
	bIsDodging = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
}

// This function executes the delayed movement
void AKnight::ExecuteDodgeMovement()
{
	FVector DodgeDirection = GetActorForwardVector() * DodgeDistance / 2;
	LaunchCharacter(DodgeDirection, true, true);
}

#pragma endregion Player Character

#pragma region Sword Combat

void AKnight::OnAttackFinished()
{
	Super::OnAttackFinished();

	// If the attack key is still down, continue attacking
	if (bIsAttacking)
	{
		PlayAttackMontageEvent();
		return;
	}

}

void AKnight::PlayAttackMontage()
{
	AMeleeEnemy* Enemy;
	if (CheckForBackstab(150.f, Enemy))
	{
		BackstabEnemy(Enemy);
		return;
	}

	Super::PlayAttackMontage();
}

bool AKnight::CheckForBackstab(float MaxDistance, AMeleeEnemy*& outFoundEnemy)
{
	// Get the player's forward direction
	FVector PlayerForward = GetActorForwardVector();
	FVector PlayerLocation = GetActorLocation();

	// Calculate the end location for the sphere trace
	FVector TraceEnd = PlayerLocation + (PlayerForward * MaxDistance);

	// Define the collision parameters
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the player character

	// Array to store hit results
	TArray<FHitResult> HitResults;

	// Perform the sphere trace
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		PlayerLocation,
		TraceEnd,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(100.0f), // Radius of the sphere
		QueryParams
	);


	// Check the hit results
	for (const FHitResult& Hit : HitResults)
	{
		AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(Hit.GetActor());
		if (Enemy)
		{
			// Check if the enemy is facing away from the player
			FVector EnemyForward = Enemy->GetActorForwardVector();
			FVector DirectionToEnemy = (Enemy->GetActorLocation() - PlayerLocation).GetSafeNormal();
			float DotProduct = FVector::DotProduct(EnemyForward, DirectionToEnemy);

			// Assuming 0.5 corresponds to approximately 60 degrees cone behind the enemy
			if (DotProduct > 0.5f && !Enemy->bPlayerSpotted)
			{
				outFoundEnemy = Enemy;
				// Enemy is within distance, in front of the player, and facing away
				return true;
			}
		}
	}

	// No valid enemy found
	return false;
}

void AKnight::OnSlashAttackFinished()
{
	Super::OnSlashAttackFinished();
	OnSlashAttackFinishedEvent();
}

void AKnight::StartBlocking()
{
	if (IsPlayingAnyMontage()) { return; }

	if (bIsAttacking)
	{
		OnAttackFinished();
	}
	bIsAttacking = false;
	bIsBlocking = true;
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	PlayBlockingMontageEvent();
}

void AKnight::BackstabEnemy(AMeleeEnemy* Enemy)
{
	bBackstab = true;
	PlayBackstabMontageEvent();
}

void AKnight::PerformAttack()
{
	Attack();
}

#pragma endregion Sword Combat
