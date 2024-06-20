// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#include "AI/BTTasks/BTTask_Attack.h"

#include "Character/BaseCharacter.h"

#include "AIController.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}



EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!OwnerComp.GetAIOwner())
	{
		return EBTNodeResult::Failed;
	}

	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (!IsValid(OwnerCharacter))
	{
		return EBTNodeResult::Failed;
	}

	if (OwnerCharacter->IsPlayingAnyMontage() || OwnerCharacter->GetIsAttacking())
	{
		return EBTNodeResult::Failed;
	}

	OwnerCharacter->Attack();

	return EBTNodeResult::InProgress;
}





