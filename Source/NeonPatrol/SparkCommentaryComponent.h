// SparkCommentaryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SparkCommentaryComponent.generated.h"

class USparkBrainComponent;
class USparkVoiceComponent;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class NEONPATROL_API USparkCommentaryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USparkCommentaryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnCombatStart();

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnEnemyKilled();

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnPlayerHit();

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnSparkHit();

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnWaveCleared();

    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void OnLowHealth();

protected:
    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> CombatStart;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> EnemyKilled;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> PlayerHit;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> SparkHit;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> WaveCleared;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> IdleChatter;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    TArray<FString> LowHealth;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    float MinCommentInterval = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Spark|Commentary")
    float IdleChatInterval = 15.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Spark|Commentary")
    void SayComment(const FString& Comment);

private:

    USparkBrainComponent* BrainComp = nullptr;
    USparkVoiceComponent* VoiceComp = nullptr;

    float LastCommentTime = 0.0f;
    float LastIdleTime = 0.0f;
};
