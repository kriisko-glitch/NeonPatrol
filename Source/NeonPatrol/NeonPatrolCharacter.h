#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NeonPatrolCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class UInputAction;
class UInputMappingContext;
class AProjectile;

struct FInputActionValue;

UCLASS(BlueprintType)
class NEONPATROL_API ANeonPatrolCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANeonPatrolCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCombatComponent> CombatComp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ShootAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ChatAction;

    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 Score;

public:
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddScore(int32 Points);

    UFUNCTION(BlueprintCallable, Category = "Components")
    UCombatComponent* GetCombatComponent() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
    void OpenChat();

private:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Shoot();
    void StartChat();

    UFUNCTION()
    void OnDeath();
};
