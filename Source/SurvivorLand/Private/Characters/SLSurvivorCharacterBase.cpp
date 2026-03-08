#include "Characters/SLSurvivorCharacterBase.h"

#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/SLWeaponBase.h"

ASLSurvivorCharacterBase::ASLSurvivorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SurvivorCombatComponent = CreateDefaultSubobject<USLSurvivorCombatComponent>(TEXT("SurvivorCombatComponent"));

	// Let the base character point to the shared combat component reference.
	CombatComponent = SurvivorCombatComponent;
}

void ASLSurvivorCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTurnInPlace(DeltaTime);
}

void ASLSurvivorCharacterBase::UpdateTurnInPlace(float DeltaSeconds)
{
	if (!IsLocallyControlled() || !Controller)
	{
		bTurningInPlace = false;
		return;
	}

	if (!IsAiming())
	{
		bTurningInPlace = false;
		return;
	}

	const float Speed2D = GetVelocity().Size2D();
	if (Speed2D > 10.f)
	{
		bTurningInPlace = false;
		return;
	}

	const float YawOffset = GetAimYawOffset();
	const float AbsYawOffset = FMath::Abs(YawOffset);

	constexpr float StartTurnThreshold = 75.f;
	constexpr float StopTurnThreshold = 35.f;
	constexpr float TurnSpeedDegPerSec = 200.f;

	if (!bTurningInPlace && AbsYawOffset > StartTurnThreshold)
	{
		bTurningInPlace = true;
	}
	else if (bTurningInPlace && AbsYawOffset < StopTurnThreshold)
	{
		bTurningInPlace = false;
	}

	if (!bTurningInPlace)
	{
		return;
	}

	const float DesiredYaw = Controller->GetControlRotation().Yaw;
	const float Step = TurnSpeedDegPerSec * DeltaSeconds;
	const float NewYaw = FMath::FixedTurn(GetActorRotation().Yaw, DesiredYaw, Step);

	SetActorRotation(FRotator(0.f, NewYaw, 0.f));
}

void ASLSurvivorCharacterBase::OnAimingChanged(bool bEnable)
{
	Super::OnAimingChanged(bEnable);
	
}

void ASLSurvivorCharacterBase::SetCombatStrafeMode(bool bEnable)
{
	if (bCombatStrafeMode == bEnable)
	{
		return;
	}

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	bCombatStrafeMode = bEnable;

	if (bEnable)
	{
		bCachedOrientToMovement = Move->bOrientRotationToMovement;
		bCachedUseControllerYaw = bUseControllerRotationYaw;
		CachedRotationRateYaw = Move->RotationRate.Yaw;

		Move->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		Move->RotationRate.Yaw = 900.f;
	}
	else
	{
		Move->bOrientRotationToMovement = bCachedOrientToMovement;
		bUseControllerRotationYaw = bCachedUseControllerYaw;
		Move->RotationRate.Yaw = CachedRotationRateYaw;
		bTurningInPlace = false;
	}
}

void ASLSurvivorCharacterBase::PlayFireMontage(bool bAiming)
{
	UAnimMontage* MontageToPlay = nullptr;

	if (bAiming && AimFireMontage)
	{
		MontageToPlay = AimFireMontage;
	}
	else
	{
		MontageToPlay = FireMontage;
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
}

ASLWeaponBase* ASLSurvivorCharacterBase::GetEquippedWeapon() const
{
	return SurvivorCombatComponent ? SurvivorCombatComponent->GetEquippedWeapon() : nullptr;
}

ASLWeaponBase* ASLSurvivorCharacterBase::GetStowedWeapon() const
{
	return SurvivorCombatComponent ? SurvivorCombatComponent->GetStowedWeapon() : nullptr;
}

USLSurvivorCombatComponent* ASLSurvivorCharacterBase::GetSurvivorCombatComponent() const
{
	return SurvivorCombatComponent;
}

bool ASLSurvivorCharacterBase::IsWeaponEquipped() const
{
	return GetEquippedWeapon() != nullptr;
}

bool ASLSurvivorCharacterBase::IsAiming() const
{
	return SurvivorCombatComponent && SurvivorCombatComponent->IsAiming();
}

FName ASLSurvivorCharacterBase::GetWeaponAttachSocket(ESLWeaponGrip Grip) const
{
	switch (Grip)
	{
	case ESLWeaponGrip::Pistol:
		return RightHandSocket_Pistol;
	case ESLWeaponGrip::Rifle:
		return RightHandSocket_Rifle;
	default:
		return NAME_None;
	}
}

FName ASLSurvivorCharacterBase::GetWeaponStowSocket(ESLWeaponGrip Grip) const
{
	switch (Grip)
	{
	case ESLWeaponGrip::Pistol:
		return HipSocket;
	case ESLWeaponGrip::Rifle:
		return BackSocket;
	default:
		return NAME_None;
	}
}