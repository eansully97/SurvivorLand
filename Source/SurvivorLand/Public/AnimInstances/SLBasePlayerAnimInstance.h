// Ean Sullivan All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/Weapon/SLWeaponData.h"
#include "SLBasePlayerAnimInstance.generated.h"

class ASLBaseGameCharacter;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class SURVIVORLAND_API USLBasePlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SL|Animation|Layers")
	TSubclassOf<UAnimInstance> DefaultUnarmedUpperBodyLayerClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimData|WeaponAim")
	FVector AimTargetWorld = FVector::ZeroVector;


protected:
	UPROPERTY()
	ASLBaseGameCharacter* OwningCharacter;

	UPROPERTY()
	UCharacterMovementComponent* OwningMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimData|Weapon")
	TWeakObjectPtr<const USLWeaponDataAsset> EquippedWeaponData;

	UPROPERTY(BlueprintReadOnly)
	float ADSAlpha = 0.f;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float GroundSpeed;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bAiming;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bWeaponEquipped;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float Direction;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bHasAcceleration;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool IsFalling;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool ShouldMove;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	FVector Velocity;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bIsCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimData|Aiming")
	float AimYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AnimData|Aiming")
	float AimPitch = 0.f;
};
