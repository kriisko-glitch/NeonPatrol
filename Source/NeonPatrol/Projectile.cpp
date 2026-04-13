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
	SphereComp->SetSphereRadius(15.0f);
	// Use overlap for reliable fast-projectile detection
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComp->SetGenerateOverlapEvents(true);
	RootComponent = SphereComp;

	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMoveComp->InitialSpeed = 3000.0f;
	ProjectileMoveComp->MaxSpeed = 3000.0f;
	ProjectileMoveComp->bRotationFollowsVelocity = true;
	ProjectileMoveComp->bShouldBounce = false;
	ProjectileMoveComp->ProjectileGravityScale = 0.0f; // No gravity — energy projectile

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	// Bind both hit and overlap for maximum coverage
	SphereComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		DealDamage(OtherActor);
	}
	Destroy();
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		DealDamage(OtherActor);
		Destroy();
	}
}

void AProjectile::DealDamage(AActor* Target)
{
	if (!Target) return;

	// Use UE5's TakeDamage — CombatEnemy overrides this and reduces HP
	FDamageEvent DamageEvent;
	Target->TakeDamage(Damage, DamageEvent, GetInstigatorController(), GetOwner() ? GetOwner() : this);

	// Also try our CombatComponent as fallback (for NeonPatrol enemies)
	if (UCombatComponent* CombatComp = Target->FindComponentByClass<UCombatComponent>())
	{
		CombatComp->TakeDamage(Damage);
	}
}

void AProjectile::FireInDirection(const FVector& Direction)
{
	ProjectileMoveComp->Velocity = Direction * ProjectileMoveComp->InitialSpeed;
}
