// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"

#include "Components/HealthComponent.h"

#include "BaseCharacter.generated.h"

// Declare a multicast delegate type
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackCompleted);




class UHealthComponent;
class USoundCue;
class UNiagaraSystem;
class UUserWidget;

UCLASS()
class SWORDCOMBATSYSTEM_API ABaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

#pragma region Inherited
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

#pragma endregion Inherited

public:
	void PlayFootstepEffects(bool bRightFootstep);

#pragma region Sword Combat
public:
	//returns if any body mesh montage is playing
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual bool IsPlayingAnyMontage() const;

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> WBP_PlayerHUD;

#pragma region Attack

public:

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	virtual bool Attack();

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	void StartAttackLineTraceTimer();

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	void StopAttackLineTraceTimer();

	/*
	*	On each sword swing.
	*	Should clear AttackedActorsArray
	*/
	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	virtual void OnSlashAttackFinished();

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	virtual void OnAttackFinished();

	virtual void OnSwordsCross();
	virtual void OnSwordSwing();

	bool GetIsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat|Attack")
	void PlayFinisherMontageEvent_Attack();


protected:

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	virtual void PlayAttackMontage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Attack")
	void PlayAttackMontageEvent();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat|Attack")
	void PlayFinisherMontageEvent_Defend();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnAttackBlockedEvent();

	virtual void PerformAttackTrace();
	virtual USkeletalMeshComponent* GetSwordMesh() { return nullptr; }

protected:
	UPROPERTY(BlueprintReadWrite, Category = "BaseCharacter|Combat|Attack")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadWrite, Category = "BaseCharacter|Combat|Attack")
	int32 AttackCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "BaseCharacter|Combat|Attack")
	int32 MaxAttackCount = 3;

	FTimerHandle AttackTimerHandle;
	FGenericTeamId TeamId;

	bool bIsDodging;

	bool bBackstab = false;
private:

	/*
	*	Array of actors that already have been hit during single hit/slash
	*/
	TArray<AActor*> AttackedActorsArray;

#pragma endregion Attack

#pragma region Blocking

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool GetIsBlocking() const { return bIsBlocking; }


protected:
	virtual void StartBlocking() {}
	virtual void StopBlocking();
	virtual void OnAttackBlocked();
	virtual bool IsDamageBlockable(FDamageEvent DamageEvent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void DamageBlockedEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayBlockingMontageEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void StopBlockingMontageEvent();

protected:
	bool bIsBlocking;

#pragma endregion Blocking

#pragma region Health related
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool IsDead() const { return bIsDead; }

protected:

	virtual void OnDamageReceived();

	virtual void OnCharacterDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	 void PlayDyingMontageEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnDamageReceivedEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Stats")
	void UpdateHealthWidgetEvent();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseCharacter|Stats")
	UHealthComponent* HealthComponent;

private:
	bool bIsDead = false;


#pragma endregion Health related

#pragma region Statistics

protected:
	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Stats")
	int32 BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseCharacter|Stats|Movement")
	float BaseMaxWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Stats|Movement")
	float MaxWalkSpeedAttacking = 0;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Stats|Movement")
	float OnDamageStepBackForce = 100.f;

#pragma endregion Statistics

#pragma region Sound Cues

protected:

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Audio")
	USoundCue* AttackGroanSoundCue;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Audio")
	USoundCue* SwordHitSoundCue;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Audio")
	USoundCue* SwordSlashSoundCue;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Audio")
	USoundCue* DamageReceivedSoundCue;

	UPROPERTY(EditAnywhere, Category = "BaseCharacter|Audio")
	USoundCue* AttackBlockedSoundCue;

#pragma endregion Sound Cues

#pragma region Particles

protected:
	//Spawns footsteps particles 
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void SpawnFootstepEmitterEvent(FVector SpawnLocation);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseCharacter|Particles")
	UNiagaraSystem* FootstepEmitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseCharacter|Particles")
	UParticleSystem* SwordImpactEmmiter;

#pragma endregion Particles

#pragma endregion Sword Combat

};


