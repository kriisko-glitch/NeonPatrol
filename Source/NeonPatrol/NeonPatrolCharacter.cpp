#include "NeonPatrolCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Projectile.h"
#include "NeonPatrol.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"

ANeonPatrolCharacter::ANeonPatrolCharacter()
{
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    MoveComp->MaxWalkSpeed = 600.0f;
    MoveComp->JumpZVelocity = 500.0f;

    Score = 0;
}

void ANeonPatrolCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    if (CombatComp)
    {
        CombatComp->OnDeath.AddDynamic(this, &ANeonPatrolCharacter::OnDeath);
    }
}

void ANeonPatrolCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANeonPatrolCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANeonPatrolCharacter::Look);
        EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ANeonPatrolCharacter::Shoot);
        EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &ANeonPatrolCharacter::StartChat);
    }
}

void ANeonPatrolCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void ANeonPatrolCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ANeonPatrolCharacter::Shoot()
{
    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    if (AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(AProjectile::StaticClass(), SpawnLocation, GetActorRotation(), SpawnParams))
    {
        Projectile->FireInDirection(GetActorForwardVector());
    }
}

void ANeonPatrolCharacter::AddScore(int32 Points)
{
    Score += Points;
}

UCombatComponent* ANeonPatrolCharacter::GetCombatComponent() const
{
    return CombatComp;
}

void ANeonPatrolCharacter::StartChat()
{
    OpenChat();
}

void ANeonPatrolCharacter::OnDeath()
{
    UE_LOG(LogNeonPatrol, Warning, TEXT("Player died"));
}
