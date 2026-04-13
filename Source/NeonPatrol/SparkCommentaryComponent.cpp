// SparkCommentaryComponent.cpp
#include "SparkCommentaryComponent.h"
#include "SparkBrainComponent.h"
#include "SparkVoiceComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

USparkCommentaryComponent::USparkCommentaryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    CombatStart = { "Enemies incoming!", "Here they come!", "Weapons hot!", "Contact!" };
    EnemyKilled = { "Got one!", "Target down!", "Scratch one!", "That's a kill!" };
    PlayerHit = { "Watch out, partner!", "You're taking hits!", "Stay sharp!" };
    SparkHit = { "I'm hit!", "Taking fire!", "Ow, my circuits!" };
    WaveCleared = { "Area clear.", "Wave down! Nice work.", "All hostiles eliminated." };
    IdleChatter = { "Quiet... too quiet.", "Sensors nominal.", "Ready when you are.", "Scanning perimeter.", "Hey partner, you good?" };
    LowHealth = { "I'm not looking great here.", "Damage critical!", "Need a breather." };
}

void USparkCommentaryComponent::BeginPlay()
{
    Super::BeginPlay();

    BrainComp = GetOwner()->FindComponentByClass<USparkBrainComponent>();
    VoiceComp = GetOwner()->FindComponentByClass<USparkVoiceComponent>();
}

void USparkCommentaryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastIdleTime >= IdleChatInterval && Now - LastCommentTime >= MinCommentInterval)
    {
        if (IdleChatter.Num() > 0)
        {
            const int32 Index = FMath::RandRange(0, IdleChatter.Num() - 1);
            SayComment(IdleChatter[Index]);
            LastIdleTime = Now;
        }
    }
}

void USparkCommentaryComponent::SayComment(const FString& Comment)
{
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastCommentTime < MinCommentInterval)
    {
        return;
    }

    if (BrainComp)
    {
        BrainComp->OnChatResponse.Broadcast("", Comment);
    }

    if (VoiceComp)
    {
        VoiceComp->SpeakText(Comment);
    }

    LastCommentTime = Now;
}

void USparkCommentaryComponent::OnCombatStart()
{
    if (CombatStart.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, CombatStart.Num() - 1);
        SayComment(CombatStart[Index]);
    }
}

void USparkCommentaryComponent::OnEnemyKilled()
{
    if (EnemyKilled.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, EnemyKilled.Num() - 1);
        SayComment(EnemyKilled[Index]);
    }
}

void USparkCommentaryComponent::OnPlayerHit()
{
    if (PlayerHit.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, PlayerHit.Num() - 1);
        SayComment(PlayerHit[Index]);
    }
}

void USparkCommentaryComponent::OnSparkHit()
{
    if (SparkHit.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, SparkHit.Num() - 1);
        SayComment(SparkHit[Index]);
    }
}

void USparkCommentaryComponent::OnWaveCleared()
{
    if (WaveCleared.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, WaveCleared.Num() - 1);
        SayComment(WaveCleared[Index]);
    }
}

void USparkCommentaryComponent::OnLowHealth()
{
    if (LowHealth.Num() > 0)
    {
        const int32 Index = FMath::RandRange(0, LowHealth.Num() - 1);
        SayComment(LowHealth[Index]);
    }
}
