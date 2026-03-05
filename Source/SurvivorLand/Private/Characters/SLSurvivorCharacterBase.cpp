// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Characters/SLSurvivorCharacterBase.h"

#include "SurvivorLandGameplayTags.h"
#include "Components/Combat/SLSurvivorCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ASLSurvivorCharacterBase::ASLSurvivorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SurvivorCombatComponent = CreateDefaultSubobject<USLSurvivorCombatComponent>(TEXT("CombatComponent"));
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

bool ASLSurvivorCharacterBase::IsWeaponEquipped() const
{
	if (!SurvivorCombatComponent) return false;
	return (SurvivorCombatComponent->GetEquippedWeapon() != nullptr);
}


ASLWeaponBase* ASLSurvivorCharacterBase::GetEquippedWeapon() const
{
	if (!SurvivorCombatComponent) return nullptr;

	const int32 Index = SurvivorCombatComponent->GetEquippedIndex();
	if (!SurvivorCombatComponent->GetInventory().IsValidIndex(Index)) return nullptr;

	return SurvivorCombatComponent->GetInventory()[Index].Get();
}

USLSurvivorCombatComponent* ASLSurvivorCharacterBase::GetCombatComponent() const
{
	return SurvivorCombatComponent;
}

bool ASLSurvivorCharacterBase::IsAiming() const
{
	return SurvivorCombatComponent && SurvivorCombatComponent->IsAiming();
}

float ASLSurvivorCharacterBase::GetAimYawOffset() const
{
	if (!Controller) return 0.f;

	const float ControlYaw = Controller->GetControlRotation().Yaw;
	const float ActorYaw   = GetActorRotation().Yaw;

	// normalized to [-180, 180]
	return FRotator::NormalizeAxis(ControlYaw - ActorYaw);
}

void ASLSurvivorCharacterBase::UpdateAimTarget(float DeltaSeconds)
{
	if (!IsLocallyControlled()) return;

	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector Start = CamLoc;
	const FVector End = Start + CamRot.Vector() * 100000.f;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AimTrace), false, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	const FVector Target = bHit ? Hit.ImpactPoint : End;

	AimTargetWorld = Target;
	AimTargetWorldSmoothed = FMath::VInterpTo(AimTargetWorldSmoothed, Target, DeltaSeconds, 15.f);
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

void ASLSurvivorCharacterBase::SetStrafeAimingMode(bool bEnable)
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


void ASLSurvivorCharacterBase::HandleAxis2D(FGameplayTag InputTag, FVector2D Value)
{
	Super::HandleAxis2D(InputTag, Value);
}

void ASLSurvivorCharacterBase::HandleActionStarted(FGameplayTag InputTag)
{
	Super::HandleActionStarted(InputTag);
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact) SurvivorCombatComponent->TryPickupWeapon();
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim) SurvivorCombatComponent->SetAiming(true);
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire) SurvivorCombatComponent->FirePressed();
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Drop) SurvivorCombatComponent->DropEquippedWeapon();
}

void ASLSurvivorCharacterBase::HandleActionCompleted(FGameplayTag InputTag)
{
	Super::HandleActionCompleted(InputTag);
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim) SurvivorCombatComponent->SetAiming(false);
}

void ASLSurvivorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASLSurvivorCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASLSurvivorCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAimTarget(DeltaTime);
	UpdateTurnInPlace(DeltaTime);
}
