// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SLCombatComponent.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLSurvivorCombatComponent.generated.h"

class ASLBaseProjectile;
class UInputMappingContext;
class UEnhancedInputComponent;
class USLWeaponDataAsset;
class ASLWeaponBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChanged, int32, AmmoInMag, EAmmoType, AmmoType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCarriedAmmoChanged, int32, NewCount, EAmmoType, AmmoType);

USTRUCT(BlueprintType)
struct FSLAmmoCount
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Ammo")
	EAmmoType AmmoType = EAmmoType::Small;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Ammo")
	int32 Count = 100;
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied      UMETA(DisplayName="Unoccupied"),
	ECS_Reloading       UMETA(DisplayName="Reloading"),
	ECS_SwappingWeapons UMETA(DisplayName="SwappingWeapons"),
	ECS_ThrowingGrenade UMETA(DisplayName="ThrowingGrenade"),
	ECS_MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponPostureState : uint8
{
	EPS_Lowered  UMETA(DisplayName="Lowered"),
	EPS_Raising  UMETA(DisplayName="Raising"),
	EPS_Hipfire  UMETA(DisplayName="Hipfire"),
	EPS_ADS      UMETA(DisplayName="ADS"),
	EPS_Lowering UMETA(DisplayName="Lowering"),
	EPS_MAX      UMETA(Hidden)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLSurvivorCombatComponent : public USLCombatComponent
{
	GENERATED_BODY()

public:
	USLSurvivorCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleActionStarted(FGameplayTag InputTag) override;
	virtual void HandleActionCompleted(FGameplayTag InputTag) override;

private:
	/* -----------------------------
	 * Core weapon inventory
	 * ----------------------------- */

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon, VisibleAnywhere, Category="SL|Combat")
	TObjectPtr<ASLWeaponBase> EquippedWeapon = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_StowedWeapon, VisibleAnywhere, Category="SL|Combat")
	TObjectPtr<ASLWeaponBase> StowedWeapon = nullptr;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_StowedWeapon();

	/* -----------------------------
	 * Action / posture state
	 * ----------------------------- */

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	// True only while the player is actively holding the aim input.
	UPROPERTY(ReplicatedUsing=OnRep_Aiming)
	bool bAiming = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SL|Movement", meta=(AllowPrivateAccess="true"))
	float CharacterBaseWalkSpeed{800};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SL|Movement", meta=(AllowPrivateAccess="true"))
	float CharacterRaisingWeaponWalkSpeed{550};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SL|Movement", meta=(AllowPrivateAccess="true"))
	float CharacterAimDownSightWalkSpeed{150};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SL|Movement", meta=(AllowPrivateAccess="true"))
	float CharacterHipfireWalkSpeed{350};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Combat|Posture", ReplicatedUsing= OnRep_WeaponPostureState, meta=(AllowPrivateAccess="true"))
	EWeaponPostureState WeaponPostureState = EWeaponPostureState::EPS_Lowered;

	void UpdateCombatStrafeMode();
	bool ShouldUseCombatStrafeMode() const;

	UFUNCTION()
	void OnRep_WeaponPostureState();

	// 0 = fully lowered, 1 = fully ADS.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SL|Combat|Posture", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float RaiseAlpha = 0.f;

	UPROPERTY(Replicated)
	bool bLocallyReloading = false;

	/* -----------------------------
	 * Input / local presentation
	 * ----------------------------- */

	UPROPERTY()
	TObjectPtr<UInputMappingContext> EquippedWeaponContext = nullptr;

	/* -----------------------------
	 * Ammo
	 * ----------------------------- */

	UPROPERTY(Replicated)
	TArray<FSLAmmoCount> CarriedAmmo;

	/* -----------------------------
	 * Fire flow
	 * ----------------------------- */

	bool bFireHeld = false;
	bool bPendingPostureShot = false;

	FTimerHandle AutoFireTimer;

	/* -----------------------------
	 * Internal weapon inventory helpers
	 * ----------------------------- */

	void EquipWeapon_Internal(ASLWeaponBase* Weapon);
	void StowWeapon_Internal(ASLWeaponBase* Weapon);
	void ApplyEquippedPresentation(const ASLWeaponBase* Weapon);
	void ClearEquippedPresentation();

	void TryPickupWeapon_Internal();
	void SwitchWeapons_Internal();
	void DropEquippedWeapon_Internal();
	void DropWeapon_Internal(ASLWeaponBase* Weapon) const;

	/* -----------------------------
	 * Posture helpers
	 * ----------------------------- */

	void UpdateWeaponPosture(float DeltaTime);
	float GetDesiredRaiseAlpha() const;
	float GetRaiseSpeed() const;
	float GetRequiredRaiseAlphaToFire() const;
	void SetCharacterWalkSpeed(float NewSpeed) const;
	void UpdateWeaponPostureState(float PreviousAlpha);
	void ResetPosture();

	/* -----------------------------
	 * Fire / reload internal flow
	 * ----------------------------- */

	bool HasUsableEquippedWeapon() const;
	bool CanStartReload() const;
	bool CanFire() const;
	void ResetCombatState();

	void StartFire();
	void StopFire();
	void RequestFire();

	void FireOnce();
	void FireOnce_Internal(const FVector& AimPoint);

	void BeginReload();
	void FinishReloading_Internal();

	int32 GetCarriedAmmo(EAmmoType AmmoType) const;
	bool ConsumeCarriedAmmo(EAmmoType AmmoType, int32 Amount);
	void AddCarriedAmmo(EAmmoType AmmoType, int32 Amount);
	int32 CalculateReloadAmount() const;
	void BroadcastAmmoState();

	void InitCarriedAmmo();

	/* -----------------------------
	 * Cosmetic helpers
	 * ----------------------------- */

	void EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority = 1);
	void UnequipWeaponContext(APlayerController* PC);

	void SpawnMuzzleFlashFX(const ASLWeaponBase* Weapon) const;
	void SpawnFireSoundFX(const ASLWeaponBase* Weapon) const;
	void SpawnImpactFX(const FVector& ImpactPoint, const FVector& ImpactNormal, const USLWeaponDataAsset* WeaponData) const;

	/* -----------------------------
	 * Ballistics / damage
	 * ----------------------------- */

	void PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const;
	void ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnCarriedAmmoChanged OnCarriedAmmoChanged;

	UPROPERTY(BlueprintAssignable)
	FOnWeaponAmmoChanged OnWeaponAmmoChanged;

	/* -----------------------------
	 * Public requests
	 * ----------------------------- */

	void TryPickupWeapon();
	void DropEquippedWeapon();
	void SwitchWeapons();
	void Reload();
	void SetAiming(bool bNewAiming);

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/* -----------------------------
	 * RPCs
	 * ----------------------------- */

	UFUNCTION(Server, Reliable)
	void Server_TryPickupWeapon();

	UFUNCTION(Server, Reliable)
	void Server_DropEquippedWeapon();

	UFUNCTION(Server, Reliable)
	void Server_SwitchWeapons();

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void Server_StartReload();

	UFUNCTION(Server, Reliable)
	void Server_FinishReload();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& AimPoint);

	UFUNCTION(Client, Reliable)
	void Client_ApplyEquippedPresentation(const USLWeaponDataAsset* WeaponData);

	UFUNCTION(Client, Reliable)
	void Client_ClearEquippedPresentation();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX();

	/* -----------------------------
	 * Accessors
	 * ----------------------------- */

	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetStowedWeapon() const { return StowedWeapon; }

	// True only while aim input is held.
	UFUNCTION(BlueprintPure)
	bool IsAiming() const { return bAiming; }

	UFUNCTION(BlueprintPure)
	bool IsReloading() const { return CombatState == ECombatState::ECS_Reloading; }

	UFUNCTION(BlueprintPure)
	bool HasEquippedWeapon() const { return EquippedWeapon != nullptr; }

	UFUNCTION(BlueprintPure)
	bool HasStowedWeapon() const { return StowedWeapon != nullptr; }

	UFUNCTION(BlueprintPure)
	float GetRaiseAlpha() const { return RaiseAlpha; }

	UFUNCTION(BlueprintPure)
	EWeaponPostureState GetWeaponPostureState() const { return WeaponPostureState; }
};