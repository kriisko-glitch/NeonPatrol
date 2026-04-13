#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RobotEnemy.h"
#include "WaveSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32, WaveNumber);

UCLASS(BlueprintType)
class NEONPATROL_API AWaveSpawner : public AActor
{
    GENERATED_BODY()

public:
    AWaveSpawner();

    UPROPERTY(BlueprintReadOnly, Category = "Wave")
    int32 CurrentWave;

    UPROPERTY(BlueprintReadOnly, Category = "Wave")
    int32 EnemiesAlive;

    UPROPERTY(EditAnywhere, Category = "Wave")
    float TimeBetweenWaves;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<FVector> SpawnPoints;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<ARobotEnemy> EnemyClass;

    UPROPERTY(BlueprintReadOnly, Category = "Wave")
    bool bWaveActive;

    UFUNCTION(BlueprintCallable, Category = "Wave")
    void StartNextWave();

    UFUNCTION(BlueprintCallable, Category = "Wave")
    void BeginWaves();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWaveStarted OnWaveStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWaveCleared OnWaveCleared;

protected:
    virtual void BeginPlay() override;

private:
    float WaveEndTime;
    FTimerHandle WaveTimerHandle;

    UFUNCTION()
    void SpawnEnemy();

    UFUNCTION()
    void OnEnemyDied(ARobotEnemy* Robot);
};
