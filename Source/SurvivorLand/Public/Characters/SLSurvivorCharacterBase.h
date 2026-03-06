// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLSurvivorCharacterBase.generated.h"

class USLBaseGameCharacter;
class USLSurvivorCombatComponent;

UCLASS()
class SURVIVORLAND_API ASLSurvivorCharacterBase : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:

	ASLSurvivorCharacterBase();
	
	void UpdateTurnInPlace(float DeltaSeconds);

	virtual void OnAimingChanged(bool bEnable) override;

protected:
	virtual void Tick(float DeltaTime) override;
	

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLSurvivorCombatComponent> SurvivorCombatComponent;

	bool bCachedOrientToMovement = true;
	bool bCachedUseControllerYaw = false;
	float CachedRotationRateYaw = 540.f;
	
	UPROPERTY(Transient)
	bool bTurningInPlace = false;
	
public:
	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetStowedWeapon() const;

	UFUNCTION(BlueprintPure)
	USLSurvivorCombatComponent* GetSurvivorCombatComponent() const;
	
	UFUNCTION(BlueprintCallable)
	FName GetWeaponAttachSocket(ESLWeaponGrip Grip) const;

	UFUNCTION(BlueprintCallable)
	FName GetWeaponStowSocket(ESLWeaponGrip Grip) const;

	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	
	virtual bool IsAiming() const override;
	
};
