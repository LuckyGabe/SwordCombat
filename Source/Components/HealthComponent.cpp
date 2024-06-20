// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "Components/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth == 0)
	{
		return 0.f;
	}
	float perc = float(CurrentHealth) / float(MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("health percent: %f"), perc);
	return perc;
}

void UHealthComponent::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("CurrentHealth: %i, MaxHealth: %i Damage: %f . UHealthComponent::ApplyDamage"), CurrentHealth, MaxHealth, DamageAmount);

	CurrentHealth -= DamageAmount;
	LastDamageCauser = DamageCauser;
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
		OnActorDeath().ExecuteIfBound();
	}
}

void UHealthComponent::SetMaxHealth(float MaximumHealth)
{
	MaxHealth = MaximumHealth;
}

void UHealthComponent::Init(float MaximumHealth)
{
	SetMaxHealth(MaximumHealth);
	CurrentHealth = MaxHealth;
}
