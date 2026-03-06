// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLBaseProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class USLWeaponDataAsset;
UCLASS()
class SURVIVORLAND_API ASLBaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASLBaseProjectile();

	void SpawnTracerFX(const USLWeaponDataAsset* WeaponData) const;
	void InitializeProjectile(AActor* InOwnerActor, AController* InInstigatorController, float InDamage);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
	
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	float LifeSeconds = 5.f;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActorRef;

	UPROPERTY()
	TObjectPtr<AController> InstigatorControllerRef;
};