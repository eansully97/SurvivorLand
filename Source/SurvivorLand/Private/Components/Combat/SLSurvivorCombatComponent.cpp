// Ean Sullivan All Rights Reserved

#include "Components/Combat/SLSurvivorCombatComponent.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Characters/SLSurvivorCharacterBase.h"
#include "Components/SLInputHandlerComponent.h"
#include "Data/Weapon/SLWeaponAnimProfile.h"
#include "Data/Weapon/SLWeaponInputProfile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Projectiles/SLBaseProjectile.h"
#include "Items/Weapons/SLWeaponBase.h"

USLSurvivorCombatComponent::USLSurvivorCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USLSurvivorCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquippedWeapon);
	DOREPLIFETIME(ThisClass, StowedWeapon);
	DOREPLIFETIME(ThisClass, CombatState);
	DOREPLIFETIME(ThisClass, bAiming);
	DOREPLIFETIME(ThisClass, bLocallyReloading);
	DOREPLIFETIME(ThisClass, CarriedAmmo);
}

void USLSurvivorCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	InitCarriedAmmo();
	ResetPosture();
}

void USLSurvivorCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateWeaponPosture(DeltaTime);
}

/* --------------------------------------------------
 * Rep notifies
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon)
	{
		ApplyEquippedPresentation(EquippedWeapon);
	}
	else
	{
		ClearEquippedPresentation();
	}

	BroadcastAmmoState();
}

void USLSurvivorCombatComponent::OnRep_StowedWeapon()
{
	// Good place later for inventory UI refresh if needed.
}

void USLSurvivorCombatComponent::OnRep_CombatState()
{
	// Good place later for UI state / crosshair updates.
}

void USLSurvivorCombatComponent::OnRep_Aiming()
{
	
}

void USLSurvivorCombatComponent::OnRep_WeaponPostureState()
{

}

/* --------------------------------------------------
 * Public action handling
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::HandleActionStarted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Shared_Interact)
	{
		TryPickupWeapon();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(true);
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{

		bFireHeld = true;
		StartFire();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Drop)
	{
		DropEquippedWeapon();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_SwitchWeapon)
	{
		SwitchWeapons();
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Reload)
	{
		Reload();
		return;
	}
}

void USLSurvivorCombatComponent::HandleActionCompleted(FGameplayTag InputTag)
{
	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Aim)
	{
		SetAiming(false);
		return;
	}

	if (InputTag == SurvivorLandGameplayTags::Input_Survivor_Fire)
	{
		bFireHeld = false;
		StopFire();
		return;
	}
}

/* --------------------------------------------------
 * Simple validation helpers
 * -------------------------------------------------- */

bool USLSurvivorCombatComponent::HasUsableEquippedWeapon() const
{
	return EquippedWeapon && EquippedWeapon->GetWeaponData();
}

float USLSurvivorCombatComponent::GetDesiredRaiseAlpha() const
{
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return RaiseAlpha;
	}

	if (bAiming)
	{
		return 1.f;
	}

	if (bFireHeld)
	{
		return EquippedWeapon->GetWeaponData()->FireSettings.HipfireThreshold;
	}

	return 0.f;
}

bool USLSurvivorCombatComponent::ShouldUseCombatStrafeMode() const
{
	// Use combat strafe whenever weapon is meaningfully in combat posture,
	// not just during true ADS.
	return WeaponPostureState == EWeaponPostureState::EPS_Raising
		|| WeaponPostureState == EWeaponPostureState::EPS_Hipfire
		|| WeaponPostureState == EWeaponPostureState::EPS_ADS
		|| WeaponPostureState == EWeaponPostureState::EPS_Lowering;
}

void USLSurvivorCombatComponent::UpdateCombatStrafeMode()
{
	if (ASLSurvivorCharacterBase* Survivor = Cast<ASLSurvivorCharacterBase>(GetOwner()))
	{
		Survivor->SetCombatStrafeMode(ShouldUseCombatStrafeMode());
	}
}

float USLSurvivorCombatComponent::GetRaiseSpeed() const
{
	if (bAiming)
	{
		return EquippedWeapon->GetWeaponData()->FireSettings.RaiseToADSSpeed;
	}

	if (bFireHeld)
	{
		return EquippedWeapon->GetWeaponData()->FireSettings.RaiseToHipfireSpeed;
	}

	return EquippedWeapon->GetWeaponData()->FireSettings.LowerSpeed;
}

float USLSurvivorCombatComponent::GetRequiredRaiseAlphaToFire() const
{
	// If aim is held, require full ADS before the shot.
	if (bAiming)
	{
		return 1.f;
	}

	// Fire-only is allowed once weapon reaches hipfire posture.
	return EquippedWeapon->GetWeaponData()->FireSettings.HipfireThreshold;
}

void USLSurvivorCombatComponent::SetCharacterWalkSpeed(float NewSpeed) const
{
	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (OwnerChar)
	{
		OwnerChar->GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}
}

void USLSurvivorCombatComponent::UpdateWeaponPostureState(float PreviousAlpha)
{
	if (RaiseAlpha <= KINDA_SMALL_NUMBER)
	{
		WeaponPostureState = EWeaponPostureState::EPS_Lowered;
		SetCharacterWalkSpeed(CharacterBaseWalkSpeed);
		return;
	}

	if (RaiseAlpha >= 1.f - KINDA_SMALL_NUMBER)
	{
		WeaponPostureState = EWeaponPostureState::EPS_ADS;
		SetCharacterWalkSpeed(CharacterAimDownSightWalkSpeed);
		return;
	}

	if (RaiseAlpha >= EquippedWeapon->GetWeaponData()->FireSettings.HipfireThreshold)
	{
		if (bAiming)
		{
			WeaponPostureState = EWeaponPostureState::EPS_Raising;
			SetCharacterWalkSpeed(CharacterRaisingWeaponWalkSpeed);
		}
		else
		{
			WeaponPostureState = EWeaponPostureState::EPS_Hipfire;
			SetCharacterWalkSpeed(CharacterHipfireWalkSpeed);
		}
		return;
	}
	if (RaiseAlpha >= PreviousAlpha)
	{
		WeaponPostureState = EWeaponPostureState::EPS_Raising;
		SetCharacterWalkSpeed(CharacterRaisingWeaponWalkSpeed);
	}
	else
	{
		WeaponPostureState = EWeaponPostureState::EPS_Lowering;
		SetCharacterWalkSpeed(CharacterRaisingWeaponWalkSpeed);
	}
}

void USLSurvivorCombatComponent::UpdateWeaponPosture(float DeltaTime)
{
	if (!HasUsableEquippedWeapon())
	{
		ResetPosture();
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	const float PreviousAlpha = RaiseAlpha;
	const float TargetAlpha = GetDesiredRaiseAlpha();
	const float Speed = GetRaiseSpeed();

	RaiseAlpha = FMath::FInterpConstantTo(RaiseAlpha, TargetAlpha, DeltaTime, Speed);
	RaiseAlpha = FMath::Clamp(RaiseAlpha, 0.f, 1.f);

	UpdateWeaponPostureState(PreviousAlpha);
	UpdateCombatStrafeMode();

	// Queued first shot once posture reaches the required threshold.
	if (bPendingPostureShot && PreviousAlpha < GetRequiredRaiseAlphaToFire() && RaiseAlpha >= GetRequiredRaiseAlphaToFire())
	{
		bPendingPostureShot = false;
		FireOnce();

		const USLWeaponDataAsset* Data = EquippedWeapon ? EquippedWeapon->GetWeaponData() : nullptr;
		if (Data && Data->FireSettings.bAutomatic && bFireHeld)
		{
			const float FireDelay = 60.f / Data->FireSettings.FireRate;
			GetWorld()->GetTimerManager().SetTimer(
				AutoFireTimer,
				this,
				&USLSurvivorCombatComponent::FireOnce,
				FireDelay,
				true
			);
		}
	}
}

void USLSurvivorCombatComponent::ResetPosture()
{
	RaiseAlpha = 0.f;
	WeaponPostureState = EWeaponPostureState::EPS_Lowered;
	bPendingPostureShot = false;
	UpdateCombatStrafeMode();
	SetCharacterWalkSpeed(CharacterBaseWalkSpeed);
}

bool USLSurvivorCombatComponent::CanStartReload() const
{
	if (!HasUsableEquippedWeapon())
	{
		return false;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return false;
	}

	if (EquippedWeapon->IsFull())
	{
		return false;
	}

	return GetCarriedAmmo(EquippedWeapon->GetAmmoType()) > 0;
}

bool USLSurvivorCombatComponent::CanFire() const
{
	return HasUsableEquippedWeapon()
		&& CombatState == ECombatState::ECS_Unoccupied
		&& !EquippedWeapon->IsEmpty()
		&& RaiseAlpha >= GetRequiredRaiseAlphaToFire();
}

void USLSurvivorCombatComponent::ResetCombatState()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
	SetCharacterWalkSpeed(CharacterBaseWalkSpeed);
	bLocallyReloading = false;
	if (CombatState != ECombatState::ECS_ThrowingGrenade)
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

/* --------------------------------------------------
 * Inventory / pickup / swap / drop
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::TryPickupWeapon()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TryPickupWeapon_Internal();
	}
	else
	{
		Server_TryPickupWeapon();
	}
}

void USLSurvivorCombatComponent::Server_TryPickupWeapon_Implementation()
{
	TryPickupWeapon_Internal();
}

void USLSurvivorCombatComponent::TryPickupWeapon_Internal()
{
	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	TArray<AActor*> Overlaps;
	OwnerChar->GetOverlappingActors(Overlaps, ASLWeaponBase::StaticClass());

	ASLWeaponBase* BestWeapon = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (AActor* Actor : Overlaps)
	{
		ASLWeaponBase* Weapon = Cast<ASLWeaponBase>(Actor);
		if (!Weapon || !Weapon->GetWeaponData() || Weapon->IsHeld())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Weapon->GetActorLocation(), OwnerChar->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestWeapon = Weapon;
		}
	}

	if (!BestWeapon)
	{
		return;
	}

	if (!EquippedWeapon)
	{
		EquipWeapon_Internal(BestWeapon);
		return;
	}

	if (!StowedWeapon)
	{
		ASLWeaponBase* OldEquipped = EquippedWeapon;
		EquippedWeapon = nullptr;

		StowWeapon_Internal(OldEquipped);
		EquipWeapon_Internal(BestWeapon);
		return;
	}

	// Inventory already full. For now: do nothing.
	// Later you can replace equipped or stowed explicitly.
}

void USLSurvivorCombatComponent::EquipWeapon_Internal(ASLWeaponBase* Weapon)
{
	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (!OwnerChar || !Weapon || !Weapon->GetWeaponData())
	{
		return;
	}

	EquippedWeapon = Weapon;
	Weapon->SetOwner(OwnerChar);

	const FName HandSocket = OwnerChar->GetWeaponAttachSocket(Weapon->GetWeaponData()->Grip);
	Weapon->AttachToCharacter(OwnerChar, HandSocket, true);

	ApplyEquippedPresentation(Weapon);
	BroadcastAmmoState();
}

void USLSurvivorCombatComponent::StowWeapon_Internal(ASLWeaponBase* Weapon)
{
	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (!OwnerChar || !Weapon || !Weapon->GetWeaponData())
	{
		return;
	}

	StowedWeapon = Weapon;
	Weapon->SetOwner(OwnerChar);

	const FName StowSocket = OwnerChar->GetWeaponStowSocket(Weapon->GetWeaponData()->Grip);
	Weapon->AttachToCharacter(OwnerChar, StowSocket, true);
}

void USLSurvivorCombatComponent::SwitchWeapons()
{
	if (!HasUsableEquippedWeapon() || !StowedWeapon)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SwitchWeapons_Internal();
	}
	else
	{
		Server_SwitchWeapons();
	}
}

void USLSurvivorCombatComponent::Server_SwitchWeapons_Implementation()
{
	SwitchWeapons_Internal();
}

void USLSurvivorCombatComponent::SwitchWeapons_Internal()
{
	if (!EquippedWeapon || !StowedWeapon)
	{
		return;
	}

	CombatState = ECombatState::ECS_SwappingWeapons;

	bFireHeld = false;
	bPendingPostureShot = false;
	StopFire();
	ClearEquippedPresentation();

	ASLWeaponBase* OldEquipped = EquippedWeapon;
	ASLWeaponBase* OldStowed = StowedWeapon;

	EquippedWeapon = nullptr;
	StowedWeapon = nullptr;

	StowWeapon_Internal(OldEquipped);
	EquipWeapon_Internal(OldStowed);

	CombatState = ECombatState::ECS_Unoccupied;
	ResetPosture();
}

void USLSurvivorCombatComponent::DropEquippedWeapon()
{
	if (!EquippedWeapon)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DropEquippedWeapon_Internal();
	}
	else
	{
		Server_DropEquippedWeapon();
	}
}

void USLSurvivorCombatComponent::Server_DropEquippedWeapon_Implementation()
{
	DropEquippedWeapon_Internal();
}

void USLSurvivorCombatComponent::DropEquippedWeapon_Internal()
{
	if (!EquippedWeapon)
	{
		return;
	}

	bFireHeld = false;
	bPendingPostureShot = false;
	StopFire();
	ClearEquippedPresentation();

	ASLWeaponBase* WeaponToDrop = EquippedWeapon;
	EquippedWeapon = nullptr;

	DropWeapon_Internal(WeaponToDrop);

	if (StowedWeapon)
	{
		ASLWeaponBase* NewEquipped = StowedWeapon;
		StowedWeapon = nullptr;
		EquipWeapon_Internal(NewEquipped);
	}
	else
	{
		ResetPosture();
		BroadcastAmmoState();
	}
}

void USLSurvivorCombatComponent::DropWeapon_Internal(ASLWeaponBase* Weapon) const
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar || !Weapon)
	{
		return;
	}

	const FVector DropLoc =
		OwnerChar->GetActorLocation() +
		OwnerChar->GetActorForwardVector() * 120.f +
		FVector(0.f, 0.f, 40.f);

	const FVector Impulse = OwnerChar->GetActorForwardVector() * 200.f;

	Weapon->SetOwner(nullptr);
	Weapon->DropFromOwner(DropLoc, Impulse);
}

/* --------------------------------------------------
 * Presentation
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::ApplyEquippedPresentation(const ASLWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->GetWeaponData())
	{
		ClearEquippedPresentation();
		return;
	}

	Client_ApplyEquippedPresentation(Weapon->GetWeaponData());
}

void USLSurvivorCombatComponent::ClearEquippedPresentation()
{
	ResetCombatState();
	ResetPosture();
	Client_ClearEquippedPresentation();
}

void USLSurvivorCombatComponent::Client_ApplyEquippedPresentation_Implementation(const USLWeaponDataAsset* WeaponData)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar || !WeaponData)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC)
	{
		return;
	}

	bFireHeld = false;
	StopFire();
	UnequipWeaponContext(PC);

	if (WeaponData->InputProfile)
	{
		if (WeaponData->InputProfile->MappingContext)
		{
			EquipWeaponContext(PC, WeaponData->InputProfile->MappingContext, 1);
		}

		if (OwnerChar->GetInputHandlerComponent())
		{
			if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerChar->InputComponent))
			{
				OwnerChar->GetInputHandlerComponent()->BindAdditionalActions(
					EIC,
					WeaponData->InputProfile->GrantedInputActions
				);
			}
		}
	}

	if (WeaponData->AnimProfile)
	{
		if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
		{
			if (WeaponData->AnimProfile->SurvivorUpperBodyLayerClass)
			{
				Anim->LinkAnimClassLayers(WeaponData->AnimProfile->SurvivorUpperBodyLayerClass);
			}
		}
	}
}

void USLSurvivorCombatComponent::Client_ClearEquippedPresentation_Implementation()
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC)
	{
		return;
	}

	bFireHeld = false;
	StopFire();

	if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
	{
		if (USLBasePlayerAnimInstance* SLAnim = Cast<USLBasePlayerAnimInstance>(Anim))
		{
			if (SLAnim->DefaultUnarmedUpperBodyLayerClass)
			{
				Anim->LinkAnimClassLayers(SLAnim->DefaultUnarmedUpperBodyLayerClass);
			}
		}
	}

	UnequipWeaponContext(PC);
}

void USLSurvivorCombatComponent::EquipWeaponContext(APlayerController* PC, UInputMappingContext* WeaponContext, int32 Priority)
{
	if (!PC || !WeaponContext)
	{
		return;
	}

	EquippedWeaponContext = WeaponContext;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(WeaponContext, Priority);
		}
	}
}

void USLSurvivorCombatComponent::UnequipWeaponContext(APlayerController* PC)
{
	if (!PC || !EquippedWeaponContext)
	{
		return;
	}

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->RemoveMappingContext(EquippedWeaponContext);
		}
	}

	EquippedWeaponContext = nullptr;
}

/* --------------------------------------------------
 * Aiming
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::SetAiming(bool bNewAiming)
{
	if (bAiming == bNewAiming)
	{
		return;
	}

	bAiming = bNewAiming;
	OnRep_Aiming();

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetAiming(bNewAiming);
	}
}

void USLSurvivorCombatComponent::Server_SetAiming_Implementation(bool bNewAiming)
{
	bAiming = bNewAiming;
	OnRep_Aiming();
}

/* --------------------------------------------------
 * Fire flow
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::StartFire()
{
	StopFire();
	RequestFire();
}

void USLSurvivorCombatComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void USLSurvivorCombatComponent::RequestFire()
{
	if (!HasUsableEquippedWeapon())
	{
		return;
	}

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_SwappingWeapons)
	{
		return;
	}

	if (EquippedWeapon->IsEmpty())
	{
		Reload();
		return;
	}

	if (!CanFire())
	{
		bPendingPostureShot = true;
		return;
	}

	FireOnce();

	const USLWeaponDataAsset* Data = EquippedWeapon->GetWeaponData();
	if (!Data || !Data->FireSettings.bAutomatic || !bFireHeld)
	{
		return;
	}

	const float FireDelay = 60.f / Data->FireSettings.FireRate;
	GetWorld()->GetTimerManager().SetTimer(
		AutoFireTimer,
		this,
		&USLSurvivorCombatComponent::FireOnce,
		FireDelay,
		true
	);
}

void USLSurvivorCombatComponent::FireOnce()
{
	if (!CanFire())
	{
		if (HasUsableEquippedWeapon() && EquippedWeapon->IsEmpty())
		{
			Reload();
		}
		else
		{
			StopFire();
		}
		return;
	}

	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar)
	{
		StopFire();
		return;
	}

	if (ASLSurvivorCharacterBase* Survivor = Cast<ASLSurvivorCharacterBase>(OwningCharacter))
	{
		Survivor->PlayFireMontage(bAiming);
	}

	const FVector AimPoint = OwnerChar->GetAimTargetWorld();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FireOnce_Internal(AimPoint);
	}
	else
	{
		Server_Fire(AimPoint);
	}
}

void USLSurvivorCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& AimPoint)
{
	FireOnce_Internal(AimPoint);
}

void USLSurvivorCombatComponent::FireOnce_Internal(const FVector& AimPoint)
{
	if (!CanFire())
	{
		return;
	}

	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar || !EquippedWeapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = EquippedWeapon->GetWeaponData();
	if (!WeaponData)
	{
		return;
	}

	if (WeaponData->FireType == ESLWeaponFireType::Projectile)
	{
		const FTransform MuzzleTransform = EquippedWeapon->GetMuzzleTransform();
		const FVector SpawnLocation = MuzzleTransform.GetLocation();
		const FVector Direction = (AimPoint - SpawnLocation).GetSafeNormal();
		const FRotator SpawnRotation = Direction.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		ASLBaseProjectile* Projectile = GetWorld()->SpawnActor<ASLBaseProjectile>(
			WeaponData->ProjectileSettings.ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (Projectile)
		{
			Projectile->InitializeProjectile(
				GetOwner(),
				Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr,
				WeaponData
			);

			Projectile->SpawnTracerFX(WeaponData);

			if (UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				MoveComp->Velocity = Direction * WeaponData->ProjectileSettings.ProjectileSpeed;
			}
		}
	}
	else if (WeaponData->FireType == ESLWeaponFireType::Hitscan)
	{
		TArray<FHitResult> Hits;
		PerformBallisticsTrace(EquippedWeapon, AimPoint, Hits);

		Hits.Sort([](const FHitResult& A, const FHitResult& B)
		{
			return A.Distance < B.Distance;
		});

		const FVector TraceStart = EquippedWeapon->GetMuzzleTransform().GetLocation();
		const FVector TraceDir = (AimPoint - TraceStart).GetSafeNormal();
		const FVector TraceEnd = TraceStart + TraceDir * WeaponData->HitscanSettings.MaxRange;
		const FVector VisualEnd = Hits.Num() > 0 ? Hits[0].ImpactPoint : TraceEnd;

		if (UParticleSystem* Trail = WeaponData->FXSettings.HitscanBeamTrail)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					this, Trail, TraceStart, FRotator::ZeroRotator, true))
			{
				Beam->SetVectorParameter(FName("Target"), VisualEnd);
			}
		}

		ResolvePenetrationAndDamage(EquippedWeapon, Hits, TraceStart);
	}
	EquippedWeapon->SpendRound();
	Multicast_PlayFireFX();
	BroadcastAmmoState();

	if (EquippedWeapon->IsEmpty())
	{
		StopFire();
	}
}

/* --------------------------------------------------
 * Reload flow
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::Reload()
{
	if (!CanStartReload())
	{
		return;
	}

	bFireHeld = false;
	bPendingPostureShot = false;
	StopFire();

	ASLSurvivorCharacterBase* OwnerChar = Cast<ASLSurvivorCharacterBase>(GetOwner());
	if (OwnerChar && OwnerChar->IsLocallyControlled())
	{
		if (EquippedWeapon && EquippedWeapon->GetWeaponData())
		{
			if (UAnimMontage* ReloadMontage = EquippedWeapon->GetWeaponData()->FireSettings.CharacterReloadMontage)
			{
				OwnerChar->PlayAnimMontage(ReloadMontage, 1.f);
			}
		}
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		BeginReload();
	}
	else
	{
		Server_StartReload();
	}
}

void USLSurvivorCombatComponent::Server_StartReload_Implementation()
{
	BeginReload();
}

void USLSurvivorCombatComponent::BeginReload()
{
	if (!CanStartReload())
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	bLocallyReloading = true;
}

void USLSurvivorCombatComponent::FinishReloading()
{
	bLocallyReloading = false;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FinishReloading_Internal();
	}
	else
	{
		Server_FinishReload();
	}
}

void USLSurvivorCombatComponent::Server_FinishReload_Implementation()
{
	FinishReloading_Internal();
}

void USLSurvivorCombatComponent::FinishReloading_Internal()
{
	if (CombatState != ECombatState::ECS_Reloading)
	{
		return;
	}

	if (HasUsableEquippedWeapon())
	{
		const int32 AmmoToLoad = CalculateReloadAmount();
		if (AmmoToLoad > 0)
		{
			const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

			if (ConsumeCarriedAmmo(AmmoType, AmmoToLoad))
			{
				EquippedWeapon->AddAmmoToMag(AmmoToLoad);
			}
		}
	}

	bLocallyReloading = false;
	CombatState = ECombatState::ECS_Unoccupied;
	BroadcastAmmoState();

	if (bFireHeld)
	{
		RequestFire();
	}
}

int32 USLSurvivorCombatComponent::CalculateReloadAmount() const
{
	if (!HasUsableEquippedWeapon())
	{
		return 0;
	}

	const int32 MagCapacity = EquippedWeapon->GetWeaponData()->FireSettings.MagazineSize;
	const int32 AmmoInMag = EquippedWeapon->GetCurrentAmmoInMag();
	const int32 RoomInMag = FMath::Max(0, MagCapacity - AmmoInMag);

	if (RoomInMag <= 0)
	{
		return 0;
	}

	const int32 Carried = GetCarriedAmmo(EquippedWeapon->GetAmmoType());
	return FMath::Clamp(RoomInMag, 0, Carried);
}

/* --------------------------------------------------
 * Ammo helpers
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::InitCarriedAmmo()
{
	if (!(GetOwner() && GetOwner()->HasAuthority()) || CarriedAmmo.Num() > 0)
	{
		return;
	}

	FSLAmmoCount SmallAmmo;
	SmallAmmo.AmmoType = EAmmoType::Small;
	SmallAmmo.Count = 100;
	CarriedAmmo.Add(SmallAmmo);

	FSLAmmoCount MediumAmmo;
	MediumAmmo.AmmoType = EAmmoType::Medium;
	MediumAmmo.Count = 50;
	CarriedAmmo.Add(MediumAmmo);

	FSLAmmoCount LargeAmmo;
	LargeAmmo.AmmoType = EAmmoType::Large;
	LargeAmmo.Count = 10;
	CarriedAmmo.Add(LargeAmmo);
}

int32 USLSurvivorCombatComponent::GetCarriedAmmo(EAmmoType AmmoType) const
{
	const FSLAmmoCount* AmmoEntry = CarriedAmmo.FindByPredicate(
		[AmmoType](const FSLAmmoCount& Entry)
		{
			return Entry.AmmoType == AmmoType;
		}
	);

	return AmmoEntry ? AmmoEntry->Count : 0;
}

bool USLSurvivorCombatComponent::ConsumeCarriedAmmo(EAmmoType AmmoType, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	FSLAmmoCount* AmmoEntry = CarriedAmmo.FindByPredicate(
		[AmmoType](const FSLAmmoCount& Entry)
		{
			return Entry.AmmoType == AmmoType;
		}
	);

	if (!AmmoEntry || AmmoEntry->Count < Amount)
	{
		return false;
	}

	AmmoEntry->Count -= Amount;
	OnCarriedAmmoChanged.Broadcast(AmmoEntry->Count, AmmoType);
	return true;
}

void USLSurvivorCombatComponent::AddCarriedAmmo(EAmmoType AmmoType, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	FSLAmmoCount* AmmoEntry = CarriedAmmo.FindByPredicate(
		[AmmoType](const FSLAmmoCount& Entry)
		{
			return Entry.AmmoType == AmmoType;
		}
	);

	if (AmmoEntry)
	{
		AmmoEntry->Count += Amount;
		OnCarriedAmmoChanged.Broadcast(AmmoEntry->Count, AmmoType);
		return;
	}

	FSLAmmoCount NewEntry;
	NewEntry.AmmoType = AmmoType;
	NewEntry.Count = Amount;
	CarriedAmmo.Add(NewEntry);

	OnCarriedAmmoChanged.Broadcast(NewEntry.Count, AmmoType);
}

void USLSurvivorCombatComponent::BroadcastAmmoState()
{
	if (!EquippedWeapon || !EquippedWeapon->GetWeaponData())
	{
		return;
	}

	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	OnWeaponAmmoChanged.Broadcast(EquippedWeapon->GetCurrentAmmoInMag(), AmmoType);
	OnCarriedAmmoChanged.Broadcast(GetCarriedAmmo(AmmoType), AmmoType);
}

/* --------------------------------------------------
 * Ballistics / damage
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const
{
	OutHits.Reset();

	if (!Weapon || !Weapon->GetWeaponData())
	{
		return;
	}

	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
	const FSLHitscanSettings& Hitscan = Data->HitscanSettings;

	const FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
	const FVector Start = MuzzleTransform.GetLocation() + MuzzleTransform.GetRotation().GetForwardVector() * 10.f;

	FVector Dir = (AimPoint - Start).GetSafeNormal();

	float Spread = Hitscan.Spread;
	if (IsAiming())
	{
		Spread *= Hitscan.AdsSpreadMultiplier;
	}

	if (Spread > 0.f)
	{
		Dir = FMath::VRandCone(Dir, FMath::DegreesToRadians(Spread));
	}

	const FVector End = Start + Dir * Hitscan.MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(Ballistics), true);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(const_cast<ASLWeaponBase*>(Weapon));

	if (Hitscan.TraceRadius > 0.f)
	{
		const FCollisionShape Shape = FCollisionShape::MakeSphere(Hitscan.TraceRadius);
		GetWorld()->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, Hitscan.TraceChannel, Shape, Params);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, Hitscan.TraceChannel, Params);
	}
}

void USLSurvivorCombatComponent::ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const
{
	if (!Weapon || !Weapon->GetWeaponData())
	{
		return;
	}

	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
	const FSLWeaponDamageSettings& DamageSettings = Data->DamageSettings;

	float RemainingPen = DamageSettings.PenetrationDepth;
	float CurrentDamage = DamageSettings.BaseDamage;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Weapon)
		{
			continue;
		}

		if (Hit.bBlockingHit && !Hit.bStartPenetrating)
		{
			SpawnImpactFX(Hit.ImpactPoint, Hit.ImpactNormal, Data);
		}

		float FinalDamage = CurrentDamage;

		if (Hit.BoneName == FName(TEXT("head")))
		{
			FinalDamage *= DamageSettings.HeadshotMultiplier;
		}

		UGameplayStatics::ApplyPointDamage(
			HitActor,
			FinalDamage,
			(Hit.ImpactPoint - TraceStart).GetSafeNormal(),
			Hit,
			Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr,
			GetOwner(),
			UDamageType::StaticClass()
		);

		float Cost = 0.f;
		const ECollisionChannel ObjectChannel = Hit.Component.IsValid() ? Hit.Component->GetCollisionObjectType() : ECC_WorldStatic;

		if (ObjectChannel == ECC_WorldStatic)
		{
			Cost = 999999.f;
		}
		else if (ObjectChannel == ECC_WorldDynamic)
		{
			Cost = 25.f;
		}
		else if (ObjectChannel == ECC_Pawn)
		{
			Cost = 0.f;
		}
		else
		{
			Cost = 10.f;
		}

		if (DamageSettings.PenetrationDepth <= 0.f)
		{
			break;
		}

		if (RemainingPen > 0.f && Cost < 999999.f)
		{
			CurrentDamage *= 0.85f;
		}

		RemainingPen -= Cost;
		if (RemainingPen <= 0.f || Cost >= 999999.f)
		{
			break;
		}
	}
}

/* --------------------------------------------------
 * Fire FX
 * -------------------------------------------------- */

void USLSurvivorCombatComponent::Multicast_PlayFireFX_Implementation()
{
	if (!EquippedWeapon)
	{
		return;
	}

	SpawnMuzzleFlashFX(EquippedWeapon);
	SpawnFireSoundFX(EquippedWeapon);
}

void USLSurvivorCombatComponent::SpawnMuzzleFlashFX(const ASLWeaponBase* Weapon) const
{
	if (!Weapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = Weapon->GetWeaponData();
	if (!WeaponData || !WeaponData->FXSettings.MuzzleFlash || !WeaponData->SocketInformation.MuzzleSocketName.IsValid())
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		WeaponData->FXSettings.MuzzleFlash,
		Weapon->GetWeaponMesh(),
		WeaponData->SocketInformation.MuzzleSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);
}

void USLSurvivorCombatComponent::SpawnFireSoundFX(const ASLWeaponBase* Weapon) const
{
	if (!Weapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = Weapon->GetWeaponData();
	if (!WeaponData || !WeaponData->FXSettings.FireSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FXSettings.FireSound, Weapon->GetActorLocation());
}

void USLSurvivorCombatComponent::SpawnImpactFX(const FVector& ImpactPoint, const FVector& ImpactNormal, const USLWeaponDataAsset* WeaponData) const
{
	if (!WeaponData)
	{
		return;
	}

	const FRotator ImpactRotation = ImpactNormal.Rotation();

	if (WeaponData->FXSettings.ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			WeaponData->FXSettings.ImpactEffect,
			ImpactPoint,
			ImpactRotation
		);
	}

	if (WeaponData->FXSettings.ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponData->FXSettings.ImpactSound,
			ImpactPoint
		);
	}
}