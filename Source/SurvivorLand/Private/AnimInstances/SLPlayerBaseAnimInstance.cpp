// Ean Sullivan All Rights Reserved

#include "KismetAnimationLibrary.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void USLBasePlayerAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ASLBaseGameCharacter>(TryGetPawnOwner());
	
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
	EquippedWeapon = OwningCharacter->GetEquippedWeapon();

	const FRotator ActorRot = OwningCharacter->GetActorRotation();
	const FRotator ControlRot = OwningCharacter->GetControlRotation();

	// Delta between where we look and where body faces
	FRotator Delta = (ControlRot - ActorRot).GetNormalized();

	AimYaw = Delta.Yaw;
	AimPitch = Delta.Pitch;
	AimYaw = FMath::Clamp(AimYaw, -90.f, 90.f);
	AimPitch = FMath::Clamp(AimPitch, -60.f, 60.f);
	
}
