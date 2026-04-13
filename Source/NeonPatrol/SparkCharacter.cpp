#include "SparkCharacter.h"
#include "CombatComponent.h"
#include "SparkBrainComponent.h"
#include "SparkVoiceComponent.h"
#include "SparkCommentaryComponent.h"
#include "RobotEnemy.h"
#include "Projectile.h"
#include "NeonPatrol.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Materials/MaterialInstanceDynamic.h"

ASparkCharacter::ASparkCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
    CombatComp->MaxHealth = 200;

    BrainComp = CreateDefaultSubobject<USparkBrainComponent>(TEXT("BrainComp"));
    VoiceComp = CreateDefaultSubobject<USparkVoiceComponent>(TEXT("VoiceComp"));
    CommentaryComp = CreateDefaultSubobject<USparkCommentaryComponent>(TEXT("CommentaryComp"));

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

    case ESparkCommand::Aggressive:
        CombatMode = ESparkCombatMode::Aggressive;
        bShouldAttack = true;
        bShouldFollow = true;
        FollowDistance = 100.f;
        AttackCooldown = 0.8f;
        AttackRange = 1200.f;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: AGGRESSIVE mode — fast fire, wide range"));
        break;

    case ESparkCommand::Defensive:
        CombatMode = ESparkCombatMode::Defensive;
        bShouldAttack = true;
        bShouldFollow = true;
        FollowDistance = 150.f;
        AttackCooldown = 2.0f;
        AttackRange = 400.f;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: DEFENSIVE mode — close protection only"));
        break;

    case ESparkCommand::Scout:
        if (PlayerPawn)
        {
            FVector ScoutTarget = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 1500.f;
            MoveTargetLocation = ScoutTarget;
            bHasMoveTarget = true;
            bShouldFollow = false;

            // Return after 3 seconds
            FTimerHandle ScoutTimer;
            GetWorld()->GetTimerManager().SetTimer(ScoutTimer, [this]()
            {
                bShouldFollow = true;
                bHasMoveTarget = false;
                if (CommentaryComp)
                {
                    CommentaryComp->SayComment(TEXT("Area scanned. Returning to you."));
                }
            }, 3.0f, false);
        }
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: SCOUT — moving ahead to scan"));
        break;

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

    // Leash check — teleport back if too far or fell off map
    EnforceLeash();

    // Update visual color based on state
    UpdateColorState(DeltaTime);

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

        // Combat commentary: entering/leaving combat
        bool bInCombat = (CurrentEnemy != nullptr);
        if (bInCombat && !bWasInCombat && CommentaryComp)
        {
            CommentaryComp->OnCombatStart();
        }
        bWasInCombat = bInCombat;
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

void ASparkCharacter::UpdateColorState(float DeltaTime)
{
    if (!BodyMesh) return;

    ColorPulseTimer += DeltaTime;
    float Pulse = (FMath::Sin(ColorPulseTimer * 6.f) + 1.f) * 0.5f; // 0-1 pulsing

    FLinearColor Color;

    if (CurrentEnemy && bShouldAttack)
    {
        // Red pulsing — actively attacking
        Color = FLinearColor::LerpUsingHSV(FLinearColor(0.8f, 0.1f, 0.1f), FLinearColor(1.f, 0.3f, 0.1f), Pulse);
    }
    else if (bShouldAttack && !CurrentEnemy)
    {
        // Check if any enemy is within 1500 units (alert range)
        bool bEnemyNearby = false;
        for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
        {
            ACharacter* C = *It;
            APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
            if (!C || C == this || C == Player) continue;
            if (!C->GetController() || C->GetController()->IsPlayerController()) continue;
            if (FVector::Dist(GetActorLocation(), C->GetActorLocation()) < 1500.f)
            {
                bEnemyNearby = true;
                break;
            }
        }
        if (bEnemyNearby)
        {
            // Orange pulsing — enemies nearby
            Color = FLinearColor::LerpUsingHSV(FLinearColor(1.f, 0.5f, 0.f), FLinearColor(1.f, 0.7f, 0.2f), Pulse);
        }
        else
        {
            // Green — all clear
            Color = FLinearColor(0.1f, 0.9f, 0.3f);
        }
    }
    else
    {
        // Blue-green — hold fire / passive
        Color = FLinearColor(0.1f, 0.6f, 0.8f);
    }

    SetSparkColor(Color);
}

void ASparkCharacter::SetSparkColor(const FLinearColor& Color)
{
    if (!BodyMesh) return;

    UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(BodyMesh->GetMaterial(0));
    if (!DynMat)
    {
        UMaterialInterface* BaseMat = BodyMesh->GetMaterial(0);
        if (BaseMat)
        {
            DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
            BodyMesh->SetMaterial(0, DynMat);
        }
    }
    if (DynMat)
    {
        DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
    }
}

void ASparkCharacter::EnforceLeash()
{
    if (!FollowTarget) return;

    float Dist = FVector::Dist(GetActorLocation(), FollowTarget->GetActorLocation());
    float Z = GetActorLocation().Z;

    if (Dist > MaxLeashDistance || Z < MinFloorZ)
    {
        // Teleport back near the player
        FVector TeleportLoc = FollowTarget->GetActorLocation() + FVector(200.f, 100.f, 0.f);
        SetActorLocation(TeleportLoc);
        bHasMoveTarget = false;
        bShouldFollow = true;
        UE_LOG(LogNeonPatrol, Log, TEXT("Spark: Leash teleport — was %.0f units away"), Dist);
    }
}
