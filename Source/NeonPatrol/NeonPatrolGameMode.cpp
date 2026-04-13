#include "NeonPatrolGameMode.h"
#include "SparkCharacter.h"
#include "WaveSpawner.h"
#include "NeonPatrolCharacter.h"
#include "CombatComponent.h"
#include "NeonPatrol.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ANeonPatrolGameMode::ANeonPatrolGameMode()
{
    SparkSpawnOffset = FVector(200.0f, 100.0f, 0.0f);
    TotalScore = 0;
    CurrentWave = 0;
    bGameOver = false;
}

void ANeonPatrolGameMode::BeginPlay()
{
    Super::BeginPlay();

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    if (SparkClass)
    {
        FVector SpawnLocation = PlayerPawn->GetActorLocation() + SparkSpawnOffset;
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SparkRef = GetWorld()->SpawnActor<ASparkCharacter>(SparkClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (SparkRef)
        {
            SparkRef->FollowTarget = PlayerPawn;
        }
    }

    if (WaveSpawnerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        WaveSpawnerRef = GetWorld()->SpawnActor<AWaveSpawner>(WaveSpawnerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (WaveSpawnerRef)
        {
            WaveSpawnerRef->OnWaveStarted.AddDynamic(this, &ANeonPatrolGameMode::HandleWaveStarted);
            WaveSpawnerRef->OnWaveCleared.AddDynamic(this, &ANeonPatrolGameMode::HandleWaveCleared);
        }
    }

    ANeonPatrolCharacter* PlayerChar = Cast<ANeonPatrolCharacter>(PlayerPawn);
    if (PlayerChar && PlayerChar->GetCombatComponent())
    {
        PlayerChar->GetCombatComponent()->OnDeath.AddDynamic(this, &ANeonPatrolGameMode::HandlePlayerDeath);
    }

    FTimerHandle TempHandle;
    GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &ANeonPatrolGameMode::StartWaves, 5.0f, false);
}

void ANeonPatrolGameMode::StartWaves()
{
    if (WaveSpawnerRef)
    {
        WaveSpawnerRef->BeginWaves();
    }
}

void ANeonPatrolGameMode::HandlePlayerDeath()
{
    bGameOver = true;
    UE_LOG(LogTemp, Log, TEXT("GAME OVER"));
    OnGameOver.Broadcast();
}

void ANeonPatrolGameMode::HandleWaveCleared(int32 WaveNumber)
{
    TotalScore += WaveNumber * 100;
    CurrentWave = WaveNumber;
    OnScoreChanged.Broadcast(TotalScore);
    UE_LOG(LogTemp, Log, TEXT("Wave %d cleared"), WaveNumber);
}

void ANeonPatrolGameMode::HandleWaveStarted(int32 WaveNumber)
{
    CurrentWave = WaveNumber;
    UE_LOG(LogTemp, Log, TEXT("Wave %d started"), WaveNumber);
}

void ANeonPatrolGameMode::RestartGame()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
