#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SLWeaponData.generated.h"

class ASLBaseProjectile;
class UNiagaraSystem;
class USkeletalMesh;
class USLWeaponInputProfile;
class USLWeaponAnimProfile;

UENUM(BlueprintType)
enum class ESLWeaponGrip : uint8
{
	Pistol UMETA(DisplayName="Pistol"),
	Rifle  UMETA(DisplayName="Rifle")
};
UENUM(BlueprintType)
enum class ESLWeaponFireType : uint8
{
	Hitscan,
	Projectile
};

USTRUCT(BlueprintType)
struct FSLWeaponFireSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float FireRate = 600.f; // rounds per minute

	UPROPERTY(EditDefaultsOnly)
	bool bAutomatic = false;
};

USTRUCT(BlueprintType)
struct FSLBallisticsConfig
{
	GENERATED_BODY()

	// Muzzle socket on the WEAPON mesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics")
	float MaxRange = 100000.f;

	// "Bullet thickness" (sphere trace). 0 = line trace.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics", meta=(ClampMin="0.0"))
	float TraceRadius = 1.5f;

	// Penetration budget in "cm of wood equivalent" (simple model)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics", meta=(ClampMin="0.0"))
	float PenetrationDepth = 0.f;

	// Damage at first hit; you can scale down after penetration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics", meta=(ClampMin="0.0"))
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float Spread = 0.5f; // degrees

	UPROPERTY(EditDefaultsOnly)
	float AdsSpreadMultiplier = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ballistics")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Attach")
	ESLWeaponGrip Grip = ESLWeaponGrip::Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<USkeletalMesh> WeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponInputProfile> InputProfile = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponAnimProfile> AnimProfile = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Firing")
	FSLBallisticsConfig Ballistics;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Fire")
	FSLWeaponFireSettings FireSettings;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BeamTrail;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Fire")
	ESLWeaponFireType FireType = ESLWeaponFireType::Hitscan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Fire")
	TSubclassOf<ASLBaseProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Fire")
	float ProjectileSpeed = 12000.f;
};
