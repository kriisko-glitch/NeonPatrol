#include "SparkCharacter.h"
#include "CombatComponent.h"
#include "SparkBrainComponent.h"
#include "SparkVoiceComponent.h"
#include "RobotEnemy.h"
#include "Projectile.h"
#include "NeonPatrol.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

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

    BrainComp = CreateDefaultSubobject<USparkBrainComponent>(TEXT("BrainComp"));
    VoiceComp = CreateDefaultSubobject<USparkVoiceComponent>(TEXT("VoiceComp"));

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(RootComponent);
    BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    GetCapsuleComponent()->SetCapsuleHalfHeight(50.0f);
    GetCapsuleComponent()->SetCapsuleRadius(30.0f);

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = 650.0f;
    Movement->bOrientRotationToMovement = true;
    Movement->DefaultLandMovementMode = EMovementMode::MOVE_Walking;

    // Auto-possess with AI controller so movement works when placed in level
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
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
        // Search all characters — fight any AI-controlled character (template CombatEnemy or our RobotEnemy)
        for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
        {
            ACharacter* Candidate = *It;
            if (!Candidate || Candidate == this || Candidate == PlayerPawn) continue;
            // Skip other friendly characters (only attack AI-controlled ones)
            if (!Candidate->GetController() || Candidate->GetController()->IsPlayerController()) continue;
            // Skip dead characters (check if movement is disabled as a proxy for death)
            if (Candidate->GetCharacterMovement() && Candidate->GetCharacterMovement()->MovementMode == MOVE_None) continue;

            float Dist = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                CurrentEnemy = Candidate;
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
