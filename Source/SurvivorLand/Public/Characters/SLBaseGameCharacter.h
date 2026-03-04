// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLBaseGameCharacter.generated.h"

class ASLWeaponBase;
class USLCombatComponent;
class UCameraComponent;
class USpringArmComponent;
class USLInputHandlerComponent;
class UDataAsset_InputConfig;

UCLASS()
class SURVIVORLAND_API ASLBaseGameCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	
	ASLBaseGameCharacter();

	UPROPERTY(BlueprintReadOnly, Category="SL|Aim")
	FVector AimTargetWorld = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="SL|Aim")
	FVector AimTargetWorldSmoothed = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category="SL|Weapons")
	FName GetWeaponAttachSocket(ESLWeaponGrip Grip) const;

	
#pragma region Inputs

	/** Input config to use for this pawn (assign in BP_SurvivorBase / BP_MonsterBase). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataAsset_InputConfig> InputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData", meta = (AllowPrivateAccess = "true"))
	float LookSensitivity = 0.3f;

#pragma endregion

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Input handler component (unified input pipeline). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLInputHandlerComponent> InputHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SL|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USLCombatComponent> CombatComponent;


#pragma endregion

protected:
	
	virtual void BeginPlay() override;
	void BindToInput(USLInputHandlerComponent* InputHandler);
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

private:
	bool bCachedOrientToMovement = true;
	bool bCachedUseControllerYaw = false;
	float CachedRotationRateYaw = 540.f;
	
	UPROPERTY(Transient)
	bool bTurningInPlace = false;
	
public:

	void SetStrafeAimingMode(bool bEnable);

	// Handlers bound to the InputHandlerComponent delegates:
	bool bInputBound = false;
	UFUNCTION()
	void HandleAxis2D(FGameplayTag InputTag, FVector2D Value);

	UFUNCTION()
	void HandleActionStarted(FGameplayTag InputTag);

	UFUNCTION()
	void HandleActionCompleted(FGameplayTag InputTag);

	UFUNCTION()
	bool IsWeaponEquipped() const;
	
	UFUNCTION()
	ASLWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintPure)
	bool IsAiming() const;
	
	float GetAimYawOffset() const;

	UFUNCTION()
	void UpdateAimTarget(float DeltaSeconds);
	void UpdateTurnInPlace(float DeltaSeconds);
};
