#include "WaveSpawner.h"
#include "RobotEnemy.h"
#include "NeonPatrol.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AWaveSpawner::AWaveSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentWave = 0;
    EnemiesAlive = 0;
    TimeBetweenWaves = 30.0f;
    bWaveActive = false;
    WaveEndTime = 0.0f;
}

void AWaveSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (SpawnPoints.Num() == 0)
    {
        SpawnPoints.Add(FVector(1000.0f, 1000.0f, 0.0f));
        SpawnPoints.Add(FVector(-1000.0f, 1000.0f, 0.0f));
        SpawnPoints.Add(FVector(1000.0f, -1000.0f, 0.0f));
        SpawnPoints.Add(FVector(-1000.0f, -1000.0f, 0.0f));
    }
}

void AWaveSpawner::BeginWaves()
{
    StartNextWave();
}

void AWaveSpawner::StartNextWave()
{
    CurrentWave++;
    int32 EnemyCount = FMath::Min(2 + CurrentWave, 15);
    bWaveActive = true;

    for (int32 i = 0; i < EnemyCount; i++)
    {
        SpawnEnemy();
    }

    OnWaveStarted.Broadcast(CurrentWave);
}

void AWaveSpawner::SpawnEnemy()
{
    if (SpawnPoints.Num() == 0 || !EnemyClass) return;

    FVector BaseLocation = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
    BaseLocation.X += FMath::RandRange(-200, 200);
    BaseLocation.Y += FMath::RandRange(-200, 200);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ARobotEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ARobotEnemy>(EnemyClass, BaseLocation, FRotator::ZeroRotator, SpawnParams);
    if (!SpawnedEnemy) return;

    if (CurrentWave % 5 == 0)
    {
        SpawnedEnemy->RobotType = ERobotType::Heavy;
    }
    else
    {
        SpawnedEnemy->RobotType = (FMath::RandRange(0, 9) < 7) ? ERobotType::Drone : ERobotType::Turret;
    }

    SpawnedEnemy->SetTarget(UGameplayStatics::GetPlayerPawn(this, 0));
    SpawnedEnemy->OnRobotDied.AddDynamic(this, &AWaveSpawner::OnEnemyDied);
    EnemiesAlive++;
}

void AWaveSpawner::OnEnemyDied(ARobotEnemy* Robot)
{
    EnemiesAlive--;

    if (EnemiesAlive <= 0 && bWaveActive)
    {
        bWaveActive = false;
        OnWaveCleared.Broadcast(CurrentWave);
        GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &AWaveSpawner::StartNextWave, TimeBetweenWaves, false);
    }
}
