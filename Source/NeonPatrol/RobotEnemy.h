#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RobotEnemy.generated.h"

class UCombatComponent;

UENUM(BlueprintType)
enum class ERobotType : uint8
{
	Drone UMETA(DisplayName = "Drone"),
	Turret UMETA(DisplayName = "Turret"),
	Heavy UMETA(DisplayName = "Heavy")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRobotDied, ARobotEnemy*, Robot);

UCLASS(BlueprintType)
class NEONPATROL_API ARobotEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ARobotEnemy();

	UPROPERTY(BlueprintAssignable, Category = "Robot")
	FOnRobotDied OnRobotDied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
	ERobotType RobotType;

	UFUNCTION(BlueprintCallable, Category = "Robot")
	void SetTarget(AActor* Target);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
	float AttackCooldown;

private:
	float LastAttackTime;
	AActor* TargetActor;

	UFUNCTION(BlueprintCallable, Category = "Robot")
	void Attack();

	UFUNCTION()
	void HandleOnDeath();

	void DeferredDestroy();
};
