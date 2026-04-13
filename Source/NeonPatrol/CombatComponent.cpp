#include "CombatComponent.h"
#include "NeonPatrol.h"

UCombatComponent::UCombatComponent()
{
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void UCombatComponent::TakeDamage(float Amount)
{
	if (Amount <= 0.0f) return;
	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

void UCombatComponent::Heal(float Amount)
{
	if (Amount <= 0.0f) return;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

bool UCombatComponent::IsAlive() const
{
	return CurrentHealth > 0.0f;
}

float UCombatComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UCombatComponent::SetMaxHealth(float NewMax)
{
	MaxHealth = NewMax;
	CurrentHealth = MaxHealth;
}
