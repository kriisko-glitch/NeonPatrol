#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SparkCharacter.generated.h"

class UCombatComponent;
class USparkBrainComponent;
class USparkVoiceComponent;
class USparkCommentaryComponent;
class ARobotEnemy;
class AProjectile;

UENUM(BlueprintType)
enum class ESparkCommand : uint8
{
    None,
    Follow,         // Follow player (default)
    Stay,           // Stop and wait
    Attack,         // Enable attacking enemies
    HoldFire,       // Stop attacking
    MoveForward,    // Move forward N units relative to player facing
    MoveLeft,
    MoveRight,
    MoveBack,
    ComeHere,       // Move directly to player
    Aggressive,     // Close range, fast fire
    Defensive,      // Stay near player, conservative
    Scout,          // Move ahead, scan, return
    FireAtTarget,   // Shoot what the player is looking at
};

UENUM(BlueprintType)
enum class ESparkCombatMode : uint8
{
    Balanced,
    Aggressive,
    Defensive,
};

UCLASS(BlueprintType)
class NEONPATROL_API ASparkCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASparkCharacter();

    virtual void Tick(float DeltaTime) override;

    // --- Behavior state (controllable via commands) ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bShouldFollow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bShouldAttack = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ESparkCombatMode CombatMode = ESparkCombatMode::Balanced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
    float FollowDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
    AActor* FollowTarget = nullptr;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackDamage = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackCooldown = 1.5f;

    // --- Command execution ---

    UFUNCTION(BlueprintCallable, Category = "Commands")
    void ExecuteCommand(ESparkCommand Command, float Param = 0.0f);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCombatComponent* CombatComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USparkBrainComponent* BrainComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USparkVoiceComponent* VoiceComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USparkCommentaryComponent* CommentaryComp;

private:
    float LastAttackTime = 0.f;
    AActor* CurrentEnemy = nullptr;

    // Movement command state
    FVector MoveTargetLocation = FVector::ZeroVector;
    bool bHasMoveTarget = false;
    bool bWasInCombat = false;

    // Color state for visual feedback
    float ColorPulseTimer = 0.f;
    void UpdateColorState(float DeltaTime);
    void SetSparkColor(const FLinearColor& Color);

    // Leash — teleport back if too far
    static constexpr float MaxLeashDistance = 2000.f;
    static constexpr float MinFloorZ = -500.f;
    void EnforceLeash();

    void ShootAtEnemy();
    void MoveTowardTarget(float DeltaTime);
    AActor* FindPlayerLookTarget() const;
};
