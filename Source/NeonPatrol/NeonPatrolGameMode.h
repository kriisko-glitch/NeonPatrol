#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NeonPatrolGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

class ASparkCharacter;
class AWaveSpawner;
class ANeonPatrolCharacter;

UCLASS(BlueprintType)
class NEONPATROL_API ANeonPatrolGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANeonPatrolGameMode();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<ASparkCharacter> SparkClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSubclassOf<AWaveSpawner> WaveSpawnerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    FVector SparkSpawnOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    int32 TotalScore;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    int32 CurrentWave;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    bool bGameOver;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGameOver OnGameOver;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreChanged OnScoreChanged;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void RestartGame();

protected:
    virtual void BeginPlay() override;

private:
    ASparkCharacter* SparkRef;
    AWaveSpawner* WaveSpawnerRef;

    UFUNCTION()
    void HandlePlayerDeath();

    UFUNCTION()
    void HandleWaveCleared(int32 WaveNumber);

    UFUNCTION()
    void HandleWaveStarted(int32 WaveNumber);

    void StartWaves();
};
