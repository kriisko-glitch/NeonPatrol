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

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
}

void ASparkCharacter::ExecuteCommand(ESparkCommand Command, float Param)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

    switch (Command)
    {
    case ESparkCommand::Follow:
        bShouldFollow = true;
        bHasMoveTarget = false;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: FOLLOW mode"));
        break;

    case ESparkCommand::Stay:
        bShouldFollow = false;
        bHasMoveTarget = false;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: STAY mode"));
        break;

    case ESparkCommand::Attack:
        bShouldAttack = true;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: ATTACK enabled"));
        break;

    case ESparkCommand::HoldFire:
        bShouldAttack = false;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: HOLD FIRE"));
        break;

    case ESparkCommand::ComeHere:
        if (PlayerPawn)
        {
            MoveTargetLocation = PlayerPawn->GetActorLocation();
            bHasMoveTarget = true;
            bShouldFollow = false;
        }
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: COME HERE"));
        break;

    case ESparkCommand::MoveForward:
    case ESparkCommand::MoveBack:
    case ESparkCommand::MoveLeft:
    case ESparkCommand::MoveRight:
    {
        if (!PlayerPawn) break;
        float Dist = (Param > 0.f) ? Param : 500.f;
        FRotator PlayerRot = PlayerPawn->GetActorRotation();
        FVector Dir;
        if (Command == ESparkCommand::MoveForward)
            Dir = FRotationMatrix(PlayerRot).GetUnitAxis(EAxis::X);
        else if (Command == ESparkCommand::MoveBack)
            Dir = -FRotationMatrix(PlayerRot).GetUnitAxis(EAxis::X);
        else if (Command == ESparkCommand::MoveLeft)
            Dir = -FRotationMatrix(PlayerRot).GetUnitAxis(EAxis::Y);
        else
            Dir = FRotationMatrix(PlayerRot).GetUnitAxis(EAxis::Y);

        MoveTargetLocation = GetActorLocation() + Dir * Dist;
        bHasMoveTarget = true;
        bShouldFollow = false;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: MOVE %s %.0f units"), *UEnum::GetValueAsString(Command), Dist);
        break;
    }

    default:
        break;
    }
}

void ASparkCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-find player
    if (!FollowTarget)
    {
        FollowTarget = UGameplayStatics::GetPlayerPawn(this, 0);
    }

    // --- Enemy targeting (only if attacking is enabled) ---
    CurrentEnemy = nullptr;
    if (bShouldAttack)
    {
        float ClosestDist = AttackRange;
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

        for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
        {
            ACharacter* Candidate = *It;
            if (!Candidate || Candidate == this || Candidate == PlayerPawn) continue;
            if (!Candidate->GetController() || Candidate->GetController()->IsPlayerController()) continue;
            if (Candidate->GetCharacterMovement() && Candidate->GetCharacterMovement()->MovementMode == MOVE_None) continue;

            float Dist = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                CurrentEnemy = Candidate;
            }
        }

        // Shoot if we have an enemy and cooldown elapsed
        if (CurrentEnemy && (GetWorld()->GetTimeSeconds() - LastAttackTime) >= AttackCooldown)
        {
            ShootAtEnemy();
        }
    }

    // --- Movement ---
    if (bHasMoveTarget)
    {
        MoveTowardTarget(DeltaTime);
    }
    else if (CurrentEnemy && bShouldAttack)
    {
        // Face enemy while in combat
        FVector Dir = (CurrentEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator LookAtRot = FRotationMatrix::MakeFromX(Dir).Rotator();
        SetActorRotation(FRotator(0.0f, LookAtRot.Yaw, 0.0f));
    }
    else if (bShouldFollow && FollowTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), FollowTarget->GetActorLocation());
        if (DistToTarget > FollowDistance)
        {
            FVector Direction = (FollowTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            AddMovementInput(Direction, 1.0f);
        }
    }
}

void ASparkCharacter::MoveTowardTarget(float DeltaTime)
{
    float Dist = FVector::Dist2D(GetActorLocation(), MoveTargetLocation);
    if (Dist < 50.f)
    {
        // Arrived
        bHasMoveTarget = false;
        return;
    }

    FVector Dir = (MoveTargetLocation - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Dir, 1.0f);
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
