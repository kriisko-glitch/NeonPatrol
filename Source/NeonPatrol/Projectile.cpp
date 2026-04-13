#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatComponent.h"
#include "Engine/DamageEvents.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(10.0f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = SphereComp;

	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMoveComp->InitialSpeed = 3000.0f;
	ProjectileMoveComp->MaxSpeed = 3000.0f;
	ProjectileMoveComp->bRotationFollowsVelocity = true;
	ProjectileMoveComp->bShouldBounce = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Load default sphere mesh for visible projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}

	InitialLifeSpan = 3.0f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComp->IgnoreActorWhenMoving(GetOwner(), true);
	SphereComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		// Use UE5's built-in TakeDamage — CombatEnemy overrides this
		FPointDamageEvent DamageEvent(Damage, Hit, (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal(), nullptr);
		AController* InstigatorController = GetInstigatorController();
		OtherActor->TakeDamage(Damage, DamageEvent, InstigatorController, GetOwner());

		// Also try our CombatComponent as fallback
		if (UCombatComponent* CombatComp = OtherActor->FindComponentByClass<UCombatComponent>())
		{
			CombatComp->TakeDamage(Damage);
		}
	}
	Destroy();
}

void AProjectile::FireInDirection(const FVector& Direction)
{
	ProjectileMoveComp->Velocity = Direction * ProjectileMoveComp->InitialSpeed;
}
