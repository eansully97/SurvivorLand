#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLSurvivorCharacterBase.generated.h"

class ASLWeaponBase;
class UAnimMontage;
class USLSurvivorCombatComponent;

UCLASS()
class SURVIVORLAND_API ASLSurvivorCharacterBase : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:
	ASLSurvivorCharacterBase();

	virtual void Tick(float DeltaTime) override;
	virtual void OnAimingChanged(bool bEnable) override;
	virtual bool IsAiming() const override;

	void UpdateTurnInPlace(float DeltaSeconds);
	void PlayFireMontage(bool bAiming);

	UFUNCTION(BlueprintPure, Category="SL|Combat")
	ASLWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintPure, Category="SL|Combat")
	ASLWeaponBase* GetStowedWeapon() const;

	UFUNCTION(BlueprintPure, Category="SL|Combat")
	USLSurvivorCombatComponent* GetSurvivorCombatComponent() const;

	UFUNCTION(BlueprintPure, Category="SL|Combat")
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	FName GetWeaponAttachSocket(ESLWeaponGrip Grip) const;

	UFUNCTION(BlueprintCallable, Category="SL|Combat")
	FName GetWeaponStowSocket(ESLWeaponGrip Grip) const;

	void SetCombatStrafeMode(bool bEnable);
    
	UFUNCTION(BlueprintPure)
	bool IsInCombatStrafeMode() const { return bCombatStrafeMode; }

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName RightHandSocket_Pistol = TEXT("RightHandSocket_Pistol");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName RightHandSocket_Rifle = TEXT("RightHandSocket_Rifle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName BackSocket = TEXT("BackSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName HipSocket = TEXT("HipSocket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Combat")
	TObjectPtr<UAnimMontage> FireMontage = nullptr;

	// Optional later if you want separate ADS / raise / hipfire animations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Combat")
	TObjectPtr<UAnimMontage> AimFireMontage = nullptr;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USLSurvivorCombatComponent> SurvivorCombatComponent = nullptr;

	// Cached movement settings while aiming
	bool bCachedOrientToMovement = true;
	bool bCachedUseControllerYaw = false;
	float CachedRotationRateYaw = 540.f;
	bool bCombatStrafeMode = false;

	UPROPERTY(Transient)
	bool bTurningInPlace = false;
};