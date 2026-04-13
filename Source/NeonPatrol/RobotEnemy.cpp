#include "RobotEnemy.h"
#include "CombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ARobotEnemy::ARobotEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	MoveSpeed = 300.0f;
	AttackDamage = 10.0f;
	AttackRange = 150.0f;
	AttackCooldown = 2.0f;
	LastAttackTime = 0.0f;
	TargetActor = nullptr;

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ARobotEnemy::BeginPlay()
{
	Super::BeginPlay();

	switch (RobotType)
	{
	case ERobotType::Drone:
		MoveSpeed = 300.0f;
		CombatComp->SetMaxHealth(30.0f);
		break;
	case ERobotType::Turret:
		MoveSpeed = 0.0f;
		CombatComp->SetMaxHealth(60.0f);
		break;
	case ERobotType::Heavy:
		MoveSpeed = 150.0f;
		CombatComp->SetMaxHealth(150.0f);
		break;
	}

	CombatComp->OnDeath.AddDynamic(this, &ARobotEnemy::HandleOnDeath);
}

void ARobotEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetActor)
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance > AttackRange)
	{
		FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);
	}
	else
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastAttackTime >= AttackCooldown)
		{
			Attack();
		}
	}
}

void ARobotEnemy::SetTarget(AActor* Target)
{
	TargetActor = Target;
}

void ARobotEnemy::Attack()
{
	if (!TargetActor)
	{
		return;
	}

	if (UCombatComponent* TargetCombat = TargetActor->FindComponentByClass<UCombatComponent>())
	{
		TargetCombat->TakeDamage(AttackDamage);
	}

	LastAttackTime = GetWorld()->GetTimeSeconds();
}

void ARobotEnemy::HandleOnDeath()
{
	OnRobotDied.Broadcast(this);
	SetActorEnableCollision(false);
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ARobotEnemy::DeferredDestroy, 1.0f, false);
}

void ARobotEnemy::DeferredDestroy()
{
	Destroy();
}
