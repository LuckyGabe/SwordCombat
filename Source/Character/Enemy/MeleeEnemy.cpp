// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "Character/Enemy/MeleeEnemy.h"

#include "Controllers/AI/MeleeEnemyAIController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AMeleeEnemy::AMeleeEnemy()
{
	SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SwordMesh");
	SwordMesh->SetupAttachment(GetMesh(), SwordSocketName);

	// Set the enemy's team ID
	TeamId = FGenericTeamId(2);
}

void AMeleeEnemy::StartBlocking()
{
	if (bIsBlocking) { return; }

	//Reset attack
	OnAttackFinished();

	bIsBlocking = true;

	//Prevent the character from moving
	GetCharacterMovement()->MaxWalkSpeed = 0.f;

	// Stop blocking after time
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	if (TimerManager.IsTimerActive(BlockingTimerHandle))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(BlockingTimerHandle, this, &AMeleeEnemy::StopBlocking, BlockingTime, false);
}

void AMeleeEnemy::StopBlocking()
{
	Super::StopBlocking();

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	if (TimerManager.IsTimerActive(BlockingTimerHandle))
	{
		TimerManager.ClearTimer(BlockingTimerHandle);
		return;
	}

	// Reset behavior tree so the AI could re-decide next move
	AMeleeEnemyAIController* AIController = Cast<AMeleeEnemyAIController>(GetController());

	if (!IsValid(AIController))
	{
		return;
	}
	AIController->ResetBehaviorTree();
}

void AMeleeEnemy::OnCharacterDeath()
{
	Super::OnCharacterDeath();
	AAIController* AIController = Cast<AAIController>(GetController());

	// If it's an AI controller, cast and stop movement
	if (IsValid(AIController))
	{
		// Stop movement
		AIController->StopMovement();

		// Optionally clear the focus and other AI states
		AIController->ClearFocus(EAIFocusPriority::Gameplay);

		// Optionally destroy or disable the AI Controller
		AIController->UnPossess();
	}

	// Boolean flag to easily chose on which enemy should the finisher be performed
	if (bFinisherOnDeath)
	{
		ABaseCharacter* Killer = Cast<ABaseCharacter>(HealthComponent->GetLastDamageCauser());
		if (IsValid(Killer))
		{
			StartFinisherSequence(Killer);
			return;
		}
		return;
	}


}


void AMeleeEnemy::OnAttackFinished()
{
	Super::OnAttackFinished();

	FTimerHandle TempHandle;

	AMeleeEnemyAIController* AIController = Cast<AMeleeEnemyAIController>(GetController());
	if (IsValid(AIController))
	{
		AIController->ResetBehaviorTree();
	}	

	GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &AMeleeEnemy::PostAttackDelay, ComboCooldown, false, ComboCooldown);
}

void AMeleeEnemy::PostAttackDelay()
{
	bIsAttacking = false;
}

void AMeleeEnemy::OnSlashAttackFinished()
{
	Super::OnSlashAttackFinished();

	AMeleeEnemyAIController* AIController = Cast<AMeleeEnemyAIController>(GetController());

	if (!IsValid(AIController) || !AIController->bPlayerInAttackRange)
	{
		OnAttackFinished();
	}
}

void AMeleeEnemy::StartFinisherSequence(ABaseCharacter* Killer)
{
	if (!Killer || !IsValid(Killer))
	{
		UE_LOG(LogTemp, Error, TEXT("Killer is invalid in AMeleeEnemy::StartFinisherSequence."));
		return;
	}

	//Reset attack
	Killer->OnAttackFinished();

	BaseDamage = 0;

	//Calculate and set the correct positions and rotations so the characters would face each other perfectly while the animations play

	FVector PlayerLocation = Killer->GetActorLocation();
	FVector AILocation = GetActorLocation();

	FVector DirectionToAI = (AILocation - PlayerLocation).GetSafeNormal();
	FVector DirectionToPlayer = (PlayerLocation - AILocation).GetSafeNormal();

	FRotator PlayerRotation = DirectionToAI.Rotation();
	FRotator AIRotation = DirectionToPlayer.Rotation();

	SetActorRotation(AIRotation);

	FVector TargetLocation = AILocation + DistanceInFinisherSequence * GetActorForwardVector();

	Killer->SetActorLocation(TargetLocation);
	Killer->SetActorRotation(PlayerRotation);

	// Play one animation on the dying AI, and the other on the player's character
	PlayFinisherMontageEvent_Defend();
	Killer->PlayFinisherMontageEvent_Attack();

}



