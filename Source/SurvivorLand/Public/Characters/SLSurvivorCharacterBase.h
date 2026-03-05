// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SLBaseGameCharacter.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLSurvivorCharacterBase.generated.h"

class USLSurvivorCombatComponent;

UCLASS()
class SURVIVORLAND_API ASLSurvivorCharacterBase : public ASLBaseGameCharacter
{
	GENERATED_BODY()

public:

	ASLSurvivorCharacterBase();

	virtual void HandleAxis2D(FGameplayTag InputTag, FVector2D Value) override;
	virtual void HandleActionStarted(FGameplayTag InputTag) override;
	virtual void HandleActionCompleted(FGameplayTag InputTag) override;

	bool IsAiming() const;
	float GetAimYawOffset() const;
	void UpdateAimTarget(float DeltaSeconds);
	void UpdateTurnInPlace(float DeltaSeconds);
	void SetStrafeAimingMode(bool bEnable);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLSurvivorCombatComponent> SurvivorCombatComponent;

	UPROPERTY()
	FVector AimTargetWorld = FVector::ZeroVector;

	UPROPERTY()
	FVector AimTargetWorldSmoothed = FVector::ZeroVector;

	bool bCachedOrientToMovement = true;
	bool bCachedUseControllerYaw = false;
	float CachedRotationRateYaw = 540.f;
	
	UPROPERTY(Transient)
	bool bTurningInPlace = false;
	
public:
	UFUNCTION(BlueprintPure)
	ASLWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintPure)
	USLSurvivorCombatComponent* GetCombatComponent() const;

	UFUNCTION(BlueprintPure)
	FVector GetAimTargetWorld() const { return AimTargetWorld; }

	UFUNCTION(BlueprintPure)
	FVector GetAimTargetWorldSmoothed() const { return AimTargetWorldSmoothed; }
	
	UFUNCTION(BlueprintCallable)
	FName GetWeaponAttachSocket(ESLWeaponGrip Grip) const;

	UFUNCTION(BlueprintPure)
	bool IsWeaponEquipped() const;
	
};
