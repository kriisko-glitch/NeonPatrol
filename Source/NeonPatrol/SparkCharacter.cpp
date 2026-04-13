#include "SparkCharacter.h"
#include "CombatComponent.h"
#include "RobotEnemy.h"
#include "Projectile.h"
#include "NeonPatrol.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASparkCharacter::ASparkCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    FollowDistance = 300.0f;
    AttackRange = 800.0f;
    AttackDamage = 15.0f;
    AttackCooldown = 1.5f;
    LastAttackTime = 0.0f;
    CurrentEnemy = nullptr;

    CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
    CombatComp->MaxHealth = 200;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(RootComponent);
    BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    GetCapsuleComponent()->SetCapsuleHalfHeight(50.0f);
    GetCapsuleComponent()->SetCapsuleRadius(30.0f);

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = 650.0f;
    Movement->bOrientRotationToMovement = true;
    Movement->DefaultLandMovementMode = EMovementMode::MOVE_Walking;
}

void ASparkCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FollowTarget)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (PlayerPawn)
        {
            FollowTarget = PlayerPawn;
        }
    }

    CurrentEnemy = nullptr;
    float ClosestDist = AttackRange;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        for (TActorIterator<ARobotEnemy> It(GetWorld()); It; ++It)
        {
            ARobotEnemy* Enemy = *It;
            if (Enemy && Enemy->GetActorLocation() != FVector::ZeroVector)
            {
                float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    CurrentEnemy = Enemy;
                }
            }
        }
    }

    if (CurrentEnemy && (GetWorld()->GetTimeSeconds() - LastAttackTime) >= AttackCooldown)
    {
        ShootAtEnemy();
    }

    if (CurrentEnemy && FVector::Dist(GetActorLocation(), CurrentEnemy->GetActorLocation()) < AttackRange)
    {
        FVector Dir = (CurrentEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator LookAtRot = FRotationMatrix::MakeFromX(Dir).Rotator();
        SetActorRotation(FRotator(0.0f, LookAtRot.Yaw, 0.0f));
    }
    else if (FollowTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), FollowTarget->GetActorLocation());
        if (DistToTarget > FollowDistance)
        {
            FVector Direction = (FollowTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            AddMovementInput(Direction, 1.0f);
        }
    }
}

void ASparkCharacter::ShootAtEnemy()
{
    if (!CurrentEnemy) return;

    FVector MuzzleLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
    FVector TargetLocation = CurrentEnemy->GetActorLocation();
    FRotator FireRotation = (TargetLocation - MuzzleLocation).Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    if (AProjectile* Proj = GetWorld()->SpawnActor<AProjectile>(AProjectile::StaticClass(), MuzzleLocation, FireRotation, SpawnParams))
    {
        Proj->Damage = AttackDamage;
        FVector Dir = (TargetLocation - MuzzleLocation).GetSafeNormal();
        Proj->FireInDirection(Dir);
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
}
