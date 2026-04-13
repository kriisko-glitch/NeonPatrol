#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class NEONPATROL_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float CurrentHealth;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetMaxHealth(float NewMax);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealthPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHealthChanged OnHealthChanged;

protected:
	virtual void BeginPlay() override;
};
