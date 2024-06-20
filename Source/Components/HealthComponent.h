// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDCOMBATSYSTEM_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* GetLastDamageCauser() const { return LastDamageCauser; }

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHealthPercent() const;
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);
	void SetMaxHealth(float MaximumHealth);
	void Init(float MaximumHealth);

	FSimpleDelegate& OnActorDeath() { return OnActorDeathDelegate; }

private:

	int32 CurrentHealth;
	int32 MaxHealth;

	AActor* LastDamageCauser;

	FSimpleDelegate OnActorDeathDelegate;

};
