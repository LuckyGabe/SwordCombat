// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MeleeEnemyAIController.generated.h"

 
class UBehaviorTree;

UCLASS()
class SWORDCOMBATSYSTEM_API AMeleeEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AMeleeEnemyAIController();

#pragma region Inherited
public:
	//virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
#pragma endregion Inherited


public:

	UBehaviorTree* GetBehaviorTree() const { return MeleeEnemyBehaviorTree; }

	void ResetBehaviorTree();

private:
	UFUNCTION()
	void OnPerceptionUpdate(const TArray<AActor*>& UpdatedActors);

	void UpdatePlayerLocation();

public:
	bool bPlayerInAttackRange = false;

private:
	UPROPERTY(EditAnywhere)
	float SightRadius = 1500.f;

	UPROPERTY(EditAnywhere)
	float LoseSightRadius = 500.f;

	UPROPERTY(EditAnywhere)
	float PeripheralVisionAngleDegrees = 90.f;

	UPROPERTY(EditAnywhere)
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly)
	UBehaviorTree* MeleeEnemyBehaviorTree;

	FTimerHandle PlayerTrackingTimerHandle;
};
