#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SparkCharacter.generated.h"

class UCombatComponent;
class ARobotEnemy;
class AProjectile;

UCLASS(BlueprintType)
class NEONPATROL_API ASparkCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASparkCharacter();

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
    float FollowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
    AActor* FollowTarget;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackCooldown;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCombatComponent* CombatComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* BodyMesh;

private:
    float LastAttackTime;
    AActor* CurrentEnemy;

    UFUNCTION()
    void ShootAtEnemy();
};
