// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SLCombatComponent.h"
#include "SLSurvivorCombatComponent.generated.h"

class ASLBaseProjectile;
class USLInputHandlerComponent;
class UInputMappingContext;
class UEnhancedInputComponent;
class USLWeaponDataAsset;
class ASLWeaponBase;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLSurvivorCombatComponent : public USLCombatComponent
{
	GENERATED_BODY()

public:

	USLSurvivorCombatComponent();
	
	UFUNCTION(Server, Reliable)
	void Server_TryPickupWeapon();
	void TryPickupWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_DropEquippedWeapon(ASLWeaponBase* Weapon);
	void DropEquippedWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& AimPoint);
	void FirePressed();

	UFUNCTION(Client, Reliable)
	void Client_ApplyEquippedPresentation(const USLWeaponDataAsset* WeaponData);
	
	UFUNCTION(Client, Reliable)
	void Client_ClearEquippedPresentation();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX();

	void SpawnMuzzleFlashFX(const ASLWeaponBase* Weapon) const;
	void SpawnFireSoundFX(const ASLWeaponBase* Weapon) const;

protected:
	
	void PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const;
	void ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const;
	
	


	// Input mapping helpers (client-side)
	void EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority = 1);
	void UnequipWeaponContext(APlayerController* PC);
	
	// Internal server logic
	void EquipWeapon_Internal(ASLWeaponBase* Weapon);
	void StowWeapon_Internal(ASLWeaponBase* Weapon);
	void TryPickupWeapon_Internal();
	void SwitchWeapons_Internal();
	void DropEquippedWeapon_Internal();
	void DropWeapon_Internal(ASLWeaponBase* Weapon) const;
	
	void ApplyEquippedPresentation(const ASLWeaponBase* Weapon);
	void ClearEquippedPresentation();

	UFUNCTION(Server, Reliable)
	void Server_SwitchWeapons();
	void SwitchWeapons();

private:

	UPROPERTY(Replicated, VisibleAnywhere, Category="SL|Combat")
	ASLWeaponBase* EquippedWeapon = nullptr;

	UPROPERTY(Replicated, VisibleAnywhere, Category="SL|Combat")
	ASLWeaponBase* StowedWeapon = nullptr;
	
	UPROPERTY()
	TObjectPtr<UInputMappingContext> EquippedWeaponContext = nullptr;
	
public:
	virtual void HandleActionStarted(FGameplayTag InputTag) override;
	virtual void HandleActionCompleted(FGameplayTag InputTag) override;
	
	void StartFire();
	void StopFire();
	bool bFireHeld = false;

	FTimerHandle AutoFireTimer;
	
	// Getters
	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetStowedWeapon() const { return StowedWeapon; }
	
	//Inline Getters
	FORCEINLINE bool IsAiming() const { return bAiming; }
	FORCEINLINE bool HasEquippedWeapon() const { return EquippedWeapon != nullptr; }
	FORCEINLINE bool HasStowedWeapon() const { return StowedWeapon != nullptr; }
};
