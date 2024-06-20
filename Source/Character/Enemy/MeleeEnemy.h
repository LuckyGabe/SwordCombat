// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "MeleeEnemy.generated.h"

/**
 *
 */
UCLASS()
class SWORDCOMBATSYSTEM_API AMeleeEnemy : public ABaseCharacter
{
	GENERATED_BODY()


public:
	AMeleeEnemy();
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

protected:
	virtual USkeletalMeshComponent* GetSwordMesh() override { return SwordMesh; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void StartBlocking() override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void StopBlocking() override;

	virtual void OnCharacterDeath() override;

public:
	virtual void OnAttackFinished() override;

	virtual void OnSlashAttackFinished() override;

private:
	void StartFinisherSequence(ABaseCharacter* Killer);

	void PostAttackDelay();

public:
	bool bPlayerSpotted = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bFinisherOnDeath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ComboCooldown = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMeshComponent* SwordMesh;

	FTimerHandle BlockingTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BlockingTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	FName SwordSocketName = "SwordSocket";

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DistanceInFinisherSequence = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	int32 BlockChancePercentage = 20.f;
};
