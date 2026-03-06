// Ean Sullivan All Rights Reserved


#include "Items/Projectiles/SLBaseProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Data/Weapon/SLWeaponData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ASLBaseProjectile::ASLBaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(4.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 12000.f;
	ProjectileMovement->MaxSpeed = 12000.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 5.f;
}

void ASLBaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (Collision)
	{
		Collision->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
	}
}

void ASLBaseProjectile::InitializeProjectile(AActor* InOwnerActor, AController* InInstigatorController, const USLWeaponDataAsset* InWeaponData)
{
	OwnerActorRef = InOwnerActor;
	InstigatorControllerRef = InInstigatorController;
	Damage = InWeaponData->Ballistics.BaseDamage;
	SourceWeaponData = InWeaponData;
}

void ASLBaseProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == OwnerActorRef)
	{
		return;
	}

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		Damage,
		GetActorForwardVector(),
		Hit,
		InstigatorControllerRef,
		OwnerActorRef,
		nullptr
	);

	if (!SourceWeaponData)
	{
		return;
	}
	if (SourceWeaponData->ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SourceWeaponData->ImpactEffect,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation()
			);
	}
	if (SourceWeaponData->ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
		this,
		SourceWeaponData->ImpactSound,
		Hit.ImpactPoint
		);
	}

	Destroy();
}

void ASLBaseProjectile::SpawnTracerFX(const USLWeaponDataAsset* WeaponData) const
{
	if (!WeaponData || !WeaponData->TracerEffect || !WeaponData->ProjectileClass)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		WeaponData->TracerEffect,
		GetRootComponent(),
		FName(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);
}


