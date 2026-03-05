// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SLCombatComponent.h"
#include "SLSurvivorCombatComponent.generated.h"

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
	void Server_DropEquippedWeapon();
	void DropEquippedWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& AimPoint);
	void FirePressed();

	UFUNCTION(Client, Reliable)
	void Client_OnWeaponEquipped(const USLWeaponDataAsset* WeaponData);
	
	UFUNCTION(Client, Reliable)
	void Client_OnWeaponUnequipped();

protected:
	
	void PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const;
	void ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const;
	
	// Internal server logic
	void TryPickupWeapon_Internal();
	void DropEquippedWeapon_Internal();

	// Input mapping helpers (client-side)
	void EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority = 1);
	void UnequipWeaponContext(APlayerController* PC);

private:

	UPROPERTY(Replicated, VisibleAnywhere, Category="SL|Combat")
	TArray<TObjectPtr<ASLWeaponBase>> Inventory;

	UPROPERTY(Replicated, VisibleAnywhere, Category="SL|Combat")
	int32 EquippedIndex = INDEX_NONE;
	
	UPROPERTY()
	TObjectPtr<UInputMappingContext> EquippedWeaponContext = nullptr;
	
public:
	virtual void HandleActionStarted(FGameplayTag InputTag) override;
	virtual void HandleActionCompleted(FGameplayTag InputTag) override;
	
	// Getters
	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetEquippedWeapon() const { return Inventory.IsValidIndex(EquippedIndex) ? Inventory[EquippedIndex] : nullptr; }
	
	//Inline Getters
	FORCEINLINE TArray<TObjectPtr<ASLWeaponBase>> GetInventory() const { return Inventory; }
	FORCEINLINE bool IsAiming() const { return bAiming; }
	FORCEINLINE int32 GetEquippedIndex() const { return EquippedIndex; }
	FORCEINLINE bool HasEquippedWeapon() const { return Inventory.IsValidIndex(EquippedIndex); }
};
