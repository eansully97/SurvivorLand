// Fill out your copyright notice in the Description page of Project Settings.
#include "Components/Combat/SLSurvivorCombatComponent.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "AnimInstances/SLBasePlayerAnimInstance.h"
#include "Characters/SLBaseGameCharacter.h"
#include "Characters/SLSurvivorCharacterBase.h"
#include "Components/SLInputHandlerComponent.h"
#include "Data/Weapon/SLWeaponAnimProfile.h"
#include "Items/Weapons/SLWeaponBase.h"
#include "Data/Weapon/SLWeaponData.h"
#include "Data/Weapon/SLWeaponInputProfile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Items/Projectiles/SLBaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

USLSurvivorCombatComponent::USLSurvivorCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USLSurvivorCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, EquippedWeapon)
	DOREPLIFETIME(ThisClass, StowedWeapon);
	DOREPLIFETIME(ThisClass, bAiming);
}

void USLSurvivorCombatComponent::Server_TryPickupWeapon_Implementation()
{
	TryPickupWeapon();
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
	Weapon->ServerAttachToOwnerSocket(OwnerChar, HandSocket, true);

	ApplyEquippedPresentation(Weapon);
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
	Weapon->ServerAttachToOwnerSocket(OwnerChar, StowSocket, true);
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

	ASLWeaponBase* Best = nullptr;
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
			Best = Weapon;
		}
	}

	if (!Best)
	{
		return;
	}

	// No equipped weapon: equip immediately
	if (!EquippedWeapon)
	{
		EquipWeapon_Internal(Best);
		return;
	}

	// Equipped but no stowed: move current equipped to stowed, equip new
	if (!StowedWeapon)
	{
		ASLWeaponBase* OldEquipped = EquippedWeapon;
		EquippedWeapon = nullptr;

		StowWeapon_Internal(OldEquipped);
		EquipWeapon_Internal(Best);
		return;
	}
}

void USLSurvivorCombatComponent::SwitchWeapons_Internal()
{
	if (!EquippedWeapon || !StowedWeapon)
	{
		return;
	}

	ClearEquippedPresentation();

	ASLWeaponBase* OldEquipped = EquippedWeapon;
	ASLWeaponBase* OldStowed = StowedWeapon;

	EquippedWeapon = nullptr;
	StowedWeapon = nullptr;

	StowWeapon_Internal(OldEquipped);
	EquipWeapon_Internal(OldStowed);
}

void USLSurvivorCombatComponent::DropEquippedWeapon_Internal()
{
	if (!EquippedWeapon)
	{
		return;
	}

	ClearEquippedPresentation();

	ASLWeaponBase* WeaponToDrop = EquippedWeapon;
	EquippedWeapon = nullptr;

	DropWeapon_Internal(WeaponToDrop);

	// Auto-equip stowed if present
	if (StowedWeapon)
	{
		ASLWeaponBase* NewEquipped = StowedWeapon;
		StowedWeapon = nullptr;

		EquipWeapon_Internal(NewEquipped);
	}
}

void USLSurvivorCombatComponent::ClearEquippedPresentation()
{
	bFireHeld = false;
	StopFire();
	Client_ClearEquippedPresentation();
}

void USLSurvivorCombatComponent::ApplyEquippedPresentation(const ASLWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->GetWeaponData())
	{
		ClearEquippedPresentation();
		return;
	}

	Client_ApplyEquippedPresentation(Weapon->GetWeaponData());
}

void USLSurvivorCombatComponent::Server_DropEquippedWeapon_Implementation(ASLWeaponBase* Weapon)
{
	DropWeapon_Internal(Weapon);
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
	Weapon->ServerDropFromOwner(DropLoc, Impulse);
}

void USLSurvivorCombatComponent::SwitchWeapons()
{
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


void USLSurvivorCombatComponent::Client_ApplyEquippedPresentation_Implementation(const USLWeaponDataAsset* WeaponData)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar || !WeaponData) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	bFireHeld = false;
	StopFire();
	UnequipWeaponContext(PC);

	// INPUT PROFILE
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

	// ANIM PROFILE
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
	if (!OwnerChar) return;

	APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
	if (!PC) return;

	bFireHeld = false;
	StopFire();

	if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
	{
		// If you stored it on the anim instance:
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

void USLSurvivorCombatComponent::FirePressed()
{
	if (!bAiming)
	{
		return;
	}
	if (!bFireHeld)
	{
		return;
	}
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = Weapon->GetWeaponData();
	if (!WeaponData)
	{
		return;
	}

	const FVector AimPoint = OwnerChar->GetAimTargetWorld();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Server_Fire_Implementation(AimPoint);
	}
	else
	{
		Server_Fire(AimPoint);
	}
}

void USLSurvivorCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& AimPoint)
{
	ASLBaseGameCharacter* OwnerChar = Cast<ASLBaseGameCharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = Weapon->GetWeaponData();
	if (!WeaponData)
	{
		return;
	}

	if (WeaponData->FireType == ESLWeaponFireType::Projectile)
	{
		const FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
		const FVector SpawnLocation = MuzzleTransform.GetLocation();
		const FVector Direction = (AimPoint - SpawnLocation).GetSafeNormal();
		const FRotator SpawnRotation = Direction.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		ASLBaseProjectile* Projectile = GetWorld()->SpawnActor<ASLBaseProjectile>(
			WeaponData->ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);
		if (Projectile)
		{
			Projectile->InitializeProjectile(
				GetOwner(),
				Cast<APawn>(GetOwner()) ? Cast<APawn>(GetOwner())->GetController() : nullptr,
				WeaponData->Ballistics.BaseDamage
			);
			Projectile->SpawnTracerFX(WeaponData);

			if (UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				MoveComp->Velocity = Direction * WeaponData->ProjectileSpeed;
			}
		}
	}
	if (WeaponData->FireType == ESLWeaponFireType::Hitscan)
	{
		TArray<FHitResult> Hits;
		PerformBallisticsTrace(Weapon, AimPoint, Hits);

		// Sort hits before using them
		Hits.Sort([](const FHitResult& A, const FHitResult& B)
		{
			return A.Distance < B.Distance;
		});
	
		const FVector TraceStart = Weapon->GetMuzzleTransform().GetLocation();
		const FVector TraceDir = (AimPoint - TraceStart).GetSafeNormal();
		const FVector TraceEnd = TraceStart + TraceDir * WeaponData->Ballistics.MaxRange;
		const FVector VisualEnd = Hits.Num() > 0 ? Hits[0].ImpactPoint : TraceEnd;

		
		if (UParticleSystem* Trail = WeaponData->BeamTrail)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					this, Trail, TraceStart, FRotator::ZeroRotator, true))
			{
				Beam->SetVectorParameter(FName("Target"), VisualEnd);
			}
		}

		ResolvePenetrationAndDamage(Weapon, Hits, TraceStart);
	}
	Multicast_PlayFireFX();
}


void USLSurvivorCombatComponent::PerformBallisticsTrace(const ASLWeaponBase* Weapon, const FVector& AimPoint, TArray<FHitResult>& OutHits) const
{
	OutHits.Reset();

	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
	if (!Data)
	{
		return;
	}

	const auto& B = Data->Ballistics;
	const FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
	const FVector Start = MuzzleTransform.GetLocation() + MuzzleTransform.GetRotation().GetForwardVector() * 10.f;

	FVector Dir = (AimPoint - Start).GetSafeNormal();

	float Spread = B.Spread;
	if (IsAiming())
	{
		Spread *= B.AdsSpreadMultiplier;
	}

	if (Spread > 0.f)
	{
		Dir = FMath::VRandCone(Dir, FMath::DegreesToRadians(Spread));
	}

	const FVector End = Start + Dir * B.MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(Ballistics), true);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(const_cast<ASLWeaponBase*>(Weapon));

	if (B.TraceRadius > 0.f)
	{
		const FCollisionShape Shape = FCollisionShape::MakeSphere(B.TraceRadius);
		GetWorld()->SweepMultiByChannel(OutHits, Start, End, FQuat::Identity, B.TraceChannel, Shape, Params);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, B.TraceChannel, Params);
	}
}


void USLSurvivorCombatComponent::ResolvePenetrationAndDamage(const ASLWeaponBase* Weapon, const TArray<FHitResult>& Hits, const FVector& TraceStart) const
{
	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
	const auto& B = Data->Ballistics;

	float RemainingPen = B.PenetrationDepth;
	float CurrentDamage = B.BaseDamage;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;
		if (HitActor == Weapon) continue;
		if (Hit.bBlockingHit && !Hit.bStartPenetrating)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5.f, 8, FColor::Green, false, 2.f);
		}
		
		// Decide penetration cost
		float Cost;

		// Example simple rules; replace with PhysMat later
		const ECollisionChannel MyChannel = Hit.Component.IsValid() ? Hit.Component->GetCollisionObjectType() : ECC_WorldStatic;

		if (MyChannel == ECC_WorldStatic)
		{
			Cost = 999999.f; // effectively stops
		}
		else if (MyChannel == ECC_WorldDynamic)
		{
			Cost = 25.f; // tune
		}
		else if (MyChannel == ECC_Pawn)
		{
			Cost = 0.f; // allow multi-hit on pawns
		}
		else
		{
			Cost = 10.f;
		}

		// Reduce damage after each penetration (optional)
		if (RemainingPen > 0.f && Cost < 999999.f)
		{
			CurrentDamage *= 0.85f; // tune
		}

		// Consume penetration budget and decide stop
		if (B.PenetrationDepth <= 0.f) // no penetration configured
		{
			break;
		}

		RemainingPen -= Cost;
		if (RemainingPen <= 0.f || Cost >= 999999.f)
		{
			break;
		}
	}
}

void USLSurvivorCombatComponent::Multicast_PlayFireFX_Implementation()
{
	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon)
	{
		return;
	}

	SpawnMuzzleFlashFX(Weapon);
	SpawnFireSoundFX(Weapon);
}

void USLSurvivorCombatComponent::SpawnMuzzleFlashFX(const ASLWeaponBase* Weapon) const
{
	if (!Weapon)
	{
		return;
	}

	const USLWeaponDataAsset* WeaponData = Weapon->GetWeaponData();
	if (!WeaponData || !WeaponData->MuzzleFlash)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		WeaponData->MuzzleFlash,
		Weapon->GetWeaponMesh(),
		WeaponData->Ballistics.MuzzleSocketName,
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
	if (!WeaponData || !WeaponData->FireSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, Weapon->GetActorLocation());
}

void USLSurvivorCombatComponent::DropEquippedWeapon()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DropEquippedWeapon_Internal();
	}
	else
	{
		Server_DropEquippedWeapon(EquippedWeapon);
	}
}

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

void USLSurvivorCombatComponent::StartFire()
{
	StopFire();

	ASLWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon) return;

	const USLWeaponDataAsset* Data = Weapon->GetWeaponData();
	if (!Data) return;

	const float FireDelay = 60.f / Data->FireSettings.FireRate;

	FirePressed();

	if (Data->FireSettings.bAutomatic)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&USLSurvivorCombatComponent::FirePressed,
			FireDelay,
			true
		);
	}
}

void USLSurvivorCombatComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}