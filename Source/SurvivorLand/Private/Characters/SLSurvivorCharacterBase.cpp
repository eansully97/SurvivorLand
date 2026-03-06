// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLSurvivorCharacterBase.h"

#include "SurvivorLandGameplayTags.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ASLSurvivorCharacterBase::ASLSurvivorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SurvivorCombatComponent = CreateDefaultSubobject<USLSurvivorCombatComponent>(TEXT("CombatComponent"));

	// Make base character routing see it as generic combat
	CombatComponent = SurvivorCombatComponent;
}

void ASLSurvivorCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTurnInPlace(DeltaTime);
}

void ASLSurvivorCharacterBase::UpdateTurnInPlace(float DeltaSeconds)
{
	if (!IsLocallyControlled() || !Controller) return;
	if (!IsAiming()) return;

	// only turn in place when basically standing still
	const float Speed2D = GetVelocity().Size2D();
	if (Speed2D > 10.f) return;

	const float YawOffset = GetAimYawOffset();
	const float AbsYaw = FMath::Abs(YawOffset);

	// thresholds you can tune
	const float StartTurnThreshold = 75.f;   // start turning past this
	const float StopTurnThreshold  = 35.f;   // stop once back under this

	if (!bTurningInPlace && AbsYaw > StartTurnThreshold) bTurningInPlace = true;
	if (bTurningInPlace && AbsYaw < StopTurnThreshold)   bTurningInPlace = false;
	if (!bTurningInPlace) return;

	// rotate actor toward control yaw smoothly
	const float TurnSpeedDegPerSec = 200.f; // tune: 180–360
	const float Step = TurnSpeedDegPerSec * DeltaSeconds;

	const float DesiredYaw = Controller->GetControlRotation().Yaw;
	const float NewYaw = FMath::FixedTurn(GetActorRotation().Yaw, DesiredYaw, Step);

	SetActorRotation(FRotator(0.f, NewYaw, 0.f));
}

FName ASLSurvivorCharacterBase::GetWeaponAttachSocket(ESLWeaponGrip Grip) const
{
	const UEnum* EnumPtr = StaticEnum<ESLWeaponGrip>();
	if (!EnumPtr) return TEXT("RightHandSocket_Pistol");

	const FString GripName = EnumPtr->GetNameStringByValue((int64)Grip);
	const FName Socket(*FString::Printf(TEXT("RightHandSocket_%s"), *GripName));

	return GetMesh() && GetMesh()->DoesSocketExist(Socket)
		? Socket
		: TEXT("RightHandSocket_Pistol");
}

FName ASLSurvivorCharacterBase::GetWeaponStowSocket(ESLWeaponGrip Grip) const
{
	switch (Grip)
	{
	case ESLWeaponGrip::Pistol:
		return TEXT("HipSocket");

	case ESLWeaponGrip::Rifle:
		return TEXT("BackSocket");

	default:
		return NAME_None;
	}
}

bool ASLSurvivorCharacterBase::IsWeaponEquipped() const
{
	return SurvivorCombatComponent && SurvivorCombatComponent->GetEquippedWeapon() != nullptr;
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

bool ASLSurvivorCharacterBase::IsAiming() const
{
	return SurvivorCombatComponent && SurvivorCombatComponent->IsAiming();
}

void ASLSurvivorCharacterBase::OnAimingChanged(bool bEnable)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	if (bEnable)
	{
		// cache once when entering
		bCachedOrientToMovement = Move->bOrientRotationToMovement;
		bCachedUseControllerYaw = bUseControllerRotationYaw;
		CachedRotationRateYaw   = Move->RotationRate.Yaw;

		Move->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

		// tune feel
		Move->RotationRate.Yaw = 900.f; // 720–1080 feels good for ADS
	}
	else
	{
		Move->bOrientRotationToMovement = bCachedOrientToMovement;
		bUseControllerRotationYaw = bCachedUseControllerYaw;
		Move->RotationRate.Yaw = CachedRotationRateYaw;
	}
}