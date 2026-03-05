// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Characters/SLBaseGameCharacter.h"
#include "SLCombatComponent.generated.h"

class USLInputHandlerComponent;
class UInputMappingContext;
class UEnhancedInputComponent;
class USLWeaponDataAsset;
class ASLWeaponBase;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SURVIVORLAND_API USLCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server: attempts to pick up nearest overlapping weapon. */
	UFUNCTION(Server, Reliable)
	void Server_TryPickupWeapon();

	/** Server: drop currently equipped weapon. */
	UFUNCTION(Server, Reliable)
	void Server_DropEquippedWeapon();
	
	void FirePressed();
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& AimPoint);
	
	void DropEquippedWeapon();
	void TryInteract();

	UPROPERTY(ReplicatedUsing=OnRep_Aiming)
	bool bAiming = false;

	UFUNCTION()
	void OnRep_Aiming();

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bNewAiming);

	void SetAiming(bool bNewAiming);
	
	bool IsAiming() const { return bAiming; }
	
	/** Inventory */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="SL|Combat")
	TArray<TObjectPtr<ASLWeaponBase>> Inventory;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="SL|Combat")
	int32 EquippedIndex = INDEX_NONE;

	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	bool HasEquippedWeapon() const { return Inventory.IsValidIndex(EquippedIndex); }

	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	ASLWeaponBase* GetEquippedWeapon() const { return Inventory.IsValidIndex(EquippedIndex) ? Inventory[EquippedIndex] : nullptr; }

	/** Client: equip inputs (mapping context + binds). */
	UFUNCTION(Client, Reliable)
	void Client_OnWeaponEquipped(UInputMappingContext* WeaponContext, const USLWeaponDataAsset* WeaponData);

	/** Client: unequip inputs (remove mapping context). */
	UFUNCTION(Client, Reliable)
	void Client_OnWeaponUnequipped();

private:

	void PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const;
	void ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart);
	// Internal server logic
	void TryPickupWeapon_Internal();
	void DropEquippedWeapon_Internal();

	// Input mapping helpers (client-side)
	void EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority = 1);
	void UnequipWeaponContext(APlayerController* PC);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> EquippedWeaponContext = nullptr;

	bool bBoundToInput = false;
};