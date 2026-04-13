// SparkCommentaryComponent.cpp
#include "SparkCommentaryComponent.h"
#include "SparkBrainComponent.h"
#include "SparkVoiceComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

USparkCommentaryComponent::USparkCommentaryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    CombatStart = {
        TEXT("Contact! Weapons hot!"),
        TEXT("Here they come! Time to earn our pay."),
        TEXT("Hostiles detected. Let's dance."),
        TEXT("Incoming! I was getting bored anyway."),
        TEXT("Enemy signatures. Finally, some excitement!"),
        TEXT("Targets acquired. Shall we?"),
    };
    EnemyKilled = {
        TEXT("Got one! That's what I call precision."),
        TEXT("Target eliminated. Next!"),
        TEXT("Scratch one hostile."),
        TEXT("Down they go!"),
        TEXT("One less problem to worry about."),
        TEXT("Nice shooting, partner!"),
        TEXT("That one won't be bothering anyone."),
    };
    PlayerHit = {
        TEXT("Hey! Watch your six, partner!"),
        TEXT("You're taking hits! Stay behind me!"),
        TEXT("Careful out there, you're not made of metal."),
        TEXT("Duck and cover! I'll draw their fire."),
    };
    SparkHit = {
        TEXT("Ow! My beautiful chassis!"),
        TEXT("Taking fire! These guys have no respect."),
        TEXT("I felt that one. Rude."),
        TEXT("Direct hit. Still operational though!"),
    };
    WaveCleared = {
        TEXT("Area secure. Good work, partner."),
        TEXT("All hostiles down. We make a good team."),
        TEXT("Wave cleared! I barely broke a sweat. Do I sweat?"),
        TEXT("Sector clear. Catch your breath."),
        TEXT("That's the last of them. For now."),
    };
    IdleChatter = {
        TEXT("Quiet... too quiet. I don't trust it."),
        TEXT("All systems green. Just vibing."),
        TEXT("Ready when you are, boss."),
        TEXT("Scanning perimeter. Nothing yet."),
        TEXT("Hey partner, you ever wonder why robots go rogue?"),
        TEXT("I could use a tune-up after this."),
        TEXT("My targeting calibration is at 99.7 percent. Just saying."),
        TEXT("You know, for a dangerous facility, the lighting is nice."),
    };
    LowHealth = {
        TEXT("Warning: hull integrity compromised!"),
        TEXT("I've seen better days. And worse ones."),
        TEXT("Systems critical. Could use some help here."),
        TEXT("Damage report: bad. Real bad."),
    };
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
    // Idle chatter disabled — commentary is now visual (color states only).
    // The color system in SparkCharacter::UpdateColorState handles all feedback.
}

void USparkCommentaryComponent::SayComment(const FString& Comment)
{
    // Commentary is now visual only (color states on Spark mesh).
    // No text, no voice — just update LastCommentTime for internal tracking.
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastCommentTime < MinCommentInterval)
    {
        return;
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
