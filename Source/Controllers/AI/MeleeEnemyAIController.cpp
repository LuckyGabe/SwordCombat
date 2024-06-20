// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "Controllers/AI/MeleeEnemyAIController.h"

#include "Character/BaseCharacter.h"
#include "Character/Enemy/MeleeEnemy.h"


#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

AMeleeEnemyAIController::AMeleeEnemyAIController()
{
	// Create and initialize the perception component
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	// Create and configure the sight configuration
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;

	// For this project the AI will detect only the player
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	// Set the sight configuration to the perception component
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());

	//Bind the handler function to perception update
	PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AMeleeEnemyAIController::OnPerceptionUpdate);
}



void AMeleeEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	if (MeleeEnemyBehaviorTree != nullptr)
	{
		GetPerceptionComponent()->Activate(true);
		RunBehaviorTree(MeleeEnemyBehaviorTree);
	}
}

void AMeleeEnemyAIController::UpdatePlayerLocation()
{
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	ABaseCharacter* EnemyCharacter = Cast<ABaseCharacter>(GetPawn());

	if (!IsValid(EnemyCharacter) || !IsValid(PlayerActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get the characters! AMeleeEnemyAIController::UpdatePlayerLocation"));

		return;
	}

	if (PlayerActor)
	{
		FVector PlayerLocation = PlayerActor->GetActorLocation();


		float DistanceToPlayer = FVector::Distance(GetPawn()->GetActorLocation(), PlayerLocation);

		// Adjust by 0.5m to avoid flickering between states
		if (DistanceToPlayer > AttackRange - 50.f)
		{
			if (!EnemyCharacter->GetIsAttacking())
			{
				GetBlackboardComponent()->SetValueAsVector("PlayerLocation", PlayerLocation);
			}

			GetBlackboardComponent()->SetValueAsBool("bPlayerInAttackRange", false);
			bPlayerInAttackRange = false;
		}
		else
		{
			GetBlackboardComponent()->SetValueAsBool("bPlayerInAttackRange", true);
			GetBlackboardComponent()->ClearValue("PlayerLocation");
			bPlayerInAttackRange = true;
		}


	}
}




void AMeleeEnemyAIController::ResetBehaviorTree()
{
	//UBehaviorTreeComponent* BehaviorTreeComp = Cast<>
	if (IsValid(GetBrainComponent()))
	{
		GetBrainComponent()->RestartLogic();
	}

}

void AMeleeEnemyAIController::OnPerceptionUpdate(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo Info;
		PerceptionComponent->GetActorsPerception(Actor, Info);

		// Actor was seen
		if (Info.LastSensedStimuli[0].WasSuccessfullySensed())
		{
			ABaseCharacter* EnemyCharacter = Cast<ABaseCharacter>(GetPawn());

			if (!IsValid(EnemyCharacter))
			{
				return;
			}

			ETeamAttitude::Type Attitude = EnemyCharacter->GetTeamAttitudeTowards(*Actor);

			// Actor is an enemy
			if (Attitude == ETeamAttitude::Hostile)
			{
				SetFocus(Actor, EAIFocusPriority::Gameplay);

				AMeleeEnemy* MeleeEnemy = Cast<AMeleeEnemy>(GetPawn());
				if (IsValid(MeleeEnemy))
				{
					MeleeEnemy->bPlayerSpotted = true;
				}

				GetWorldTimerManager().SetTimer(PlayerTrackingTimerHandle, this, &AMeleeEnemyAIController::UpdatePlayerLocation, 0.1f, true);
			}
		}
		// Actor was lost
		else
		{
			AMeleeEnemy* MeleeEnemy = Cast<AMeleeEnemy>(GetPawn());
			if (IsValid(MeleeEnemy))
			{
				MeleeEnemy->bPlayerSpotted = false;
			}
			GetWorldTimerManager().ClearTimer(PlayerTrackingTimerHandle);
			GetBlackboardComponent()->ClearValue("PlayerLocation");
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}



