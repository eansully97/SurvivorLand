#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SLWeaponData.generated.h"

class ASLBaseProjectile;
class UNiagaraSystem;
class UParticleSystem;
class USoundBase;
class UAnimationAsset;
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
enum class EAmmoType : uint8
{
	Small        UMETA(DisplayName = "Small"),
	Medium       UMETA(DisplayName = "Medium"),
	Large      UMETA(DisplayName = "Large"),
	MAX         UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESLWeaponFireType : uint8
{
	Hitscan UMETA(DisplayName="Hitscan"),
	Projectile UMETA(DisplayName="Projectile")
};

USTRUCT(BlueprintType)
struct FSLWeaponFireSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	float FireRate = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	bool bAutomatic = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	int32 MagazineSize = 30;
	
	UPROPERTY(EditDefaultsOnly, Category="Fire", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HipfireThreshold = 0.6f;
    
    UPROPERTY(EditDefaultsOnly, Category="Fire", meta=(ClampMin="0.01"))
    float RaiseToHipfireSpeed = 3.5f;
    
    UPROPERTY(EditDefaultsOnly, Category="Fire", meta=(ClampMin="0.01"))
    float RaiseToADSSpeed = 5.5f;
    
    UPROPERTY(EditDefaultsOnly, Category="Fire", meta=(ClampMin="0.01"))
    float LowerSpeed = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	EAmmoType AmmoType = EAmmoType::Small;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	TObjectPtr<UAnimationAsset> WeaponFireAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	TObjectPtr<UAnimMontage> CharacterReloadMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FSLSocketInformation
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName MuzzleSocketName = TEXT("Muzzle");
};

USTRUCT(BlueprintType)
struct FSLWeaponDamageSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0"))
	float BaseDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="1.0"))
	float HeadshotMultiplier = 1.5f;

	// Simple penetration budget. Can be interpreted however you want.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0"))
	float PenetrationDepth = 0.f;
};

USTRUCT(BlueprintType)
struct FSLWeaponFXSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlash = nullptr;

	// Used by projectile weapons or beam/tracer FX if desired.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<UNiagaraSystem> ProjectileTracerEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<UNiagaraSystem> ImpactEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<UParticleSystem> HitscanBeamTrail = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<USoundBase> FireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FX")
	TObjectPtr<USoundBase> ImpactSound = nullptr;
};

USTRUCT(BlueprintType)
struct FSLHitscanSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hitscan", meta=(ClampMin="0.0"))
	float MaxRange = 100000.f;

	// 0 = line trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hitscan", meta=(ClampMin="0.0"))
	float TraceRadius = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hitscan", meta=(ClampMin="0.0"))
	float Spread = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hitscan", meta=(ClampMin="0.0"))
	float AdsSpreadMultiplier = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hitscan")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

USTRUCT(BlueprintType)
struct FSLProjectileSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	TSubclassOf<ASLBaseProjectile> ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.0"))
	float ProjectileSpeed = 12000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile", meta=(ClampMin="0.0"))
	float ProjectileLifeSeconds = 5.f;
};

UCLASS(BlueprintType)
class SURVIVORLAND_API USLWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	ESLWeaponGrip Grip = ESLWeaponGrip::Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	ESLWeaponFireType FireType = ESLWeaponFireType::Hitscan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Visual")
	TObjectPtr<USkeletalMesh> WeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponInputProfile> InputProfile = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Profiles")
	TObjectPtr<USLWeaponAnimProfile> AnimProfile = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Fire")
	FSLWeaponFireSettings FireSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Damage")
	FSLWeaponDamageSettings DamageSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|FX")
	FSLWeaponFXSettings FXSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Hitscan", meta=(EditCondition="FireType == ESLWeaponFireType::Hitscan", EditConditionHides))
	FSLHitscanSettings HitscanSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Projectile", meta=(EditCondition="FireType == ESLWeaponFireType::Projectile", EditConditionHides))
	FSLProjectileSettings ProjectileSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Sockets")
	FSLSocketInformation SocketInformation;
};