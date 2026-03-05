// Ean Sullivan All Rights Reserved

#include "KismetAnimationLibrary.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Characters/SLSurvivorCharacterBase.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/SLWeaponBase.h"

void USLBasePlayerAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ASLSurvivorCharacterBase>(TryGetPawnOwner());
	
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void USLBasePlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	Velocity = OwningCharacter->GetVelocity();
	Velocity.Z = 0.f;
	GroundSpeed = Velocity.Size();
	IsFalling = OwningCharacter->GetMovementComponent()->IsFalling();
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().Size() > 0.f;
	bIsCrouched = OwningCharacter->bIsCrouched;
	ShouldMove = bHasAcceleration && GroundSpeed > 0.01f;
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity,OwningCharacter->GetActorRotation());
	bWeaponEquipped = OwningCharacter->IsWeaponEquipped();
	AimTargetWorld = OwningCharacter->GetAimTargetWorldSmoothed();
	bAiming = OwningCharacter->IsAiming();
	
	if (ASLWeaponBase* W = OwningCharacter->GetEquippedWeapon())
	{
		EquippedWeaponData = W->GetWeaponData();
	}
	else
	{
		EquippedWeaponData = nullptr;
	}

	const bool bAim = OwningCharacter && OwningCharacter->GetSurvivorCombatComponent()
	? OwningCharacter->GetSurvivorCombatComponent()->IsAiming()
	: false;

	const float Speed = bAim ? 10.f : 14.f;
	ADSAlpha = FMath::FInterpTo(ADSAlpha, bAim ? 1.f : 0.f, DeltaSeconds, Speed);

	const FRotator ActorRot = OwningCharacter->GetActorRotation();
	const FRotator ControlRot = OwningCharacter->GetControlRotation();
	FRotator Delta = (ControlRot - ActorRot).GetNormalized();

	AimYaw = FMath::Clamp(Delta.Yaw, -90.f, 90.f);
	AimPitch = FMath::Clamp(Delta.Pitch, -60.f, 60.f);

	if (OwningCharacter && OwningCharacter->GetEquippedWeapon())
	{
		LeftHandTransform = OwningCharacter->GetEquippedWeapon()->GetWeaponMesh()->GetSocketTransform("IK_LeftHand", RTS_World);
	}
}
