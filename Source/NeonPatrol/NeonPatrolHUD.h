#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NeonPatrolHUD.generated.h"

class UProgressBar;
class UTextBlock;
class ANeonPatrolGameMode;

UCLASS(BlueprintType)
class NEONPATROL_API UNeonPatrolHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void UpdateHealth(float Percent);

    UFUNCTION(BlueprintCallable)
    void UpdateWave(int32 Wave);

    UFUNCTION(BlueprintCallable)
    void UpdateScore(int32 Score);

    UFUNCTION(BlueprintCallable)
    void ShowGameOver();

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* WaveText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;
};
