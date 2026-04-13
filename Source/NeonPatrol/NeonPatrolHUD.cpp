#include "NeonPatrolHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CombatComponent.h"
#include "NeonPatrolCharacter.h"
#include "NeonPatrolGameMode.h"
#include "NeonPatrol.h"
#include "Kismet/GameplayStatics.h"

void UNeonPatrolHUD::NativeConstruct()
{
    Super::NativeConstruct();
    if (GameOverText)
    {
        GameOverText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UNeonPatrolHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (Pawn)
    {
        if (UCombatComponent* CombatComp = Pawn->FindComponentByClass<UCombatComponent>())
        {
            UpdateHealth(CombatComp->GetHealthPercent());
        }
    }

    if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this))
    {
        if (ANeonPatrolGameMode* NeonGM = Cast<ANeonPatrolGameMode>(GameMode))
        {
            UpdateWave(NeonGM->CurrentWave);
            UpdateScore(NeonGM->TotalScore);
            if (NeonGM->bGameOver)
            {
                ShowGameOver();
            }
        }
    }
}

void UNeonPatrolHUD::UpdateHealth(float Percent)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(Percent);
        if (Percent > 0.5f)
        {
            HealthBar->SetFillColorAndOpacity(FLinearColor::Green);
        }
        else if (Percent > 0.25f)
        {
            HealthBar->SetFillColorAndOpacity(FLinearColor::Yellow);
        }
        else
        {
            HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
        }
    }
}

void UNeonPatrolHUD::UpdateWave(int32 Wave)
{
    if (WaveText)
    {
        WaveText->SetText(FText::FromString(FString::Printf(TEXT("WAVE %d"), Wave)));
    }
}

void UNeonPatrolHUD::UpdateScore(int32 InScore)
{
    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("SCORE: %d"), InScore)));
    }
}

void UNeonPatrolHUD::ShowGameOver()
{
    if (GameOverText)
    {
        GameOverText->SetText(FText::FromString(TEXT("GAME OVER")));
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}
